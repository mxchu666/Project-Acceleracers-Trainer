#pragma once
#include "game.hpp"

struct Settings {
    bool  unlimitedNitrox   = false;
    bool  unlimitedJumpJets = false;
    bool  perfectHandling   = false;
    bool  noclip            = false;
    bool  godMode           = false;
    bool  hydroWater        = false;
    bool  noSandStall       = false;
    bool  accelechargerBoost = false;
    bool  engineBoost        = false;
    int   accelechargerUses  = 99;
    float engineHp           = 1500.0f;
    float grip              = 10.0f;
    float noclipSpeed       = 2200.0f;
    float sandFloor         = 800.0f;
    float recordSeconds     = 1.0f;
};

// Queued one-shot actions. The menu only sets bits; the game thread runs them
// from the ProcessEvent hook, because UE4 refuses engine calls made off it.
enum ActionBits : unsigned {
    ACT_UnlockCars    = 1u << 0,
    ACT_UnlockAccel   = 1u << 1,
    ACT_SaveProgress  = 1u << 2,
    ACT_SetRecords    = 1u << 3,
    ACT_FinishRealm   = 1u << 4,
};

extern Settings           g_cfg;
extern volatile LONG      g_running;
extern volatile bool      g_menuOpen;

namespace features {
void Start();
void Stop();
void Queue(unsigned bits);
void Frame();   // called from the game thread only
} // namespace features
