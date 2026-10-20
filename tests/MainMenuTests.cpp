#include "../src/Game/State/Inventory.h"
#include "../src/Game/Items/ItemDefinition.h"
#include "../src/Game/State/PlayerState.h"
#include "../src/Game/Input/InputState.h"
#include "../src/Game/UI/MainMenu.h"

#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

namespace
{
bool Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}
}

int main()
{
    Inventory inventory;
    inventory.AddItem("club");
    inventory.AddItem("key");
    PlayerState player;
    if (!player.Initialize("Cecil", 1, 27, 142, 36, 12, 5, 9))
    {
        return 1;
    }
    const std::unordered_map<int, int> experience{{1, 100}};
    const std::unordered_map<std::string, ItemDefinition> items{
        {"club", {"club", "Club", "A rough weapon.", 4}},
        {"key", {"key", "Key", "Opens a door.", 0, "use_key.lua"}}};
    MainMenu menu(inventory, player, experience, items, "pointer");
    menu.SetSaveEnabled(true);
    menu.SetLoadEnabled(true);

    if (!Expect(menu.OpenInventory(false),
            "Inventory shortcut could not open InventoryMenu")
        || !Expect(menu.IsOpen()
                && menu.GetPage() == MainMenuPage::Inventory,
            "Inventory shortcut opened the wrong page")
        || !Expect(menu.ConsumeOpenFeedback(),
            "Inventory shortcut did not request open feedback")
        || !Expect(!menu.ConsumeOpenFeedback(),
            "Menu open feedback was not consumed"))
    {
        return 1;
    }
    menu.Close();
    if (!Expect(menu.ConsumeCloseFeedback(),
            "Closing InventoryMenu did not request close feedback")
        || !Expect(!menu.ConsumeCloseFeedback(),
            "Menu close feedback was not consumed"))
    {
        return 1;
    }
    menu.Open(false);
    if (!Expect(menu.ConsumeOpenFeedback(),
            "Opening MainMenu did not request open feedback"))
    {
        return 1;
    }

    InputState confirm;
    confirm.confirmPressed = true;
    menu.Update(0.0F, confirm);
    if (!Expect(menu.ConsumeSelectionFeedback(),
            "Accepted Save selection did not request feedback")
        || !Expect(!menu.ConsumeSelectionFeedback(),
            "Selection feedback was not consumed")
        || !Expect(menu.ConsumeSaveRequest(),
            "Save did not produce a one-shot request")
        || !Expect(menu.IsOpen(), "Save unexpectedly closed MainMenu")
        || !Expect(!menu.ConsumeSaveRequest(),
            "Save request was not consumed"))
    {
        return 1;
    }

    InputState down;
    down.navigateDownPressed = true;
    menu.Update(0.0F, down);
    if (!Expect(menu.ConsumeNavigationFeedback(),
            "Root-menu movement did not request feedback")
        || !Expect(!menu.ConsumeNavigationFeedback(),
            "Navigation feedback was not consumed"))
    {
        return 1;
    }
    menu.Update(0.0F, confirm);
    if (!Expect(menu.ConsumeLoadRequest(),
            "Load did not produce a one-shot request")
        || !Expect(!menu.IsOpen(), "Load did not close MainMenu")
        || !Expect(!menu.ConsumeLoadRequest(),
            "Load request was not consumed"))
    {
        return 1;
    }

    menu.SetSaveEnabled(false);
    menu.SetLoadEnabled(false);
    menu.Open(false);

    menu.Update(0.0F, down);
    menu.Update(0.0F, down);
    menu.Update(0.0F, confirm);
    if (!Expect(menu.GetPage() == MainMenuPage::Inventory,
            "Inventory option did not open InventoryMenu"))
    {
        return 1;
    }
    menu.Update(1.0F, {});
    menu.Update(0.0F, down);
    if (!Expect(menu.GetInventorySelectedIndex() == 1,
            "InventoryMenu did not move its own selection")
        || !Expect(menu.ConsumeNavigationFeedback(),
            "Inventory movement did not request feedback"))
    {
        return 1;
    }
    menu.Update(0.0F, confirm);
    if (!Expect(menu.ConsumeSelectionFeedback(),
            "Usable-item selection did not request feedback"))
    {
        return 1;
    }
    const std::optional<std::string> itemUse =
        menu.ConsumeItemUseRequest();
    if (!Expect(itemUse.has_value() && *itemUse == "key",
            "Usable inventory item did not produce a use request")
        || !Expect(!menu.ConsumeItemUseRequest().has_value(),
            "Inventory use request was not consumed"))
    {
        return 1;
    }

    InputState escape;
    escape.menuPressed = true;
    menu.Update(0.0F, escape);
    if (!Expect(menu.GetPage() == MainMenuPage::Root && menu.IsOpen(),
            "Escape did not return from InventoryMenu to MainMenu"))
    {
        return 1;
    }

    menu.Update(0.0F, down);
    menu.Update(0.0F, confirm);
    if (!Expect(menu.GetPage() == MainMenuPage::Status,
            "Status option did not open StatusMenu"))
    {
        return 1;
    }
    menu.Update(1.0F, {});
    menu.Update(0.0F, escape);
    menu.Update(0.0F, down);
    menu.Update(0.0F, confirm);
    return Expect(menu.ConsumeQuitRequest(),
            "Quit did not produce a one-shot request")
        && Expect(!menu.IsOpen(), "Quit did not close MainMenu")
        && Expect(menu.ConsumeCloseFeedback(),
            "Quit did not request close feedback")
        && Expect(!menu.ConsumeQuitRequest(),
            "Quit request was not consumed")
        ? 0
        : 1;
}
