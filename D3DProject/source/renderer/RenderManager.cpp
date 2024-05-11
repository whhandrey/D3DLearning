#include <renderer/RenderManager.h>
#include <common/Com.h>

namespace {
    ComPtr<ID3D11Buffer> CreateVertexBuffer(ID3D11Device* device, const std::vector<Vertex>& vertices) {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pSysMem = vertices.data();

        ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
        com::ThrowIfFailed(
            device->CreateBuffer(
                &bufferDesc,
                &subresourceData,
                vertexBuffer.GetAddressOf()
            )
        );

        return vertexBuffer;
    }

    ComPtr<ID3D11Buffer> CreateIndexBuffer(ID3D11Device* device, const std::vector<uint32_t>& indices) {
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.ByteWidth = sizeof(uint32_t) * indices.size();
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pSysMem = indices.data();

        ComPtr<ID3D11Buffer> indexBuffer = nullptr;
        com::ThrowIfFailed(
            device->CreateBuffer(
                &bufferDesc,
                &subresourceData,
                indexBuffer.GetAddressOf()
            )
        );

        return indexBuffer;
    }
}

namespace render {
    RenderManager::RenderManager(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<State>& state)
        : m_renderer{ renderer }
        , m_state{ state }
    {
    }

    size_t RenderManager::AddObject(const ObjectData& object) {
        auto vertexBuffer = CreateVertexBuffer(m_state->Device().Get(), object.m_vertices);
        auto indexBuffer = CreateIndexBuffer(m_state->Device().Get(), object.m_indices);

        m_buffers.emplace(m_nextId, ObjectBuffer{ vertexBuffer, indexBuffer, object.m_indices.size() });
        return m_nextId++;
    }

    void RenderManager::RemoveObject(size_t id) {
        const auto& buffers = m_buffers.at(id);

        buffers.m_indexBuffer->Release();
        buffers.m_vertexBuffer->Release();

        m_buffers.erase(id);
    }

    void RenderManager::Draw(size_t id) {
        const auto& buffers = m_buffers.at(id);

        m_renderer->Bind(buffers.m_vertexBuffer, buffers.m_indexBuffer);
        m_renderer->Draw(buffers.m_indexCount);
    }

    void RenderManager::DrawAll() {
        for (const auto& buffers : m_buffers) {
            Draw(buffers.first);
        }
    }
}
