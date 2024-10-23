package party

import "context"

type ClientID = uint8

type client struct {
	ID          ClientID
	IsHost      bool
	DisplayName string
	Context     context.Context
}
