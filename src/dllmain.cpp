#include "features.hpp"
#include "hooks.hpp"

#include <windows.h>

namespace {

DWORD WINAPI Boot(LPVOID) {
    // Present is hooked first so the overlay is up before the first feature
    // write lands; the first real frame builds the ImGui backend.
    hooks::Install();
    features::Start();
    return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(mod);
        HANDLE t = CreateThread(nullptr, 0, &Boot, nullptr, 0, nullptr);
        if (t) CloseHandle(t);
    }
    return TRUE;
}
