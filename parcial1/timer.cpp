#include "timer.h"

Timer::Timer() : is_running(false) {
    // Constructor vacío, los time_point se inicializan por defecto
}

Timer::~Timer() {
    // Destructor vacío
}

void Timer::start() {
    start_time = std::chrono::high_resolution_clock::now();
    is_running = true;
}

void Timer::stop() {
    if (is_running) {
        end_time = std::chrono::high_resolution_clock::now();
        is_running = false;
    }
}

void Timer::reset() {
    is_running = false;
    // Los time_points se pueden dejar como están, serán sobrescritos en start()
}

double Timer::getElapsedMilliseconds() const {
    if (is_running) {
        // Si está corriendo, calcular desde start_time hasta ahora
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time);
        return duration.count() / 1000.0;
    } else {
        // Si no está corriendo, usar end_time
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
}

double Timer::getElapsedSeconds() const {
    return getElapsedMilliseconds() / 1000.0;
}

long long Timer::getElapsedMicroseconds() const {
    if (is_running) {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time);
        return duration.count();
    } else {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count();
    }
}

bool Timer::isRunning() const {
    return is_running;
}

double Timer::getCurrentElapsedMilliseconds() const {
    if (is_running) {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time);
        return duration.count() / 1000.0;
    }
    return 0.0;
}

double Timer::getCurrentElapsedSeconds() const {
    return getCurrentElapsedMilliseconds() / 1000.0;
}