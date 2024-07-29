#include "Timer.h"

Timer::Timer(const std::string& name, bool enable_print)
    : m_name(name), m_enable_print(enable_print),
    m_is_stopped(false),
    m_start(std::chrono::steady_clock::now())
{}

Timer::~Timer()
{
    stop();

    if (!m_enable_print)
        return;

    printf("[Timer]: %s took %lu ms\n", m_name.c_str(), get_duration_ms());
}

void Timer::stop()
{
    if (m_is_stopped)
        return;

    m_end = std::chrono::steady_clock::now();
    m_is_stopped = true;
}

std::size_t Timer::get_duration_ms() const
{
    auto end = m_is_stopped ? m_end : std::chrono::steady_clock::now();
    auto duration = end - m_start;

    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}