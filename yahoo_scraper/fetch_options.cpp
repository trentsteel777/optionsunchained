#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <thread>
#include <curl/curl.h>
#include <string>
#include <ctime>
#include <sys/stat.h>   // For Unix-based systems
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>     // For Windows
#endif

#include <nlohmann/json.hpp>

#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>

// Declare a global logger pointer
std::shared_ptr<spdlog::logger> app_logger;
std::shared_ptr<spdlog::logger> error_logger;

// Function to initialize spdlog with file rotation
void init_spdlog() {
    // Create log directory if it doesn't exist
    std::filesystem::create_directory("log");

    // Setup app.log with daily rotation at midnight
    app_logger = spdlog::daily_logger_mt("app_logger", "log/app.log", 0, 0);
    app_logger->set_level(spdlog::level::info); // Set log level for general logs
    app_logger->flush_on(spdlog::level::info); // Flush after every info log

    // Setup error.log with daily rotation at midnight
    error_logger = spdlog::daily_logger_mt("error_logger", "log/error.log", 0, 0);
    error_logger->set_level(spdlog::level::err); // Set log level for error logs
    error_logger->flush_on(spdlog::level::err); // Flush after every error log

    // Example log messages
    app_logger->info("This is a general info log.");
    app_logger->warn("This is a warning log.");
    app_logger->debug("This is a debug message (won't be logged).");

    error_logger->error("This is an error message.");
    error_logger->critical("This is a critical error.");
}

// Callback to store data received from CURL
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to retrieve headers for the API request
struct curl_slist *get_headers() {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:126.0) Gecko/20100101 Firefox/126.0");
    headers = curl_slist_append(headers, "Accept: */*");
    headers = curl_slist_append(headers, "Accept-Language: en-GB,en;q=0.5");
    //headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br, zstd");
    headers = curl_slist_append(headers, "Referer: https://finance.yahoo.com/quote/AAPL/options/");
    headers = curl_slist_append(headers, "Content-Type: text/plain");
    headers = curl_slist_append(headers, "Origin: https://finance.yahoo.com");
    headers = curl_slist_append(headers, "Connection: keep-alive");
    headers = curl_slist_append(headers, "Cookie: GUC=AQABCAFmM-VmZEIeLAQy&s=AQAAAFcOYMD8&g=ZjKe3g; A1=d=AQABBAMFn2UCELaarNGHlTIObFHZwR1VlYQFEgABCAHlM2ZkZuIjb2UB9qMAAAcIAQWfZYKw0mI&S=AQAAAspdWcZCG_vcCQOG5cwIauQ; A3=d=AQABBAMFn2UCELaarNGHlTIObFHZwR1VlYQFEgABCAHlM2ZkZuIjb2UB9qMAAAcIAQWfZYKw0mI&S=AQAAAspdWcZCG_vcCQOG5cwIauQ; cmp=t=1717606887&j=1&u=1---&v=29; PRF=t%3DELOX%252BPANW%252BPYPL%252BCOIN%252BSLV%252BEURUSD%253DX%252BNVDA%252BPLTR%252BNKE%252BTSLA%252BSPY%252BGLD%252BTLT%252BIEF%252BBB%26newChartbetateaser%3D1; axids=gam=y-IVEA9vlE2uJkoK8j5IpbEbjgu6168y.F~A&dv360=eS0ySmc1Snh0RTJ1SDVKb0NNOTQ4TjBkMXhSQTZ2WEVEWH5B&ydsp=y-.h8eHlBE2uLifzKlhnTatUkr8toNw.Rj~A&tbla=y-pz4K8WVE2uKYEA.FnTcZun2697i1uAi6~A; tbla_id=a42f4d8d-6685-4b70-b592-29ca1fba5071-tuctd2c245e; A1S=d=AQABBAMFn2UCELaarNGHlTIObFHZwR1VlYQFEgABCAHlM2ZkZuIjb2UB9qMAAAcIAQWfZYKw0mI&S=AQAAAspdWcZCG_vcCQOG5cwIauQ; EuConsent=CP4KqwAP4KqwAAOACBENA3EoAP_gAEPgACiQJhNB9G7WTXFneXp2YPskOYUX0VBJ4MAwBgCBAcABzBIUIAwGVmAzJEyIICACGAIAIGJBIABtGAhAQEAAYIAFAABIAEEAABAAIGAAACAAAABACAAAAAAAAAAQgEAXMBQgmAZEAFoIQUhAhgAgAQAAIAAEAIgBAgQAEAAAQAAICAAIACgAAgAAAAAAAAAEAFAIEQAAAAECAotkfQTBADINSogCLAkJCIQcIIEAIgoCACgQAAAAECAAAAmCAoQBgEqMBEAIEQAAAAAAAAQEACAAACABCAAIAAgQAAAAAQAAAAACAAAEAAAAAAAAAAAAAAAAAAAAAAAAAMQAhBAACAACAAgoAAAABAAAAAAAAIARAAAAAAAAAAAAAAAAARAAAAAAAAAAAAAAAAAAAQIAAAAAAABAAILAAA");
    headers = curl_slist_append(headers, "Sec-Fetch-Dest: empty");
    headers = curl_slist_append(headers, "Sec-Fetch-Mode: cors");
    headers = curl_slist_append(headers, "Sec-Fetch-Site: same-site");
    headers = curl_slist_append(headers, "Priority: u=4");
    headers = curl_slist_append(headers, "TE: trailers");

    return headers;
}

// Function to retrieve current date in YYYYmmdd format
std::string get_current_date() {
    time_t t = time(nullptr);
    struct tm tm = *localtime(&t);
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%Y%m%d", &tm);
    return std::string(buffer);
}

// Function to convert a timestamp to YYYYmmdd format
std::string format_timestamp_to_date(const std::string& timestamp) {
    time_t t = std::stol(timestamp);
    struct tm tm = *localtime(&t);
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%Y%m%d", &tm);
    return std::string(buffer);
}

// Function to create directories recursively
void create_directories(const std::string& path) {
    try {
        std::filesystem::create_directories(path);  // This will create the full directory structure if it doesn't exist
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating directories: " << e.what() << std::endl;
    }
}

// Method to call the API and return the crumb
std::string getCrumb() {
    const char* url = "https://query2.finance.yahoo.com/v1/test/getcrumb";
    std::string response; // Holds the response body

    // Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL." << std::endl;
        return "";
    }

    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url);                       // Set the URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);   // Set the write callback
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);           // Pass the response string to the callback
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);             // Follow redirects if any
    
    struct curl_slist *headers = get_headers();
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the API request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }

    // Clean up CURL
    curl_easy_cleanup(curl);

    return response; // Return the response body
}


// Callback function for writing data to a file
static size_t WriteCallbackJsonToFile(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* outFile = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;

    // Write the data to the file
    if (outFile && outFile->is_open()) {
        outFile->write(static_cast<char*>(contents), totalSize);
    }
    return totalSize;
}

// Function to read ticker symbols from the watchlist.txt file
std::vector<std::string> read_watchlist(const std::string& watchlist_path) {
    std::vector<std::string> tickers;
    std::ifstream watchlist_file(watchlist_path);
    if (!watchlist_file.is_open()) {
        std::cerr << "Failed to open the file: " << watchlist_path << std::endl;
        return tickers;  // Return empty vector if file cannot be opened
    }

    std::string ticker;
    while (std::getline(watchlist_file, ticker)) {
        tickers.push_back(ticker);
    }
    watchlist_file.close();
    return tickers;
}

// Function to get expiration dates from the Yahoo Finance API
std::vector<std::string> getExpirationDates(const std::string& ticker, const std::string& crumb) {
    // Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        error_logger->error("Failed to initialize CURL.");
        return {};
    }

    // Construct the URL using the ticker and crumb parameters
    std::string url = "https://query2.finance.yahoo.com/v7/finance/options/" + ticker + "?crumb=" + crumb;

    // Set up CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    struct curl_slist *headers = get_headers();
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Prepare a string to capture the response
    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        error_logger->error("CURL request failed: {}", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return {};
    }

    // Clean up CURL
    curl_easy_cleanup(curl);

    // Parse the JSON response
    nlohmann::json json = nlohmann::json::parse(response_string);

    // Extract expiration dates as a vector of integers
    std::vector<int64_t> expiration_dates = json["optionChain"]["result"][0]["expirationDates"];

    // Vector to hold the formatted date strings
    std::vector<std::string> formatted_dates;

    // Convert each timestamp to a string and add it to the vector
    for (int64_t timestamp : expiration_dates) {
        formatted_dates.push_back(std::to_string(timestamp));
    }

    // Sleep for 1 second
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return formatted_dates;
}

int main() {
    init_spdlog();

    // Log messages
    //spdlog::get("app_logger")->info("This is an info message");
    //spdlog::get("app_logger")->warn("This is a warning message");
    //spdlog::get("app_logger")->error("This is an error message");

    std::string crumb = getCrumb();
    if (!crumb.empty()) {
        app_logger->info("Crumb: {}", crumb);
    } else {
        app_logger->error("Failed to retrieve crumb.");
    }

    // Read ticker symbols into a vector
    std::vector<std::string> tickers = read_watchlist("watchlist.txt");
    if (tickers.empty()) {
        error_logger->error("No tickers found in the watchlist.");
        return 1;
    }

    // Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        error_logger->error("Failed to initialize CURL.");
        return 1;
    }

    // Vector to hold tickers that encountered errors
    std::vector<std::string> errorTickers;

    // Iterate over each ticker symbol and generate the updated URL
    const char* url_template = "https://query2.finance.yahoo.com/v7/finance/options/%s?formatted=true&lang=en-US&region=US&date=%s&crumb=%s"; // Example API URL
    std::string currentDate = get_current_date();
    for (const std::string& ticker : tickers) {
        try {
            std::vector<std::string> expirationDates = getExpirationDates(ticker, crumb);
            
            std::string outputDir = "option_chains/" + currentDate + "/" + ticker;
            create_directories(outputDir);
            
            for (const std::string& expirationDate : expirationDates) {
                app_logger->info("Expiration Date: {}", expirationDate);
                
                std::string formattedDate = format_timestamp_to_date(expirationDate);
                std::string outputPath = outputDir + "/" + formattedDate + ".json";

                bool success = false;  // Track if the operation succeeds
                for (int attempt = 0; attempt < 2; ++attempt) {  // Retry up to 2 times
                    try {
                    
                        // Open the output file
                        std::ofstream outFile(outputPath);
                        if (!outFile.is_open()) {
                            error_logger->error("Failed to open file for writing: {}", outputPath);
                            return 1;
                        }

                        // Print the generated URL (or use it in your request)
                        char url[512];  // Buffer to hold the final URL
                        std::sprintf(url, url_template, ticker.c_str(), expirationDate.c_str(), crumb.c_str());
                        app_logger->info("Generated URL for {}: {}", ticker, url);

                        // Set CURL options
                        curl_easy_setopt(curl, CURLOPT_URL, url);                         // API URL
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackJsonToFile);     // Callback function
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);              // Pass the file stream to the callback
                        //curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
                        struct curl_slist *headers = get_headers();
                        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                        // Perform the API request
                        CURLcode res = curl_easy_perform(curl);
                        if (res != CURLE_OK) {
                            error_logger->error("CURL request failed: {}", curl_easy_strerror(res));
                        } else {
                            app_logger->info("JSON response saved to {}", outputPath);
                        }

                        // Reset the CURL handle to clear previous settings
                        curl_easy_reset(curl);
                        outFile.close();

                        // Sleep for 1 second
                        std::this_thread::sleep_for(std::chrono::milliseconds(2500));

                        success = true;  // Mark operation as successful
                        break;  // Exit retry loop
                    } catch (const std::exception& e) {
                        error_logger->error("Error processing expiration date {} (attempt {}): {}", expirationDate, (attempt + 1), e.what());
                    } catch (...) {
                        error_logger->error("Unknown error processing expiration date {} (attempt {}).", expirationDate, (attempt + 1));
                    }
                }
            }
        
        } catch (const std::exception& e) {
            // Catch any standard exception and log the ticker
            error_logger->error("Error processing ticker {}: {}", ticker, e.what());
            errorTickers.push_back(ticker);
        } catch (...) {
            // Catch any non-standard exception
            error_logger->error("Unknown error processing ticker {}", ticker);
            errorTickers.push_back(ticker);
        }
    }

    // Clean up
    curl_easy_cleanup(curl);
    return 0;
}
