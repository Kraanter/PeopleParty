package partymanager

import (
	"context"
	"peopleparty_backend/pkg/party"
)

type PartyCode = uint32

type PartyManager struct {
	partyDict map[PartyCode]party.Party

	// List of all active party codes
	// This list should only be read from because it is crucial for the generation of new codes
	activePartyCodes []PartyCode
}

func CreatePartyManager() PartyManager {
	return PartyManager{
		partyDict: make(map[PartyCode]party.Party),
	}

}

func (pm *PartyManager) GetParty(partyCode PartyCode) *party.Party {
	party, ok := pm.partyDict[partyCode]
	if !ok {
		return nil
	}

	return &party
}

// Returns a copy of the active party codes list
func (pm *PartyManager) GetActivePartyCodes() []PartyCode {
	codes := make([]PartyCode, len(pm.activePartyCodes))
	copy(codes, pm.activePartyCodes)

	return codes
}

func (pm *PartyManager) AppendParty(context context.Context) (PartyCode, error) {
	code := generateUniquePartyCode(pm.activePartyCodes)

	pm.partyDict[code] = party.CreateParty(context)
	pm.activePartyCodes = append(pm.activePartyCodes, code)

	go func() {
		<-context.Done()
		if pm.GetParty(code) != nil {
			pm.RemoveParty(code)
		}
	}()

	return code, nil
}

func (pm *PartyManager) RemoveParty(codeToRemove PartyCode) {
	for index, code := range pm.activePartyCodes {
		if code == codeToRemove {
			// Remove the index of the party code form the array
			pm.activePartyCodes = append(pm.activePartyCodes[:index], pm.activePartyCodes[index+1:]...)
			delete(pm.partyDict, code)
			return
		}
	}
}
