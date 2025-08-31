#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    bool is_running;

public:
    Timer();
    ~Timer();
    
    void start();
    void stop();
    void reset();
    
    // Tiempo en milisegundos
    double getElapsedMilliseconds() const;
    
    // Tiempo en segundos
    double getElapsedSeconds() const;
    
    // Tiempo en microsegundos
    long long getElapsedMicroseconds() const;
    
    // Verificar si el timer está corriendo
    bool isRunning() const;
    
    // Obtener tiempo actual sin detener el timer
    double getCurrentElapsedMilliseconds() const;
    double getCurrentElapsedSeconds() const;
};

#endif