package websocket

import (
	"context"
	partymanager "peopleparty_backend/pkg/party-manager"
	"strconv"

	"github.com/gorilla/websocket"
)

type partyHub struct {
	partyManager partymanager.PartyManager
}

func createPartyHub() *partyHub {
	return &partyHub{
		partyManager: *partymanager.CreatePartyManager(),
	}
}

func (h *partyHub) Host(name string, socketConn *websocket.Conn) *connection {
	ctx, cancel := context.WithCancel(context.Background())
	partyCode, err := h.partyManager.AppendParty(name, ctx)
	if err != nil {
		return nil
	}

	party := h.partyManager.GetParty(partyCode)
	if party == nil {
		return nil
	}

	clientConnection := &connection{
		party:        party,
		hub:          h,
		client:       party.GetClient(0),
		conn:         socketConn,
		send:         make(chan []byte),
		Context:      ctx,
		onDisconnect: []func(){cancel, func() { socketConn.Close() }},
	}

	go func() {
		clientConnection.client.IO.Output <- []byte("Party: " + strconv.Itoa(int(partyCode)))
	}()

	return clientConnection
}

func (h *partyHub) Join(name string, code partymanager.PartyCode, socketConn *websocket.Conn) *connection {
	party := h.partyManager.GetParty(code)
	if party == nil {
		return nil
	}

	ctx, cancel := context.WithCancel(context.Background())

	client := party.AddClient(name, ctx)

	clientConnection := &connection{
		party:        party,
		hub:          h,
		client:       client,
		conn:         socketConn,
		send:         make(chan []byte),
		Context:      ctx,
		onDisconnect: []func(){cancel, func() { socketConn.Close() }},
	}

	return clientConnection
}
