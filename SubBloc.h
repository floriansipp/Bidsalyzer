#ifndef SUBBLOC_H
#define SUBBLOC_H

#include <iostream>
#include "Window.h"
#include "Event.h"
#include "Icon.h"
#include <algorithm>

namespace InsermLibrary
{
    class SubBloc
    {
    public:
        SubBloc();
        SubBloc(std::string name, int order, MainSecondaryEnum type, Window window, Window baseline, std::vector<Event> events, std::vector<Icon> icons, std::string uid);
        ~SubBloc();

        inline const std::string Name() { return m_name; }
        inline void Name(std::string name) { m_name = name; }
        inline int Order() { return m_order; }
        inline void Order(int order) { m_order = order; }
        inline MainSecondaryEnum Type() { return m_type; }
        inline void Type(MainSecondaryEnum type){ m_type = type; }
        inline Window MainWindow() { return m_window; }
        inline void MainWindow(Window window){ m_window = window; }
        inline Window Baseline() { return m_baseline; }
        inline void Baseline(Window baseline){ m_baseline = baseline; }
        inline Event MainEvent()
        {
            auto it = std::find_if(m_events.begin(), m_events.end(), [&](Event a){ return a.Type() == MainSecondaryEnum::Main; });
            return it != m_events.end() ? *it : Event();
        }
        inline std::vector<Event> SecondaryEvents()
        {
            std::vector<Event> secondaryEvents;
            auto it = m_events.begin();
            while ((it = std::find_if(it, m_events.end(), [&](Event a){ return a.Type() == MainSecondaryEnum::Secondary; })) != m_events.end())
            {
                secondaryEvents.push_back(*it);
                it++;
            }
            return secondaryEvents;
        }
        inline std::vector<Event>& Events() { return m_events; }
        inline std::vector<Icon>& Icons() { return m_icons; }
        inline const std::string GUID() { return m_uid; }

    private :
        std::string m_name;
        int m_order = 0;
        MainSecondaryEnum m_type;
        Window m_window, m_baseline;
        std::vector<Event> m_events;
        std::vector<Icon> m_icons;
        std::string m_uid = "";
    };
}
#endif // SUBBLOC_H
