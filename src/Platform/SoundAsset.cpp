#include "SoundAsset.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>

namespace
{
bool ReadBytes(std::ifstream& file, void* destination, std::size_t size)
{
    if (size > static_cast<std::size_t>(
            std::numeric_limits<std::streamsize>::max()))
    {
        return false;
    }

    file.read(
        static_cast<char*>(destination),
        static_cast<std::streamsize>(size));
    return file.good();
}

bool IsChunk(const std::array<char, 4>& id, const char (&expected)[5])
{
    return std::equal(id.begin(), id.end(), expected);
}
}

bool SoundAsset::Load(const std::wstring& filename)
{
    std::ifstream file(std::filesystem::path(filename), std::ios::binary);
    if (!file)
    {
        return false;
    }

    std::array<char, 4> riffId{};
    std::uint32_t riffSize = 0;
    std::array<char, 4> waveId{};
    if (!ReadBytes(file, riffId.data(), riffId.size())
        || !ReadBytes(file, &riffSize, sizeof(riffSize))
        || !ReadBytes(file, waveId.data(), waveId.size())
        || !IsChunk(riffId, "RIFF")
        || !IsChunk(waveId, "WAVE"))
    {
        return false;
    }
    static_cast<void>(riffSize);

    WAVEFORMATEXTENSIBLE parsedFormat{};
    auto parsedAudio = std::make_shared<std::vector<std::uint8_t>>();
    bool foundFormat = false;
    bool foundAudio = false;

    while (file && (!foundFormat || !foundAudio))
    {
        std::array<char, 4> chunkId{};
        std::uint32_t chunkSize = 0;
        if (!ReadBytes(file, chunkId.data(), chunkId.size())
            || !ReadBytes(file, &chunkSize, sizeof(chunkSize)))
        {
            break;
        }

        if (IsChunk(chunkId, "fmt "))
        {
            if (foundFormat
                || chunkSize < 16
                || chunkSize > sizeof(WAVEFORMATEXTENSIBLE))
            {
                return false;
            }

            std::array<std::uint8_t, sizeof(WAVEFORMATEXTENSIBLE)> bytes{};
            if (!ReadBytes(file, bytes.data(), chunkSize))
            {
                return false;
            }
            std::memcpy(&parsedFormat, bytes.data(), chunkSize);
            if (chunkSize == 16)
            {
                parsedFormat.Format.cbSize = 0;
            }
            else if (parsedFormat.Format.cbSize > chunkSize - 18)
            {
                return false;
            }
            foundFormat = true;
        }
        else if (IsChunk(chunkId, "data"))
        {
            if (foundAudio || chunkSize == 0)
            {
                return false;
            }

            parsedAudio->resize(chunkSize);
            if (!ReadBytes(file, parsedAudio->data(), parsedAudio->size()))
            {
                return false;
            }
            foundAudio = true;
        }
        else
        {
            file.seekg(chunkSize, std::ios::cur);
            if (!file)
            {
                return false;
            }
        }

        if ((chunkSize & 1U) != 0U)
        {
            file.seekg(1, std::ios::cur);
        }
    }

    const WAVEFORMATEX& format = parsedFormat.Format;
    const bool supportedFormat = format.wFormatTag == WAVE_FORMAT_PCM
        || format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT
        || format.wFormatTag == WAVE_FORMAT_EXTENSIBLE;
    if (!foundFormat
        || !foundAudio
        || !supportedFormat
        || format.nChannels == 0
        || format.nSamplesPerSec == 0
        || format.nBlockAlign == 0
        || format.nAvgBytesPerSec == 0
        || format.wBitsPerSample == 0
        || parsedAudio->size() % format.nBlockAlign != 0)
    {
        return false;
    }

    format_ = parsedFormat;
    audioData_ = std::move(parsedAudio);
    return true;
}

const WAVEFORMATEX& SoundAsset::GetFormat() const noexcept
{
    return format_.Format;
}

std::shared_ptr<const std::vector<std::uint8_t>>
SoundAsset::GetAudioData() const noexcept
{
    return audioData_;
}
