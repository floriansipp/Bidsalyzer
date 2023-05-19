#ifndef ICON_H
#define ICON_H

#include <iostream>
#include "Window.h"

namespace InsermLibrary
{
    class Icon
    {
    public:
        Icon();
        Icon(std::string name, std::string path, Window window, std::string uid);
        ~Icon();

        inline const std::string Name() { return m_name; }
        inline void Name(std::string name) { m_name = name; }
        inline const std::string Path() { return m_path; }
        inline void Path(std::string path) { m_path = path; }
        inline Window DisplayWindow() { return m_window; }
        inline void DisplayWindow(Window window){ m_window = window; }
        inline const std::string GUID() { return m_uid; }

    private:
        std::string m_name = "";
        std::string m_path = "";
        Window m_window;
        std::string m_uid = "";
    };
}

#endif
