TARGET: siktacka-server siktacka-client

CC      = g++
CXXFLAGS        = -Wall -O2 -g3 -std=c++14
ROOT_DIR = $(shell pwd)

%.o : %.cpp
	$(CC) $(CXXFLAGS) -I$(ROOT_DIR) -c $^ -o $@

siktacka-server : def/ipaddr.o parse/parser.o def/util.o def/binary.o conn/ClientPackage.o server/main.o server/GameServer.o server/connect.o
	$(CC) $(CXXFLAGS) $^ -lz -o $@

siktacka-client: def/ipaddr.o parse/parser.o def/util.o def/binary.o conn/ClientPackage.o client/GameClient.o client/GUIConnection.o client/ServerConnection.o client/main.o
	$(CC) $(CXXFLAGS) $^ -lz -o $@

.PHONY: clean TARGET
clean:
	rm siktacka-server siktacka-client *.o *~ *.bak
