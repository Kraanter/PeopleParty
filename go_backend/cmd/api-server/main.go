package main

import (
	"peopleparty_backend/pkg/api"
)

func main() {
	host, port := api.GetHostAndPort()

	cfg := api.ServerConfig{
		Host: host,
		Port: port,
	}

	server := api.CreateServer(cfg)

	server.Run()
}
