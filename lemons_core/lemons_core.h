#pragma once

// minimal stub umbrella for DSP builds
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

namespace lemons
{
    struct Version
    {
        int major = 0;
        int minor = 0;
        int patch = 0;

        constexpr Version(int maj = 0, int min = 0, int p = 0) noexcept
            : major(maj), minor(min), patch(p) {}
    };
}
