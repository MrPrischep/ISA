CC = g++
CFLAGS = -Wall -Wextra
LDFLAGS	= -lpcap

.PHONY: all myClient.cpp myClient

all: myClient.cpp myClient

myClient: myClient.o 
	$(CC) $^ -o $@ $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) -I. $< -o $@

clean:
	rm -rf myClient.o myClient