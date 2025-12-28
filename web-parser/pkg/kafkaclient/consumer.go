package kafkaclient

import (
	"encoding/json"
	"fmt"
	"log"
	"strings"
	"time"

	"github.com/confluentinc/confluent-kafka-go/kafka"
)

type Consumer struct {
	consumer *kafka.Consumer
}

func NewConsumer(addresses []string, groupID string) (*Consumer, error) {
	newConsumer, err := kafka.NewConsumer(&kafka.ConfigMap{
		"bootstrap.servers":  strings.Join(addresses, ","),
		"group.id":           groupID,
		"enable.auto.commit": false,
		"auto.offset.reset":  "earliest",
	})
	if err != nil {
		return nil, fmt.Errorf("error creating consumer %s", err)
	}
	return &Consumer{
		consumer: newConsumer,
	}, nil
}

// func (c *Consumer) Read(topic string, recivedChan chan<- KafkaMsg) {
// 	err := c.consumer.Subscribe(topic, nil)
// 	if err != nil {
// 		log.Fatalf("error subscripe to topic %s", err)
// 	}

// 	sigChan := make(chan os.Signal, 1)
// 	signal.Notify(sigChan, syscall.SIGTERM, syscall.SIGINT)

// 	run := true
// 	for run {
// 		select {
// 		case sig := <-sigChan:
// 			log.Printf("получен сигнал прерывания %v", sig)
// 			run = false
// 		default:
// 			jsonMsg, err := c.consumer.ReadMessage(-1)
// 			if err == nil {
// 				var msg KafkaMsg
// 				err = json.Unmarshal(jsonMsg.Value, &msg)
// 				if err != nil {
// 					log.Printf("error umarshaling data %s", err)
// 				}
// 				fmt.Println("consumer read data: ", msg)
// 				recivedChan <- msg
// 				fmt.Println("consumer write data in recived chan")
// 			} else {
// 				log.Printf("error reading data %s", err)
// 			}
// 			time.Sleep(20 * time.Second)
// 		}
// 	}
// }

func (c *Consumer) Read(topic string, recivedChan chan<- KafkaMsg) {
	err := c.consumer.Subscribe(topic, nil)
	if err != nil {
		log.Fatalf("error subscribe to topic %s", err)
	}

	for {
		jsonMsg, err := c.consumer.ReadMessage(-1)
		if err != nil {
			log.Printf("error reading data %s", err)
			time.Sleep(5 * time.Second)
			continue
		}

		var msg KafkaMsg
		if err := json.Unmarshal(jsonMsg.Value, &msg); err != nil {
			log.Printf("error unmarshaling data %s", err)
			continue
		}

		msg.TopicPartition = jsonMsg.TopicPartition
		fmt.Println("consumer read data: ", msg)

		recivedChan <- msg
	}
}

func (c *Consumer) Commit(tp kafka.TopicPartition) error {
	_, err := c.consumer.CommitOffsets([]kafka.TopicPartition{tp})
	return err
}

func (c *Consumer) Close() {
	c.consumer.Close()
}
