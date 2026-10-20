#include "ImageAsset.h"

#include <wincodec.h>

#include <utility>

bool ImageAsset::Load(
    ID2D1RenderTarget* renderTarget,
    const std::wstring& filename)
{
    bitmap_.Reset();

    if (renderTarget == nullptr || filename.empty())
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
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeMedianCut);

    if (FAILED(result))
    {
        return false;
    }

    Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap;
    result = renderTarget->CreateBitmapFromWicBitmap(
        converter.Get(),
        nullptr,
        bitmap.GetAddressOf());

    if (FAILED(result))
    {
        return false;
    }

    bitmap_ = std::move(bitmap);
    return true;
}

void ImageAsset::Reset() noexcept
{
    bitmap_.Reset();
}

ID2D1Bitmap* ImageAsset::GetBitmap() const noexcept
{
    return bitmap_.Get();
}
