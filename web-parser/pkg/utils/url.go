package utils

const (
	YandexMarket = "https://market.yandex.ru"
)

func NormalizeUrl(url string) string {
	for i := range url {
		if url[i] == '?' {
			return url[:i]
		}
	}

	return url
}
