package websocket

import (
	"bytes"
	"context"
	"log"
	"net/http"
	"time"

	"github.com/gorilla/websocket"
	"peopleparty_backend/pkg/party"
)

const (
	// Time allowed to write a message to the peer.
	writeWait = 10 * time.Second

	// Time allowed to read the next pong message from the peer.
	pongWait = 60 * time.Second

	// Send pings to peer with this period. Must be less than pongWait.
	pingPeriod = (pongWait * 9) / 10

	// Maximum message size allowed from peer.
	maxMessageSize = 512
)

var (
	newline = []byte{'\n'}
	space   = []byte{' '}
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

// connection is a middleman between the websocket connection and the partyHub.
type connection struct {
	// The connected hub
	hub *partyHub

	// The connected party
	party *party.Party

	// The client ID of this connection
	client *party.Client

	// The websocket connection.
	conn *websocket.Conn

	// Context
	Context      context.Context
	onDisconnect []func()

	// Buffered channel of outbound messages.
	send chan []byte
}

// serveWs handles websocket requests from the peer.
func serveHostWs(hub *partyHub, w http.ResponseWriter, r *http.Request) {
	name := extractDisplayName(r)
	if name == "" {
		w.WriteHeader(404)
		return
	}

	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		// TODO: make this proper loggging
		log.Println(err)
		return
	}

	client := hub.Host(name, conn)
	if client == nil {
		w.WriteHeader(404)
		return
	}

	client.onDisconnect = append(client.onDisconnect, func() { conn.Close() })

	// Allow collection of memory referenced by the caller by doing all work in
	// new goroutines.
	go client.writePump()
	go client.readPump()
}

// serveWs handles websocket requests from the peer.
func serveJoinWs(hub *partyHub, w http.ResponseWriter, r *http.Request) {
	name := extractDisplayName(r)
	if name == "" {
		w.WriteHeader(404)
		return
	}

	code := extractPartyCode(r)
	if !hub.partyManager.PartyAvailable(code) {
		w.WriteHeader(404)
		return
	}

	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		// TODO: make this proper loggging
		log.Println(err)
		return
	}

	client := hub.Join(name, code, conn)
	if client == nil {
		w.WriteHeader(404)
		return
	}

	client.onDisconnect = append(client.onDisconnect, func() { conn.Close() })

	// Allow collection of memory referenced by the caller by doing all work in
	// new goroutines.
	go client.writePump()
	go client.readPump()
}

func (c *connection) Disconnect() {
	for _, callback := range c.onDisconnect {
		callback()
	}
}

// readPump pumps messages from the websocket connection to the hub.
//
// The application runs readPump in a per-connection goroutine. The application
// ensures that there is at most one reader on a connection by executing all
// reads from this goroutine.
func (c *connection) readPump() {
	defer c.Disconnect()

	c.conn.SetReadLimit(maxMessageSize)
	c.conn.SetReadDeadline(time.Now().Add(pongWait))
	c.conn.SetPongHandler(func(string) error { c.conn.SetReadDeadline(time.Now().Add(pongWait)); return nil })
	for {
		_, message, err := c.conn.ReadMessage()
		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				log.Printf("error: %v", err)
			}
			break
		}
		message = bytes.TrimSpace(bytes.Replace(message, newline, space, -1))
		c.client.IO.Input <- message
	}
}

// writePump pumps messages from the hub to the websocket connection.
//
// A goroutine running writePump is started for each connection. The
// application ensures that there is at most one writer to a connection by
// executing all writes from this goroutine.
func (c *connection) writePump() {
	ticker := time.NewTicker(pingPeriod)
	defer func() {
		ticker.Stop()
		c.conn.Close()
	}()
	for {
		select {
		case message, ok := <-c.client.IO.Output:
			c.conn.SetWriteDeadline(time.Now().Add(writeWait))
			if !ok {
				// The hub closed the channel.
				c.conn.WriteMessage(websocket.CloseMessage, []byte{})
				return
			}

			err := c.conn.WriteMessage(websocket.BinaryMessage, message)
			if err != nil {
				return
			}
		case <-ticker.C:
			c.conn.SetWriteDeadline(time.Now().Add(writeWait))
			if err := c.conn.WriteMessage(websocket.PingMessage, nil); err != nil {
				return
			}
		}
	}
}
