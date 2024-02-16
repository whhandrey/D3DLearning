#include "Renderer.h"
#include <common/Com.h>
#include <window/Window.h>

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

namespace {
    std::string GetProjectName() {
        return "D3DProject";
    }

    std::string GetShaderPath(const std::string& shaderName) {
        const std::string projectName = GetProjectName();
        auto path = fs::current_path();

        // Find the project root directory by going up the directory tree
        while (path.filename() != projectName) {
            path = path.parent_path();
        }

        const auto shaderPath = path / "source/shader" / shaderName;
        return shaderPath.string();
    }

    std::string ReadShaderFile(const std::string& shaderFilePath) {
        std::ifstream file(shaderFilePath);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + shaderFilePath);
        }

        std::string fileLine;
        std::stringstream fileContent;

        while (std::getline(file, fileLine)) {
            fileContent << fileLine << '\n';
        }

        file.close();
        return fileContent.str();
    }
}

namespace d3dcompile {
    ComPtr<ID3D10Blob> CompileShader(const std::string& shaderName, const std::string& entryPoint, const std::string& shaderVersion) {
        const auto shaderPath = GetShaderPath(shaderName);
        const auto shaderSource = ReadShaderFile(shaderPath);

        ComPtr<ID3D10Blob> shaderByteCode;
        ComPtr<ID3D10Blob> errorMessage;

        com::ThrowIfCompileFailed(
            D3DCompile(
                shaderSource.c_str(),
                shaderSource.size(),
                nullptr,
                nullptr,
                nullptr,
                entryPoint.c_str(),
                shaderVersion.c_str(),
                D3DCOMPILE_ENABLE_STRICTNESS,
                0,
                shaderByteCode.GetAddressOf(),
                errorMessage.GetAddressOf()), errorMessage.Get()
        );

        return shaderByteCode;
    }
}

Renderer::Renderer(Window& window)
    : m_window{ window }
{
    CreateDevice();
    CreateSwapChain();
    CreateRenderTargetView();
    CreateShaders();
    CreateInputLayout();
    SetPipeline();
    CreateAndBindVertexBuffer();
    CreateAndBindIndexBuffer();
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
            m_context.GetAddressOf()
        )
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

void Renderer::CreateShaders()
{
    ComPtr<ID3D10Blob> pixelShaderByteCode = d3dcompile::CompileShader("PixelShader.hlsl", "main", "ps_5_0");

    com::ThrowIfFailed(
        m_device->CreatePixelShader(
            pixelShaderByteCode->GetBufferPointer(),
            pixelShaderByteCode->GetBufferSize(),
            nullptr,
            m_pixelShader.GetAddressOf()
        )
    );

    m_vertexShaderByteCode = d3dcompile::CompileShader("VertexShader.hlsl", "main", "vs_5_0");

    com::ThrowIfFailed(
        m_device->CreateVertexShader(
            m_vertexShaderByteCode->GetBufferPointer(),
            m_vertexShaderByteCode->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf()
        )
    );
}

void Renderer::CreateInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC inElemDesc[] = {
        { "POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    com::ThrowIfFailed(
        m_device->CreateInputLayout(
            inElemDesc,
            2,
            m_vertexShaderByteCode->GetBufferPointer(),
            m_vertexShaderByteCode->GetBufferSize(),
            m_inputLayout.GetAddressOf()
        )
    );
}

void Renderer::CreateAndBindVertexBuffer()
{
    struct Vertex {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    Vertex vertices[] = {
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(Vertex) * 3;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = vertices;

    com::ThrowIfFailed(
        m_device->CreateBuffer(
            &bufferDesc,
            &subresourceData,
            m_vertexBuffer.GetAddressOf()
        )
    );

    uint32_t stride = sizeof(Vertex);
    uint32_t offset = 0;

    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
}

void Renderer::CreateAndBindIndexBuffer()
{
    int indices[] = { 0, 1, 2 };

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(int) * 3;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = indices;

    com::ThrowIfFailed(
        m_device->CreateBuffer(
            &bufferDesc,
            &subresourceData,
            m_indexBuffer.GetAddressOf()
        )
    );

    m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void Renderer::SetPipeline()
{
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(m_window.GetSize().m_width);
    viewport.Height = static_cast<float>(m_window.GetSize().m_height);
    viewport.MaxDepth = 1.0f;
    viewport.MinDepth = 0.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    m_context->RSSetViewports(1, &viewport);
}

void Renderer::Clear(XMUINT4 color)
{
    float in_color[] = {color.x / 255.0f, color.y / 255.0f, color.z / 255.0f, color.w / 255.0f};
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), in_color);
}

void Renderer::Draw(uint32_t indexCount)
{
    m_context->DrawIndexed(indexCount, 0, 0);
}

void Renderer::Present()
{
    m_swapChain->Present(1, 0);
}
