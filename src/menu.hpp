#pragma once

struct ImFont;

namespace menu {

void Init();
void Draw();
bool WantsInput();

// Loads the default fonts into the current atlas (which must already be
// cleared — the ui manager drives atlas switching). TitleFont() returns the
// bold face BuildFonts() registered.
void BuildFonts();
ImFont* TitleFont();

} // namespace menu
