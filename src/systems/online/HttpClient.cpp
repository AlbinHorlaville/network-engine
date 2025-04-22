#include "systems/online/HttpClient.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <httplib.h>

using json = nlohmann::json;

HttpClient::HttpClient() = default;

bool HttpClient::login(const std::string& username, const std::string& password) {
    json payload = {
        {"username", username},
        {"password", password}
    };

    Headers headers = {
        {"Content-Type", "application/json"}
    };

    std::string response = post("/Auth/login", payload.dump(), headers);
    if (!response.empty()) {
        auto json = nlohmann::json::parse(response);
        _token = json["token"].get<std::string>();
        return true;
    }

    return false;
}

bool HttpClient::registerUser(const std::string& username, const std::string& password) {
    json payload = {
        {"username", username},
        {"password", password}
    };

    Headers headers = {
        {"Content-Type", "application/json"}
    };

    std::string response = post("/Auth/register", payload.dump(), headers);
    if (!response.empty()) {
        auto json = nlohmann::json::parse(response);
        _token = json["token"].get<std::string>();
        return true;
    }

    return false;
}

bool HttpClient::queuePlayer() {
    Headers headers = authorizedHeader();  // if token is needed
    std::string response = post("/Matchmaking/queue", "", headers);
    return !response.empty();
}

bool HttpClient::unqueuePlayer() {
    Headers headers = authorizedHeader();  // if token is needed
    std::string response = post("/Matchmaking/unqueue", "", headers);
    return !response.empty();
}

std::string HttpClient::getMatchStatus() {
    Headers headers = authorizedHeader();  // if token is needed
    std::string response = get("/Matchmaking/status", headers);

    if (!response.empty()) {
        nlohmann::json jsonData;
        try {
            jsonData = nlohmann::json::parse(response);
            std::string raw = jsonData["server"];  // "[::]:5555"
            response = raw;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse JSON server IP: " << e.what() << std::endl;
            return {};
        }
    }

    return response;
}

PlayerStats HttpClient::getStatsParsed() {
    Headers headers = authorizedHeader();
    std::string response = get("/Stats", headers);

    PlayerStats stats{0, 0, 0, 0};

    if (response.empty()) {
        std::cerr << "Failed to get stats.\n";
        return stats;
    }

    try {
        auto jsonData = nlohmann::json::parse(response);
        stats.gamesWon = jsonData["gamesWon"].get<int>();
        stats.gamesPlayed = jsonData["gamesPlayed"].get<int>();
        stats.cubesPushed = jsonData["cubesPushed"].get<int>();
        stats.maxCubesPushedInOneGame = jsonData["maxCubesPushedInOneGame"].get<int>();
    } catch (const std::exception& e) {
        std::cerr << "Error parsing stats JSON: " << e.what() << "\n";
    }

    return stats;
}

bool HttpClient::setStats(const std::string& username, PlayerStats stats) {
    json payload = {
        {"gamesWon", stats.gamesWon},
        { "gamesPlayed", stats.gamesPlayed},
        {"cubesPushed", stats.cubesPushed},
        {"maxCubesPushedInOneGame", stats.maxCubesPushedInOneGame}
    };

    Headers headers = authorizedHeader();  // assuming the endpoint requires auth
    std::string url = "/Stats/" + username;

    std::string response = post(url, payload.dump(), headers);
    return !response.empty();
}

bool HttpClient::registerServerMatchmaking(const std::string& ip) {
    Headers headers = authorizedHeader();  // optional
    std::string url = "/Matchmaking/register?ip=" + ip;
    std::string response = post(url, "", headers);
    return !response.empty();
}

bool HttpClient::unregisterServerMatchmaking(const std::string& ip) {
    Headers headers = authorizedHeader();  // optional
    std::string url = "/Matchmaking/unregister?ip=" + ip;
    std::string response = post(url, "", headers);
    return !response.empty();
}

bool HttpClient::removePlayerFromMatch(const std::string& username) {
    Headers headers = authorizedHeader();  // If token-based auth is used
    std::string url = "/Matchmaking/match/" + username;

    std::string response = delete_(url, headers);
    return !response.empty();  // Consider returning true only if deletion succeeded and response is not empty
}

std::string HttpClient::get(const std::string& url, const Headers& headers) {
    httplib::Client cli(_baseUrl, PORT);
    httplib::Headers reqHeaders;
    for (const auto& [key, value] : headers) {
        reqHeaders.emplace(key, value);
    }

    auto res = cli.Get(url.c_str(), reqHeaders);
    if (res && res->status == 200) {
        return res->body;
    }

    std::cerr << "GET request failed. HTTP code: " << (res ? res->status : 0) << "\n";
    return {};
}

std::string HttpClient::post(const std::string& url, const std::string& body, const Headers& headers) {
    httplib::Client cli(_baseUrl, PORT);
    httplib::Headers reqHeaders;
    for (const auto& [key, value] : headers) {
        reqHeaders.emplace(key, value);
    }

    auto res = cli.Post(url.c_str(), reqHeaders, body, "application/json");
    if (res && res->status >= 200 && res->status < 300) {
        return res->body;
    }

    std::cerr << "POST request failed. HTTP code: " << (res ? res->status : 0) << "\n";
    return {};
}

std::string HttpClient::delete_(const std::string& url, const Headers& headers) {
    httplib::Client cli(_baseUrl, PORT);
    httplib::Headers reqHeaders;
    for (const auto& [key, value] : headers) {
        reqHeaders.emplace(key, value);
    }

    auto res = cli.Delete(url.c_str(), reqHeaders);
    if (res && res->status >= 200 && res->status < 300) {
        return res->body;
    }

    std::cerr << "DELETE request failed. HTTP code: " << (res ? res->status : 0) << "\n";
    return {};
}
