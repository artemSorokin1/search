FROM golang:1.25 AS builder

RUN apt-get update && apt-get install -y librdkafka-dev build-essential

WORKDIR /app

COPY go.mod go.sum ./
RUN go mod download

COPY . .

# CGO включён, без GOARCH/GOOS
RUN CGO_ENABLED=1 go build -o web_parser cmd/main.go

# 👇 тут главное — debian12
FROM gcr.io/distroless/base-debian12
WORKDIR /app

COPY --from=builder /app/web_parser .
COPY --from=builder /app/config/config.yml ./config/

CMD ["./web_parser"]
