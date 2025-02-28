package gamerunner

import (
	"context"
	"fmt"
	"peopleparty_backend/pkg/cmd"
)

type gameRunner struct {
	context    context.Context
	StdoutChan chan []byte
	exit       func()
	Process    cmd.Cmder
}

func CreateGameRunner(command string, ctx context.Context) *gameRunner {
	gameRunnerCtx, cancel := context.WithCancel(ctx)
	cmder := cmd.NewCmder(command, gameRunnerCtx)
	stdoutChan := make(chan []byte)

	go func() {
		<-gameRunnerCtx.Done()
		close(stdoutChan)
	}()

	cmder.WithOutFn(func(b []byte) (int, error) {
		stdoutChan <- b
		return len(b), nil
	}).WithErrFn(func(b []byte) (int, error) {
		fmt.Printf("Game info (%v) -> %v", command, string(b))
		return len(b), nil
	})

	return &gameRunner{
		Process:    *cmder,
		exit:       cancel,
		context:    gameRunnerCtx,
		StdoutChan: stdoutChan,
	}
}

func (runner *gameRunner) Start() error {
	return runner.Process.Run([]string{})
}

func (runner *gameRunner) WriteMessage(message []byte) error {
	return runner.Process.WriteLine(message)
}
