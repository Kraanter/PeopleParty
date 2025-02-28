package websocket

import (
	"net/http"
	"strconv"

	partymanager "peopleparty_backend/pkg/party-manager"
)

func extractPartyCode(r *http.Request) partymanager.PartyCode {
	code := r.URL.Query().Get("code")
	if code == "" {
		return 0
	}
	codeInt, err := strconv.Atoi(code)
	if err != nil {
		return 0
	}
	partyCode := partymanager.PartyCode(codeInt)

	return partyCode
}

func extractDisplayName(r *http.Request) string {
	return r.URL.Query().Get("name")
}
