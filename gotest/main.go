package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	pb "minigameTest/protobuf"
	"os/exec"
	"strconv"

	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/reflect/protoreflect"
)

type Game int8

const (
	Fallback Game = iota
	BusinessBailout
	kokosnoot
)

var gameSlice = []Game{Fallback, BusinessBailout, kokosnoot}

func (g Game) String() string {
	switch g {
	case BusinessBailout:
		return "BusinessBailout"
	}

	return "Undefined"
}

func (g Game) Path() string {
	switch g {
	case BusinessBailout:
		return "../minigames/build/kokosnoot"
	}

	return "./test"
}

func promptSelectGame() Game {
	println("Available games:")

	for i, v := range gameSlice {
		if v == Fallback {
			continue
		}
		println(" - (" + strconv.Itoa(i) + ") " + v.String())
	}

	i := getInput[int]("Select a game: ")

	if len(gameSlice) < i || i < 0 {
		log.Fatal("Invalid input!")
	}

	return gameSlice[i]
}

func startGame(path string) (*io.WriteCloser, error) {
	cmd := exec.Command(path)
	input, err := cmd.StdinPipe()
	output, err := cmd.StdoutPipe()
	errpip, err := cmd.StderrPipe()
	if err != nil {
		return nil, err
	}

	err = cmd.Start()
	if err != nil {
		panic(err)
	}

	go func() {
		in := bufio.NewReader(output)
		for {
			s, err := in.ReadBytes('\n')
			if err != nil {
				panic(err)
			}

			gameUpdate := &pb.Proto_GameUpdate{}
			if err = proto.Unmarshal(s, gameUpdate); err == nil {
				fmt.Printf("====> %v", gameUpdate)
			} else {
				fmt.Printf("----> %v", string(s))
			}
		}
	}()

	go func() {
		in := bufio.NewReader(errpip)
		for {
			s, err := in.ReadBytes('\n')
			if err != nil {
				panic(err)
			}
			fmt.Printf("----> \033[44;1mINFO\033[0m: %v", string(s))
		}
	}()

	go func() {
		if e := cmd.Wait(); e != nil {
			panic(e)
		}
	}()

	return &input, nil
}

func sendProtobuf(message protoreflect.ProtoMessage, writer io.Writer) error {
	serializedData, err := proto.Marshal(message)
	if err != nil {
		return err
	}

	fmt.Printf("<---- %v len(%v)\n", serializedData, len(serializedData))

	err = binary.Write(writer, binary.LittleEndian, uint32(len(serializedData)))
	if err != nil {
		return err
	}

	_, err = writer.Write(serializedData)
	if err != nil {
		return err
	}

	return nil
}

func playGame(input io.Writer) error {
	for {
		gameUpdate := &pb.Proto_GameUpdate{}

		gameUpdate.DeltaTime = getInput[int32]("deltatime: ")
		state := &pb.ProtoGameState{
			Host:    "100",
			Global:  "",
			Players: make(map[string]string),
		}
		gameUpdate.GameState = state

		isPlayerupdate := "y" == getInput[string]("Is player update (y/n)? ")
		if isPlayerupdate {
			gameUpdate.Reason = pb.PROTO_GAME_UPDATE_REASON_PLAYER_INPUT
			playerName := getInput[string]("Player name: ")
			gameUpdate.Playername = &playerName

			gameUpdate.GameState.Players[playerName] = "-1"
		} else {
			gameUpdate.Reason = pb.PROTO_GAME_UPDATE_REASON_GAME_TICK
		}
		gameUpdate.Payload = "1"
		err := sendProtobuf(gameUpdate, input)
		if err != nil {
			return err
		}

		getInput[[]byte]("")
	}
}

func createGameUpdate() {

}

func getInput[T any](question string) T {
	fmt.Print(question)
	var i T
	_, err := fmt.Scan(&i)
	if err != nil {
		log.Fatal("Could not parse input ", question, err.Error())
	}
	return i
}

func main() {
	fmt.Print("\033[H\033[2J")
	selectedGame := promptSelectGame()
	if selectedGame == Fallback {
		log.Fatal("Could not select game")
	}

	input, err := startGame(selectedGame.Path())
	if err != nil {
		log.Fatal("Could not start game")
	}

	err = playGame(*input)
	if err != nil {
		log.Fatal("Error while playing game ", err)
	}
}
