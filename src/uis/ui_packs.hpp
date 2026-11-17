#pragma once

// Uniform per-pack interface: every adapter loads only that pack's fonts into
// the (already cleared) atlas and draws its menu. The selector lives in
// ui_manager.

namespace uinlcs2    { void BuildFonts(); void Draw(); }
namespace uinllegacy { void BuildFonts(); void Draw(); }
namespace uionetap   { void BuildFonts(); void Draw(); }
namespace uifatal    { void BuildFonts(); void Draw(); }
