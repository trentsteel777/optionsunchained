This program scrapes finviz to generate watchlist.txt.

It finds all the optionable US stocks.

Generated file should be dropped into yahoo_scraper folder.

scraper.cpp
    Run:
        g++ -o scraper scraper.cpp -I/usr/include/libxml2 -lcurl -lxml2 && ./scraper

