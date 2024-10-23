package partymanager

import (
	"context"
	"peopleparty_backend/pkg/party"
	"sync"
)

type PartyCode = uint32

type PartyManager struct {
	mutex     sync.Mutex
	partyDict map[PartyCode]*party.Party

	// List of all active party codes
	// This list should only be read from because it is crucial for the generation of new codes
	activePartyCodes []PartyCode
}

func CreatePartyManager() *PartyManager {
	return &PartyManager{
		partyDict: make(map[PartyCode]*party.Party),
	}

}

func (pm *PartyManager) PartyAvailable(partyCode PartyCode) bool {
	return pm.GetParty(partyCode) != nil
}

func (pm *PartyManager) GetParty(partyCode PartyCode) *party.Party {
	pm.mutex.Lock()
	defer pm.mutex.Unlock()
	party, ok := pm.partyDict[partyCode]
	if !ok {
		return nil
	}

	return party
}

// Returns a copy of the active party codes list
func (pm *PartyManager) GetActivePartyCodes() []PartyCode {
	codes := make([]PartyCode, len(pm.activePartyCodes))
	copy(codes, pm.activePartyCodes)

	return codes
}

func (pm *PartyManager) AppendParty(hostName string, ctx context.Context) (PartyCode, error) {
	code := generateUniquePartyCode(pm.activePartyCodes)

	createdParty := party.CreateParty(hostName, ctx)
	pm.partyDict[code] = createdParty
	pm.activePartyCodes = append(pm.activePartyCodes, code)

	go func() {
		// Remove the party if the party is done
		<-createdParty.Context.Done()

		if pm.GetParty(code) != nil {
			pm.removeParty(code)
		}
	}()

	return code, nil
}

// The only way a party can be removed is by stopping the context it is bound to
func (pm *PartyManager) removeParty(codeToRemove PartyCode) {
	pm.mutex.Lock()
	defer pm.mutex.Unlock()
	for index, code := range pm.activePartyCodes {
		if code == codeToRemove {
			// Remove the index of the party code form the array
			pm.activePartyCodes = append(pm.activePartyCodes[:index], pm.activePartyCodes[index+1:]...)
			delete(pm.partyDict, code)
			return
		}
	}
}
