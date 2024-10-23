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

func CreateParty(hostDisplayName string, context context.Context) *Party {
	party := Party{
		currentGame: nil,
		context:     context,
		clients:     make(map[ClientID]client),
		idCounter:   1,
	}

	party.clients[0] = client{
		DisplayName: hostDisplayName,
		IsHost:      true,
		ID:          0,
		Context:     context,
	}

	return &party
}

func (p *Party) AddClient(displayName string, ctx context.Context) *client {
	if ClientID(p.GetClientCount()) == ^ClientID(0) {
		return nil
	}

	p.mutex.Lock()
	defer p.mutex.Unlock()
	for i := ClientID(0); i < ^ClientID(0); i++ {
		p.idCounter++
		if _, ok := p.clients[p.idCounter]; !ok {
			p.clients[p.idCounter] = client{
				DisplayName: displayName,
				ID:          p.idCounter,
				IsHost:      false,
				Context:     ctx,
			}

			client := p.GetClient(p.idCounter)
			assert.NotNil(client, "Could not get newly created client")

			go func() {
				<-ctx.Done()
				p.mutex.Lock()
				defer p.mutex.Unlock()

				client := p.GetClient(client.ID)
				if client != nil {
					p.RemoveCient(client.ID)
				}
			}()

			return client
		}
	}

	return nil
}

func (p *Party) RemoveCient(id ClientID) {
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
