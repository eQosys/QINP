#pragma once

#include <iostream>
#include <string>
#include <chrono>

class Timer
{
public:
    Timer() = delete;
    Timer(const std::string& name, bool enable_print);
    ~Timer();
public:
    void stop();
    std::size_t get_duration_ms() const;
private:
    std::string m_name;
    bool m_enable_print;
    bool m_is_stopped;
    std::chrono::steady_clock::time_point m_start;
    std::chrono::steady_clock::time_point m_end;
};