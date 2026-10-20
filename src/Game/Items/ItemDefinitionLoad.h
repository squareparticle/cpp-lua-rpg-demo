#pragma once

#include "ItemDefinition.h"

#include <string>

bool LoadItemDefinition(
    const std::string& id,
    const std::string& filename,
    ItemDefinition& item);
