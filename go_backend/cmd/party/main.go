package main

import (
	"context"
	"time"

	"peopleparty_backend/pkg/assert"
	"peopleparty_backend/pkg/party"
)

func main() {
	hostName := "host"
	partyInstance := party.CreateParty(hostName, context.Background())

	host := partyInstance.GetClient(0)
	assert.NotNil(host, "Couldn't get host client from party instantly after creating the party", host, partyInstance)
	assert.Assert(host.IsHost == true, "Host client doesn't have the isHost flag")
	assert.Assert(host.DisplayName == hostName, "Host client DisplayName is not set correctly", host, hostName, partyInstance)

	nilClient := partyInstance.GetClient(6)
	assert.Assert(nilClient == nil, "Getting undefined ClientID returns a client", nilClient, partyInstance)

	clientName := "Client"
	clientContext, cancel := context.WithCancel(context.Background())
	client := partyInstance.AddClient(clientName, clientContext)
	assert.NotNil(client, "Couldn't get client from party instantly after adding to party", client, partyInstance)
	assert.Assert(client.IsHost == false, "Client doesn't have the isHost flag set to false")
	assert.Assert(client.DisplayName == clientName, "Client DisplayName is not set correctly", client, clientName, partyInstance)

	clientId := client.ID

	newClient := partyInstance.GetClient(clientId)
	assert.NotNil(newClient, "Couldn't get client from party with get client on added ID", newClient, client, partyInstance)

	cancel()
	time.Sleep(1 * time.Millisecond)

	nilClient = partyInstance.GetClient(clientId)
	assert.Assert(nilClient == nil, "Getting context cancelled client returns a client", nilClient, clientId, partyInstance)

	allClientContext, cancel := context.WithCancel(context.Background())
	for i := party.ClientID(1); i < ^party.ClientID(0); i++ {
		client := partyInstance.AddClient("client", allClientContext)
		assert.NotNil(client, "Failed to add client during mass client addition", i)
	}

	assert.Assert(party.ClientID(partyInstance.GetClientCount()) == ^party.ClientID(0), "Not all clients created")

	nilClient = partyInstance.AddClient("client", context.Background())
	assert.Assert(nilClient == nil, "Client not null after adding on full party", nilClient)

	cancel()
	time.Sleep(1 * time.Millisecond)

	clientCount := partyInstance.GetClientCount()
	assert.Assert(clientCount == 1, "After canceling context not all clients are removed", clientCount)
}
