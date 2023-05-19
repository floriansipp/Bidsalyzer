#include "Icon.h"

InsermLibrary::Icon::Icon()
{
	m_name = "New Icon";
	m_path = "";
	m_window = Window(-300, 300);
	//TODO : generate uid 
}

InsermLibrary::Icon::Icon(std::string name, std::string path, Window window, std::string uid)
{
	m_name = name;
	m_path = path;
	m_window = Window(window);
	m_uid = uid;
}

InsermLibrary::Icon::~Icon()
{

}