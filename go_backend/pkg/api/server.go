package api

import (
	"context"
	"log/slog"

	"peopleparty_backend/pkg/assert"
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

func (s *ApiServer) Run(outerCtx context.Context) error {
	// ctx, cancel := context.WithCancel(outerCtx)
	s.logger.Warn("ApiServer#Run...")

	// Create server and start it

	return nil
}
