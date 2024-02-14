#include "Com.h"
#include <stdexcept>
#include <Windows.h>
#include <comdef.h>

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
}
