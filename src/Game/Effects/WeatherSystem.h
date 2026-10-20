#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

class GlobalState;
class Graphics;
class ResourceManager;

class WeatherSystem
{
public:
    WeatherSystem();

    void Update(float deltaTime, const GlobalState& globalState);
    bool BeginFadeOut(float durationSeconds, GlobalState& globalState);
    bool IsFadeOutComplete() const noexcept;
    void Render(
        const Graphics& graphics,
        const ResourceManager& resources) const;

private:
    enum class Type
    {
        None,
        Rain,
        Fairies
    };

    struct Particle
    {
        float x = 0.0F;
        float y = 0.0F;
        float velocityX = 0.0F;
        float velocityY = 0.0F;
        float animationTime = 0.0F;
        std::uint32_t frame = 0;
    };

    Type ResolveType(const GlobalState& globalState) const noexcept;
    void ChangeType(Type type);
    void ResetParticle(Particle& particle, bool distributeOnScreen);
    float RandomFloat(float minimum, float maximum);

    Type type_ = Type::None;
    float opacity_ = 1.0F;
    float fadeDurationSeconds_ = 0.0F;
    float fadeElapsedSeconds_ = 0.0F;
    bool fadingOut_ = false;
    bool fadeOutComplete_ = false;
    std::vector<Particle> particles_;
    std::mt19937 random_;
    bool nextParticleEntersFromTop_ = true;
};
