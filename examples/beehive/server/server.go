//server.go

package main

import (
	"code.google.com/p/gopacket"
	"code.google.com/p/gopacket/layers"
	"fmt"
	"net"
	"net/http"
	"os"
	"strings"
)

const (
	CONN_HOST = ""
	CONN_PORT = "3333"
	CONN_TYPE = "tcp"
)

func main() {
	// Listen for incoming connections.
	l, err := net.Listen(CONN_TYPE, ":"+CONN_PORT)
	if err != nil {
		fmt.Println("Error listening:", err.Error())
		os.Exit(1)
	}
	// Close the listener when the application closes.
	defer l.Close()
	fmt.Println("Listening on " + CONN_HOST + ":" + CONN_PORT)
	for {
		// Listen for an incoming connection.
		conn, err := l.Accept()
		if err != nil {
			fmt.Println("Error accepting: ", err.Error())
			os.Exit(1)
		}

		//logs an incoming message
		fmt.Printf("Received message %s -> %s \n", conn.RemoteAddr(), conn.LocalAddr())

		// Handle connections in a new goroutine.
		go handleRequest(conn)
	}
}

// Handles incoming requests.
func handleRequest(conn net.Conn) {
	// Make a buffer to hold incoming data.
	buf := make([]byte, 2024)
	// Read the incoming connection into the buffer.
	reqLen, err := conn.Read(buf)
	if err != nil {
		fmt.Println("Error reading:", err.Error())
	}
	// n := bytes.Index(buf, []byte{0})
	fmt.Printf("Message: %s, length: %d \r\n", string(buf[:reqLen]), reqLen)

	//ippacket := gopacket.NewPacket(buf, layers.LayerTypeIPv6, gopacket.Default)
	//fmt.Println(ippacket)
	//payload := ippacket.TransportLayer().LayerPayload()
	//requeststr := []string{"http://beehivemonitor.azurewebsites.net/RestApi/PutData?br=1&n=1&s=weight&v=", string(payload), "&t=now"}
	//request := strings.Join(requeststr, "")
	//client := &http.Client{}
	//req, err := http.NewRequest("GET", request, nil)
	//req.Header.Set("User-Agent", "Border-router forwarder")
	//resp, err := client.Do(req)
	//fmt.Println(resp)
	//fmt.Println(err)
	//fmt.Println(request)
	defer resp.Body.Close()
	conn.Close()
}
