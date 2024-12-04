Installation:
    sudo apt-get install libcurl4-openssl-dev libcjson-dev libboost-filesystem-dev libboost-system-dev
    sudo apt-get install nlohmann-json3-dev libxml2-dev
    sudo apt install gdb

Clean:
    rm -f *.o fetch_options finviz_scraper
    

fetch_options.cpp
    Run:
        g++ fetch_options.cpp -o fetch_options -lcurl && ./fetch_options
    
    Debug compile:
        needed to hit VS Code breakpoints
            g++ fetch_options.cpp -g -o fetch_options -lcurl


