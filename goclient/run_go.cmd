go mod init goclient
go get github.com/gorilla/websocket 
go build -o main.wasm
go run main.go
