#include "Renderer.h"
#include <common/Com.h>
#include <window/Window.h>
#include <object/ObjectData.h>

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

namespace render {
    State::State(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context)
        : m_device{ device }
        , m_context{ context }
    {
    }

    ComPtr<ID3D11Device> State::Device() {
        return m_device;
    }

    ComPtr<ID3D11DeviceContext> State::Context() {
        return m_context;
    }

    std::shared_ptr<State> State::Create() {
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;

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
                device.GetAddressOf(),
                nullptr,
                context.GetAddressOf()
            )
        );

        return std::shared_ptr<State>(new State(device, context));
    }

    Renderer::Renderer(Window& window, const std::shared_ptr<State>& state)
        : m_window{ window }
        , m_state{ state }
    {
        CreateSwapChain();
        CreateRenderTargetView();
        CreateShaders();
        CreateInputLayout();
        SetPipeline();
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
        com::ThrowIfFailed(m_state->Device().As(&dxgiDevice));

        ComPtr<IDXGIAdapter> dxgiAdapter;
        com::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        ComPtr<IDXGIFactory> dxgiFactory;
        com::ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory));

        com::ThrowIfFailed(dxgiFactory->CreateSwapChain(m_state->Device().Get(), &swapChainDesc, m_swapChain.GetAddressOf()));
    }

    void Renderer::CreateRenderTargetView()
    {
        ComPtr<ID3D11Texture2D> backBuffer;
        com::ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));

        com::ThrowIfFailed(m_state->Device()->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf()));
    }

    void Renderer::CreateShaders()
    {
        ComPtr<ID3D10Blob> pixelShaderByteCode = d3dcompile::CompileShader("PixelShader.hlsl", "main", "ps_5_0");

        com::ThrowIfFailed(
            m_state->Device()->CreatePixelShader(
                pixelShaderByteCode->GetBufferPointer(),
                pixelShaderByteCode->GetBufferSize(),
                nullptr,
                m_pixelShader.GetAddressOf()
            )
        );

        m_vertexShaderByteCode = d3dcompile::CompileShader("VertexShader.hlsl", "main", "vs_5_0");

        com::ThrowIfFailed(
            m_state->Device()->CreateVertexShader(
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
            m_state->Device()->CreateInputLayout(
                inElemDesc,
                2,
                m_vertexShaderByteCode->GetBufferPointer(),
                m_vertexShaderByteCode->GetBufferSize(),
                m_inputLayout.GetAddressOf()
            )
        );
    }

    void Renderer::SetPipeline()
    {
        m_state->Context()->IASetInputLayout(m_inputLayout.Get());
        m_state->Context()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        m_state->Context()->VSSetShader(m_vertexShader.Get(), nullptr, 0);
        m_state->Context()->PSSetShader(m_pixelShader.Get(), nullptr, 0);
        m_state->Context()->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(m_window.GetSize().m_width);
        viewport.Height = static_cast<float>(m_window.GetSize().m_height);
        viewport.MaxDepth = 1.0f;
        viewport.MinDepth = 0.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;

        m_state->Context()->RSSetViewports(1, &viewport);
    }

    void Renderer::Clear(XMUINT4 color)
    {
        float in_color[] = { color.x / 255.0f, color.y / 255.0f, color.z / 255.0f, color.w / 255.0f };
        m_state->Context()->ClearRenderTargetView(m_renderTargetView.Get(), in_color);
    }

    void Renderer::Bind(const ComPtr<ID3D11Buffer>& vertexBuffer, const ComPtr<ID3D11Buffer>& indexBuffer) {
        uint32_t stride = sizeof(Vertex);
        uint32_t offset = 0;

        m_state->Context()->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
        m_state->Context()->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    }

    void Renderer::Draw(uint32_t indexCount)
    {
        m_state->Context()->DrawIndexed(indexCount, 0, 0);
    }

    void Renderer::Present()
    {
        m_swapChain->Present(1, 0);
    }
}
