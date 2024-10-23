package party

import (
	"context"
	"peopleparty_backend/pkg/assert"
	"sync"
)

type Party struct {
	context     context.Context
	currentGame *interface{}
	clients     map[ClientID]client
	idCounter   ClientID
	mutex       sync.Mutex
}

func CreateParty(hostDisplayName string, ctx context.Context) *Party {
	partyContext, cancel := context.WithCancel(ctx)

	party := Party{
		currentGame: nil,
		context:     partyContext,
		clients:     make(map[ClientID]client),
		idCounter:   1,
	}

	party.clients[0] = client{
		DisplayName: hostDisplayName,
		IsHost:      true,
		ID:          0,
		Context:     partyContext,
		disconnect:  cancel,
	}

	go func() {
		select {
		case <-ctx.Done():
			cancel()
			break
		case <-partyContext.Done():
		}
	}()

	return &party
}

// Returns nil if client couldn't be added because id was invalid or already used
func (p *Party) addClient(id ClientID, displayName string, ctx context.Context) *client {
	if _, ok := p.clients[p.idCounter]; ok {
		return nil
	}

	clientContext, cancel := context.WithCancel(ctx)

	p.clients[p.idCounter] = client{
		DisplayName: displayName,
		ID:          p.idCounter,
		IsHost:      false,
		Context:     ctx,
		// To disconnect a client just stop the client context
		disconnect: cancel,
	}

	client := p.GetClient(p.idCounter)
	assert.NotNil(client, "Could not get newly created client")

	go func() {
		select {
		// If the creator context or the partycontext is done, remove the client
		case <-ctx.Done():
		case <-p.context.Done():
			cancel()
			break
		// Or the client is disconnected with client.disconnect()
		case <-clientContext.Done():
		}
		p.mutex.Lock()
		defer p.mutex.Unlock()

		client := p.GetClient(client.ID)
		if client != nil {
			p.removeCient(client.ID)
		}
	}()

	return client
}

func (p *Party) AddClient(displayName string, ctx context.Context) *client {
	if ClientID(p.GetClientCount()) == ^ClientID(0) {
		return nil
	}

	p.mutex.Lock()
	defer p.mutex.Unlock()
	for i := ClientID(0); i < ^ClientID(0); i++ {
		p.idCounter++
		if client := p.addClient(p.idCounter, displayName, ctx); client != nil {
			return client
		}
	}

	return nil
}

func (p *Party) DisconnectClient(id ClientID) {
	client := p.GetClient(id)
	if client != nil {
		client.disconnect()
	}
}

func (p *Party) removeCient(id ClientID) {
	delete(p.clients, id)
}

func (p *Party) GetClient(id ClientID) *client {
	if client, ok := p.clients[id]; ok {
		return &client
	}

	return nil
}

func (p *Party) GetClientCount() int {
	return len(p.clients)
}
