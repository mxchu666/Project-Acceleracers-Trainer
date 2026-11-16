#pragma once
// Minimal UE4.27 reflection layer for Project Acceleracers.
// Signature block is taken verbatim from the Dumper-7 dump in
// "sdk latest\4.27.2-18319896+++UE4+Release-4.27-ProjectAcceleracer\Dumpspace\OffsetsInfo.json".

#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <cstring>

#include "gamethread.hpp"

namespace ue {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

namespace off {
constexpr std::uintptr_t GWorld       = 0x09D60268;
constexpr std::uintptr_t GNames       = 0x09B96C80;
constexpr std::uintptr_t GObjects     = 0x09BDD9B0;
constexpr std::uintptr_t ProcessEvent = 0x025EA5D0;
} // namespace off

struct FName     { u32 index; u32 number; };
struct FString   { wchar_t* data; i32 num; i32 max; };
struct FVector   { float x, y, z; };
struct FRotator  { float pitch, yaw, roll; };
struct FTimespan { i64 ticks; };

struct UObject {
    void** vtable;   // 0x00
    i32    flags;    // 0x08
    i32    index;    // 0x0C
    void*  cls;      // 0x10
    FName  name;     // 0x18
    void*  outer;    // 0x20
};

namespace lay {
// CoreUObject
constexpr std::size_t UField_Next         = 0x28;
constexpr std::size_t UStruct_SuperStruct = 0x40;
constexpr std::size_t UStruct_Children    = 0x48;
constexpr std::size_t UFunction_ParmsSize = 0xB6;
// Engine
constexpr std::size_t UWorld_PersistentLevel     = 0x30;
constexpr std::size_t UWorld_Levels              = 0x138;
constexpr std::size_t UWorld_OwningGameInstance  = 0x180;
constexpr std::size_t ULevel_Actors              = 0x98;
constexpr std::size_t UGameInstance_LocalPlayers = 0x38;
constexpr std::size_t UPlayer_PlayerController   = 0x30;
constexpr std::size_t AController_Pawn           = 0x250;
constexpr std::size_t AController_ControlRotation = 0x288;
constexpr std::size_t AActor_RootComponent       = 0x130;
constexpr std::size_t ULevel_OwningWorld         = 0xB8;
constexpr std::size_t UObject_Class              = 0x10;
constexpr std::size_t UObject_Name               = 0x18;
constexpr std::size_t UObject_Outer              = 0x20;
} // namespace lay

inline std::uintptr_t Base() {
    static const std::uintptr_t b = reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    return b;
}

// Every read/write is guarded: a stale pointer during a level transition must not take the process down.
inline bool SafeCopy(void* dst, const void* src, std::size_t n) {
    __try {
        std::memcpy(dst, src, n);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

template <class T>
inline bool Read(std::uintptr_t addr, T& out) {
    return addr != 0 && SafeCopy(&out, reinterpret_cast<const void*>(addr), sizeof(T));
}

template <class T>
inline bool Write(std::uintptr_t addr, const T& v) {
    return addr != 0 && SafeCopy(reinterpret_cast<void*>(addr), &v, sizeof(T));
}

inline bool Plausible(std::uintptr_t p) {
    return p > 0x10000 && p < 0x00007FFFFFFFFFFFULL && (p & 0x3) == 0;
}

// Decodes one FName against a given FNamePool base.
inline bool DecodeName(const FName& n, std::uintptr_t pool, char* out, std::size_t cap) {
    const u32 block  = n.index >> 16;
    const u32 offset = n.index & 0xFFFF;
    if (block >= 8192) return false;

    std::uintptr_t blockPtr = 0;
    if (!Read(pool + 0x10 + static_cast<std::uintptr_t>(block) * 8, blockPtr) || !Plausible(blockPtr)) return false;

    const std::uintptr_t entry = blockPtr + static_cast<std::uintptr_t>(offset) * 2;
    u16 header = 0;
    if (!Read(entry, header)) return false;

    const bool wide = (header & 1u) != 0;
    const int  len  = static_cast<int>(header >> 6);
    if (len <= 0 || static_cast<std::size_t>(len) >= cap) return false;

    if (wide) {
        wchar_t tmp[512];
        if (static_cast<std::size_t>(len) >= (sizeof(tmp) / sizeof(tmp[0]))) return false;
        if (!SafeCopy(tmp, reinterpret_cast<const void*>(entry + 2), static_cast<std::size_t>(len) * 2)) return false;
        for (int i = 0; i < len; ++i) out[i] = static_cast<char>(tmp[i] & 0x7F);
        out[len] = 0;
    } else {
        if (!SafeCopy(out, reinterpret_cast<const void*>(entry + 2), static_cast<std::size_t>(len))) return false;
        out[len] = 0;
    }
    return true;
}

// Dumper-7 reports OFFSET_GNAMES as the address of the FNamePool object itself,
// not as a pointer to it. Older builds store a pointer there instead, so both
// layouts are tried before a name is declared unreadable.
inline bool FNameToString(const FName& n, char* out, std::size_t cap) {
    if (cap == 0) return false;
    out[0] = 0;
    if (n.index == 0) return false;

    const std::uintptr_t direct = Base() + off::GNames;
    if (DecodeName(n, direct, out, cap)) return true;

    std::uintptr_t indirect = 0;
    if (Read(direct, indirect) && Plausible(indirect) && DecodeName(n, indirect, out, cap)) return true;

    out[0] = 0;
    return false;
}

// Leaves the caller's buffer untouched when the name cannot be resolved.
inline bool ObjectName(std::uintptr_t obj, char* out, std::size_t cap) {
    if (!obj || !out || cap == 0) return false;
    FName n{};
    if (!Read(obj + lay::UObject_Name, n)) return false;
    char tmp[512];
    if (!FNameToString(n, tmp, sizeof(tmp))) return false;
    std::size_t i = 0;
    for (; tmp[i] && i + 1 < cap; ++i) out[i] = tmp[i];
    out[i] = 0;
    return true;
}

inline std::uintptr_t ClassOf(std::uintptr_t obj) {
    std::uintptr_t k = 0;
    Read(obj + lay::UObject_Class, k);
    return Plausible(k) ? k : 0;
}

inline bool NameIs(std::uintptr_t obj, const char* wanted) {
    char buf[128];
    return obj && ObjectName(obj, buf, sizeof(buf)) && std::strcmp(buf, wanted) == 0;
}

inline bool ClassNameIs(std::uintptr_t obj, const char* wanted) {
    return NameIs(ClassOf(obj), wanted);
}

inline std::uintptr_t OuterOf(std::uintptr_t obj) {
    std::uintptr_t o = 0;
    if (!Read(obj + lay::UObject_Outer, o) || !Plausible(o)) return 0;
    return o;
}

inline std::uintptr_t OwningWorldOf(std::uintptr_t level) {
    std::uintptr_t w = 0;
    if (!Read(level + lay::ULevel_OwningWorld, w) || !Plausible(w)) return 0;
    return w;
}

// Actors are outered to the ULevel they were spawned into, so this reaches the
// gameplay world without trusting GWorld.
inline std::uintptr_t LevelOfActor(std::uintptr_t actor) {
    const std::uintptr_t o = OuterOf(actor);
    return NameIs(o, "Level") ? o : 0;
}

// Walks Children of the class and every super, comparing FName strings.
inline void* FindFunction(std::uintptr_t klass, const char* wanted) {
    char buf[192];
    for (int depth = 0; depth < 12 && Plausible(klass); ++depth) {
        std::uintptr_t child = 0;
        if (!Read(klass + lay::UStruct_Children, child)) return nullptr;

        int guard = 0;
        while (Plausible(child) && guard++ < 8192) {
            if (ObjectName(child, buf, sizeof(buf)) && std::strcmp(buf, wanted) == 0)
                return reinterpret_cast<void*>(child);
            std::uintptr_t next = 0;
            if (!Read(child + lay::UField_Next, next)) break;
            child = next;
        }

        std::uintptr_t super = 0;
        if (!Read(klass + lay::UStruct_SuperStruct, super)) return nullptr;
        klass = super;
    }
    return nullptr;
}

using ProcessEventFn = void (*)(void*, void*, void*);

// Params buffer must be at least the UFunction's ParmsSize and zero initialised.
inline bool Call(std::uintptr_t obj, void* fn, void* params) {
    if (!Plausible(obj) || !fn) return false;
    // Fail closed off the game thread: a stray call from a worker crashes the game
    // inside the engine, which is a far worse outcome than a dropped toggle.
    if (!gt::Current()) return false;
    auto pe = reinterpret_cast<ProcessEventFn>(Base() + off::ProcessEvent);
    if (!pe) return false;
    pe(reinterpret_cast<void*>(obj), fn, params);
    return true;
}

inline bool CallByName(std::uintptr_t obj, const char* fnName, void* params) {
    const std::uintptr_t klass = ClassOf(obj);
    if (!klass) return false;
    void* fn = FindFunction(klass, fnName);
    if (!fn) return false;
    return Call(obj, fn, params);
}

inline std::uintptr_t World() {
    std::uintptr_t w = 0;
    if (!Read(Base() + off::GWorld, w) || !Plausible(w)) return 0;
    return w;
}

inline std::uintptr_t GameInstance() {
    const std::uintptr_t w = World();
    if (!w) return 0;
    std::uintptr_t gi = 0;
    if (!Read(w + lay::UWorld_OwningGameInstance, gi) || !Plausible(gi)) return 0;
    return gi;
}

inline std::uintptr_t PersistentLevel(std::uintptr_t world) {
    std::uintptr_t lvl = 0;
    if (!world || !Read(world + lay::UWorld_PersistentLevel, lvl) || !Plausible(lvl)) return 0;
    return lvl;
}

inline std::uintptr_t Level() {
    return PersistentLevel(World());
}

inline int32_t ActorCount(std::uintptr_t level) {
    int32_t n = 0;
    if (!level || !Read(level + lay::ULevel_Actors + 8, n)) return 0;
    return (n > 0 && n < 1000000) ? n : 0;
}

inline std::uintptr_t ActorAt(std::uintptr_t level, int32_t i) {
    std::uintptr_t arr = 0;
    if (!Read(level + lay::ULevel_Actors, arr) || !Plausible(arr)) return 0;
    std::uintptr_t a = 0;
    if (!Read(arr + static_cast<std::uintptr_t>(i) * 8, a) || !Plausible(a)) return 0;
    return a;
}

inline int32_t LevelCount(std::uintptr_t world) {
    int32_t n = 0;
    if (!world || !Read(world + lay::UWorld_Levels + 8, n)) return 0;
    return (n > 0 && n < 4096) ? n : 0;
}

inline std::uintptr_t LevelAt(std::uintptr_t world, int32_t i) {
    std::uintptr_t arr = 0;
    if (!Read(world + lay::UWorld_Levels, arr) || !Plausible(arr)) return 0;
    std::uintptr_t lvl = 0;
    if (!Read(arr + static_cast<std::uintptr_t>(i) * 8, lvl) || !Plausible(lvl)) return 0;
    return lvl;
}

template <class Fn>
inline void ForEachActorInLevel(std::uintptr_t level, Fn&& fn) {
    const int32_t n = ActorCount(level);
    for (int32_t i = 0; i < n; ++i) {
        const std::uintptr_t a = ActorAt(level, i);
        if (a) fn(a);
    }
}

// A realm's actors live in streaming levels, not in the persistent level, so the
// whole UWorld::Levels array is walked. PersistentLevel is only used as a fallback
// because it is normally a member of Levels too, and a second visit would make a
// sole-portal map look like a two-portal one.
template <class Fn>
inline void ForEachActor(std::uintptr_t world, Fn&& fn) {
    if (!world) return;
    const int32_t levels = LevelCount(world);
    for (int32_t i = 0; i < levels; ++i) {
        const std::uintptr_t lvl = LevelAt(world, i);
        if (lvl) ForEachActorInLevel(lvl, fn);
    }
    if (levels == 0) {
        const std::uintptr_t lvl = PersistentLevel(world);
        if (lvl) ForEachActorInLevel(lvl, fn);
    }
}

inline std::uintptr_t LocalPlayerController() {
    const std::uintptr_t gi = GameInstance();
    if (!gi) return 0;
    std::uintptr_t arr = 0;
    i32 num = 0;
    if (!Read(gi + lay::UGameInstance_LocalPlayers, arr) || !Read(gi + lay::UGameInstance_LocalPlayers + 8, num)) return 0;
    if (!Plausible(arr) || num <= 0 || num > 8) return 0;
    std::uintptr_t lp = 0;
    if (!Read(arr, lp) || !Plausible(lp)) return 0;
    std::uintptr_t pc = 0;
    if (!Read(lp + lay::UPlayer_PlayerController, pc) || !Plausible(pc)) return 0;
    return pc;
}

inline std::uintptr_t PlayerPawn() {
    const std::uintptr_t pc = LocalPlayerController();
    if (!pc) return 0;
    std::uintptr_t pawn = 0;
    if (!Read(pc + lay::AController_Pawn, pawn) || !Plausible(pawn)) return 0;
    return pawn;
}

// GWorld can still hold the previous map while a travel is in flight, and this
// build hands back an object that is not always the gameplay world. The car's own
// level is asked first because that is where the car was actually spawned.
inline std::uintptr_t PlayWorld() {
    const std::uintptr_t pawn = PlayerPawn();
    const std::uintptr_t lvl  = pawn ? LevelOfActor(pawn) : 0;
    const std::uintptr_t w    = lvl ? OwningWorldOf(lvl) : 0;
    if (w && NameIs(w, "World")) return w;

    const std::uintptr_t g = World();
    if (g && NameIs(g, "World")) return g;
    return g;
}

} // namespace ue
