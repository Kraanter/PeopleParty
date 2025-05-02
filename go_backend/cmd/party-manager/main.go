package main

import (
	"context"
	"fmt"
	"time"

	"peopleparty_backend/pkg/assert"
	"peopleparty_backend/pkg/party-manager"
)

func main() {
	manager := partymanager.CreatePartyManager()

	partyContext, cancel := context.WithCancel(context.Background())
	code, err := manager.AppendParty("host", partyContext)
	assert.NoError(err, "Error while appending party")

	party := manager.GetParty(code)
	assert.NotNil(party, "Could not get appended party")

	activePartyCodes := manager.GetActivePartyCodes()
	assert.Assert(len(activePartyCodes) == 1, "The generated party code is not appended to the list", manager, activePartyCodes)
	assert.Assert(activePartyCodes[0] == code, "The generated party code is not registered in the list", manager, activePartyCodes)

	cancel()
	// It takes a few moments for the context to cancel and remove the party
	time.Sleep(150 * time.Microsecond)

	assert.Assert(len(activePartyCodes) == 1, "The active party codes getter is not making a copy of the party codes slice", activePartyCodes)
	assert.Assert(len(manager.GetActivePartyCodes()) == 0, "The generated party code is not removed after context cancel", manager)

	allPartyContext, cancel := context.WithCancel(context.Background())
	for i := 0; i < partymanager.MaxPartyCount; i++ {
		manager.AppendParty("host", allPartyContext)
	}

	assert.Assert(len(manager.GetActivePartyCodes()) == partymanager.MaxPartyCount, "Can't create the maximum amount of parties", len(manager.GetActivePartyCodes()), partymanager.MaxPartyCount)
	fmt.Printf("Succesfully created %v parties\n", len(manager.GetActivePartyCodes()))

	cancel()
	time.Sleep(100 * time.Millisecond)

	assert.Assert(len(manager.GetActivePartyCodes()) == 0, "Cancelling all party context didn't remove all parties", len(manager.GetActivePartyCodes()))
	fmt.Println("Succesfully removed all parties")

	code, err = manager.AppendParty("host", context.Background())
	party = manager.GetParty(code)
	assert.NotNil(party, "Party was not created correctly, can't be found by using PartyCode", code, manager)
	// Disconnect the host to delete party
	party.Destory()
	time.Sleep(10 * time.Millisecond)

	party = manager.GetParty(code)
	assert.Assert(party == nil, "Party was not removed from manager after destroy is called", party, manager)
	assert.Assert(len(manager.GetActivePartyCodes()) == 0, "Party code was not removed from manager after destroy is called", party, manager)
}
