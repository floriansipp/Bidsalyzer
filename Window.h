#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>

namespace InsermLibrary
{
    class Window
    {
    public:
        Window();
        Window(int start, int end);
        ~Window();

        inline int Start() const { return m_Start; }
        inline void Start(int start) { m_Start = start; }
        inline int End() const { return m_End; }
        inline void End(int end) { m_End = end; }
        inline int Length() const { return m_End - m_Start; }

    private :
        int m_Start = 0;
        int m_End = 0;
    };
}

#endif // WINDOW_H
