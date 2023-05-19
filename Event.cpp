#include "Event.h"

InsermLibrary::Event::Event()
{
    m_name = "New Event";
    m_codes = std::vector<int>();
    m_type = MainSecondaryEnum::Main;
    //TODO : generate uid 
}

InsermLibrary::Event::Event(std::string name, std::vector<int> codes, MainSecondaryEnum type, std::string uid)
{
    m_name = name;
    m_codes = std::vector<int>(codes);
    m_type = type;
    m_uid = uid;
}

InsermLibrary::Event::~Event()
{

}
