#include "Parser.hpp"
#include <cctype>

Command parseCommand(const std::string& raw)
{
    Command result;
    std::string line = raw;
    
    size_t pos = 0;
    while (pos < line.size() && std::isspace(line[pos]))
        ++pos;
    if (pos >= line.size())
        return result;
    line = line.substr(pos);
    
    pos = line.size();
    while (pos > 0 && std::isspace(line[pos - 1]))
        --pos;
    line = line.substr(0, pos);
    
    if (line.empty())
        return result;
    
    if (line[0] == ':')
    {
        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos)
        {
            result.prefix = line.substr(1);
            return result;
        }
        result.prefix = line.substr(1, spacePos - 1);
        line = line.substr(spacePos + 1);
        
        pos = 0;
        while (pos < line.size() && std::isspace(line[pos]))
            ++pos;
        if (pos >= line.size())
            return result;
        line = line.substr(pos);
    }
    
    size_t spacePos = line.find(' ');
    if (spacePos == std::string::npos)
    {
        result.cmd = line;
        for (size_t i = 0; i < result.cmd.size(); ++i)
            result.cmd[i] = std::toupper(result.cmd[i]);
        return result;
    }
    
    result.cmd = line.substr(0, spacePos);
    for (size_t i = 0; i < result.cmd.size(); ++i)
        result.cmd[i] = std::toupper(result.cmd[i]);
    
    line = line.substr(spacePos + 1);
    
    while (!line.empty())
    {
        pos = 0;
        while (pos < line.size() && std::isspace(line[pos]))
            ++pos;

        if (pos >= line.size())
            break;
        line = line.substr(pos);
        
        if (line[0] == ':')
        {
            result.trailing = line.substr(1);
			result.hasTrailing = true;
            break;
        }
        
        spacePos = line.find(' ');
        if (spacePos == std::string::npos)
        {
            result.params.push_back(line);
            break;
        }
        else
        {
            result.params.push_back(line.substr(0, spacePos));
            line = line.substr(spacePos + 1);
        }
    }
    
    return result;
}