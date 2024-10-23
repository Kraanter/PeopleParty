package party

import (
	"context"
)

type ClientID = uint8

type client struct {
	ID          ClientID
	IsHost      bool
	DisplayName string
	Context     context.Context
	disconnect  func()

	IO *IOChannel
}

func createClient(id ClientID, name string, isHost bool, ctx context.Context) client {
	clientContext, cancel := context.WithCancel(ctx)
	return client{
		IsHost:      isHost,
		DisplayName: name,
		ID:          id,
		Context:     clientContext,
		// To disconnect a client just stop the client context
		disconnect: cancel,

		IO: CreateIOChannel(clientContext),
	}
}
