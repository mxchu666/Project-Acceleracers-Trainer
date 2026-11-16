param([string]$Dir = "F:\Project Acceleracers Trainer\Maps Acceleracers Only")

function Unwrap([string]$s) {
    if (-not $s) { return $null }
    $i = $s.IndexOf("'")
    if ($i -ge 0) { return $s.Substring($i + 1).TrimEnd("'") }
    return $s
}

Get-ChildItem $Dir -Filter *.json | Where-Object { $_.Length -gt 20000 } | ForEach-Object {
    $map = $_.BaseName
    $j = [IO.File]::ReadAllText($_.FullName) | ConvertFrom-Json
    $byKey = @{}
    foreach ($o in $j) {
        $outer = Unwrap $o.Outer.ObjectName
        if ($outer -and $o.Name) { $byKey[$outer + '|' + $o.Name] = $o }
    }
    foreach ($o in $j) {
        if ($o.Type -ne 'Portal_C') { continue }
        $rootPath = Unwrap $o.Properties.RootComponent.ObjectName
        $k = $rootPath.LastIndexOf('.')
        $root = $byKey[$rootPath.Substring(0, $k) + '|' + $rootPath.Substring($k + 1)]
        "=== $map / $($o.Name) ==="
        "  root      loc=($($root.Properties.RelativeLocation.X), $($root.Properties.RelativeLocation.Y), $($root.Properties.RelativeLocation.Z)) rot=($($root.Properties.RelativeRotation.Pitch), $($root.Properties.RelativeRotation.Yaw), $($root.Properties.RelativeRotation.Roll))"
        foreach ($cname in @('Portal', 'SymbolMesh')) {
            $cp = $o.Properties.$cname.ObjectName
            if (-not $cp) { continue }
            $p = Unwrap $cp
            $kk = $p.LastIndexOf('.')
            $comp = $byKey[$p.Substring(0, $kk) + '|' + $p.Substring($kk + 1)]
            if (-not $comp) { "  $cname : <missing>"; continue }
            $l = $comp.Properties.RelativeLocation
            $s = $comp.Properties.RelativeScale3D
            "  $cname loc=($($l.X), $($l.Y), $($l.Z)) scale=($($s.X), $($s.Y), $($s.Z)) sphereR=$($comp.Properties.SphereRadius)"
        }
    }
}
