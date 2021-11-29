
CFLAGS=-std=c++17 -Wall -Werror
CC=g++

all:
	$(CC) $(CFLAGS) daemon.cpp -o diffda
