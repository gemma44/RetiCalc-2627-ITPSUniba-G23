CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lws2_32

all: serverTCP_G23.exe

serverTCP_G23.exe: server.c protocol.h
	$(CC) $(CFLAGS) -o serverTCP_G23.exe server.c $(LDFLAGS)

clean:
	rm -f serverTCP_G23.exe *.o