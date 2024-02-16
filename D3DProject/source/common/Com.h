#pragma once
#include <d3dcommon.h>

namespace com {
    void ThrowIfFailed(HRESULT result);
    void ThrowIfCompileFailed(HRESULT result, ID3DBlob* errorBlob);
}
