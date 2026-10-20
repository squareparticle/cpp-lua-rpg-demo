#include "../src/Platform/SoundAsset.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
{
void WriteU16(std::ofstream& file, std::uint16_t value)
{
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void WriteU32(std::ofstream& file, std::uint32_t value)
{
    file.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void WriteFourCC(std::ofstream& file, const char (&value)[5])
{
    file.write(value, 4);
}

bool WriteTestWave(const std::filesystem::path& path)
{
    constexpr std::array<std::uint8_t, 4> Samples{128, 255, 0, 128};
    std::ofstream file(path, std::ios::binary);
    if (!file)
    {
        return false;
    }

    WriteFourCC(file, "RIFF");
    WriteU32(file, 40);
    WriteFourCC(file, "WAVE");
    WriteFourCC(file, "fmt ");
    WriteU32(file, 16);
    WriteU16(file, WAVE_FORMAT_PCM);
    WriteU16(file, 1);
    WriteU32(file, 8000);
    WriteU32(file, 8000);
    WriteU16(file, 1);
    WriteU16(file, 8);
    WriteFourCC(file, "data");
    WriteU32(file, static_cast<std::uint32_t>(Samples.size()));
    file.write(
        reinterpret_cast<const char*>(Samples.data()),
        static_cast<std::streamsize>(Samples.size()));
    return file.good();
}

bool Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

struct TemporaryFile
{
    std::filesystem::path path;

    ~TemporaryFile()
    {
        std::error_code error;
        std::filesystem::remove(path, error);
    }
};
}

int main()
{
    TemporaryFile file{
        std::filesystem::temp_directory_path()
            / "rpg2002_ship_sound_asset_test.wav"};
    if (!Expect(WriteTestWave(file.path), "Could not write the test WAV"))
    {
        return 1;
    }

    SoundAsset sound;
    if (!Expect(sound.Load(file.path.wstring()), "Valid PCM WAV did not load"))
    {
        return 1;
    }

    const WAVEFORMATEX& format = sound.GetFormat();
    const auto data = sound.GetAudioData();
    return Expect(format.wFormatTag == WAVE_FORMAT_PCM,
            "Loaded WAV has the wrong format")
        && Expect(format.nChannels == 1 && format.nSamplesPerSec == 8000,
            "Loaded WAV has the wrong channel or sample rate")
        && Expect(format.wBitsPerSample == 8 && format.nBlockAlign == 1,
            "Loaded WAV has the wrong sample layout")
        && Expect(data != nullptr && data->size() == 4,
            "Loaded WAV has the wrong sample data size")
        && Expect((*data)[0] == 128 && (*data)[1] == 255,
            "Loaded WAV did not preserve sample data")
        ? 0
        : 1;
}
