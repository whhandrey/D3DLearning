#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
using namespace DirectX;

#include <memory>
#include <wrl.h>
using namespace Microsoft::WRL;

class Window;

namespace render {
    class State {
    private:
        State(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context);

    public:
        static std::shared_ptr<State> Create();

        ComPtr<ID3D11Device> Device();
        ComPtr<ID3D11DeviceContext> Context();

    private:
        ComPtr<ID3D11Device> m_device;
        ComPtr<ID3D11DeviceContext> m_context;
    };

    class Renderer {
    public:
        Renderer(Window& window, const std::shared_ptr<State>& state);
        ~Renderer() = default;

    public:
        void Clear(XMUINT4 color);
        void Bind(const ComPtr<ID3D11Buffer>& vertexBuffer, const ComPtr<ID3D11Buffer>& indexBuffer);

        void Draw(uint32_t indexCount);
        void Present();

    private:
        void CreateSwapChain();
        void CreateRenderTargetView();
        void CreateShaders();
        void CreateInputLayout();
        void SetPipeline();

    private:
        Window& m_window;
        std::shared_ptr<State> m_state;

        ComPtr<IDXGISwapChain> m_swapChain = nullptr;
        ComPtr<ID3D11RenderTargetView> m_renderTargetView = nullptr;

        ComPtr<ID3D11VertexShader> m_vertexShader = nullptr;
        ComPtr<ID3D10Blob> m_vertexShaderByteCode = nullptr;

        ComPtr<ID3D11PixelShader> m_pixelShader = nullptr;
        ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;

        ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
        ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;
    };
}
