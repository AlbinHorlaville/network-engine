//
// Created by User on 19/04/2025.
//

#ifndef PINGHANDLER_H
#define PINGHANDLER_H

struct PingHandler {
private:
    std::chrono::steady_clock::time_point _lastFrameTime;
    std::uint64_t _pingSum = 0;
    int _nbPings = 0;
    float _accumulatedTime = 0.0f;
    uint8_t _pingAverage = 0.0f;
public:
    void init() {
        _lastFrameTime = std::chrono::steady_clock::now();
    }

    void update(std::uint64_t ping, std::uint64_t pong) {
        auto now = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - _lastFrameTime).count();
        _lastFrameTime = now;

        _accumulatedTime += deltaTime;
        _pingSum += pong - ping;
        _nbPings++;

        if (_accumulatedTime >= 1.0f) {
            _pingAverage = _pingSum / _nbPings;
            _pingSum = 0;
            _nbPings = 0;
            _accumulatedTime = 0.0f;
        }
    }
    uint8_t get() const {
        return _pingAverage;
    }
};

#endif //PINGHANDLER_H
