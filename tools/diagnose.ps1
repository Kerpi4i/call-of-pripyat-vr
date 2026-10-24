param([string]$GamePath = 'C:\Program Files (x86)\Steam\steamapps\common\Stalker Call of Pripyat')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
function Get-PeMachine([string]$Path) {
    $stream = [IO.File]::OpenRead($Path)
    $reader = New-Object IO.BinaryReader($stream)
    try {
        $stream.Position = 0x3c
        $offset = $reader.ReadInt32()
        $stream.Position = $offset
        if ($reader.ReadUInt32() -ne 0x4550) { throw "Invalid PE file: $Path" }
        switch ($reader.ReadUInt16()) { 0x14c { 'x86' }; 0x8664 { 'x64' }; default { 'other' } }
    } finally { $reader.Dispose(); $stream.Dispose() }
}
$binaries = foreach ($name in @('xrEngine.exe','xrGame.dll','xrRender_R4.dll')) {
    $path = Join-Path $GamePath "bin\$name"
    if (Test-Path -LiteralPath $path) {
        [ordered]@{ name=$name; architecture=Get-PeMachine $path; sha256=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash; version=(Get-Item -LiteralPath $path).VersionInfo.FileVersion }
    }
}
$runtime = (Get-ItemProperty 'HKLM:\SOFTWARE\Khronos\OpenXR\1' -ErrorAction SilentlyContinue).ActiveRuntime
$manifest = $null
if ($runtime -and (Test-Path -LiteralPath $runtime)) { $manifest = Get-Content -LiteralPath $runtime -Raw | ConvertFrom-Json }
$report = [ordered]@{
    capturedUtc=[DateTime]::UtcNow.ToString('o'); gamePath=$GamePath
    binaries=@($binaries); activeRuntime=$runtime; runtimeManifest=$manifest
    headsetConnectionTested=$false; stereoRenderingImplemented=$false; engineIntegrationImplemented=$false
    note='Read-only environment inspection. Runtime registration does not prove headset connectivity.'
}
New-Item -ItemType Directory -Force (Join-Path $root 'reports') | Out-Null
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $root 'reports\environment.json') -Encoding UTF8
$report | ConvertTo-Json -Depth 8
