DEL /Q main.wasm
ECHO main.wasm deleted
DIR
go mod init goclient
go get github.com/gorilla/websocket 
SET GOARCH=wasm
SET GOOS=js
go build -o main.wasm
ECHO *** main.wasm created ***
DIR

SET GOARCH=amd64
SET GOOS=windows
go run main.go
