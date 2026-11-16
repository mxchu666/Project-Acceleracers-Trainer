param(
    [string]$Dir = "F:\Project Acceleracers Trainer\Maps Acceleracers Only",
    [string]$Out = "F:\Project Acceleracers Trainer\AcceleracersTrainer\build\realm_dump.json"
)

function Unwrap([string]$s) {
    if (-not $s) { return $null }
    $i = $s.IndexOf("'")
    if ($i -ge 0) { return $s.Substring($i + 1).TrimEnd("'") }
    return $s
}

$rows = New-Object System.Collections.Generic.List[object]

Get-ChildItem $Dir -Filter *.json | Where-Object { $_.Length -gt 20000 } | ForEach-Object {
    $map = $_.BaseName
    $j = [IO.File]::ReadAllText($_.FullName) | ConvertFrom-Json

    $byKey = @{}
    foreach ($o in $j) {
        $outer = Unwrap $o.Outer.ObjectName
        if ($outer -and $o.Name) { $byKey[$outer + '|' + $o.Name] = $o }
    }

    foreach ($o in $j) {
        if ($o.Type -notlike 'CP_*' -and $o.Type -ne 'Portal_C') { continue }
        $rootPath = Unwrap $o.Properties.RootComponent.ObjectName
        if (-not $rootPath) { continue }
        $k = $rootPath.LastIndexOf('.')
        if ($k -lt 0) { continue }
        $key = $rootPath.Substring(0, $k) + '|' + $rootPath.Substring($k + 1)
        if (-not $byKey.ContainsKey($key)) { continue }
        $root = $byKey[$key]
        $loc = $root.Properties.RelativeLocation
        if (-not $loc) { continue }
        $rot = $root.Properties.RelativeRotation

        $rows.Add([pscustomobject]@{
            map      = $map
            type     = $o.Type
            name     = $o.Name
            index    = $o.Properties.index
            levelIn  = $o.Properties.LevelIn
            levelOut = $o.Properties.LevelOut
            accel    = $o.Properties.Acceledrome
            x        = $loc.X
            y        = $loc.Y
            z        = $loc.Z
            pitch    = $rot.Pitch
            yaw      = $rot.Yaw
            roll     = $rot.Roll
        })
    }
}

$rows | ConvertTo-Json -Depth 3 | Set-Content -Encoding UTF8 $Out
"rows: $($rows.Count) -> $Out"
