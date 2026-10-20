#include "Audio.h"

#include "../SoundAsset.h"

#include <algorithm>
#include <limits>

Audio::~Audio()
{
    Shutdown();
}

bool Audio::Initialize()
{
    if (engine_ != nullptr)
    {
        return true;
    }

    if (FAILED(XAudio2Create(engine_.GetAddressOf())))
    {
        return false;
    }

    if (FAILED(engine_->CreateMasteringVoice(&masteringVoice_)))
    {
        engine_.Reset();
        return false;
    }

    return true;
}

void Audio::Update()
{
    std::erase_if(
        sounds_,
        [](Playback& playback)
        {
            XAUDIO2_VOICE_STATE state{};
            playback.voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
            if (state.BuffersQueued != 0)
            {
                return false;
            }
            DestroyPlayback(playback);
            return true;
        });

    if (music_.has_value())
    {
        XAUDIO2_VOICE_STATE state{};
        music_->voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
        if (state.BuffersQueued == 0)
        {
            DestroyPlayback(*music_);
            music_.reset();
        }
    }
}

bool Audio::PlaySound(const SoundAsset& sound)
{
    Playback playback;
    if (!CreatePlayback(sound, false, playback))
    {
        return false;
    }

    sounds_.push_back(std::move(playback));
    return true;
}

bool Audio::PlayMusic(const SoundAsset& music, bool loop)
{
    StopMusic();

    Playback playback;
    if (!CreatePlayback(music, loop, playback))
    {
        return false;
    }

    music_ = std::move(playback);
    return true;
}

bool Audio::SetMusicVolume(float volume) noexcept
{
    return music_.has_value()
        && volume >= 0.0F
        && volume <= 1.0F
        && SUCCEEDED(music_->voice->SetVolume(volume));
}

bool Audio::IsMusicPlaying() const noexcept
{
    return music_.has_value();
}

void Audio::StopMusic() noexcept
{
    if (music_.has_value())
    {
        DestroyPlayback(*music_);
        music_.reset();
    }
}

void Audio::Shutdown() noexcept
{
    StopMusic();
    for (Playback& sound : sounds_)
    {
        DestroyPlayback(sound);
    }
    sounds_.clear();

    if (masteringVoice_ != nullptr)
    {
        masteringVoice_->DestroyVoice();
        masteringVoice_ = nullptr;
    }
    engine_.Reset();
}

bool Audio::CreatePlayback(
    const SoundAsset& sound,
    bool loop,
    Playback& playback)
{
    const auto audioData = sound.GetAudioData();
    if (engine_ == nullptr
        || audioData == nullptr
        || audioData->empty()
        || audioData->size() > std::numeric_limits<UINT32>::max())
    {
        return false;
    }

    IXAudio2SourceVoice* voice = nullptr;
    if (FAILED(engine_->CreateSourceVoice(&voice, &sound.GetFormat())))
    {
        return false;
    }

    XAUDIO2_BUFFER buffer{};
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = static_cast<UINT32>(audioData->size());
    buffer.pAudioData = audioData->data();
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
    if (FAILED(voice->SubmitSourceBuffer(&buffer))
        || FAILED(voice->Start()))
    {
        voice->DestroyVoice();
        return false;
    }

    playback.voice = voice;
    playback.audioData = audioData;
    return true;
}

void Audio::DestroyPlayback(Playback& playback) noexcept
{
    if (playback.voice != nullptr)
    {
        playback.voice->DestroyVoice();
        playback.voice = nullptr;
    }
    playback.audioData.reset();
}
