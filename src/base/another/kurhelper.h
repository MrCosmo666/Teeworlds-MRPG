#ifndef BASE_HELPER_SQL_H
#define BASE_HELPER_SQL_H

#include <base/threadpool.h>

namespace kurosio
{
    // тоже самое что и set_pause_function только укороченно без написания лямбды
    #define kpause(microsec) set_pause_function([](){}, microsec) \

    // Внимание: осторожно с этим обычно многое не доживает до обращения к этому (так как отдельный поток)
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

    // настраиваем шаблонную функцию чтобы потом использовать lambda
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

    // вывести от числа процент пример ((100, 10%) = 10)
    template <typename T>
    inline float translate_to_procent_rest(T count, T procent) { return (((float)count/(float)100.0f)*(float)procent); }

    // прибавить к числу процент пример ((100, 10%) = 110)
    template <typename T>
    inline float add_procent_to_source(T *count, float procent)
    {
        // выводим кол-во остатка (числа от процента)
 	   	*count = ((float)*count * ((float)1.0f + (procent/100.0f)));
		return (float)(*count);
    }

    // перевести от первого числа второе в проценты пример ((10, 5) = 50%)
    template <typename T>
    inline float translate_to_procent(T count, T count2) { return (((float)count/(float)count2)*(float)100.0f); }
}

#endif