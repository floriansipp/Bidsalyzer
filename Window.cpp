#include "Window.h"

InsermLibrary::Window::Window()
{
    m_Start = -300;
    m_End = 300;
}

InsermLibrary::Window::Window(int start, int end)
{
    m_Start = start;
    m_End = end;
}

InsermLibrary::Window::~Window()
{

}
