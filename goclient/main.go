// +build js,wasm

package main

import (
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall/js"
	"time"

	"github.com/gorilla/websocket"
)

func goMyFunc(value js.Value, args []js.Value) interface{} {
	fmt.Println("golang: goMyFunc: Invoked.")
	js.Global().Call("updateDOM", js.ValueOf("goMyFunc: This was called from Golang!"))
	return nil
}

func main() {

	// Register the Go function as a JavaScript function
	js.Global().Set("goMyFunc", js.FuncOf(goMyFunc))

	fmt.Println("This is the start.")
	//js.Global().Call("console.log", "This is the start.")

	time.Sleep(time.Second)

	fmt.Println("Invoke updateDOM...")

	js.Global().Call("updateDOM", "This is called from Golang!")


	// WebSocket server address
	serverAddr := "ws://localhost:8080/ws"

	// Establish WebSocket connection
	conn, _, err := websocket.DefaultDialer.Dial(serverAddr, nil)
	if err != nil {
		log.Fatal("Error connecting to WebSocket server:", err)
	}
	defer conn.Close()

	fmt.Println("Connected to WebSocket server")

	// Start a goroutine to read and print messages from the server
	go readMessages(conn)

	// Wait for interruption (Ctrl+C) to gracefully close the connection
	interrupt := make(chan os.Signal, 1)
	signal.Notify(interrupt, os.Interrupt)

	select {
	case <-interrupt:
		fmt.Println("Interrupt received, closing connection.")
		conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseNormalClosure, ""))
		time.Sleep(time.Second)
		return
	}
}

func readMessages(conn *websocket.Conn) {
	for {
		_, message, err := conn.ReadMessage()
		if err != nil {
			log.Println("Error reading message:", err)
			return
		}
		fmt.Println("Received message:", string(message))
	}
}
