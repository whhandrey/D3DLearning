#include "Renderer.h"
#include <common/Com.h>
#include <window/Window.h>

Renderer::Renderer(Window& window)
    : m_window{ window }
{
    CreateDevice();
    CreateSwapChain();
    CreateRenderTargetView();
}

void Renderer::CreateDevice()
{
    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    com::ThrowIfFailed(
        D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            featureLevels,
            1,
            D3D11_SDK_VERSION,
            m_device.GetAddressOf(),
            nullptr,
            m_context.GetAddressOf())
    );
}

void Renderer::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = m_window.GetSize().m_width;
    swapChainDesc.BufferDesc.Height = m_window.GetSize().m_height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = m_window.GetNativeHandle();
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    ComPtr<IDXGIDevice> dxgiDevice;
    com::ThrowIfFailed(m_device.As(&dxgiDevice));

    ComPtr<IDXGIAdapter> dxgiAdapter;
    com::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

    ComPtr<IDXGIFactory> dxgiFactory;
    com::ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory));

    com::ThrowIfFailed(dxgiFactory->CreateSwapChain(m_device.Get(), &swapChainDesc, m_swapChain.GetAddressOf()));
}

void Renderer::CreateRenderTargetView()
{
    ComPtr<ID3D11Texture2D> backBuffer;
    com::ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));

    com::ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf()));
}

void Renderer::Clear(XMUINT4 color)
{
    float in_color[] = {color.x / 255.0f, color.y / 255.0f, color.z / 255.0f, color.w / 255.0f};
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), in_color);
}

void Renderer::Present()
{
    m_swapChain->Present(1, 0);
}
