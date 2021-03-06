#include "util.h"
#include "args.h"
#include "generator_handler.h"
#include "logging.h"
#include <algorithm>


inline bool ArgEqual(const char* name, const char* shortHand, const char* arg)
{
    return (strncmp(name, arg, sizeof(const char*)) == 0 || strncmp(shortHand, arg, sizeof(const char*)) == 0);
}


// stupid
inline bool ArgParser::CheckOtherArg(int i)
{
    return (std::string(argv[i]).substr(0, 1) == "-");
}


bool ArgParser::CheckParam(const char* name, const char* shortHand)
{
    for (int i = 0; i < argc; i++)
    {
        if (ArgEqual(name, shortHand, argv[i]))
            return true;
    }

    return false;
}


const char* ArgParser::GetParamValueStr(const char* name, const char* shortHand, const char* defaultValue)
{
    for (int i = 0; i < argc; i++)
    {
        if (!ArgEqual(name, shortHand, argv[i]))
            continue;

        if (i + 1 < argc)
            return argv[i + 1];

        break;
    }

    return defaultValue;
}


std::vector<std::string> ArgParser::GetParamList(const char* name, const char* shortHand, std::vector<std::string> defaultValue, std::vector<std::string> choices)
{
    for (int i = 0; i < argc; i++)
    {
        if (!ArgEqual(name, shortHand, argv[i]))
            continue;

        std::vector<std::string> paramList;

        for (i++; i < argc; ++i)
        {
            if (CheckOtherArg(i))
                break;

            if (choices.empty() || vec_contains<std::string>(choices, argv[i]))
                paramList.push_back(argv[i]);
            else
                warning("Invalid choice: \"%s\"", argv[i]);
        }

        return paramList;
    }

    return defaultValue;
}




template <class T>
std::vector<T> ArgParser::GetParamList(const char* name, const char* shortHand, EnumParamConvertFunc func, std::vector<T> defaultValue)
{
    for (int i = 0; i < argc; i++)
    {
        if (!ArgEqual(name, shortHand, argv[i]))
            continue;

        std::vector<T> paramList;

        for (; i + 1 < argc; ++i)
        {
            // probably another arg
            std::string itemStr = argv[i + 1]; 
            if (itemStr.substr(0, 1) == "-")
                break;

            int item = func(itemStr.c_str());
            if (item == -1)
                continue;

            paramList.push_back( (T)item );
        }

        return paramList;
    }

    return defaultValue;
}


StringUMap ArgParser::GetParamStringMap(const char* name, const char* shortHand)
{
    for (int i = 0; i < argc; i++)
    {
        if (!ArgEqual(name, shortHand, argv[i]))
            continue;

        StringUMap paramMap;

        for (i++; i < argc; i++)
        {
            std::string key = argv[i];
            std::string value = "1";

            // probably another arg
            if (key.substr(0, 1) == "-")
                break;

            if (key.find("=") != std::string::npos)
            {
                int pos = key.find("=");

                if (pos == key.length() - 1)
                    warning("Command Line Macro \"%s\" has trailing \"=\", setting to 1", key.c_str());
                else
                    value = key.substr(pos + 1, key.length() - pos);

                key = key.substr(0, pos);
            }

            paramMap[key] = value;
        }

        return paramMap;
    }

    return {};
}



// shit
int StrToPlatformArgs(std::string item)
{
    std::transform(item.begin(), item.end(), item.begin(), ::toupper);
    return (int)StrToPlatform(item);
}

int StrToArchArgs(std::string item)
{
    std::transform(item.begin(), item.end(), item.begin(), ::toupper);
    return (int)StrToArch(item);
}



void ArgParser::ParseArgs(int argc, const char** argv)
{
	this->argc = argc;
	this->argv = argv;

    Args& args = GetArgs();

	args.rootDir = GetParamValueStr("--root-dir", "-R", GetCurrentDir().c_str());
	args.baseFile = GetParamValueStr("--base-file", "-b");
	args.masterFile = GetParamValueStr("--master-file", "-m");

	args.verbose = CheckParam("--verbose", "-v");
	args.force = CheckParam("--force", "-f");
	args.forceMaster = CheckParam("--force-master", "-fm");
	args.hideWarnings = CheckParam("--hide-warnings", "-w");

    args.add = GetParamList("--add", "-a");
    args.addDepend = GetParamList("--add-depend", "-ad");
    args.addTree = GetParamList("--add-tree", "-at");

    args.remove = GetParamList("--remove", "-r");
    args.configs = GetParamList("--configs", "-c", {"Debug", "Release"});
    args.generators = GetParamList("--generators", "-g",
                                   GetGeneratorHandler().GetArgNames(),
                                   GetGeneratorHandler().GetArgNames());

    args.platforms = GetParamList<Platform>("--platforms", "-p", &StrToPlatformArgs, {GetSysPlatform()});
    args.archs = GetParamList<Arch>("--archs", "-ar", &StrToArchArgs, {GetSysArch()});

    args.macros = GetParamStringMap("--macros", "-D");
}

