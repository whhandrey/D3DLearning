#include "Com.h"
#include <stdexcept>
#include <Windows.h>
#include <comdef.h>
#include <sstream>

namespace com {
    std::string GetErrorInfo(HRESULT result) {
        _com_error err(result);
        return err.ErrorMessage();
    }

    void ThrowIfFailed(HRESULT result) {
        if (SUCCEEDED(result))
            return;

        throw std::runtime_error("Failed to perform call, error message: " + GetErrorInfo(result));
    }

    void ThrowIfCompileFailed(HRESULT result, ID3DBlob* errorBlob) {
        if (SUCCEEDED(result))
            return;

        std::stringstream ss;
        ss << "Failed to compile shader: " << GetErrorInfo(result) << '\n';

        if (errorBlob && errorBlob->GetBufferPointer()) {
            ss << static_cast<const char*>(errorBlob->GetBufferPointer());
        }

        throw std::runtime_error(ss.str());
    }
}
