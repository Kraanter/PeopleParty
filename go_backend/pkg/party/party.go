package party

import (
	"context"
	"sync"

	"peopleparty_backend/pkg/assert"
)

type Party struct {
	Context     context.Context
	gameManager *interface{}
	clients     map[ClientID]Client
	idCounter   ClientID
	mutex       sync.Mutex
}

func CreateParty(hostDisplayName string, ctx context.Context) *Party {
	host := createClient(0, hostDisplayName, true, ctx)
	party := Party{
		gameManager: nil,
		Context:     host.Context,
		clients:     make(map[ClientID]Client),
		idCounter:   1,
	}
	party.clients[0] = host

	go func() {
		for data := range host.IO.Input {
			party.updateGame(&host, data)
		}
	}()

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
func (p *Party) addClient(id ClientID, displayName string, ctx context.Context) *Client {
	if _, ok := p.clients[p.idCounter]; ok {
		return nil
	}

	p.clients[p.idCounter] = createClient(p.idCounter, displayName, false, ctx)

	client := p.GetClient(p.idCounter)
	assert.NotNil(client, "Could not get newly created client")

	go func() {
		for data := range client.IO.Input {
			p.updateGame(client, data)
		}
	}()

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

func (p *Party) AddClient(displayName string, ctx context.Context) *Client {
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

func (p *Party) GetClient(id ClientID) *Client {
	if client, ok := p.clients[id]; ok {
		return &client
	}

	return nil
}

func (p *Party) GetClientCount() int {
	return len(p.clients)
}

// TODO: implement gamemanager logic
func (p *Party) updateGame(player *Client, data ChannelData) {
	// p.gameManager
	updateMap := make(map[string]ChannelData)

	for _, client := range p.clients {
		// TODO: Make sure no duplicate names
		updateMap[client.DisplayName] = ChannelData(player.DisplayName + " to " + client.DisplayName + ": " + string(data))
	}

	p.broadCastData(ChannelData(player.DisplayName+"Called update"), updateMap)
}

func (p *Party) broadCastData(host ChannelData, clients map[string]ChannelData) {
	hostClient := p.GetClient(0)
	assert.NotNil(hostClient, "Could not broadcast to host because it is not defined this should not be possible")
	hostClient.IO.Output <- host

	nameMap := p.genDisplayNameToClientIDMap()
	for name, data := range clients {
		client := p.GetClient(nameMap[name])
		if client != nil {
			client.IO.Output <- data
		}
	}
}

func (p *Party) genDisplayNameToClientIDMap() map[string]ClientID {
	nameMap := make(map[string]ClientID)

	for ID, client := range p.clients {
		nameMap[client.DisplayName] = ID
	}

	return nameMap
}
