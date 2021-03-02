#ifndef BASE_HELPER_SQL_H
#define BASE_HELPER_SQL_H

#include <base/detect.h>
#include <base/threadpool.h>

#if defined(CONF_FAMILY_UNIX)
#include <cmath>
#endif

namespace kurosio
{
    #define kpause(microsec) set_pause_function([](){}, microsec) \

    template <typename T>
    void set_timer_detach(T header, int miliseconds) 
    {
        std::thread t([=]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(miliseconds));
            header();
        });
        t.detach();
    }

    template <typename T>
    void set_pause_function(T header, int miliseconds) 
    {
        std::thread t([=]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(miliseconds));
            header();
        });
        t.join();
    }

	inline int computeExperience(int Level)
	{
        if (Level == 1)
            return 18;
        return (int)(24 * pow(Level, 2)) - (24 * Level);
	}
}

#endif