#pragma once

#include <cstddef>
#include <string>
#include <string_view>

class Graphics;
class ResourceManager;
struct InputState;

enum class DialogueWindowState
{
    Closed,
    Opening,
    Typing,
    Waiting,
    Advancing
};

class DialogueWindow
{
public:
    explicit DialogueWindow(float charactersPerSecond = 42.0F) noexcept;

    bool Show(
        std::string_view text,
        std::string_view portraitImageId = {},
        bool animateOpen = true);
    void Update(float deltaTime, const InputState& input);
    void Render(
        const Graphics& graphics,
        const ResourceManager& resources) const;
    void Close() noexcept;

    bool IsModalActive() const noexcept;
    bool ConsumeAdvanceRequest() noexcept;
    bool ConsumePageShownFeedback() noexcept;
    DialogueWindowState GetState() const noexcept;
    std::size_t GetVisibleCharacterCount() const noexcept;
    std::size_t GetCharacterCount() const noexcept;

private:
    static std::size_t CountUtf8Characters(
        std::string_view text) noexcept;

    DialogueWindowState state_ = DialogueWindowState::Closed;
    std::string text_;
    std::string portraitImageId_;
    std::size_t characterCount_ = 0;
    std::size_t visibleCharacters_ = 0;
    float charactersPerSecond_ = 42.0F;
    float openProgress_ = 0.0F;
    float typewriterAccumulator_ = 0.0F;
    bool advanceRequested_ = false;
    bool pageShownFeedbackPending_ = false;
};
