#include "WeatherSystem.h"

#include "../../Platform/Graphics.h"
#include "../../Framework/Resources/ResourceManager.h"
#include "../State/GlobalState.h"

#include <d2d1.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr char RainTag[] = "weather=rain";
constexpr char FairiesTag[] = "weather=fairies";
constexpr char RainImageId[] = "weatherRain";
constexpr char FairiesImageId[] = "weatherFairies";

constexpr std::size_t RainParticleCount = 300;
constexpr std::size_t FairyParticleCount = 24;
constexpr std::uint32_t FairyFrameCount = 6;
constexpr float FairyFrameDuration = 0.12F;
constexpr float FairyScale = 0.18F;
}

WeatherSystem::WeatherSystem()
    : random_(std::random_device{}())
{
}

void WeatherSystem::Update(
    float deltaTime,
    const GlobalState& globalState)
{
    const Type requestedType = ResolveType(globalState);
    if (fadingOut_)
    {
        fadeElapsedSeconds_ += std::max(deltaTime, 0.0F);
        opacity_ = 1.0F - std::clamp(
            fadeElapsedSeconds_ / fadeDurationSeconds_, 0.0F, 1.0F);
        if (fadeElapsedSeconds_ >= fadeDurationSeconds_)
        {
            fadingOut_ = false;
            fadeOutComplete_ = true;
            ChangeType(requestedType);
        }
    }
    else if (requestedType != type_)
    {
        ChangeType(requestedType);
    }

    if (type_ == Type::None)
    {
        return;
    }

    const float elapsed = std::clamp(deltaTime, 0.0F, 0.1F);
    for (Particle& particle : particles_)
    {
        particle.x += particle.velocityX * elapsed;
        particle.y += particle.velocityY * elapsed;

        if (type_ == Type::Fairies)
        {
            particle.animationTime += elapsed;
            while (particle.animationTime >= FairyFrameDuration)
            {
                particle.animationTime -= FairyFrameDuration;
                particle.frame = (particle.frame + 1) % FairyFrameCount;
            }
        }

        if (particle.x < -0.1F || particle.y > 1.1F)
        {
            ResetParticle(particle, false);
        }
    }
}

bool WeatherSystem::BeginFadeOut(
    float durationSeconds,
    GlobalState& globalState)
{
    if (fadingOut_ || !std::isfinite(durationSeconds)
        || durationSeconds <= 0.0F)
    {
        return false;
    }

    // A script may enable weather and request its fade in the same frame.
    if (type_ == Type::None)
    {
        ChangeType(ResolveType(globalState));
    }

    // Retire only the active weather selection. Other enabled weather can
    // take over after the outgoing particles have fully faded.
    if (type_ == Type::Fairies)
    {
        globalState.SetTag(FairiesTag, false);
    }
    else if (type_ == Type::Rain)
    {
        globalState.SetTag(RainTag, false);
    }

    fadeDurationSeconds_ = durationSeconds;
    fadeElapsedSeconds_ = 0.0F;
    fadingOut_ = type_ != Type::None;
    fadeOutComplete_ = !fadingOut_;
    return true;
}

bool WeatherSystem::IsFadeOutComplete() const noexcept
{
    return fadeOutComplete_;
}

void WeatherSystem::Render(
    const Graphics& graphics,
    const ResourceManager& resources) const
{
    if (type_ == Type::None || graphics.GetRenderTarget() == nullptr)
    {
        return;
    }

    const ImageAsset* image = resources.GetImage(
        type_ == Type::Rain ? RainImageId : FairiesImageId);
    if (image == nullptr)
    {
        return;
    }

    const D2D1_SIZE_F viewport = graphics.GetRenderTarget()->GetSize();
    for (const Particle& particle : particles_)
    {
        const float x = particle.x * viewport.width;
        const float y = particle.y * viewport.height;
        if (type_ == Type::Rain)
        {
            graphics.DrawImageFrame(
                *image,
                1,
                1,
                0,
                x,
                y,
                1.0F,
                0.8F * opacity_);
        }
        else
        {
            graphics.DrawImageFrame(
                *image,
                2,
                3,
                particle.frame,
                x,
                y,
                FairyScale,
                0.95F * opacity_);
        }
    }
}

WeatherSystem::Type WeatherSystem::ResolveType(
    const GlobalState& globalState) const noexcept
{
    if (globalState.GetTag(FairiesTag))
    {
        return Type::Fairies;
    }
    if (globalState.GetTag(RainTag))
    {
        return Type::Rain;
    }
    return Type::None;
}

void WeatherSystem::ChangeType(Type type)
{
    type_ = type;
    opacity_ = 1.0F;
    particles_.clear();
    nextParticleEntersFromTop_ = true;
    if (type_ == Type::None)
    {
        return;
    }

    const std::size_t count = type_ == Type::Rain
        ? RainParticleCount
        : FairyParticleCount;
    particles_.resize(count);
    for (Particle& particle : particles_)
    {
        ResetParticle(particle, true);
    }
}

void WeatherSystem::ResetParticle(
    Particle& particle,
    bool distributeOnScreen)
{
    if (distributeOnScreen)
    {
        particle.x = RandomFloat(0.0F, 1.0F);
        particle.y = RandomFloat(0.0F, 1.0F);
    }
    else if (nextParticleEntersFromTop_)
    {
        particle.x = RandomFloat(0.0F, 1.0F);
        particle.y = -0.05F;
    }
    else
    {
        particle.x = 1.05F;
        particle.y = RandomFloat(0.0F, 1.0F);
    }
    nextParticleEntersFromTop_ = !nextParticleEntersFromTop_;

    if (type_ == Type::Rain)
    {
        particle.velocityX = RandomFloat(-0.32F, -0.20F);
        particle.velocityY = RandomFloat(0.65F, 0.90F);
        particle.frame = 0;
        particle.animationTime = 0.0F;
        return;
    }

    particle.velocityX = RandomFloat(-0.05F, -0.015F);
    particle.velocityY = RandomFloat(0.03F, 0.09F);
    particle.frame = static_cast<std::uint32_t>(
        RandomFloat(0.0F, static_cast<float>(FairyFrameCount)));
    particle.frame = std::min(particle.frame, FairyFrameCount - 1);
    particle.animationTime = RandomFloat(0.0F, FairyFrameDuration);
}

float WeatherSystem::RandomFloat(float minimum, float maximum)
{
    return std::uniform_real_distribution<float>(minimum, maximum)(random_);
}
