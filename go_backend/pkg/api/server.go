package api

import (
	"log/slog"
	"net/http"
	"strconv"

	"peopleparty_backend/pkg/assert"
	"peopleparty_backend/pkg/websocket"
)

type ServerConfig struct {
	Host string
	Port int
}

type ApiServer struct {
	config ServerConfig
	logger *slog.Logger
}

func CreateServer(config ServerConfig) *ApiServer {
	assert.NotNil(config, "Must provide config to create server")
	assert.NotNil(config.Host, "Must provide host to create server")
	assert.NotNil(config.Port, "Must provide port to create server")

	logger := slog.Default().With("area", "ApiServer")

	return &ApiServer{
		config: config,
		logger: logger,
	}
}

func (s *ApiServer) Run() {
	// ctx, cancel := context.WithCancel(outerCtx)
	s.logger.Warn("ApiServer#Run...")

	// Create server and start it
	router := http.NewServeMux()

	websocket.InitWebsocket(router)

	println("listening on port:", s.config.Port)
	err := http.ListenAndServe(":"+strconv.Itoa(s.config.Port), router)

	assert.NoError(err, "HTTP server crashed", err)
}
