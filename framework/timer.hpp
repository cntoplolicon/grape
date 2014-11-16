#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdexcept>
#include <cmath>

template <typename T>
T clamp(T value, T minValue, T maxValue)
{
    value = value < minValue ? minValue : value;
    value = value > maxValue ? maxValue : value;
    return value;
}

class Timer
{
public:
    enum Type
    {
        TT_LOOP,
        TT_SINGLE,
        TT_INFINITE,

        NUM_TIMER_TYPES,
    };


    Timer(Type eType = TT_INFINITE, float fDuration = 1.0f);

    void Reset();

    bool TogglePause();

    void SetPause(bool pause = true);

    bool IsPaused() const;

    bool Update(float elapsedTime);

    void Rewind(float secRewind);

    void Fastforward(float secFF);

    float GetAlpha() const;

    float GetProgression() const;

    float GetTimeSinceStart() const;

    float GetDuration() const {return m_secDuration;}

private:
    Type m_eType;
    float m_secDuration;

    bool m_isPaused;

    float m_secAccumTime;
};

Timer::Timer( Type eType, float fDuration )
    : m_eType(eType)
    , m_secDuration(fDuration)
    , m_isPaused(false)
      , m_secAccumTime(0.0f)
{
    if(m_eType != TT_INFINITE) {
        std::invalid_argument("duration must be positive");
    }
}

void Timer::Reset()
{
    m_secAccumTime = 0.0f;
}

bool Timer::TogglePause()
{
    m_isPaused = !m_isPaused;
    return m_isPaused;
}

bool Timer::IsPaused() const
{
    return m_isPaused;
}

void Timer::SetPause(bool pause)
{
    m_isPaused = pause;
}

bool Timer::Update(float deltaTime)
{
    if (m_isPaused) {
        return false;
    }

    m_secAccumTime += deltaTime;
    if(m_eType == TT_SINGLE) {
        return m_secAccumTime > m_secDuration;
    }

    return false;
}

void Timer::Rewind(float secRewind)
{
    m_secAccumTime -= secRewind;
    if(m_secAccumTime < 0.0f) {
        m_secAccumTime = 0.0f;
    }
}

void Timer::Fastforward( float secFF )
{
    m_secAccumTime += secFF;
}

float Timer::GetAlpha() const
{
    switch(m_eType)
    {
        case TT_LOOP:
            return fmod(m_secAccumTime, m_secDuration) / m_secDuration;
        case TT_SINGLE:
            return clamp(m_secAccumTime / m_secDuration, 0.0f, 1.0f);
        default:
            break;
    }
    return -1.0f;
}

float Timer::GetProgression() const
{
    switch(m_eType)
    {
        case TT_LOOP:
            return fmod(m_secAccumTime, m_secDuration);
        case TT_SINGLE:
            return clamp(m_secAccumTime, 0.0f, m_secDuration);
        default:
            break;
    }

    return -1.0f;
}

float Timer::GetTimeSinceStart() const
{
    return m_secAccumTime;
}

#endif
