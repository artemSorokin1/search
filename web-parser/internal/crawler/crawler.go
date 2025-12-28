package crawler

import (
	"context"
	"fmt"
	"log"
	"math/rand"
	"net/http"
	"net/http/cookiejar"
	"strings"
	"sync"
	"time"
	"web-parser/internal/config"
	"web-parser/pkg/kafkaclient"
	"web-parser/pkg/storage"
	"web-parser/pkg/utils"

	"github.com/PuerkitoBio/goquery"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo/options"
)

const (
	NumWorkers = 1
)

type ProductCrawler struct {
	delay    time.Duration
	consumer *kafkaclient.Consumer
}

func NewProductCrawler(cfg config.LogicConfig, consumer *kafkaclient.Consumer) *ProductCrawler {
	return &ProductCrawler{
		consumer: consumer,
		delay:    cfg.DelayTime,
	}
}

func (pc *ProductCrawler) Get(readTopic string, mongoClient *storage.Client) {
	var wg sync.WaitGroup
	readChan := make(chan kafkaclient.KafkaMsg)

	wg.Add(1)
	go func() {
		defer wg.Done()
		pc.consumer.Read(readTopic, readChan)
		close(readChan)
	}()

	for i := 0; i < NumWorkers; i++ {
		wg.Add(1)
		go func(workerID int) {
			defer wg.Done()
			for msg := range readChan {
				fmt.Printf("worker %d received msg: %+v\n", workerID, msg)

				if err := pc.makeRequestToCard(msg, mongoClient); err != nil {
					log.Printf("worker %d: error processing msg: %v", workerID, err)
				}
			}
			log.Printf("worker %d: channel closed, exiting", workerID)
		}(i)
	}

	wg.Wait()
}

func (pc *ProductCrawler) makeRequestToCard(data kafkaclient.KafkaMsg, mongoClient *storage.Client) error {
	time.Sleep(pc.delay)
	fmt.Println("make request to url: ", data.Url)

	links := GetLinks(data.Url)

	jar, _ := cookiejar.New(nil)
	client := &http.Client{
		Timeout: pc.delay,
		Jar:     jar,
	}

	fetchDoc := func(url string) (*goquery.Document, int, string, error) {
		req, err := http.NewRequest("GET", url, nil)
		if err != nil {
			return nil, 0, "", err
		}

		req.Header.Set("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0 Safari/537.36")
		req.Header.Set("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8")
		req.Header.Set("Accept-Language", "en-US,en;q=0.9,ru;q=0.8")
		req.Header.Set("Cache-Control", "no-cache")
		req.Header.Set("Pragma", "no-cache")

		resp, err := client.Do(req)
		if err != nil {
			return nil, 0, "", err
		}

		defer resp.Body.Close()

		doc, err := goquery.NewDocumentFromReader(resp.Body)
		if err != nil {
			return nil, resp.StatusCode, resp.Request.URL.String(), err
		}

		return doc, resp.StatusCode, resp.Request.URL.String(), nil
	}

	isEbayAntibot := func(doc *goquery.Document) bool {
		html, _ := doc.Html()
		low := strings.ToLower(html)

		return strings.Contains(low, "checking your browser") ||
			strings.Contains(low, "enable javascript") ||
			strings.Contains(low, "px-captcha") ||
			strings.Contains(low, "datadome") ||
			strings.Contains(low, "access denied")
	}

	for _, link := range links {
		if strings.HasPrefix(link, utils.YandexMarket) {
			time.Sleep(pc.delay)
		} else {
			time.Sleep(time.Duration(3+rand.Intn(7)) * time.Second)
		}

		doc, status, finalURL, err := fetchDoc(link)
		if err != nil {
			return fmt.Errorf("error fetching %s: %w", link, err)
		}
		fmt.Printf("fetched: %s status=%d final=%s\n", link, status, finalURL)

		var description, title, sourceName string

		switch {
		case strings.HasPrefix(link, utils.YandexMarket):
			sourceName = "Яндекс Маркет"
			description = strings.TrimSpace(doc.Find(`[data-auto="product-description"]`).Text())
			title = strings.TrimSpace(doc.Find(`h1[data-auto="productCardTitle"]`).Text())

		case strings.HasPrefix(link, utils.Ebay):
			sourceName = "Ebay"

			if isEbayAntibot(doc) {
				return fmt.Errorf("ebay antibot page received for %s (final=%s, status=%d)", link, finalURL, status)
			}

			title = strings.TrimSpace(doc.Find("h1.x-item-title__mainTitle span").First().Text())
			if title == "" {
				title = strings.TrimSpace(doc.Find("h1").First().Text())
			}

			if src, ok := doc.Find("iframe#desc_ifr").Attr("src"); ok && src != "" {
				iframeDoc, _, _, err := fetchDoc(src)
				if err == nil {
					description = strings.TrimSpace(iframeDoc.Find("#ds_div").Text())

					if description == "" {
						iframeDoc.Find("script, style, noscript").Remove()
						description = strings.TrimSpace(iframeDoc.Find("body").Text())
					}

					description = strings.Join(strings.Fields(description), " ")
				}
			}

		default:
			continue
		}

		if title == "" {
			continue
		}

		item := storage.Item{
			Description: description,
			Title:       title,
			UrlHash:     utils.NormalizeUrl(link),
			TimeOfLoad:  data.ParceTime,
			SourceName:  sourceName,
		}

		filter := bson.M{"url": item.UrlHash}

		fmt.Println("insert in mongo called")
		_, err = mongoClient.Coll.ReplaceOne(
			context.Background(),
			filter,
			item,
			options.Replace().SetUpsert(true),
		)
		if err != nil {
			return fmt.Errorf("error upserting item: %w", err)
		}
	}

	if err := pc.consumer.Commit(data.TopicPartition); err != nil {
		log.Printf("error committing offset: %v", err)
	}

	return nil
}

func GetLinks(mainLink string) []string {
	if mainLink[:len(utils.Ebay)] == utils.Ebay {
		return []string{mainLink}
	}
	resp, err := http.Get(mainLink)
	if err != nil {
		fmt.Println("error make request to mainLink", err)
	}

	defer resp.Body.Close()

	doc, err := goquery.NewDocumentFromReader(resp.Body)
	if err != nil {
		fmt.Println("error read html from mainLink", err)
	}
	var links []string

	doc.Find(`[data-auto="snippet-link"]`).Each(func(i int, s *goquery.Selection) {
		if href, ok := s.Attr("href"); ok {
			links = append(links, "https://market.yandex.ru"+href)
		}
	})

	res := make([]string, len(links)/3)

	for i := 0; i < len(res); i++ {
		res[i] = links[i*3]
	}
	return res
}
