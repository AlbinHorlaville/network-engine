//
// Created by Albin Horlaville on 19/04/2025.
//

#ifndef FPSHANDLER_H
#define FPSHANDLER_H

struct FPSHandler {
private:
    std::chrono::steady_clock::time_point _lastFrameTime;
    int _frameCount = 0;
    float _accumulatedTime = 0.0f;
    uint16_t _fps = 0;
public:
    void init() {
        _lastFrameTime = std::chrono::steady_clock::now();
    }

    void update() {
        const auto now = std::chrono::steady_clock::now();
        const float deltaTime = std::chrono::duration<float>(now - _lastFrameTime).count();
        _lastFrameTime = now;

        _accumulatedTime += deltaTime;
        _frameCount++;

        if (_accumulatedTime >= 1.0f) {
            _fps = _frameCount / _accumulatedTime;
            _frameCount = 0;
            _accumulatedTime = 0.0f;
        }
    }
    uint16_t get() const {
        return _fps;
    }
};

#endif //FPSHANDLER_H
