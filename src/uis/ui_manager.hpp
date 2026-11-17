#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace ui {

// Which renderer owns the overlay. The four pack menus are the vendored
// imGUIS UIs; Default is the trainer's own menu.
enum class Kind {
    Default,        // trainer menu (theme system)
    NeverloseCS2,   // neverlose.cc ImGui - CS2 build (rar pack)
    NeverloseLegacy,// neverlose.cc ImGui - CSGO:Legacy build (zip pack)
    Onetap,         // onetap ui remake
    Fatal,          // project-139 / FatalUI
    Count
};

const char* Name(Kind k);
const char* Source(Kind k);   // which imGUIS pack it came from

// Creates the ImGui context wiring. Call once after ImGui::CreateContext().
// Applies the default theme and builds the Default font atlas.
void Init(ID3D11Device* device, ID3D11DeviceContext* context);

// INSERT - show/hide the active UI (the HOME selector stays independent).
void Toggle();

// HOME - open/close the renderer selector overlay.
void OpenSelector();

// Switches the active renderer (also used by the Default menu's UI tab).
void Select(Kind k);

// The renderer currently driving the overlay.
Kind Current();

// True while any overlay (active UI or selector) wants mouse/keyboard.
bool WantsInput();

// Draws the active UI and the selector overlay when open.
void Draw();

} // namespace ui
