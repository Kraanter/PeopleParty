package party

import "context"

type Party struct {
	context     context.Context
	currentGame *interface{}
	Clients     []Client
}

var counter int = 1

func CreateParty(context context.Context) Party {
	party := Party{
		currentGame: nil,
		context:     context,
		Clients:     make([]Client, 0),
	}

	return party
}
