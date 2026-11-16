#pragma once
// Game-thread gate. UE4 refuses off-thread engine calls, and the failure is not
// graceful: a UFunction invoked from a worker thread runs overlap/Blueprint logic
// there, and a level travel started that way trips
//   Assertion failed: PlatformTLS::GetCurrentThreadId() == GGameThreadId
//   [Engine/Private/AudioThread.cpp:291]
// The id is learned from the first ProcessEvent the game itself makes, which is
// the game thread; every engine call is then refused on any other thread.

#include <windows.h>
#include <atomic>

namespace gt {

inline std::atomic<unsigned long>& ThreadId() {
    static std::atomic<unsigned long> id{0};
    return id;
}

// Returns true when the calling thread is the game thread, learning the id on the
// first call. Safest possible default: before an id exists, nothing is claimed.
inline bool Claim() {
    unsigned long expected = 0;
    const unsigned long me = GetCurrentThreadId();
    return ThreadId().compare_exchange_strong(expected, me) || expected == me;
}

inline bool Current() {
    const unsigned long id = ThreadId().load();
    return id == 0 || GetCurrentThreadId() == id;
}

} // namespace gt
