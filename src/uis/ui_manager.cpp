#include "ui_manager.hpp"
#include "ui_packs.hpp"
#include "ui_assets.hpp"

#include "../menu.hpp"
#include "../features.hpp"
#include "../themes.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>

namespace ui {
namespace {

ID3D11Device*        g_device = nullptr;
ID3D11DeviceContext* g_ctx    = nullptr;

Kind g_active   = Kind::Default;
bool g_selector = false;

struct Entry { const char* name; const char* source; };
const Entry kEntries[] = {
    {"Trainer (default)",    "AcceleracersTrainer"},
    {"Neverlose  -  CS2",    "neverlose.cc ImGui / CS2 pack"},
    {"Neverlose  -  Legacy", "neverlose.cc ImGui / CSGO:Legacy pack"},
    {"Onetap",               "onetap ui remake"},
    {"Fatal",                "project-139 / FatalUI"},
};
static_assert(sizeof(kEntries) / sizeof(kEntries[0]) == static_cast<int>(Kind::Count));

// Restores the style the selected UI expects, then rebuilds the font atlas.
// The pack adapters append only their own fonts to the cleared atlas, so the
// io.Fonts->Fonts[...] indices the vendored code relies on hold.
void RebuildAtlas(Kind k) {
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplDX11_InvalidateDeviceObjects();
    io.Fonts->Clear();

    switch (k) {
    case Kind::Default:         menu::BuildFonts();      break;
    case Kind::NeverloseCS2:    uinlcs2::BuildFonts();    break;
    case Kind::NeverloseLegacy: uinllegacy::BuildFonts(); break;
    case Kind::Onetap:          uionetap::BuildFonts();   break;
    case Kind::Fatal:           uifatal::BuildFonts();    break;
    default: break;
    }

    io.Fonts->Build();
    io.Fonts->GetTexDataAsRGBA32(nullptr, nullptr, nullptr, nullptr); // force upload
    ImGui_ImplDX11_CreateDeviceObjects();

    // Style: packs expect a dark base plus their own overrides; Default hands
    // control back to the theme system.
    if (k == Kind::Default) {
        theme::Apply(theme::Current());
    } else {
        ImGui::StyleColorsDark();
        if (k == Kind::Onetap) {
            // onetap main.cpp 128-136
            ImGuiStyle& s = ImGui::GetStyle();
            s.Colors[ImGuiCol_WindowBg]      = ImVec4(0.035f, 0.035f, 0.035f, 1.0f);
            s.Colors[ImGuiCol_PopupBg]       = ImVec4(0.055f, 0.055f, 0.055f, 1.0f);
            s.Colors[ImGuiCol_Header]        = ImVec4(0.20f, 0.20f, 0.24f, 1.0f);
            s.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.34f, 1.0f);
            s.Colors[ImGuiCol_HeaderActive]  = ImVec4(0.33f, 0.32f, 0.42f, 1.0f);
            s.AntiAliasedLines       = false;
            s.AntiAliasedLinesUseTex = false;
            s.AntiAliasedFill        = false;
            s.Alpha                  = 1.0f;
        }
    }
}

void DrawSelector() {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f - 190.f,
                                   vp->WorkPos.y + vp->WorkSize.y * 0.5f - 130.f),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(380.f, 250.f), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(theme::panel));
    ImGui::PushStyleColor(ImGuiCol_Border,   ImGui::ColorConvertU32ToFloat4(theme::stroke));
    ImGui::PushStyleColor(ImGuiCol_Text,     ImGui::ColorConvertU32ToFloat4(theme::text));
    ImGui::Begin("##ui_selector", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    ImDrawList* dl     = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImGui::GetWindowPos();
    const float  width  = ImGui::GetWindowSize().x;

    dl->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + 44.f), theme::titleBg);
    dl->AddRectFilled(origin, ImVec2(origin.x + 4.f, origin.y + 44.f), theme::accent);

    ImGui::SetCursorScreenPos(ImVec2(origin.x + 18.f, origin.y + 11.f));
    ImGui::PushFont(menu::TitleFont());
    ImGui::TextUnformatted("RENDERER");
    ImGui::PopFont();

    ImGui::SetCursorScreenPos(ImVec2(origin.x + 10.f, origin.y + 54.f));
    ImGui::BeginGroup();
    for (int i = 0; i < static_cast<int>(Kind::Count); ++i) {
        ImGui::PushID(i);
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float  w   = width - 20.f;
        const float  h   = 40.f;

        ImGui::InvisibleButton("##ui", ImVec2(w, h));
        if (ImGui::IsItemClicked()) {
            Select(static_cast<Kind>(i));
            g_selector = false;
        }

        const bool  on      = (g_active == static_cast<Kind>(i));
        const bool  hovered = ImGui::IsItemHovered();
        ImDrawList* d       = ImGui::GetWindowDrawList();

        d->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h),
                         on ? theme::panel2 : theme::panel, 6.f);
        d->AddRect(pos, ImVec2(pos.x + w, pos.y + h),
                   on ? theme::accent : (hovered ? theme::track : theme::stroke),
                   6.f, 0, on ? 1.5f : 1.f);
        d->AddText(ImVec2(pos.x + 12.f, pos.y + 6.f),  on ? theme::text : theme::dim,
                   kEntries[i].name);
        d->AddText(ImVec2(pos.x + 12.f, pos.y + 22.f), theme::muted, kEntries[i].source);
        if (on)
            d->AddText(ImVec2(pos.x + w - 66.f, pos.y + 12.f), theme::accent, "ACTIVE");
        ImGui::PopID();
    }
    ImGui::EndGroup();

    ImGui::SetCursorScreenPos(ImVec2(origin.x + 18.f, origin.y + 250.f - 22.f));
    dl->AddText(ImGui::GetCursorScreenPos(), theme::muted,
                "HOME  selector      INSERT  toggle menu");
    ImGui::End();
}

} // namespace

const char* Name(Kind k)   { return kEntries[static_cast<int>(k)].name; }
const char* Source(Kind k) { return kEntries[static_cast<int>(k)].source; }

ID3D11Device* D3DDevice() { return g_device; }

void Init(ID3D11Device* device, ID3D11DeviceContext* context) {
    g_device = device;
    g_ctx    = context;

    menu::Init();          // theme + io config
    RebuildAtlas(Kind::Default);
}

void Select(Kind k) {
    if (k == g_active) return;
    g_active = k;
    RebuildAtlas(k);
}

Kind Current() {
    return g_active;
}

void Toggle() {
    g_menuOpen = !g_menuOpen;
    if (!g_menuOpen) g_selector = false;
}

void OpenSelector() {
    g_selector = !g_selector;
}

bool WantsInput() {
    return g_menuOpen || g_selector;
}

void Draw() {
    if (WantsInput())
        ClipCursor(nullptr);

    if (g_menuOpen) {
        switch (g_active) {
        case Kind::Default:         menu::Draw();       break;
        case Kind::NeverloseCS2:    uinlcs2::Draw();    break;
        case Kind::NeverloseLegacy: uinllegacy::Draw(); break;
        case Kind::Onetap:          uionetap::Draw();   break;
        case Kind::Fatal:           uifatal::Draw();    break;
        default: break;
        }
    }

    if (g_selector)
        DrawSelector();
}

} // namespace ui
