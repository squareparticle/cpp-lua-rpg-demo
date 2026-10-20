#pragma once

#include <string>

struct ItemDefinition
{
    std::string id;
    std::string name;
    std::string description;
    int value = 0;
    std::string useScript;
};
