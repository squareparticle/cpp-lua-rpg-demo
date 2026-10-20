#include "DebugLog.h"

#include <windows.h>

#include <string>

void WriteDebugMessage(std::string_view message)
{
    std::string line(message);
    line.push_back('\n');
    OutputDebugStringA(line.c_str());
}
