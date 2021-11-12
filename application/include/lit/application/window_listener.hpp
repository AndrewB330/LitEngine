#pragma once

#ifdef _MSC_VER
#include <SDL_config_winrt.h>
#include <SDL_events.h>
#elif defined(__MINGW32__)
#include <SDL_events.h>
#endif

namespace lit::application {

    class WindowListener {
    public:
        WindowListener() = default;

        virtual ~WindowListener() = default;

        virtual bool ProcessEvent(const SDL_Event &event) = 0;

        virtual void StartFrameEvent() = 0;
    };

}
