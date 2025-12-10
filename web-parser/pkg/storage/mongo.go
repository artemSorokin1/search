package storage

import (
	"context"
	"fmt"
	"log"
	"time"
	"web-parser/internal/config"

	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

type Item struct {
	ID          primitive.ObjectID `bson:"_id,omitempty"`
	Description string             `bson:"description"`
	Title       string             `bson:"title"`
	UrlHash     string             `bson:"url"`
	SourceName  string             `bson:"source_name"`
	HtmlText    string             `bson:"html_text"`
	TimeOfLoad  int64              `bson:"time_of_load"`
}

type Client struct {
	Coll *mongo.Collection
	cli  *mongo.Client
}

func NewClient(cfg config.StorageConfig) (*Client, error) {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	client, err := mongo.Connect(ctx, options.Client().ApplyURI(fmt.Sprintf("mongodb://%s:%s", cfg.Host, cfg.Port)))
	if err != nil {
		log.Fatalf("error connecting to mongo %s", err)
	}

	idx := mongo.IndexModel{
		Keys:    bson.D{{Key: "url", Value: 1}},
		Options: options.Index().SetUnique(true),
	}

	coll := client.Database(cfg.DB).Collection(cfg.Collection)
	_, err = coll.Indexes().CreateOne(context.Background(), idx)
	if err != nil {
		return nil, fmt.Errorf("error creating unique index on url %s", err)
	}

	return &Client{
		cli:  client,
		Coll: coll,
	}, nil
}

func (c *Client) CloseConnection() {
	c.cli.Disconnect(context.Background())
}
