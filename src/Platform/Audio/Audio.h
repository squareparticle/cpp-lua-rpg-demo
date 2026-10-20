#pragma once

#include <wrl/client.h>
#include <xaudio2.h>

#include <memory>
#include <optional>
#include <vector>

class SoundAsset;

class Audio
{
public:
    Audio() = default;
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    bool Initialize();
    void Update();
    bool PlaySound(const SoundAsset& sound);
    bool PlayMusic(const SoundAsset& music, bool loop = true);
    bool SetMusicVolume(float volume) noexcept;
    bool IsMusicPlaying() const noexcept;
    void StopMusic() noexcept;
    void Shutdown() noexcept;

private:
    struct Playback
    {
        IXAudio2SourceVoice* voice = nullptr;
        std::shared_ptr<const std::vector<std::uint8_t>> audioData;
    };

    bool CreatePlayback(
        const SoundAsset& sound,
        bool loop,
        Playback& playback);
    static void DestroyPlayback(Playback& playback) noexcept;

    Microsoft::WRL::ComPtr<IXAudio2> engine_;
    IXAudio2MasteringVoice* masteringVoice_ = nullptr;
    std::vector<Playback> sounds_;
    std::optional<Playback> music_;
};
