package party

import "context"

type IOChannel struct {
	input  chan []byte
	output chan []byte
}

func CreateIOChannel(ctx context.Context) *IOChannel {
	ioChan := &IOChannel{
		input:  make(chan []byte),
		output: make(chan []byte),
	}

	go func() {
		<-ctx.Done()
		ioChan.close()
	}()

	return ioChan
}

func (ioc *IOChannel) close() {
	close(ioc.input)
	close(ioc.output)
}
