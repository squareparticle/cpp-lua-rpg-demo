#pragma once

#include <windows.h>
#include <mmreg.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class SoundAsset
{
public:
    bool Load(const std::wstring& filename);

    const WAVEFORMATEX& GetFormat() const noexcept;
    std::shared_ptr<const std::vector<std::uint8_t>>
        GetAudioData() const noexcept;

private:
    WAVEFORMATEXTENSIBLE format_{};
    std::shared_ptr<std::vector<std::uint8_t>> audioData_;
};
