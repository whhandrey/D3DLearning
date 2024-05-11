#pragma once
#include <d3d11.h>
#include <object/ObjectData.h>
#include <renderer/Renderer.h>

#include <unordered_map>
#include <memory>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace render {
    class RenderManager {
    public:
        RenderManager(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<State>& state);

        size_t AddObject(const ObjectData& object);
        void RemoveObject(size_t id);

        void Draw(size_t id);
        void DrawAll();

    private:
        using BufferPtr = ComPtr<ID3D11Buffer>;

        struct ObjectBuffer {
            BufferPtr m_vertexBuffer;
            BufferPtr m_indexBuffer;
            size_t m_indexCount;
        };

        size_t m_nextId = 0;
        std::unordered_map<size_t, ObjectBuffer> m_buffers;

        std::shared_ptr<Renderer> m_renderer;
        std::shared_ptr<State> m_state;
    };
}
