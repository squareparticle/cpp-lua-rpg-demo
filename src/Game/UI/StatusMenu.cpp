#include "StatusMenu.h"

#include <utility>

void StatusMenu::SetStatus(PlayerStatusMenuData status)
{
    status_ = std::move(status);
}
