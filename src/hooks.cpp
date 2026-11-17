#include "hooks.hpp"
#include "features.hpp"
#include "menu.hpp"
#include "uis/ui_manager.hpp"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <MinHook.h>

#include <atomic>

// imgui_impl_win32.h leaves this declaration commented out on purpose; the host
// is expected to forward it itself.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace hooks {
namespace {

using PresentFn  = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
using Present1Fn = HRESULT(WINAPI*)(IDXGISwapChain1*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*);
using ResizeFn   = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

constexpr int kSwapPresent  = 8;
constexpr int kSwapResize   = 13;
constexpr int kSwap1Present1 = 22;

PresentFn  g_origPresent  = nullptr;
Present1Fn g_origPresent1 = nullptr;
ResizeFn   g_origResize   = nullptr;

// UObject::ProcessEvent. It is the game thread's heartbeat: every Blueprint tick
// runs through it, so it is the one place per-frame work can run on the right
// thread without a second hook on UWorld::Tick.
using ProcessEventFn = void (*)(void*, void*, void*);
ProcessEventFn    g_origProcessEvent = nullptr;
std::atomic<bool> g_inFrame{false};

ID3D11Device*           g_device = nullptr;
ID3D11DeviceContext*    g_ctx    = nullptr;
ID3D11RenderTargetView* g_rtv    = nullptr;
IDXGISwapChain*         g_swap   = nullptr;
HWND                    g_hwnd   = nullptr;
WNDPROC                 g_origWndProc = nullptr;

bool g_backendReady  = false;
bool g_backendFailed = false;   // set once the device query fails; the swap chain is not D3D11

void ReleaseTarget() {
    if (g_rtv) {
        g_rtv->Release();
        g_rtv = nullptr;
    }
}

void EnsureTarget() {
    if (g_rtv || !g_device || !g_swap) return;
    ID3D11Texture2D* backBuffer = nullptr;
    if (SUCCEEDED(g_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))) && backBuffer) {
        g_device->CreateRenderTargetView(backBuffer, nullptr, &g_rtv);
        backBuffer->Release();
    }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_INSERT && !(lParam & 0x40000000)) {
        ui::Toggle();
        return 0;
    }
    if (msg == WM_KEYDOWN && wParam == VK_HOME && !(lParam & 0x40000000)) {
        ui::OpenSelector();
        return 0;
    }

    if (ui::WantsInput()) {
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        switch (msg) {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
        case WM_MOUSEWHEEL:  case WM_MOUSEHWHEEL:
        case WM_KEYDOWN:     case WM_KEYUP:
        case WM_CHAR:        case WM_SYSKEYDOWN: case WM_SYSKEYUP:
            return 1;
        default:
            break;
        }
    }
    return CallWindowProcW(g_origWndProc, hWnd, msg, wParam, lParam);
}

bool InitBackend(IDXGISwapChain* sc) {
    if (g_backendReady) return true;
    if (g_backendFailed) return false;

    if (FAILED(sc->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&g_device))) || !g_device) {
        g_backendFailed = true;
        return false;
    }
    g_device->GetImmediateContext(&g_ctx);

    DXGI_SWAP_CHAIN_DESC desc{};
    if (!g_ctx || FAILED(sc->GetDesc(&desc)) || !desc.OutputWindow) {
        g_backendFailed = true;
        return false;
    }

    g_hwnd = desc.OutputWindow;
    g_swap = sc;

    ImGui::CreateContext();
    menu::Init();
    ui::Init(g_device, g_ctx);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    if (!ImGui_ImplWin32_Init(g_hwnd) || !ImGui_ImplDX11_Init(g_device, g_ctx)) {
        g_backendFailed = true;
        return false;
    }

    g_origWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WndProc)));

    g_backendReady = true;
    return true;
}

void RenderFrame(IDXGISwapChain* sc) {
    if (!g_backendReady && !InitBackend(sc)) return;

    EnsureTarget();
    if (!g_rtv) return;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ui::Draw();

    ImGui::Render();
    g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

HRESULT WINAPI hkPresent(IDXGISwapChain* sc, UINT syncInterval, UINT flags) {
    RenderFrame(sc);
    return g_origPresent(sc, syncInterval, flags);
}

HRESULT WINAPI hkPresent1(IDXGISwapChain1* sc, UINT syncInterval, UINT flags, const DXGI_PRESENT_PARAMETERS* p) {
    RenderFrame(sc);
    return g_origPresent1(sc, syncInterval, flags, p);
}

HRESULT WINAPI hkResize(IDXGISwapChain* sc, UINT bufferCount, UINT w, UINT h, DXGI_FORMAT fmt, UINT flags) {
    if (g_ctx) g_ctx->OMSetRenderTargets(0, nullptr, nullptr);
    ReleaseTarget();
    return g_origResize(sc, bufferCount, w, h, fmt, flags);
}

// The frame work runs here, before the game's own call, so it is inside the game
// thread and outside any Blueprint that could be mid-travel. g_inFrame stops a
// nested call (our own ProcessEvent traffic) from re-entering the frame.
void hkProcessEvent(void* obj, void* fn, void* params) {
    if (obj && fn && gt::Claim()) {
        bool idle = false;
        if (g_inFrame.compare_exchange_strong(idle, true)) {
            features::Frame();
            g_inFrame = false;
        }
    }
    g_origProcessEvent(obj, fn, params);
}

} // namespace

void Install() {
    if (MH_Initialize() != MH_OK) return;

    // Hooked before the swap chain probe: gameplay work must land on the game
    // thread even if the overlay backend never comes up.
    if (MH_CreateHook(reinterpret_cast<void*>(ue::Base() + ue::off::ProcessEvent),
                      reinterpret_cast<void*>(&hkProcessEvent),
                      reinterpret_cast<void**>(&g_origProcessEvent)) != MH_OK) {
        g_origProcessEvent = nullptr;
    }

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = DefWindowProcW;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"acc_trainer_probe";
    RegisterClassExW(&wc);

    HWND probe = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 64, 64,
                                 nullptr, nullptr, wc.hInstance, nullptr);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount       = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Width  = 64;
    sd.BufferDesc.Height = 64;
    sd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow      = probe;
    sd.SampleDesc.Count  = 1;
    sd.Windowed          = TRUE;
    sd.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain*      dummySwap = nullptr;
    ID3D11Device*        dummyDev  = nullptr;
    ID3D11DeviceContext* dummyCtx  = nullptr;
    D3D_FEATURE_LEVEL    level     = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                               D3D11_SDK_VERSION, &sd, &dummySwap, &dummyDev, &level, &dummyCtx);
    if (FAILED(hr)) {
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
                                           D3D11_SDK_VERSION, &sd, &dummySwap, &dummyDev, &level, &dummyCtx);
    }

    if (SUCCEEDED(hr) && dummySwap) {
        void** vt = *reinterpret_cast<void***>(dummySwap);
        MH_CreateHook(vt[kSwapPresent], reinterpret_cast<void*>(&hkPresent),
                      reinterpret_cast<void**>(&g_origPresent));
        MH_CreateHook(vt[kSwapResize], reinterpret_cast<void*>(&hkResize),
                      reinterpret_cast<void**>(&g_origResize));

        IDXGISwapChain1* sc1 = nullptr;
        if (SUCCEEDED(dummySwap->QueryInterface(__uuidof(IDXGISwapChain1), reinterpret_cast<void**>(&sc1))) && sc1) {
            void** vt1 = *reinterpret_cast<void***>(sc1);
            MH_CreateHook(vt1[kSwap1Present1], reinterpret_cast<void*>(&hkPresent1),
                          reinterpret_cast<void**>(&g_origPresent1));
            sc1->Release();
        }

        MH_EnableHook(MH_ALL_HOOKS);

        dummySwap->Release();
        if (dummyCtx) dummyCtx->Release();
        if (dummyDev) dummyDev->Release();
    }

    MH_EnableHook(MH_ALL_HOOKS);

    if (probe) DestroyWindow(probe);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

} // namespace hooks
