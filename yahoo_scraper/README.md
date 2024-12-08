Installation:
    sudo apt-get install libcurl4-openssl-dev libcjson-dev libboost-filesystem-dev libboost-system-dev
    sudo apt-get install nlohmann-json3-dev libxml2-dev libspdlog-dev
    sudo apt install gdb

    Check boost version:
        cat /usr/include/boost/version.hpp | grep "#define BOOST_LIB_VERSION"

Clean:
    rm -f *.o fetch_options finviz_scraper
    

fetch_options.cpp
    Run:
        g++ fetch_options.cpp -o fetch_options -lcurl && ./fetch_options
    
    Debug compile:
        needed to hit VS Code breakpoints
            g++ fetch_options.cpp -g -o fetch_options -lcurl


crontab:
    chmod +x run.sh
    0 17 * * 1-5 cd /home/trentsteel777/workspace/optionsunchained/yahoo_scraper/ && ./run.sh


GCP VM:
    Setting system clock to America/New_York
        https://www.inmotionhosting.com/support/product-guides/cloud-server/ubuntu-timezone-timedatectl/
        timedatectl list-timezones | grep "New"
        sudo timedatectl set-timezone America/New_York
        timedatectl

    rclone
        Do remote setup
            For auto-config choose No, then in main machine get success code from URL path query param
        Mounting:
            rclone mount gdrive:/option_chains option_chains --daemon --vfs-cache-mode full

        Copy:
            rclone copy option_chains/ gdrive:/option_chains