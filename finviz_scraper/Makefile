CC = g++
CFLAGS = -I/usr/include/libxml2
LDFLAGS = -lcurl -lxml2

all: scraper

scraper: scraper.cpp
	$(CC) $(CFLAGS) scraper.cpp $(LDFLAGS) -o scraper

clean:
	rm -f scraper
