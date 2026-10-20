#include "Graphics.h"

#include "ImageAsset.h"

#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>
#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <string>

using Microsoft::WRL::ComPtr;

namespace
{
D2D1_RECT_F ToD2DRectangle(GraphicsRect rectangle)
{
    return D2D1::RectF(
        rectangle.x,
        rectangle.y,
        rectangle.x + rectangle.width,
        rectangle.y + rectangle.height);
}

D2D1_COLOR_F ToD2DColor(GraphicsColor color)
{
    return D2D1::ColorF(color.rgb, color.opacity);
}

std::wstring Utf8ToWide(std::string_view text)
{
    if (text.empty())
    {
        return {};
    }

    const int requiredLength = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        text.data(),
        static_cast<int>(text.size()),
        nullptr,
        0);
    if (requiredLength <= 0)
    {
        return {};
    }

    std::wstring wideText(static_cast<std::size_t>(requiredLength), L'\0');
    const int convertedLength = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        text.data(),
        static_cast<int>(text.size()),
        wideText.data(),
        requiredLength);
    if (convertedLength != requiredLength)
    {
        return {};
    }
    return wideText;
}

std::size_t Utf16UnitsForCharacters(
    std::wstring_view text,
    std::size_t visibleCharacters)
{
    std::size_t units = 0;
    std::size_t characters = 0;
    while (units < text.size() && characters < visibleCharacters)
    {
        const wchar_t current = text[units++];
        if (current >= 0xD800 && current <= 0xDBFF
            && units < text.size()
            && text[units] >= 0xDC00 && text[units] <= 0xDFFF)
        {
            ++units;
        }
        ++characters;
    }
    return units;
}
}

struct Graphics::TextResources
{
    ComPtr<IDWriteFactory> factory;
};

Graphics::Graphics(ID2D1RenderTarget* renderTarget)
    : renderTarget_(renderTarget),
      textResources_(std::make_unique<TextResources>())
{
}

Graphics::~Graphics() = default;

bool Graphics::Initialize()
{
    if (textResources_->factory != nullptr)
    {
        return true;
    }

    return SUCCEEDED(DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(
            textResources_->factory.GetAddressOf())));
}

void Graphics::SetRenderTarget(ID2D1RenderTarget* renderTarget) noexcept
{
    renderTarget_ = renderTarget;
}

ID2D1RenderTarget* Graphics::GetRenderTarget() const noexcept
{
    return renderTarget_;
}

void Graphics::DrawImage(const ImageAsset& image, float x, float y) const
{
    if (renderTarget_ == nullptr || image.GetBitmap() == nullptr)
    {
        return;
    }

    const D2D1_SIZE_F size = image.GetBitmap()->GetSize();
    const D2D1_RECT_F destination = D2D1::RectF(
        x,
        y,
        x + size.width,
        y + size.height);

    renderTarget_->DrawBitmap(image.GetBitmap(), destination);
}

void Graphics::DrawImageFrame(
    const ImageAsset& image,
    std::uint32_t rows,
    std::uint32_t columns,
    std::uint32_t cell,
    float x,
    float y,
    float scale,
    float opacity) const
{
    if (renderTarget_ == nullptr
        || image.GetBitmap() == nullptr
        || rows == 0
        || columns == 0
        || cell >= rows * columns
        || scale <= 0.0F)
    {
        return;
    }

    const D2D1_SIZE_F imageSize = image.GetBitmap()->GetSize();
    const float frameWidth = imageSize.width / static_cast<float>(columns);
    const float frameHeight = imageSize.height / static_cast<float>(rows);
    const auto column = static_cast<float>(cell % columns);
    const auto row = static_cast<float>(cell / columns);

    const D2D1_RECT_F source = D2D1::RectF(
        column * frameWidth,
        row * frameHeight,
        (column + 1.0F) * frameWidth,
        (row + 1.0F) * frameHeight);
    const D2D1_RECT_F destination = D2D1::RectF(
        x,
        y,
        x + frameWidth * scale,
        y + frameHeight * scale);

    renderTarget_->DrawBitmap(
        image.GetBitmap(),
        destination,
        std::clamp(opacity, 0.0F, 1.0F),
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
        source);
}

void Graphics::DrawImageFrameTiled(
    const ImageAsset& image,
    std::uint32_t rows,
    std::uint32_t columns,
    std::uint32_t cell,
    float offsetX,
    float offsetY,
    float opacity) const
{
    if (renderTarget_ == nullptr
        || image.GetBitmap() == nullptr
        || rows == 0
        || columns == 0
        || cell >= rows * columns)
    {
        return;
    }

    const D2D1_SIZE_F imageSize = image.GetBitmap()->GetSize();
    const float frameWidth = imageSize.width / static_cast<float>(columns);
    const float frameHeight = imageSize.height / static_cast<float>(rows);
    const D2D1_SIZE_F renderSize = renderTarget_->GetSize();

    float startX = std::fmod(offsetX, frameWidth);
    float startY = std::fmod(offsetY, frameHeight);
    if (startX > 0.0F)
    {
        startX -= frameWidth;
    }
    if (startY > 0.0F)
    {
        startY -= frameHeight;
    }

    for (float y = startY; y < renderSize.height; y += frameHeight)
    {
        for (float x = startX; x < renderSize.width; x += frameWidth)
        {
            DrawImageFrame(
                image,
                rows,
                columns,
                cell,
                x,
                y,
                1.0F,
                opacity);
        }
    }
}

void Graphics::DrawImageInRectangle(
    const ImageAsset& image,
    GraphicsRect rectangle) const
{
    if (renderTarget_ == nullptr
        || image.GetBitmap() == nullptr
        || rectangle.width <= 0.0F
        || rectangle.height <= 0.0F)
    {
        return;
    }

    renderTarget_->DrawBitmap(
        image.GetBitmap(),
        ToD2DRectangle(rectangle),
        1.0F,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
}

void Graphics::FillRectangle(
    GraphicsRect rectangle,
    GraphicsColor color) const
{
    if (renderTarget_ == nullptr
        || rectangle.width <= 0.0F
        || rectangle.height <= 0.0F)
    {
        return;
    }

    ComPtr<ID2D1SolidColorBrush> brush;
    if (SUCCEEDED(renderTarget_->CreateSolidColorBrush(
            ToD2DColor(color),
            brush.GetAddressOf())))
    {
        renderTarget_->FillRectangle(ToD2DRectangle(rectangle), brush.Get());
    }
}

void Graphics::DrawRectangle(
    GraphicsRect rectangle,
    GraphicsColor color,
    float strokeWidth) const
{
    if (renderTarget_ == nullptr
        || rectangle.width <= 0.0F
        || rectangle.height <= 0.0F
        || strokeWidth <= 0.0F)
    {
        return;
    }

    ComPtr<ID2D1SolidColorBrush> brush;
    if (SUCCEEDED(renderTarget_->CreateSolidColorBrush(
            ToD2DColor(color),
            brush.GetAddressOf())))
    {
        renderTarget_->DrawRectangle(
            ToD2DRectangle(rectangle),
            brush.Get(),
            strokeWidth);
    }
}

void Graphics::PushClip(GraphicsRect rectangle) const
{
    if (renderTarget_ != nullptr)
    {
        renderTarget_->PushAxisAlignedClip(
            ToD2DRectangle(rectangle),
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }
}

void Graphics::PopClip() const
{
    if (renderTarget_ != nullptr)
    {
        renderTarget_->PopAxisAlignedClip();
    }
}

void Graphics::DrawTextLayout(
    std::string_view text,
    GraphicsRect rectangle,
    float fontSize,
    std::size_t visibleCharacters,
    GraphicsColor color,
    bool centered) const
{
    if (renderTarget_ == nullptr
        || textResources_->factory == nullptr
        || text.empty()
        || rectangle.width <= 0.0F
        || rectangle.height <= 0.0F
        || fontSize <= 0.0F)
    {
        return;
    }

    const std::wstring wideText = Utf8ToWide(text);
    if (wideText.empty())
    {
        return;
    }

    ComPtr<IDWriteTextFormat> textFormat;
    if (FAILED(textResources_->factory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_SEMI_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"en-us",
            textFormat.GetAddressOf())))
    {
        return;
    }
    textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    if (centered)
    {
        textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    ComPtr<IDWriteTextLayout> layout;
    if (FAILED(textResources_->factory->CreateTextLayout(
            wideText.data(),
            static_cast<UINT32>(wideText.size()),
            textFormat.Get(),
            rectangle.width,
            rectangle.height,
            layout.GetAddressOf())))
    {
        return;
    }

    ComPtr<ID2D1SolidColorBrush> visibleBrush;
    ComPtr<ID2D1SolidColorBrush> hiddenBrush;
    if (FAILED(renderTarget_->CreateSolidColorBrush(
            ToD2DColor(color),
            visibleBrush.GetAddressOf()))
        || FAILED(renderTarget_->CreateSolidColorBrush(
            D2D1::ColorF(0xFFFFFF, 0.0F),
            hiddenBrush.GetAddressOf())))
    {
        return;
    }

    // Layout the complete string once, then make the unrevealed range
    // transparent. DirectWrite therefore chooses the final line breaks from
    // the beginning instead of reflowing the paragraph on every character.
    if (visibleCharacters != ShowAllCharacters)
    {
        const std::size_t visibleUnits = Utf16UnitsForCharacters(
            wideText,
            visibleCharacters);
        if (visibleUnits < wideText.size())
        {
            layout->SetDrawingEffect(
                hiddenBrush.Get(),
                DWRITE_TEXT_RANGE{
                    static_cast<UINT32>(visibleUnits),
                    static_cast<UINT32>(wideText.size() - visibleUnits)});
        }
    }

    renderTarget_->DrawTextLayout(
        D2D1::Point2F(rectangle.x, rectangle.y),
        layout.Get(),
        visibleBrush.Get(),
        D2D1_DRAW_TEXT_OPTIONS_CLIP);
}
