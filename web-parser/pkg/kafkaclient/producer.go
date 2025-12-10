package kafkaclient

import (
	"encoding/json"
	"fmt"
	"log"
	"log/slog"
	"strings"

	"github.com/confluentinc/confluent-kafka-go/kafka"
)

type KafkaMsg struct {
	Url       string `json:"url"`
	ParceTime int64  `json:"parce_time"`

	TopicPartition kafka.TopicPartition `json:"-"`
}

type Producer struct {
	producer *kafka.Producer
}

func NewProducer(addresses []string) (*Producer, error) {
	p, err := kafka.NewProducer(&kafka.ConfigMap{
		"bootstrap.servers": strings.Join(addresses, ","),
	})
	if err != nil {
		return nil, err
	}

	return &Producer{
		producer: p,
	}, nil
}

func (p *Producer) Produce(topic string, msg KafkaMsg) error {
	deliveryChan := make(chan kafka.Event)

	jsonMsg, err := json.Marshal(msg)
	if err != nil {
		slog.Warn("error serializing data", err)
	}

	err = p.producer.Produce(&kafka.Message{
		TopicPartition: kafka.TopicPartition{Partition: kafka.PartitionAny, Topic: &topic},
		Value:          jsonMsg,
	}, deliveryChan)
	if err != nil {
		log.Printf("error produce msg %s", err.Error())
	}

	e := <-deliveryChan
	m := e.(*kafka.Message)

	if m.TopicPartition.Error != nil {
		fmt.Printf("nothing response from kafka %s", err.Error)
	}

	close(deliveryChan)
	return nil
}

func (p *Producer) Close() {
	p.producer.Close()
}
