all: client server

client:client.cpp Interface.cpp
	g++ client.cpp Interface.cpp -o client
server:server.cpp
	g++ server.cpp -o server

clean:
	rm server client
