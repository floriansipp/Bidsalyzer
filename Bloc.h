#ifndef BLOC_H
#define BLOC_H

#include <iostream>
#include "SubBloc.h"

namespace InsermLibrary
{
    class Bloc
    {
    public:
        Bloc();
        Bloc(std::string name, int order, std::string illustrationPath, std::string sort, std::vector<SubBloc> subBlocs, std::string uid);
        ~Bloc();

        //Getter
        inline const std::string Name() { return m_name; }
        inline int Order() { return m_order; }
        inline const std::string IllustrationPath() { return m_illustrationPath; }
        inline const std::string Sort() { return m_sort; }
        inline std::vector<SubBloc>& SubBlocs() { return m_subBlocs; }
        inline SubBloc& MainSubBloc()
        {
            for (int i = 0; i < m_subBlocs.size(); i++)
            {
                if (m_subBlocs[i].Type() == MainSecondaryEnum::Main)
                {
                    return m_subBlocs[i];
                }
            }
            return m_subBlocs[0]; //that's ugly as shit, it should never arrive at this point
        }
        inline const std::string GUID() { return m_uid; }
        // Setter
        inline void Name(std::string name) { m_name = name; }
        inline void Order(int order) { m_order = order; }
        inline void IllustrationPath(std::string illustrationPath) { m_illustrationPath = illustrationPath; }
        inline void Sort(std::string sort) { m_sort = sort; }
        inline void SubBlocs(std::vector<SubBloc> subBlocs) { m_subBlocs = std::vector<SubBloc>(subBlocs); }

    private:
        std::string m_name = "";
        int m_order = 0;
        std::string m_illustrationPath = "";
        std::string m_sort = "";
        std::vector<SubBloc> m_subBlocs;
        std::string m_uid = "";
    };
}
#endif // BLOC_H
