all: server client

server: server.o Uti.o CS450Header7.h 450UtilsUDP.h
	g++ -o server server.o Uti.o -Wall

client: client.o Uti.o CS450Header7.h 450UtilsUDP.h
	g++ -o client client.o Uti.o -Wall

server.o: server.cpp
	g++ -c server.cpp

client.o: client.cpp
	g++ -c client.cpp

Uti.o: Uti.cpp
	g++ -c Uti.cpp
