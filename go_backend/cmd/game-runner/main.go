package main

import (
	"context"
	"fmt"
	"peopleparty_backend/pkg/assert"
	gamerunner "peopleparty_backend/pkg/game-runner"
	"time"
)

func main() {
	timeContext, _ := context.WithTimeout(context.Background(), 1*time.Second)
	runner := gamerunner.CreateGameRunner("../minigames/build/kokosnoot", timeContext)

	go runner.Start()

	time.Sleep(10 * time.Millisecond)
	go func() {
		err := runner.WriteMessage([]byte{13, 0, 0, 0, 18, 5, 10, 3, 49, 48, 48, 26, 1, 49, 32, 232, 7})
		assert.NoError(err, "Failed to write line")
	}()

	go func() {
		for msg := range runner.StdoutChan {
			fmt.Printf("output: %v", msg)
		}
	}()
	<-timeContext.Done()
}
