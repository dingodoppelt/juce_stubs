#pragma once

#include "../juce_compat.h"

namespace juce
{
    class Timer
    {
    public:
        Timer() = default;
        virtual ~Timer() = default;

        void startTimer(int intervalMs)
        {
            intervalMs_ = intervalMs;
            running_ = true;
        }

        void stopTimer()
        {
            running_ = false;
            intervalMs_ = 0;
        }

        bool isTimerRunning() const noexcept
        {
            return running_;
        }

        virtual void timerCallback() {}

    private:
        int intervalMs_ = 0;
        bool running_ = false;
    };
}
