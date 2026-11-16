#pragma once
// Realm exit-portal coordinates, extracted from the .umap dumps in
// "Maps Acceleracers Only". Regenerate with build/gen_realms.ps1.

namespace acc {

struct RealmPortal {
    const char* map;      // UWorld name of the realm package
    const char* name;     // display name
    float       x, y, z;  // portal actor root, world space of that map
    bool        hasCoords;
};

inline const RealmPortal kRealmPortals[] = {
    {"Realm_Blizzard", "Blizzard", 64741.457f, -147699.19f, 832.614f, true},
    {"Realm_Canyon", "Canyon", 77536.055f, 190135.62f, -2347.810f, true},
    {"Realm_Cavern", "Cavern", 280875.44f, -142968.69f, -17075.113f, true},
    {"Realm_Chrome", "Chrome", 86828.336f, -157478.75f, 57501.402f, true},
    {"Realm_Cliffside", "Cliffside", 19157.0f, 32167.0f, -154338.0f, true},
    {"Realm_Cosmic", "Cosmic", 378950.0f, 121940.0f, 187950.0f, true},
    {"Realm_Cybergrid", "Cybergrid", 27655.0f, 44960.0f, 14580.0f, true},
    {"Realm_Desert", "Desert", 83207.18f, -385333.8f, 10862.208f, true},
    {"Realm_Glass", "Glass", 591171.5f, 421441.12f, 7032.312f, true},
    {"Realm_Ice", "Ice", 351908.0f, 161083.0f, -97507.0f, true},
    {"Realm_Lava", "Lava", 0.f, 0.f, 0.f, false},  // dump had no actors
    {"Realm_Metro", "Metro", 451289.0f, -585377.0f, 2661.0f, true},
    {"Realm_Micro_Card", "Micro Card", 169071.95f, 931789.75f, -156841.2f, true},
    {"Realm_Monument", "Monument", 0.f, 0.f, 0.f, false},  // dump had no actors
    {"Realm_Pipeline", "Pipeline", 958771.75f, 192031.5f, -297515.0f, true},
    {"Realm_Reactor", "Reactor", -76044.945f, -17351.113f, -77753.21f, true},
    {"Realm_Sky", "Sky", -137096.77f, -95906.0f, 743446.75f, true},
    {"Realm_Solar", "Solar", -23594.365f, -220799.7f, -188287.16f, true},
    {"Realm_Sound", "Sound", 138965.0f, -42310.0f, -34685.0f, true},
    {"Realm_Storm", "Storm", 252431.89f, -820642.2f, 63799.742f, true},
    {"Realm_Swamp", "Swamp", 0.f, 0.f, 0.f, false},  // dump had no actors
    {"Realm_Warped", "Warped", -182738.64f, 18149.62f, 96766.17f, true},
    {"Realm_Water", "Water", -52668.61f, 36967.797f, 218723.5f, true},
};

constexpr int kRealmPortalCount = static_cast<int>(sizeof(kRealmPortals) / sizeof(kRealmPortals[0]));

} // namespace acc

