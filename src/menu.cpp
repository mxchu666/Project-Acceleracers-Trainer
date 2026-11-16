#include "menu.hpp"
#include "features.hpp"
#include "themes.hpp"

#include <imgui.h>
#include <unordered_map>
#include <cstdio>
#include <cctype>

namespace menu {
namespace {

// The menu draws against the live palette in theme::; the alias keeps the
// original c:: spelling of every reference below.
namespace c = theme;

ImFont* g_title = nullptr;
ImFont* g_body  = nullptr;

int g_tab = 0;

std::unordered_map<ImGuiID, float> g_knob;

// Draws text centred vertically on the current line; ImGui's own layout is
// then advanced by the caller.
void TextAt(ImDrawList* dl, ImVec2 p, ImU32 col, const char* t, ImFont* f = nullptr, float size = 0.f) {
    if (f) dl->AddText(f, size > 0.f ? size : f->FontSize, p, col, t);
    else   dl->AddText(p, col, t);
}

bool ToggleRow(const char* label, bool* v, const char* hint = nullptr) {
    ImGui::PushID(label);
    const float  h   = hint ? 40.f : 28.f;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float  w   = ImGui::GetContentRegionAvail().x;

    ImGui::InvisibleButton("##row", ImVec2(w, h));
    bool changed = false;
    if (ImGui::IsItemClicked()) {
        *v = !*v;
        changed = true;
    }
    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float ty = pos.y + (hint ? 3.f : 6.f);
    dl->AddText(ImVec2(pos.x + 2.f, ty), hovered ? c::text : c::dim, label);
    if (hint) dl->AddText(ImVec2(pos.x + 2.f, ty + 16.f), c::muted, hint);

    const float  sw = 40.f, sh = 20.f;
    const ImVec2 sp(pos.x + w - sw - 6.f, pos.y + (h - sh) * 0.5f);
    dl->AddRectFilled(sp, ImVec2(sp.x + sw, sp.y + sh), *v ? c::accent : c::track, sh * 0.5f);

    float& a = g_knob[ImGui::GetID("##knob")];
    a += ((*v ? 1.f : 0.f) - a) * 0.28f;

    const float kw = sh - 4.f;
    const float kx = sp.x + 2.f + a * (sw - kw - 4.f);
    dl->AddCircleFilled(ImVec2(kx + kw * 0.5f, sp.y + sh * 0.5f), kw * 0.5f, IM_COL32(255, 255, 255, 255));

    ImGui::PopID();
    return changed;
}

bool ActionButton(const char* label, const char* hint) {
    ImGui::PushID(label);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float  w   = ImGui::GetContentRegionAvail().x;
    const float  h   = 44.f;

    ImGui::InvisibleButton("##btn", ImVec2(w, h));
    const bool clicked = ImGui::IsItemClicked();
    const bool hovered = ImGui::IsItemHovered();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h),
                      hovered ? c::panel2 : c::panel, 8.f);
    if (hovered)
        dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h), c::accent, 8.f, 0, 1.5f);

    dl->AddText(ImVec2(pos.x + 12.f, pos.y + 7.f), hovered ? c::text : c::dim, label);
    if (hint) dl->AddText(ImVec2(pos.x + 12.f, pos.y + 24.f), c::muted, hint);

    ImGui::PopID();
    return clicked;
}

void SectionLabel(const char* t) {
    ImGui::Dummy(ImVec2(0, 4));
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float  w = ImGui::GetContentRegionAvail().x;

    char upper[96];
    int  n = 0;
    for (; t[n] && n < 95; ++n) upper[n] = static_cast<char>(toupper(static_cast<unsigned char>(t[n])));
    upper[n] = 0;

    dl->AddText(p, c::muted, upper);
    const float tw = ImGui::CalcTextSize(upper).x;
    dl->AddLine(ImVec2(p.x + tw + 10.f, p.y + 7.f), ImVec2(p.x + w, p.y + 7.f), c::stroke, 1.f);
    ImGui::Dummy(ImVec2(0, 14));
}

void Tabs() {
    const char* names[4] = {"PLAYER", "PROGRESSION", "WORLD", "THEME"};
    const float w        = ImGui::GetContentRegionAvail().x / 4.f;
    ImDrawList* dl       = ImGui::GetWindowDrawList();

    for (int i = 0; i < 4; ++i) {
        ImGui::PushID(i);
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##tab", ImVec2(w - 4.f, 34.f));
        if (ImGui::IsItemClicked()) g_tab = i;

        const bool   on = (g_tab == i);
        const ImU32  tc = on ? c::text : (ImGui::IsItemHovered() ? c::dim : c::muted);
        const float  tw = ImGui::CalcTextSize(names[i]).x;
        dl->AddText(ImVec2(pos.x + (w - 4.f - tw) * 0.5f, pos.y + 8.f), tc, names[i]);
        if (on)
            dl->AddRectFilled(ImVec2(pos.x + 10.f, pos.y + 32.f), ImVec2(pos.x + w - 14.f, pos.y + 34.f),
                              c::accent, 1.f);
        ImGui::PopID();
        if (i < 3) ImGui::SameLine(0.f, 4.f);
    }
}

void PlayerTab() {
    SectionLabel("vehicle");
    ToggleRow("Unlimited Nitrox", &g_cfg.unlimitedNitrox, "Refills Nitrox every frame");
    ToggleRow("Unlimited Jump Jets", &g_cfg.unlimitedJumpJets, "Accelecharger uses topped up, cooldowns cleared");
    ToggleRow("Perfect Handling", &g_cfg.perfectHandling, "Maxes tire friction on every wheel");

    ImGui::BeginDisabled(!g_cfg.perfectHandling);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110.f);
    ImGui::SliderFloat("##grip", &g_cfg.grip, 2.f, 30.f, "Grip %.1f");
    ImGui::EndDisabled();

    SectionLabel("accelecharger");
    ToggleRow("Accelecharger Charges", &g_cfg.accelechargerBoost, "Charges left on the equipped Accelecharger");
    ImGui::BeginDisabled(!g_cfg.accelechargerBoost);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110.f);
    ImGui::SliderInt("##uses", &g_cfg.accelechargerUses, 1, 999, "Charges %d");
    ImGui::EndDisabled();

    SectionLabel("engine");
    ToggleRow("Engine HP Boost", &g_cfg.engineBoost, "Engine health points, clears EngineBlown");
    ImGui::BeginDisabled(!g_cfg.engineBoost);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110.f);
    ImGui::SliderFloat("##ehp", &g_cfg.engineHp, 100.f, 20000.f, "HP %.0f");
    ImGui::EndDisabled();

    SectionLabel("movement");
    ToggleRow("Noclip", &g_cfg.noclip, "WASD + Space / Ctrl, teleports the car");
    ImGui::BeginDisabled(!g_cfg.noclip);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110.f);
    ImGui::SliderFloat("##ns", &g_cfg.noclipSpeed, 300.f, 8000.f, "Speed %.0f");
    ImGui::EndDisabled();

    SectionLabel("hazards");
    ToggleRow("God Mode", &g_cfg.godMode, "Immune to Storm lightning, spikes and wind push");
    ToggleRow("Hydro Tires: No Water Slow", &g_cfg.hydroWater, "Water volumes stop dragging the car");
    ToggleRow("Spike Tires: No Sand Stall", &g_cfg.noSandStall, "Keeps forward speed when Desert sand grabs");
    ImGui::BeginDisabled(!g_cfg.noSandStall);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110.f);
    ImGui::SliderFloat("##sand", &g_cfg.sandFloor, 200.f, 3000.f, "Floor %.0f");
    ImGui::EndDisabled();
}

void ProgressionTab() {
    SectionLabel("unlocks");
    if (ActionButton("Unlock All Cars", "Every ECarList entry 0-112"))     features::Queue(ACT_UnlockCars);
    if (ActionButton("Unlock All AcceleChargers", "Every EAccelechargers 0-16")) features::Queue(ACT_UnlockAccel);
    if (ActionButton("Save Progress", "Writes through the game's own Save_Game")) features::Queue(ACT_SaveProgress);

    SectionLabel("realm records");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110.f);
    ImGui::InputFloat("##rec", &g_cfg.recordSeconds, 0.1f, 1.f, "%.2f s");
    if (ActionButton("Set All Realm Records", "Calls SaveGameTime for all 47 realms")) features::Queue(ACT_SetRecords);
}

void WorldTab() {
    const std::uintptr_t car = acc::LocalCar();
    const std::uintptr_t gi  = ue::GameInstance();

    const acc::RealmProbeData rp       = acc::Probe();
    const int                 realmIdx = rp.realm;

    SectionLabel("realm");
    ImGui::PushStyleColor(ImGuiCol_Text, realmIdx >= 0 ? c::cyan : c::muted);
    ImGui::TextUnformatted(realmIdx >= 0 ? "In Realm" : "Acceledrome / Menu");
    ImGui::PopStyleColor();

    char world[192] = "not identified";
    char lvlOut[192] = {};
    if (rp.portal) acc::ReadFNameAt(rp.portal, acc::off::Portal_LevelOut, lvlOut, sizeof(lvlOut));

    if (realmIdx >= 0) {
        std::snprintf(world, sizeof(world), "%s", acc::kRealmPortals[realmIdx].name);
    } else if (lvlOut[0]) {
        std::snprintf(world, sizeof(world), "Portal_C LevelOut=%s%s", lvlOut, rp.hub ? "  (hub)" : "");
    } else if (!ue::ObjectName(rp.world, world, sizeof(world))) {
        std::snprintf(world, sizeof(world), "%s",
                      rp.portals ? "portal without LevelOut" : "no Portal_C in any loaded level");
    }

    char line[224];
    std::snprintf(line, sizeof(line), "Map    %s", world);
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    ue::FVector target{};
    bool        fromTable = false;
    // The button only needs a portal, not a name match: parking the car in the
    // exit portal ends the realm even if the map name never resolved.
    const bool  haveTarget = acc::RealmPortalTarget(target, &fromTable);

    if (haveTarget) {
        std::snprintf(line, sizeof(line), "Portal %.0f / %.0f / %.0f  (%s)",
                      target.x, target.y, target.z, fromTable ? "dump" : "live");
    } else {
        std::snprintf(line, sizeof(line), "Portal %s", realmIdx < 0 ? "no realm loaded" : "unresolved");
    }
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    ImGui::Dummy(ImVec2(0, 4));
    ImGui::BeginDisabled(!haveTarget);
    if (ActionButton("Complete Current Realm", "Parks the car inside the realm's exit portal"))
        features::Queue(ACT_FinishRealm);
    ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(0, 6));
    if (ImGui::CollapsingHeader("Racing Realms")) {
        ImGui::BeginChild("##realmlist", ImVec2(0, 168.f), true);
        for (int i = 0; i < acc::kRealmPortalCount; ++i) {
            const acc::RealmPortal& r = acc::kRealmPortals[i];
            const bool current = (i == realmIdx);

            ImGui::PushID(i);
            if (!current) ImGui::BeginDisabled();
            if (ImGui::Selectable(r.name, false) && current && acc::RealmPortalTarget(target))
                features::Queue(ACT_FinishRealm);
            if (!current) ImGui::EndDisabled();

            ImGui::SameLine(150.f);
            if (!r.hasCoords)                            ImGui::TextColored(ImVec4(0.42f, 0.42f, 0.50f, 1.f), "live");
            else if (current)                            ImGui::TextColored(ImVec4(0.13f, 0.83f, 0.93f, 1.f), "ACTIVE");
            else                                         ImGui::TextColored(ImVec4(0.33f, 0.33f, 0.40f, 1.f), "%.0f / %.0f", r.x, r.y);
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    SectionLabel("state");
    if (car) {
        char cls[128] = "?";
        ue::ObjectName(ue::ClassOf(car), cls, sizeof(cls));
        std::snprintf(line, sizeof(line), "Pawn   %s", cls);
    } else {
        std::snprintf(line, sizeof(line), "Pawn   not found");
    }
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    // Raw reflection state. If a realm stops resolving, these numbers say which
    // link broke: name pool, world, level list, actor list or portal count.
    char wname[128] = "?";
    char wcls[128]  = "?";
    const std::uintptr_t gworld = ue::World();
    ue::ObjectName(rp.world, wname, sizeof(wname));
    ue::ObjectName(ue::ClassOf(gworld), wcls, sizeof(wcls));

    std::snprintf(line, sizeof(line), "World  %s  \"%s\"", rp.world ? "ok" : "null", wname);
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    std::snprintf(line, sizeof(line), "GWorld %s  class %s", gworld ? "ok" : "null", wcls);
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    std::snprintf(line, sizeof(line), "Levels %d   Actors %d   Portals %d",
                  rp.levels, rp.actors, rp.portals);
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    std::snprintf(line, sizeof(line), "Inst   %s", gi ? "MyGameInstance_C" : "null");
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    uint8_t flag = 0;
    if (gi) ue::Read(gi + acc::off::GI_IsInRealm, flag);
    std::snprintf(line, sizeof(line), "InRealm flag %d", static_cast<int>(flag));
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    // The game thread is learned from the first ProcessEvent; if this ever shows
    // equal ids, the claim went to the wrong thread and engine calls are refused.
    std::snprintf(line, sizeof(line), "GT %lu   Render %lu",
                  static_cast<unsigned long>(gt::ThreadId().load()),
                  static_cast<unsigned long>(GetCurrentThreadId()));
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);

    if (ImGui::CollapsingHeader("Loaded Levels")) {
        const int32_t levels = ue::LevelCount(rp.world);
        for (int32_t i = 0; i < levels && i < 12; ++i) {
            const std::uintptr_t level = ue::LevelAt(rp.world, i);
            char lname[128] = "?";
            ue::ObjectName(level, lname, sizeof(lname));
            std::snprintf(line, sizeof(line), "%2d  %s  actors %d", i, lname, ue::ActorCount(level));
            ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);
        }
        if (levels == 0)
            ImGui::TextColored(ImVec4(0.83f, 0.33f, 0.33f, 1.f), "UWorld::Levels empty");

        const std::uintptr_t persistent = ue::PersistentLevel(rp.world);
        char pname[128] = "?";
        ue::ObjectName(persistent, pname, sizeof(pname));
        std::snprintf(line, sizeof(line), "PL %s  actors %d", pname, ue::ActorCount(persistent));
        ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);
    }

    const acc::CarStats st = acc::ReadCarStats(car);
    if (st.valid)
        std::snprintf(line, sizeof(line), "Nitrox %.0f / %.0f   Uses %d", st.nitrox, st.maxNitrox, st.uses);
    else
        std::snprintf(line, sizeof(line), "Nitrox n/a");
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f), "%s", line);
}

void ThemeTab() {
    SectionLabel("interface");
    for (int i = 0; i < theme::Count; ++i) {
        const theme::Palette& p = theme::kPalettes[i];

        ImGui::PushID(i);
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const float  w   = ImGui::GetContentRegionAvail().x;
        const float  h   = 48.f;

        ImGui::InvisibleButton("##theme", ImVec2(w, h));
        const bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked())
            theme::Apply(i);

        const bool  on = (theme::Current() == i);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h),
                          on ? c::panel2 : c::panel, ImGui::GetStyle().ChildRounding);
        dl->AddRect(pos, ImVec2(pos.x + w, pos.y + h),
                    on ? c::accent : (hovered ? c::track : c::stroke),
                    ImGui::GetStyle().ChildRounding, 0, on ? 1.5f : 1.f);

        dl->AddText(ImVec2(pos.x + 12.f, pos.y + 7.f), on ? c::text : c::dim, p.name);
        dl->AddText(ImVec2(pos.x + 12.f, pos.y + 25.f), c::muted, p.credit);

        // swatches: accent, panel, track, text
        float sx = pos.x + w - 16.f;
        const ImU32 sw[4] = {p.accent, p.panel, p.track, p.text};
        for (int s = 0; s < 4; ++s) {
            dl->AddCircleFilled(ImVec2(sx, pos.y + h * 0.5f), 5.5f, sw[s]);
            dl->AddCircle(ImVec2(sx, pos.y + h * 0.5f), 5.5f, c::stroke, 12);
            sx -= 17.f;
        }
        if (on)
            dl->AddText(ImVec2(pos.x + w - 92.f, pos.y + 7.f), c::accent, "ACTIVE");

        ImGui::PopID();
    }

    SectionLabel("info");
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f),
                       "Palette sources audited from the imGUIS packs.");
    ImGui::TextColored(ImVec4(0.49f, 0.50f, 0.57f, 1.f),
                       "Applying rebuilds the ImGui style instantly.");
}

} // namespace

void Init() {
    theme::Apply(0);

    ImGuiIO& io = ImGui::GetIO();
    g_body  = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 15.0f);
    g_title = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 19.0f);
    if (!g_body)  g_body  = io.Fonts->AddFontDefault();
    if (!g_title) g_title = g_body;

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
}

void Draw() {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImVec2 size(480.f, 566.f);
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + 60.f, vp->WorkPos.y + 60.f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##acc_trainer", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
    ImGui::PopStyleVar();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImGui::GetWindowPos();

    // ---- title bar
    const float rounding = ImGui::GetStyle().WindowRounding;
    dl->AddRectFilled(origin, ImVec2(origin.x + size.x, origin.y + 62.f), c::titleBg, rounding,
                      ImDrawFlags_RoundCornersTop);
    dl->AddRectFilled(origin, ImVec2(origin.x + 4.f, origin.y + 62.f), c::accent, 2.f, ImDrawFlags_RoundCornersTopLeft);

    ImGui::SetCursorScreenPos(ImVec2(origin.x + 22.f, origin.y + 12.f));
    ImGui::PushFont(g_title);
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(c::accent), "ACCELERACERS");
    ImGui::PopFont();

    dl->AddText(ImVec2(origin.x + 22.f, origin.y + 36.f), c::muted, "PROJECT ACCELERACERS  /  TRAINER");

    dl->AddCircleFilled(ImVec2(origin.x + size.x - 24.f, origin.y + 26.f), 4.f,
                        ue::GameInstance() ? c::cyan : c::muted);

    // ---- tabs
    ImGui::SetCursorScreenPos(ImVec2(origin.x + 16.f, origin.y + 72.f));
    ImGui::BeginGroup();
    Tabs();
    ImGui::EndGroup();

    // ---- body
    ImGui::SetCursorScreenPos(ImVec2(origin.x + 18.f, origin.y + 118.f));
    ImGui::BeginChild("##body", ImVec2(size.x - 36.f, size.y - 168.f), false,
                      ImGuiWindowFlags_NoScrollbar);
    if      (g_tab == 0) PlayerTab();
    else if (g_tab == 1) ProgressionTab();
    else if (g_tab == 2) WorldTab();
    else                 ThemeTab();
    ImGui::EndChild();

    // ---- footer
    dl->AddLine(ImVec2(origin.x + 18.f, origin.y + size.y - 44.f),
                ImVec2(origin.x + size.x - 18.f, origin.y + size.y - 44.f), c::stroke, 1.f);
    dl->AddText(ImVec2(origin.x + 22.f, origin.y + size.y - 32.f), c::muted, "INSERT  toggle menu");
    dl->AddText(ImVec2(origin.x + size.x - 118.f, origin.y + size.y - 32.f), c::accent, "ofx");

    ImGui::End();
}

bool WantsInput() { return g_menuOpen; }

} // namespace menu
