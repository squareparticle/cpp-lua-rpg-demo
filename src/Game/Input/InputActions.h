#pragma once

#include <string_view>

namespace InputActions
{
inline constexpr std::string_view MoveLeft = "moveLeft";
inline constexpr std::string_view MoveRight = "moveRight";
inline constexpr std::string_view MoveUp = "moveUp";
inline constexpr std::string_view MoveDown = "moveDown";
inline constexpr std::string_view NavigateUp = "navigateUp";
inline constexpr std::string_view NavigateDown = "navigateDown";
inline constexpr std::string_view PrimaryAction = "primaryAction";
inline constexpr std::string_view Menu = "menu";
inline constexpr std::string_view Inventory = "inventory";
inline constexpr std::string_view Confirm = "confirm";
}
