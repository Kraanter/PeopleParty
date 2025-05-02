package party

import "context"

type ChannelData = []byte

type IOChannel struct {
	Input  chan ChannelData
	Output chan ChannelData
}

func CreateIOChannel(ctx context.Context) *IOChannel {
	ioChan := &IOChannel{
		Input:  make(chan ChannelData),
		Output: make(chan ChannelData),
	}

	go func() {
		<-ctx.Done()
		ioChan.close()
	}()

	return ioChan
}

func (ioc *IOChannel) close() {
	close(ioc.Input)
	close(ioc.Output)
}
