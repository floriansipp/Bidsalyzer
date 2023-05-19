#include "ProvFile.h"

InsermLibrary::ProvFile::ProvFile()
{
    m_name = "New Protocol";
}

InsermLibrary::ProvFile::ProvFile(const std::string& filePath)
{
    try
    {
        m_filePath = std::string(filePath);
        Load(filePath);
    }
    catch (std::exception& e)
    {
        std::cerr << "[Exception Caught] : " + std::string(e.what()) << std::endl;
    }
}

InsermLibrary::ProvFile::~ProvFile()
{

}

void InsermLibrary::ProvFile::Load(const std::string& filePath)
{
    std::ifstream jsonFile(filePath);
    if (jsonFile.is_open())
    {
        std::stringstream buffer;
        buffer << jsonFile.rdbuf();
        jsonFile.close();

        m_id = nlohmann::json::parse(buffer.str())["ID"].get<std::string>();
        m_name = nlohmann::json::parse(buffer.str())["Name"].get<std::string>();
        nlohmann::json rawData = nlohmann::json::parse(buffer.str())["Blocs"];
        FillProtocolInformations(rawData);
    }
    else
    {
        throw std::runtime_error("ProvFile Load => : Could not open json file");
    }
}

void InsermLibrary::ProvFile::FillProtocolInformations(nlohmann::json jsonObject)
{
    for (nlohmann::json::iterator it = jsonObject.begin(); it != jsonObject.end(); ++it)
    {
        std::vector<SubBloc> subBlocs;
        std::string blocName = "", blocIllustrationPath = "", blocSort = "";
        int blocOrder = 0;

        nlohmann::json jsonArea = it.value();
        if (!(jsonArea["Name"].is_null())){ blocName = jsonArea["Name"].get<std::string>(); }
        if (!(jsonArea["IllustrationPath"].is_null())){ blocIllustrationPath = jsonArea["IllustrationPath"].get<std::string>(); }
        if (!(jsonArea["Order"].is_null())){ blocOrder = jsonArea["Order"].get<int>(); }
        if (!(jsonArea["Sort"].is_null())){ blocSort = jsonArea["Sort"].get<std::string>(); }
        if (!(jsonArea["SubBlocs"].is_null()))
        {
            for (nlohmann::json::iterator it2 = jsonArea["SubBlocs"].begin(); it2 != jsonArea["SubBlocs"].end(); ++it2)
            {
                nlohmann::json jsonArea2 = it2.value();
                std::string subBlocName = jsonArea2["Name"].get<std::string>();
                int subBlocOrder = jsonArea2["Order"].get<int>();
                MainSecondaryEnum type = (MainSecondaryEnum)jsonArea2["Type"].get<int>();
                //==
                int begWindow = jsonArea2["Window"]["Start"].get<int>();
                int endWindow = jsonArea2["Window"]["End"].get<int>();
                Window subBlocWIndow(begWindow, endWindow);
                //==
                begWindow = jsonArea2["Baseline"]["Start"].get<int>();
                endWindow = jsonArea2["Baseline"]["End"].get<int>();
                Window baseLineWindow(begWindow, endWindow);

                std::vector<Event> events;
                for (nlohmann::json::iterator it3 = jsonArea2["Events"].begin(); it3 != jsonArea2["Events"].end(); ++it3)
                {
                    nlohmann::json jsonArea3 = it3.value();
                    std::string name = jsonArea3["Name"].get<std::string>();
                    int codeCount = jsonArea3["Codes"].size();
                    std::vector<int> codes;
                    for(int i = 0; i < codeCount; i++)
                    {
                        codes.push_back(jsonArea3["Codes"][i].get<int>());
                    }
                    MainSecondaryEnum type = (MainSecondaryEnum)jsonArea3["Type"].get<int>();

                    std::string uid = "";
                    if (!(jsonArea3["ID"].is_null()))
                    {
                        uid = jsonArea3["ID"].get<std::string>();
                    }
                    events.push_back(Event(name, codes, type, uid));
                }

                std::vector<Icon> icons;
                for (nlohmann::json::iterator it3 = jsonArea2["Icons"].begin(); it3 != jsonArea2["Icons"].end(); ++it3)
                {
                    nlohmann::json jsonArea3 = it3.value();
                    std::string imagePath = jsonArea3["ImagePath"].get<std::string>();
                    std::string imageName = jsonArea3["Name"].get<std::string>();
                    int begWindow = jsonArea3["Window"]["Start"].get<int>();
                    int endWindow = jsonArea3["Window"]["End"].get<int>();
                    std::string uid = "";
                    if (!(jsonArea3["ID"].is_null()))
                    {
                        uid = jsonArea3["ID"].get<std::string>();
                    }
                    icons.push_back(Icon(imageName, imagePath, Window(begWindow, endWindow), uid));
                }

                std::string uid = "";
                if (!(jsonArea2["ID"].is_null()))
                {
                    uid = jsonArea2["ID"].get<std::string>();
                }
                subBlocs.push_back(SubBloc(subBlocName, subBlocOrder, type, subBlocWIndow, baseLineWindow, events, icons, uid));
            }
        }

        std::string uid = "";
        if (!(jsonArea["ID"].is_null()))
        {
            uid = jsonArea["ID"].get<std::string>();
        }
        m_blocs.push_back(Bloc(blocName, blocOrder, blocIllustrationPath, blocSort, subBlocs, uid));
    }

    std::sort(m_blocs.begin(), m_blocs.end(), [](Bloc a, Bloc b) { return (a.Order() < b.Order()); });
}

nlohmann::ordered_json InsermLibrary::ProvFile::GetWritableJsonObject()
{
    nlohmann::ordered_json jsonArea;
    jsonArea["ID"] = m_id;
    jsonArea["Name"] = m_name;
    for (int i = 0; i < m_blocs.size(); i++)
    {
        Bloc bloc = m_blocs[i];

        nlohmann::ordered_json jsonBloc;
        jsonBloc["IllustrationPath"] = bloc.IllustrationPath();
        jsonBloc["Name"] = bloc.Name();
        jsonBloc["Order"] = bloc.Order();
        jsonBloc["Sort"] = bloc.Sort();
        for (int j = 0; j < bloc.SubBlocs().size(); j++)
        {
            SubBloc subBloc = bloc.SubBlocs()[j];

            nlohmann::ordered_json jsonSubBloc;
            jsonSubBloc["Name"] = subBloc.Name();
            jsonSubBloc["Order"] = subBloc.Order();
            jsonSubBloc["Type"] = subBloc.Type();
            jsonSubBloc["Window"] = { { "Start" , subBloc.MainWindow().Start() }, { "End" , subBloc.MainWindow().End() } };
            jsonSubBloc["Baseline"] = { { "Start" , subBloc.Baseline().Start() }, { "End" , subBloc.Baseline().End() } };
            jsonSubBloc["Events"] = nlohmann::json::array();

            nlohmann::ordered_json jsonMainEvent;
            jsonMainEvent["Name"] = subBloc.MainEvent().Name();
            for (int k = 0; k < subBloc.MainEvent().Codes().size(); k++)
            {
                jsonMainEvent["Codes"].push_back(subBloc.MainEvent().Codes()[k]);
            }
            jsonMainEvent["Type"] = subBloc.MainEvent().Type();
            jsonMainEvent["ID"] = subBloc.MainEvent().GUID();

            //==
            jsonSubBloc["Events"].push_back(jsonMainEvent);
            //===

            for (int k = 0; k < subBloc.SecondaryEvents().size(); k++)
            {
                nlohmann::ordered_json jsonSecondaryEvent;
                jsonSecondaryEvent["Name"] = subBloc.SecondaryEvents()[k].Name();
                for (int l = 0; l < subBloc.SecondaryEvents()[k].Codes().size(); l++)
                {
                    jsonSecondaryEvent["Codes"].push_back(subBloc.SecondaryEvents()[k].Codes()[l]);
                }
                jsonSecondaryEvent["Type"] = subBloc.SecondaryEvents()[k].Type();
                jsonSecondaryEvent["ID"] = subBloc.SecondaryEvents()[k].GUID();

                //==
                jsonSubBloc["Events"].push_back(jsonSecondaryEvent);
                //===
            }

            jsonSubBloc["Icons"] = nlohmann::json::array();
            for (int k = 0; k < subBloc.Icons().size(); k++)
            {
                Icon icon = subBloc.Icons()[k];

                nlohmann::ordered_json jsonIcon;
                jsonIcon["ImagePath"] = icon.Path();
                jsonIcon["Name"] = icon.Name();
                jsonIcon["Window"] = { { "Start" , icon.DisplayWindow().Start() }, { "End" , icon.DisplayWindow().End() } };
                jsonIcon["ID"] = icon.GUID();

                //==
                jsonSubBloc["Icons"].push_back(jsonIcon);
                //==
            }

            jsonSubBloc["Treatments"] = nlohmann::json::array();
            jsonSubBloc["ID"] = subBloc.GUID();
            //===
            jsonBloc["SubBlocs"].push_back(jsonSubBloc);
        }
        jsonBloc["ID"] = bloc.GUID();
        //===
        jsonArea["Blocs"].push_back(jsonBloc);
    }

    return jsonArea;
}

void InsermLibrary::ProvFile::SaveAs(const std::string& filePath)
{
    std::string outputFile = (filePath == "") ? m_filePath : filePath;
    std::ofstream jsonFile(outputFile, std::ios::out);
    if (jsonFile.is_open())
    {
        jsonFile << GetWritableJsonObject().dump(2) << std::endl;
        jsonFile.close();
    }
    else
    {
        throw std::runtime_error("ProvFile Save => : Could not open json file");
    }
}
