package main

import (
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"
	"web-parser/internal/config"
	"web-parser/internal/crawler"
	"web-parser/internal/prerocess"
	"web-parser/pkg/kafkaclient"
	"web-parser/pkg/storage"
)

func main() {
	cfg := config.LoadConfig()

	time.Sleep(5 * time.Second)

	addresses := cfg.KafkaCfg.Brokers
	preprocessorProducerYandex, err := kafkaclient.NewProducer(addresses)
	if err != nil {
		log.Fatalf("error creating yandex producer for preprocessor %s", err)
	}

	preprocessorProducerEbay, err := kafkaclient.NewProducer(addresses)
	if err != nil {
		log.Fatalf("error creating ebay producer for preprocessor %s", err)
	}

	preprocYandex := prerocess.NewPreprocessor(preprocessorProducerYandex)

	preprocEbay := prerocess.NewPreprocessor(preprocessorProducerEbay)

	go func() {
		log.Println("start first yandex preprocess")
		preprocYandex.PreprocessYandex(cfg.KafkaCfg.YandexTopic)
		log.Println("first yandex preprocess finished")

		ticker := time.NewTicker(7 * 24 * time.Hour)
		defer ticker.Stop()

		for range ticker.C {
			log.Println("start yandex preprocess run")
			preprocYandex.PreprocessYandex(cfg.KafkaCfg.YandexTopic)
			log.Println("yandex preprocess finished")
		}
	}()

	go func() {
		log.Println("start ebay first preprocess run")
		preprocEbay.PreprocessEbay(cfg.KafkaCfg.EbayTopic)
		log.Println("first ebay preprocess finished")

		ticker := time.NewTicker(7 * 24 * time.Hour)
		defer ticker.Stop()

		for range ticker.C {
			log.Println("start ebay preprocess run")
			preprocEbay.PreprocessEbay(cfg.KafkaCfg.EbayTopic)
			log.Println("ebay preprocess finished")
		}
	}()

	crawlerConsumerYandex, err := kafkaclient.NewConsumer(addresses, cfg.KafkaCfg.YandexConsumerGroup)
	if err != nil {
		log.Fatalf("error creating consumer for yandex crawler %s", err)
	}

	crawlerConsumerEbay, err := kafkaclient.NewConsumer(addresses, cfg.KafkaCfg.EbayConsumerGroup)
	if err != nil {
		log.Fatalf("error creating consumer for ebay crawler %s", err)
	}

	crawlerYandex := crawler.NewProductCrawler(cfg.LogicCfg, crawlerConsumerYandex)

	crawlerEbay := crawler.NewProductCrawler(cfg.LogicCfg, crawlerConsumerEbay)

	mongoClient, err := storage.NewClient(cfg.StorageCfg)
	if err != nil {
		log.Fatalf("error creating mongo client %s", err)
	}

	go crawlerYandex.Get(cfg.KafkaCfg.YandexTopic, mongoClient)

	go crawlerEbay.Get(cfg.KafkaCfg.EbayTopic, mongoClient)

	sigchan := make(chan os.Signal, 1)
	signal.Notify(sigchan, syscall.SIGTERM, syscall.SIGINT)

	<-sigchan

	log.Println("stop signal")
	log.Println("closing kafka connections")

	preprocessorProducerYandex.Close()
	crawlerConsumerYandex.Close()
	preprocessorProducerEbay.Close()
	crawlerConsumerEbay.Close()

}
