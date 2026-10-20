#include "DialogueWindow.h"

#include "../Input/InputState.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float OpenDurationSeconds = 0.18F;
}

DialogueWindow::DialogueWindow(float charactersPerSecond) noexcept
    : charactersPerSecond_(charactersPerSecond)
{
}

bool DialogueWindow::Show(
    std::string_view text,
    std::string_view portraitImageId,
    bool animateOpen)
{
    const bool continuingDialogue =
        state_ == DialogueWindowState::Advancing;
    if ((IsModalActive() && !continuingDialogue) || text.empty())
    {
        return false;
    }

    text_.assign(text);
    portraitImageId_.assign(portraitImageId);
    characterCount_ = CountUtf8Characters(text_);
    visibleCharacters_ = 0;
    typewriterAccumulator_ = 0.0F;
    advanceRequested_ = false;
    pageShownFeedbackPending_ = true;
    if (continuingDialogue)
    {
        // Consecutive Say calls replace the page without reopening the frame.
        openProgress_ = 1.0F;
        state_ = DialogueWindowState::Typing;
    }
    else
    {
        openProgress_ = animateOpen ? 0.0F : 1.0F;
        state_ = animateOpen
            ? DialogueWindowState::Opening
            : DialogueWindowState::Typing;
    }
    return true;
}

void DialogueWindow::Update(float deltaTime, const InputState& input)
{
    if (state_ == DialogueWindowState::Closed)
    {
        return;
    }
    if (state_ == DialogueWindowState::Opening)
    {
        openProgress_ = std::min(
            1.0F,
            openProgress_ + std::max(deltaTime, 0.0F) / OpenDurationSeconds);
        if (openProgress_ >= 1.0F)
        {
            state_ = DialogueWindowState::Typing;
        }
        return;
    }
    if (state_ == DialogueWindowState::Typing)
    {
        if (input.primaryActionPressed)
        {
            visibleCharacters_ = characterCount_;
            typewriterAccumulator_ = 0.0F;
            state_ = DialogueWindowState::Waiting;
            return;
        }

        typewriterAccumulator_ += std::max(deltaTime, 0.0F)
            * charactersPerSecond_;
        const auto newlyVisible = static_cast<std::size_t>(
            std::floor(typewriterAccumulator_));
        if (newlyVisible > 0)
        {
            typewriterAccumulator_ -= static_cast<float>(newlyVisible);
            visibleCharacters_ = std::min(
                characterCount_,
                visibleCharacters_ + newlyVisible);
            if (visibleCharacters_ == characterCount_)
            {
                state_ = DialogueWindowState::Waiting;
            }
        }
        return;
    }
    if (state_ == DialogueWindowState::Waiting && input.primaryActionPressed)
    {
        state_ = DialogueWindowState::Advancing;
        advanceRequested_ = true;
    }
}

void DialogueWindow::Close() noexcept
{
    state_ = DialogueWindowState::Closed;
    openProgress_ = 0.0F;
    advanceRequested_ = false;
    pageShownFeedbackPending_ = false;
}

bool DialogueWindow::IsModalActive() const noexcept
{
    return state_ != DialogueWindowState::Closed;
}

bool DialogueWindow::ConsumeAdvanceRequest() noexcept
{
    const bool requested = advanceRequested_;
    advanceRequested_ = false;
    return requested;
}

bool DialogueWindow::ConsumePageShownFeedback() noexcept
{
    const bool pending = pageShownFeedbackPending_;
    pageShownFeedbackPending_ = false;
    return pending;
}

DialogueWindowState DialogueWindow::GetState() const noexcept
{
    return state_;
}

std::size_t DialogueWindow::GetVisibleCharacterCount() const noexcept
{
    return visibleCharacters_;
}

std::size_t DialogueWindow::GetCharacterCount() const noexcept
{
    return characterCount_;
}

std::size_t DialogueWindow::CountUtf8Characters(
    std::string_view text) noexcept
{
    return static_cast<std::size_t>(std::count_if(
        text.begin(),
        text.end(),
        [](char byte)
        {
            return (static_cast<unsigned char>(byte) & 0xC0U) != 0x80U;
        }));
}
