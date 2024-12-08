#include <iostream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <regex>
#include <sstream>
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip> 

// Helper function to write received data from curl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch HTML content from a URL
std::string fetchHTML(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string htmlContent;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlContent);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }

    return htmlContent;
}

// Function to extract tickers from HTML content
std::vector<std::string> extractTickers(const std::string& htmlContent) {
    std::vector<std::string> tickers;

    // Updated regular expression to extract the content between <!-- TS and TE -->
    std::regex tsRegex("<!--\\s*TS([\\s\\S]*?)TE\\s*-->");
    std::smatch match;

    if (std::regex_search(htmlContent, match, tsRegex)) {
        // Extract the content between <!-- TS and TE -->
        std::string tsContent = match[1];

        // Split the content into lines
        std::istringstream ss(tsContent);
        std::string line;
        
        while (std::getline(ss, line)) {
            // Find the ticker symbol (everything before the first '|')
            size_t pos = line.find('|');
            if (pos != std::string::npos) {
                std::string ticker = line.substr(0, pos);
                tickers.push_back(ticker);
            }
        }
    } else {
        std::cerr << "TS section not found in the HTML content." << std::endl;
    }

    return tickers;
}

// Function to check if we are on the last page
bool isLastPage(const std::string& htmlContent) {
    // Regular expression to match the 'selected' option with the page number
    std::regex pageRegex(R"(<option selected="selected" value="\d+">Page \d+ / \d+</option>)");
    std::smatch match;

    // Search for the regex match in the HTML content
    if (std::regex_search(htmlContent, match, pageRegex)) {
        return true; // Stop when the last page is found
    }
    return false;
}

// Function to check if rate limit message is in the HTML content
bool isRateLimited(const std::string& htmlContent) {
    // Regex to match the phrase "rate limited"
    std::regex rateLimitRegex("rate limited", std::regex::icase);
    return std::regex_search(htmlContent, rateLimitRegex);
}

int main() {
    int r_value = 1; // Starting value for r
    const int max_tick_count = 20; // Minimum number of tickers to exit the loop

    // Create the filename with the format watchlist_YYYYmmddss.txt
    std::string filename = "watchlist.txt";

    // Open watchlist.txt file in append mode
    std::ofstream watchlistFile(filename, std::ios::app);
    if (!watchlistFile) {
        std::cerr << "Error opening file watchlist.txt for writing." << std::endl;
        return 1;
    }

    while (true) {
        // Create the URL with the updated r value
        std::string url = "https://finviz.com/screener.ashx?v=111&f=sh_opt_option&r=" + std::to_string(r_value);

        // Fetch HTML content
        std::string htmlContent = fetchHTML(url);

        // Check if rate limit message is in the response
        if (isRateLimited(htmlContent)) {
            std::cerr << "Rate limit reached. Exiting program." << std::endl;
            watchlistFile.close(); // Close the file before exiting
            return 1; // Exit the program
        }

        // Extract tickers
        std::vector<std::string> tickers = extractTickers(htmlContent);

        // Print extracted tickers
        std::cout << "Extracted Tickers (" << r_value << "):" << std::endl;
        for (const std::string& ticker : tickers) {
            std::cout << ticker << std::endl;
            // Write each ticker to the file
            watchlistFile << ticker << std::endl;
        }

        // Check if we have reached the last page and exit the loop
        if (isLastPage(htmlContent) || tickers.size() == 1) {
            std::cout << "Last page found, exiting." << std::endl;
            break;
        }

        // Increment r_value by 20 for the next iteration
        r_value += 20;

        // Sleep for 0.5 seconds before the next request
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    return 0;
}
