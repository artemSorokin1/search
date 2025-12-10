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
	preprocessorProducer, err := kafkaclient.NewProducer(addresses)
	if err != nil {
		log.Fatalf("error creating producer for preprocessor %s", err)
	}

	preproc := prerocess.NewPreprocessor(preprocessorProducer)

	// go preproc.Preprocess(cfg.KafkaCfg.Topic)

	go func() {
		log.Println("start first preprocess run")
		preproc.PreprocessYandex(cfg.KafkaCfg.YandexTopic)
		log.Println("first preprocess finished")

		ticker := time.NewTicker(7 * 24 * time.Hour)
		defer ticker.Stop()

		for range ticker.C {
			log.Println("start preprocess run")
			preproc.PreprocessYandex(cfg.KafkaCfg.WBTopic)
			log.Println("preprocess finished")
		}
	}()

	// go func() {
	// 	log.Println("start first preprocess run")
	// 	preproc.PreprocessEbay(cfg.KafkaCfg.YandexTopic)
	// 	log.Println("first preprocess finished")

	// 	ticker := time.NewTicker(7 * 24 * time.Hour)
	// 	defer ticker.Stop()

	// 	for range ticker.C {
	// 		log.Println("start preprocess run")
	// 		preproc.PreprocessEbay(cfg.KafkaCfg.WBTopic)
	// 		log.Println("preprocess finished")
	// 	}
	// }()

	crawlerConsumer, err := kafkaclient.NewConsumer(addresses, cfg.KafkaCfg.Group)
	if err != nil {
		log.Fatalf("error creating consumer for crawler %s", err)
	}

	crawlerProducer, err := kafkaclient.NewProducer(addresses)
	if err != nil {
		log.Fatalf("error creating producer for crawler %s", err)
	}

	crawler := crawler.NewProductCrawler(cfg.LogicCfg, crawlerConsumer, crawlerProducer)

	mongoClient, err := storage.NewClient(cfg.StorageCfg)
	if err != nil {
		log.Fatalf("error creating mongo client %s", err)
	}

	go crawler.Get(cfg.KafkaCfg.YandexTopic, mongoClient)

	sigchan := make(chan os.Signal, 1)
	signal.Notify(sigchan, syscall.SIGTERM, syscall.SIGINT)

	<-sigchan

	log.Println("stop signal")
	log.Println("closing kafka connections")

	preprocessorProducer.Close()
	crawlerConsumer.Close()
	crawlerProducer.Close()

	// TODO: разобраться с горутинами(поставить таймаут на запросы 20 с) и писать в mongodb(писать хэш от ссылки, описание и имя товара), убрать запись в кафку уже нормализованной информации

}
