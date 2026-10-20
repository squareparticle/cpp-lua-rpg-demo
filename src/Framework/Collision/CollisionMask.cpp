#include "CollisionMask.h"

#include <wincodec.h>
#include <wrl/client.h>

#include <limits>
#include <utility>

bool CollisionMask::Load(const std::wstring& filename)
{
    width_ = 0;
    height_ = 0;
    pixels_.clear();

    if (filename.empty())
    {
        return false;
    }

    Microsoft::WRL::ComPtr<IWICImagingFactory> imagingFactory;
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;

    HRESULT result = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(imagingFactory.GetAddressOf()));
    if (FAILED(result))
    {
        return false;
    }

    result = imagingFactory->CreateDecoderFromFilename(
        filename.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        decoder.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    result = decoder->GetFrame(0, frame.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    result = imagingFactory->CreateFormatConverter(converter.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    result = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeMedianCut);
    if (FAILED(result))
    {
        return false;
    }

    UINT width = 0;
    UINT height = 0;
    result = converter->GetSize(&width, &height);
    if (FAILED(result) || width == 0 || height == 0)
    {
        return false;
    }

    constexpr std::size_t bytesPerPixel = 4;
    const std::size_t stride = static_cast<std::size_t>(width) * bytesPerPixel;
    const std::size_t bufferSize = stride * static_cast<std::size_t>(height);
    if (stride > std::numeric_limits<UINT>::max()
        || bufferSize > std::numeric_limits<UINT>::max())
    {
        return false;
    }

    std::vector<std::uint8_t> pixels(bufferSize);
    result = converter->CopyPixels(
        nullptr,
        static_cast<UINT>(stride),
        static_cast<UINT>(bufferSize),
        pixels.data());
    if (FAILED(result))
    {
        return false;
    }

    width_ = width;
    height_ = height;
    pixels_ = std::move(pixels);
    return true;
}

std::optional<std::uint32_t> CollisionMask::GetColorAt(
    int x,
    int y) const noexcept
{
    if (x < 0
        || y < 0
        || static_cast<std::uint32_t>(x) >= width_
        || static_cast<std::uint32_t>(y) >= height_)
    {
        return std::nullopt;
    }

    constexpr std::size_t bytesPerPixel = 4;
    const std::size_t index =
        (static_cast<std::size_t>(y) * width_ + static_cast<std::size_t>(x))
        * bytesPerPixel;
    const std::uint32_t red = pixels_[index];
    const std::uint32_t green = pixels_[index + 1];
    const std::uint32_t blue = pixels_[index + 2];
    return (red << 16U) | (green << 8U) | blue;
}
