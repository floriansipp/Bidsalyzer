#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include <vector>

namespace InsermLibrary
{
    enum class MainSecondaryEnum { Main, Secondary };

    class Event
    {
    public:
        Event();
        Event(std::string name, std::vector<int> codes, MainSecondaryEnum type, std::string uid);
        ~Event();

        //Getter / Setter
        inline const std::string Name() { return m_name; }
        inline void Name(std::string name) { m_name = name; }
        inline const std::vector<int>& Codes() { return m_codes; }
        inline void Codes(const std::vector<int> codes) { m_codes = std::vector<int>(codes); }
        inline MainSecondaryEnum Type() { return m_type; }
        inline void Type(MainSecondaryEnum type) { m_type = type; }
        inline const std::string GUID() { return m_uid; }

        bool operator==(const Event& other) const
        {
            if(m_codes.size() != other.m_codes.size()) return false;
            for(int i = 0; i < m_codes.size(); i++)
            {
                if(m_codes[i] != other.m_codes[i]) return false;
            }

            return m_name == other.m_name && m_type == other.m_type;
        }
        bool operator!=(const Event& other) const
        {
            return !(*this == other);
        }

        //Functions
        //Get string from code array.
        //Get code array from string

    private :
        std::string m_name = "";
        std::vector<int> m_codes;
        MainSecondaryEnum m_type;
        std::string m_uid = "";
    };
}


#endif // EVENT_H
