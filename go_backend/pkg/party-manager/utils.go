package partymanager

import (
	"math"
	"math/rand/v2"
	"peopleparty_backend/pkg/assert"
)

const PartyCodeLength = 3

var MinPartyCode = int(math.Pow10(PartyCodeLength - 1)) // With length 4 => 1000
var MaxPartyCode = int(math.Pow10(PartyCodeLength)) - 1 // With length 4 => 9999
var MaxPartyCount = MaxPartyCode - MinPartyCode         // => 8999

func GeneratePartyCode() PartyCode {
	// example: 1000 + rand.IntN(8999) => min output: 1000 + 0 = 1000; max output: 1000 + (9999 - 1000) = 9999
	return PartyCode(MinPartyCode + rand.IntN(MaxPartyCode-MinPartyCode))
}

// TODO: In theory this function can cause a stackoverflow
// What if?
func generateUniqueCode(notAllowedList []PartyCode) PartyCode {
	if len(notAllowedList) == MaxPartyCount {
		assert.Never("No more available party codes", MaxPartyCount)
	}

	isUsed := func(newCode PartyCode) bool {
		for _, code := range notAllowedList {
			if code == newCode {
				return true
			}
		}

		return false
	}

	for {
		genCode := GeneratePartyCode()

		if isUsed(genCode) {
			continue
		}

		return genCode
	}
}
