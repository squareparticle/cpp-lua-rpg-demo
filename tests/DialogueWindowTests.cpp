#include "../src/Game/Input/InputState.h"
#include "../src/Game/UI/DialogueWindow.h"

#include <iostream>
#include <string>

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

bool TestDialogueStateMachine()
{
    DialogueWindow dialogue;
    if (!Expect(dialogue.Show("A short message", {}, false),
            "Dialogue did not open")
        || !Expect(dialogue.IsModalActive(), "Dialogue was not modal")
        || !Expect(dialogue.GetState() == DialogueWindowState::Typing,
            "Instant-open dialogue did not begin typing")
        || !Expect(dialogue.ConsumePageShownFeedback(),
            "Opening dialogue did not request page feedback")
        || !Expect(!dialogue.ConsumePageShownFeedback(),
            "Dialogue page feedback was not consumed"))
    {
        return false;
    }

    dialogue.Update(0.1F, {});
    if (!Expect(dialogue.GetVisibleCharacterCount() > 0,
            "Typewriter did not reveal characters")
        || !Expect(
            dialogue.GetVisibleCharacterCount()
                < dialogue.GetCharacterCount(),
            "Typewriter revealed the whole message too soon"))
    {
        return false;
    }

    InputState confirm;
    confirm.primaryActionPressed = true;
    dialogue.Update(0.0F, confirm);
    if (!Expect(dialogue.GetState() == DialogueWindowState::Waiting,
            "First confirm did not finish typing")
        || !Expect(
            dialogue.GetVisibleCharacterCount()
                == dialogue.GetCharacterCount(),
            "First confirm did not reveal all text"))
    {
        return false;
    }

    dialogue.Update(0.0F, confirm);
    if (!Expect(dialogue.GetState() == DialogueWindowState::Advancing,
            "Second confirm did not enter the advancing state")
        || !Expect(dialogue.ConsumeAdvanceRequest(),
            "Advanced dialogue did not request Lua resumption")
        || !Expect(!dialogue.ConsumeAdvanceRequest(),
            "Dialogue advance request was not consumed"))
    {
        return false;
    }

    return Expect(dialogue.Show("The next page"),
            "Consecutive dialogue page was rejected")
        && Expect(dialogue.GetState() == DialogueWindowState::Typing,
            "Consecutive page replayed the opening animation")
        && Expect(dialogue.ConsumePageShownFeedback(),
            "Consecutive page did not request page feedback");
}

bool TestOpenAnimation()
{
    DialogueWindow dialogue;
    if (!dialogue.Show("Animated", "captainPortrait"))
    {
        return false;
    }
    dialogue.Update(0.05F, {});
    if (!Expect(dialogue.GetState() == DialogueWindowState::Opening,
            "Opening animation ended too early"))
    {
        return false;
    }
    dialogue.Update(1.0F, {});
    return Expect(dialogue.GetState() == DialogueWindowState::Typing,
        "Opening animation did not reach typing");
}

bool TestConfigurableTypewriterSpeed()
{
    DialogueWindow dialogue(10.0F);
    if (!dialogue.Show(std::string(100, 'A'), {}, false))
    {
        return false;
    }
    dialogue.Update(0.5F, {});
    return Expect(dialogue.GetVisibleCharacterCount() == 5,
        "Dialogue did not use its configured reveal speed");
}
}

int main()
{
    return TestDialogueStateMachine()
        && TestOpenAnimation()
        && TestConfigurableTypewriterSpeed() ? 0 : 1;
}
