#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

struct ObjectData {
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};
