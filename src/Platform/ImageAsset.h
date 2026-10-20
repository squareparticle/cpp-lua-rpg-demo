#pragma once

#include <d2d1.h>
#include <wrl/client.h>

#include <string>

class ImageAsset
{
public:
    bool Load(ID2D1RenderTarget* renderTarget, const std::wstring& filename);
    void Reset() noexcept;
    ID2D1Bitmap* GetBitmap() const noexcept;

private:
    Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap_;
};
