#include "ui_assets.hpp"

#include <d3d11.h>
#include <wincodec.h>
#include <shlwapi.h>

#include <cwchar>
#include <vector>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shlwapi.lib")

namespace ui {
namespace {

template <typename T>
void SafeRelease(T*& p) {
    if (p) { p->Release(); p = nullptr; }
}

// HMODULE of this DLL, learned from an address inside it.
const wchar_t* ModuleDirectory(wchar_t* buffer, std::size_t capacity) {
    HMODULE mod = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&ModuleDirectory), &mod))
        return nullptr;

    const DWORD n = GetModuleFileNameW(mod, buffer, static_cast<DWORD>(capacity));
    if (n == 0 || n >= capacity) return nullptr;

    wchar_t* slash = wcsrchr(buffer, L'\\');
    if (!slash) return nullptr;
    *slash = L'\0';
    return buffer;
}

} // namespace

ID3D11ShaderResourceView* CreateTextureFromMemory(ID3D11Device* device,
                                                  const unsigned char* data,
                                                  std::size_t size) {
    if (!device || !data || size == 0) return nullptr;
    IWICImagingFactory* factory = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&factory))))
        return nullptr;

    IStream* stream = SHCreateMemStream(data, static_cast<UINT>(size));
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    ID3D11Texture2D* texture = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;

    do {
        if (!stream ||
            FAILED(factory->CreateDecoderFromStream(stream, nullptr,
                                                    WICDecodeMetadataCacheOnDemand, &decoder)) ||
            FAILED(decoder->GetFrame(0, &frame)) ||
            FAILED(factory->CreateFormatConverter(&converter)) ||
            FAILED(converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
                                         WICBitmapDitherTypeNone, nullptr, 0.0,
                                         WICBitmapPaletteTypeCustom)))
            break;

        UINT w = 0, h = 0;
        if (FAILED(converter->GetSize(&w, &h)) || w == 0 || h == 0) break;

        const UINT rowPitch = w * 4;
        std::vector<unsigned char> pixels(static_cast<std::size_t>(rowPitch) * h);
        if (FAILED(converter->CopyPixels(nullptr, rowPitch,
                                         static_cast<UINT>(pixels.size()), pixels.data())))
            break;

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width              = w;
        desc.Height             = h;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count   = 1;
        desc.Usage              = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem          = pixels.data();
        init.SysMemPitch      = rowPitch;

        if (FAILED(device->CreateTexture2D(&desc, &init, &texture)) ||
            FAILED(device->CreateShaderResourceView(texture, nullptr, &srv)))
            break;
    } while (false);

    SafeRelease(texture);
    SafeRelease(converter);
    SafeRelease(frame);
    SafeRelease(decoder);
    if (stream) stream->Release();
    factory->Release();
    return srv;
}

std::string DllRelativePath(const char* relative) {
    if (!relative) return {};

    wchar_t wide[MAX_PATH];
    if (MultiByteToWideChar(CP_UTF8, 0, relative, -1, wide, MAX_PATH) == 0) return {};

    std::wstring w = DllRelativePathW(wide);
    if (w.empty()) return {};

    char narrow[MAX_PATH * 2];
    const int n = WideCharToMultiByte(CP_ACP, 0, w.c_str(), -1, narrow, sizeof(narrow), nullptr, nullptr);
    if (n <= 0) return {};
    return std::string(narrow, narrow + n - 1);
}

std::wstring DllRelativePathW(const wchar_t* relative) {
    wchar_t dir[MAX_PATH];
    if (!relative || !ModuleDirectory(dir, MAX_PATH)) return {};

    wchar_t path[MAX_PATH];
    if (_snwprintf_s(path, MAX_PATH, _TRUNCATE, L"%s\\%s", dir, relative) < 0) return {};

    const DWORD attr = GetFileAttributesW(path);
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) return {};

    return std::wstring(path);
}

} // namespace ui
