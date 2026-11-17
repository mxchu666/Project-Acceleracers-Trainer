#pragma once

#include <cstddef>
#include <string>

struct ID3D11Device;
struct ID3D11ShaderResourceView;

namespace ui {

// The D3D11 device the trainer injected with (set by ui::Init).
ID3D11Device* D3DDevice();

// Decodes an in-memory image (PNG/JPG) through WIC and wraps it as a D3D11
// shader resource view usable as ImTextureID. Returns nullptr on failure.
ID3D11ShaderResourceView* CreateTextureFromMemory(ID3D11Device* device,
                                                  const unsigned char* data,
                                                  std::size_t size);

// Loads a file next to the trainer DLL (e.g. "assets\\nl_legacy\\SSTMedium.TTF")
// and returns its absolute path, or an empty string when it does not exist.
std::string DllRelativePath(const char* relative);

// Same, for wide relative paths (font files with Unicode names).
std::wstring DllRelativePathW(const wchar_t* relative);

} // namespace ui
