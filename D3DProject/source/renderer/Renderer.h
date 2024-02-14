#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
using namespace DirectX;

#include <wrl.h>
using namespace Microsoft::WRL;

class Window;

class Renderer {
public:
    Renderer(Window& window);
    ~Renderer() = default;

public:
    void Clear(XMUINT4 color);
    void Present();

private:
    void CreateDevice();
    void CreateSwapChain();
    void CreateRenderTargetView();

private:
    Window& m_window;

    ComPtr<ID3D11Device> m_device = nullptr;
    ComPtr<ID3D11DeviceContext> m_context = nullptr;

    ComPtr<IDXGISwapChain> m_swapChain = nullptr;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;
};
