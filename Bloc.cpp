#include "Bloc.h"

InsermLibrary::Bloc::Bloc()
{
    m_name = "New Bloc";
    m_order = 0;
    m_illustrationPath = "";
    m_sort = "";
    m_subBlocs = std::vector<SubBloc>();
    //TODO : generate uid 
}

InsermLibrary::Bloc::Bloc(std::string name, int order, std::string illustrationPath, std::string sort, std::vector<SubBloc> subBlocs, std::string uid)
{
    m_name = name;
    m_order = order;
    m_illustrationPath = illustrationPath;
    m_sort = sort;
    m_subBlocs = std::vector<SubBloc>(subBlocs);
    m_uid = uid;
}

InsermLibrary::Bloc::~Bloc()
{

}
