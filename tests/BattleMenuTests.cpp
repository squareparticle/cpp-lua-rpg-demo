#include "../src/Game/Input/InputState.h"
#include "../src/Game/UI/BattleMenu.h"

#include <iostream>
#include <vector>

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
    BattleMenu menu;
    menu.Configure(
        BattleStats{"Cecil", 1, 142, 142, 36, 36, 12, 5},
        std::vector<BattleMenuTarget>{
            {"goblinSword", "Goblin A"},
            {"goblinClub", "Goblin B"}});

    InputState down;
    down.navigateDownPressed = true;
    menu.Update(down);
    if (!Expect(menu.ConsumeNavigationFeedback(),
            "Command movement did not request feedback"))
    {
        return 1;
    }

    InputState confirm;
    confirm.confirmPressed = true;
    menu.Update(confirm);
    if (!Expect(menu.GetCommandIndex() == 1,
            "Down did not select the disabled Magic command")
        || !Expect(menu.GetState() == BattleMenuState::Commands,
            "Disabled Magic command opened target selection"))
    {
        return 1;
    }

    InputState up;
    up.navigateUpPressed = true;
    menu.Update(up);
    if (!Expect(menu.ConsumeNavigationFeedback(),
            "Command movement back to Attack did not request feedback"))
    {
        return 1;
    }
    menu.Update(confirm);
    if (!Expect(menu.GetState() == BattleMenuState::Targets,
            "Attack did not open target selection")
        || !Expect(menu.ConsumeSelectionFeedback(),
            "Attack selection did not request feedback"))
    {
        return 1;
    }

    menu.Update(down);
    if (!Expect(menu.ConsumeNavigationFeedback(),
            "Target movement did not request feedback"))
    {
        return 1;
    }
    menu.Update(confirm);
    if (!Expect(menu.GetState() == BattleMenuState::TargetChosen,
            "Target confirmation did not stop at the chosen-target state")
        || !Expect(menu.GetSelectedTargetId() == "goblinClub",
            "Target confirmation preserved the wrong enemy ID")
        || !Expect(menu.ConsumeSelectionFeedback(),
            "Target confirmation did not request feedback"))
    {
        return 1;
    }

    menu.SetBusy();
    menu.Update(confirm);
    if (!Expect(menu.GetState() == BattleMenuState::Busy,
            "Busy battle menu accepted input"))
    {
        return 1;
    }

    menu.ShowVictory(27, 73, {"Rusty Dagger", "Crude Club"});
    if (!Expect(!menu.ConsumeVictoryConfirmed(),
            "Victory was confirmed before Enter was pressed"))
    {
        return 1;
    }
    menu.Update(confirm);
    return Expect(menu.ConsumeVictoryConfirmed(),
            "Enter did not confirm the victory screen")
        && Expect(menu.ConsumeSelectionFeedback(),
            "Victory confirmation did not request feedback")
        && Expect(!menu.ConsumeVictoryConfirmed(),
            "Victory confirmation was not consumed as a one-shot request")
        ? 0
        : 1;
}
