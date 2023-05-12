#include "SubBloc.h"

InsermLibrary::SubBloc::SubBloc()
{
    m_name = "New SubBloc";
    m_order = 0;
    m_window = Window(-300,300);
    m_baseline = Window(-300,0);
    m_type = MainSecondaryEnum::Main;
    m_events = std::vector<Event>();
    m_icons = std::vector<Icon>();
    //TODO : generate uid 
}

InsermLibrary::SubBloc::SubBloc(std::string name, int order, MainSecondaryEnum type, Window window, Window baseline, std::vector<Event> events, std::vector<Icon> icons, std::string uid)
{
    m_name = name;
    m_order = order;
    m_window = window;
    m_baseline = baseline;
    m_type = type;
    m_events = events;
    m_icons = icons;
    m_uid = uid;
}

InsermLibrary::SubBloc::~SubBloc()
{

}
