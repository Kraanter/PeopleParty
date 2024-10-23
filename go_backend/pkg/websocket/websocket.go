package websocket

import "net/http"

func InitWebsocket(router *http.ServeMux) {
	hub := createPartyHub()
	router.HandleFunc("/join", func(w http.ResponseWriter, r *http.Request) {
		serveJoinWs(hub, w, r)
	})
	router.HandleFunc("/host", func(w http.ResponseWriter, r *http.Request) {
		serveHostWs(hub, w, r)
	})
}
