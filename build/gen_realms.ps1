param(
    [string]$Dump = "F:\Project Acceleracers Trainer\AcceleracersTrainer\build\realm_dump.json",
    [string]$Out  = "F:\Project Acceleracers Trainer\AcceleracersTrainer\src\realms.hpp"
)

$rows = Get-Content $Dump -Raw | ConvertFrom-Json
$portals = $rows | Where-Object { $_.type -eq 'Portal_C' }

# one entry per realm package; duplicate .umap dumps collapse onto the same LevelOut
$byLevel = @{}
foreach ($p in $portals) {
    if (-not $byLevel.ContainsKey($p.levelOut)) { $byLevel[$p.levelOut] = $p }
}

# maps whose .umap dump carried no actor data: coords unknown, runtime actor scan handles them
foreach ($m in @('Realm_Lava', 'Realm_Monument', 'Realm_Swamp')) {
    if (-not $byLevel.ContainsKey($m)) { $byLevel[$m] = $null }
}

$lines = New-Object System.Collections.Generic.List[string]
$lines.Add('#pragma once')
$lines.Add('// Realm exit-portal coordinates, extracted from the .umap dumps in')
$lines.Add('// "Maps Acceleracers Only". Regenerate with build/gen_realms.ps1.')
$lines.Add('')
$lines.Add('namespace acc {')
$lines.Add('')
$lines.Add('struct RealmPortal {')
$lines.Add('    const char* map;      // UWorld name of the realm package')
$lines.Add('    const char* name;     // display name')
$lines.Add('    float       x, y, z;  // portal actor root, world space of that map')
$lines.Add('    bool        hasCoords;')
$lines.Add('};')
$lines.Add('')

$entries = foreach ($k in ($byLevel.Keys | Sort-Object)) {
    $p = $byLevel[$k]
    $disp = $k -replace '^Realm_', '' -replace '_', ' '
    if ($p) {
        "    {{`"{0}`", `"{1}`", {2}f, {3}f, {4}f, true}}," -f $k, $disp,
            [math]::Round($p.x, 3), [math]::Round($p.y, 3), [math]::Round($p.z, 3)
    } else {
        "    {{`"{0}`", `"{1}`", 0.f, 0.f, 0.f, false}},  // dump had no actors" -f $k, $disp
    }
}

$lines.Add('inline const RealmPortal kRealmPortals[] = {')
foreach ($e in $entries) { $lines.Add($e) }
$lines.Add('};')
$lines.Add('')
$lines.Add('constexpr int kRealmPortalCount = static_cast<int>(sizeof(kRealmPortals) / sizeof(kRealmPortals[0]));')
$lines.Add('')
$lines.Add('} // namespace acc')
$lines.Add('')

Set-Content -Encoding UTF8 -Path $Out -Value ($lines -join "`r`n")
"realms: $($byLevel.Count) -> $Out"
