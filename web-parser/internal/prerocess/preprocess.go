package prerocess

import (
	"compress/gzip"
	"encoding/xml"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"time"
	"web-parser/pkg/kafkaclient"
)

const (
	YandexMarketSitemapUrl = "https://market.yandex.ru/sitemap/sitemap-all-offers.xml"
	EbaySitemapUrl         = "https://www.ebay.com/lst/AUCTION-0-index.xml"
)

type SitemapIndex struct {
	Sitemaps []SitemapEntry `xml:"sitemap"`
}

type SitemapEntry struct {
	Loc string `xml:"loc"`
}

type UrlSet struct {
	URLs []URL `xml:"url"`
}

type URL struct {
	Loc string `xml:"loc"`
}

type Preprocessor struct {
	kafkaProducer *kafkaclient.Producer
}

func NewPreprocessor(producer *kafkaclient.Producer) *Preprocessor {
	return &Preprocessor{
		kafkaProducer: producer,
	}
}

func (p *Preprocessor) PreprocessYandex(topic string) {
	fmt.Println("preprocess start yandex")
	resp, err := http.Get(YandexMarketSitemapUrl)
	if err != nil {
		slog.Warn("error parsing sitemap yandex", err)
	}
	defer resp.Body.Close()

	fmt.Println("got general xml file yandex")
	indexXML, err := io.ReadAll(resp.Body)
	if err != nil {
		panic(err)
	}

	var index SitemapIndex
	if err := xml.Unmarshal(indexXML, &index); err != nil {
		panic(err)
	}

	for _, sm := range index.Sitemaps {
		resp, err = http.Get(sm.Loc)
		if err != nil {
			slog.Warn("error parcing gz.xml yandex", err)
		}
		func() {
			defer resp.Body.Close()

			gz, err := gzip.NewReader(resp.Body)
			if err != nil {
				panic(err)
			}

			defer gz.Close()

			dataXML, err := io.ReadAll(gz)
			if err != nil {
				panic(err)
			}

			var urls UrlSet
			if err := xml.Unmarshal(dataXML, &urls); err != nil {
				panic(err)
			}
			for _, v := range urls.URLs {
				p.kafkaProducer.Produce(topic, kafkaclient.KafkaMsg{
					Url:       v.Loc,
					ParceTime: time.Now().Unix(),
				})
			}
		}()

		time.Sleep(1000 * time.Second)
	}
}

func (p *Preprocessor) PreprocessEbay(topic string) {
	resp, err := http.Get(EbaySitemapUrl)
	if err != nil {
		slog.Warn("error parsing sitemap ebay", err)
	}
	defer resp.Body.Close()

	fmt.Println("got general xml file ebay")
	indexXML, err := io.ReadAll(resp.Body)
	if err != nil {
		panic(err)
	}

	var index SitemapIndex
	if err := xml.Unmarshal(indexXML, &index); err != nil {
		panic(err)
	}

	for _, sm := range index.Sitemaps {
		resp, err = http.Get(sm.Loc)
		if err != nil {
			slog.Warn("error parcing gz.xml ebay", err)
		}
		func() {
			defer resp.Body.Close()

			gz, err := gzip.NewReader(resp.Body)
			if err != nil {
				panic(err)
			}

			defer gz.Close()

			dataXML, err := io.ReadAll(gz)
			if err != nil {
				panic(err)
			}

			var urls UrlSet
			if err := xml.Unmarshal(dataXML, &urls); err != nil {
				panic(err)
			}
			for _, v := range urls.URLs {
				p.kafkaProducer.Produce(topic, kafkaclient.KafkaMsg{
					Url:       v.Loc,
					ParceTime: time.Now().Unix(),
				})
			}
		}()

		time.Sleep(1000 * time.Second)
	}
}

func parce(s []byte) UrlSet {
	var res UrlSet
	err := xml.Unmarshal(s, &res)
	if err != nil {
		panic(err)
	}

	return res
}
