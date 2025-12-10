package crawler

import (
	"context"
	"fmt"
	"log"
	"net/http"
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
	producer *kafkaclient.Producer
	consumer *kafkaclient.Consumer
}

func NewProductCrawler(cfg config.LogicConfig, consumer *kafkaclient.Consumer, producer *kafkaclient.Producer) *ProductCrawler {
	return &ProductCrawler{
		consumer: consumer,
		producer: producer,
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

	for _, link := range links {
		time.Sleep(pc.delay)
		linkResp, err := http.Get(link)
		if err != nil {
			fmt.Println("error make request to link", err)
		}

		doc, err := goquery.NewDocumentFromReader(linkResp.Body)
		if err != nil {
			fmt.Println("error read html text from link", err)
		}
		// htmlText, err := doc.Html()
		if err != nil {
			fmt.Printf("error get source html text from url = %s", link)
		}

		description := doc.Find(`[data-auto="product-description"]`).Text()
		title := doc.Find(`h1[data-auto="productCardTitle"]`).Text()
		var sourceName string
		if link[:len(utils.YandexMarket)] == utils.YandexMarket {
			sourceName = "Яндекс Маркет"
		}

		// normilizeUrl := utils.NormalizeUrl(link)
		// fmt.Println(normilizeUrl)
		// hashUrl := sha256.Sum256([]byte(normilizeUrl))
		// stringHashUrl := hex.EncodeToString(hashUrl[:])
		item := storage.Item{
			Description: description,
			Title:       title,
			UrlHash:     utils.NormalizeUrl(link),
			TimeOfLoad:  data.ParceTime,
			// HtmlText:    htmlText,
			SourceName: sourceName,
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
			return fmt.Errorf("error upserting item %s", err)
		}
	}

	if err := pc.consumer.Commit(data.TopicPartition); err != nil {
		log.Printf("error committing offset: %v", err)
	}

	return nil
}

func GetLinks(mainLink string) []string {
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
