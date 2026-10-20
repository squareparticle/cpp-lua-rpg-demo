#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>

inline constexpr std::size_t ShowAllCharacters =
    std::numeric_limits<std::size_t>::max();

struct ID2D1RenderTarget;
class ImageAsset;

struct GraphicsRect
{
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;
};

struct GraphicsColor
{
    std::uint32_t rgb = 0;
    float opacity = 1.0F;
};

class Graphics
{
public:
    explicit Graphics(ID2D1RenderTarget* renderTarget);
    ~Graphics();

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    bool Initialize();
    void SetRenderTarget(ID2D1RenderTarget* renderTarget) noexcept;
    ID2D1RenderTarget* GetRenderTarget() const noexcept;
    void DrawImage(const ImageAsset& image, float x, float y) const;
    void DrawImageFrame(
        const ImageAsset& image,
        std::uint32_t rows,
        std::uint32_t columns,
        std::uint32_t cell,
        float x,
        float y,
        float scale,
        float opacity = 1.0F) const;
    void DrawImageFrameTiled(
        const ImageAsset& image,
        std::uint32_t rows,
        std::uint32_t columns,
        std::uint32_t cell,
        float offsetX,
        float offsetY,
        float opacity = 1.0F) const;
    void DrawImageInRectangle(
        const ImageAsset& image,
        GraphicsRect rectangle) const;
    void FillRectangle(
        GraphicsRect rectangle,
        GraphicsColor color) const;
    void DrawRectangle(
        GraphicsRect rectangle,
        GraphicsColor color,
        float strokeWidth) const;
    void PushClip(GraphicsRect rectangle) const;
    void PopClip() const;
    void DrawTextLayout(
        std::string_view text,
        GraphicsRect rectangle,
        float fontSize,
        std::size_t visibleCharacters,
        GraphicsColor color = {0xFFFFFF, 1.0F},
        bool centered = false) const;

private:
    struct TextResources;

    ID2D1RenderTarget* renderTarget_ = nullptr;
    std::unique_ptr<TextResources> textResources_;
};
