go mod init goclient
go get github.com/gorilla/websocket 
SET GOARCH=wasm
SET GOOS=js
go build -o main.wasm

SET GOARCH=amd64
SET GOOS=windows
go run main.go
