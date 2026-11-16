#pragma once

namespace hooks {

// Spins up the dummy device, installs the Present/ResizeBuffers detours and
// waits for the first real frame. Safe to call from a worker thread.
void Install();

} // namespace hooks
