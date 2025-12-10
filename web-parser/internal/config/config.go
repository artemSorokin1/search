package config

import (
	"os"
	"time"

	"github.com/ilyakaznacheev/cleanenv"
)

type Config struct {
	StorageCfg StorageConfig `yaml:"db"`
	LogicCfg   LogicConfig   `yaml:"logic"`
	KafkaCfg   KafkaConfig   `yaml:"kafka"`
}

type StorageConfig struct {
	Host       string `yaml:"host"`
	Port       string `yaml:"port"`
	DB         string `yaml:"db"`
	Collection string `yaml:"collection"`
}

type LogicConfig struct {
	DelayTime time.Duration `yaml:"delay_time"`
}

type KafkaConfig struct {
	Brokers     []string `yaml:"brokers"`
	Group       string   `yaml:"group"`
	YandexTopic string   `yaml:"yandex_topic"`
	WBTopic     string   `yaml:"wb_topic"`
}

func LoadConfig() *Config {
	cfgPath := os.Getenv("CONFIG_PATH")
	_, err := os.Stat(cfgPath)
	if cfgPath == "" || os.IsNotExist(err) {
		panic(err)
	}

	var cfg Config
	cleanenv.ReadConfig(cfgPath, &cfg)

	return &cfg
}
