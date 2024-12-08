CC = g++
CFLAGS = -I/usr/include
LDFLAGS = -lcurl -lfmt

all: fetch_options

fetch_options: fetch_options.cpp
	$(CC) $(CFLAGS) fetch_options.cpp $(LDFLAGS) -o fetch_options

debug: CFLAGS += -g
debug: fetch_options

clean:
	rm -f fetch_options
