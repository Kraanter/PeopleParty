package party

import (
	"context"
	"peopleparty_backend/pkg/assert"
	"sync"
)

type Party struct {
	Context     context.Context
	currentGame *interface{}
	clients     map[ClientID]client
	idCounter   ClientID
	mutex       sync.Mutex
}

func CreateParty(hostDisplayName string, ctx context.Context) *Party {
	host := createClient(0, hostDisplayName, true, ctx)
	party := Party{
		currentGame: nil,
		Context:     host.Context,
		clients:     make(map[ClientID]client),
		idCounter:   1,
	}
	party.clients[0] = host

	go func() {
		select {
		case <-ctx.Done():
			host.disconnect()
			break
		case <-party.Context.Done():
		}
	}()

	return &party
}

// Returns nil if client couldn't be added because id was invalid or already used
func (p *Party) addClient(id ClientID, displayName string, ctx context.Context) *client {
	if _, ok := p.clients[p.idCounter]; ok {
		return nil
	}

	p.clients[p.idCounter] = createClient(p.idCounter, displayName, false, ctx)

	client := p.GetClient(p.idCounter)
	assert.NotNil(client, "Could not get newly created client")

	go func() {
		select {
		// If the creator context or the partycontext is done, remove the client
		case <-p.Context.Done():
			client.disconnect()
			break
		// Or the client is disconnected with client.disconnect()
		case <-client.Context.Done():
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

func (p *Party) Destory() {
	p.DisconnectClient(0)
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
