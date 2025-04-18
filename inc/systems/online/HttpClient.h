//
// Created by orgor on 16/04/2025.
//
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#define BASE_URL "localhost"
#define PORT 5187

#include <string>
#include <map>
#include <cpr/cpr.h>

class HttpClient {
public:
    using Headers = std::map<std::string, std::string>;
    HttpClient();
    bool login(const std::string& username, const std::string& password);
    bool registerUser(const std::string& username, const std::string& password);
    bool queuePlayer();
    bool unqueuePlayer();
    std::string getMatchStatus();
    bool registerServerMatchmaking(const std::string& ip);
    bool unregisterServerMatchmaking(const std::string& ip);

private:
    std::string get(const std::string& url, const Headers& headers = {});
    std::string post(const std::string& url, const std::string& body, const Headers& headers = {});

    std::string _baseUrl = BASE_URL;
    std::string _token;

    [[nodiscard]] Headers authorizedHeader() const {
        return {
                        {"Authorization", "Bearer " + _token},
                        {"Content-Type", "application/json"}
        };
    }
};

#endif //HTTPCLIENT_H
