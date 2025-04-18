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
    return response;
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
