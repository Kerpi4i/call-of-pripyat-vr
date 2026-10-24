$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$engine = Join-Path $root 'engine'
$revision = 'c62d9d343981d50f6ed176412b85d94c61b578b5'
if (!(Test-Path -LiteralPath (Join-Path $engine '.git'))) {
    & git clone https://github.com/OpenXRay/xray-16.git $engine
    if ($LASTEXITCODE -ne 0) { throw 'OpenXRay clone failed' }
    & git -C $engine checkout --detach $revision
    if ($LASTEXITCODE -ne 0) { throw 'Pinned engine checkout failed' }
}
$actual = & git -C $engine rev-parse HEAD
if ($actual -ne $revision) { throw "Engine revision differs; refusing to reset local checkout: $actual" }
& git -C $engine submodule update --init --recursive --depth 1
if ($LASTEXITCODE -ne 0) { throw 'Engine submodule preparation failed' }
$toolFolder = Join-Path $PSScriptRoot 'engine-build'
New-Item -ItemType Directory -Force $toolFolder | Out-Null
$nuget = Join-Path $toolFolder 'nuget.exe'
if (!(Test-Path -LiteralPath $nuget)) {
    Invoke-WebRequest 'https://dist.nuget.org/win-x86-commandline/v6.14.0/nuget.exe' -OutFile $nuget
}
$signature = Get-AuthenticodeSignature -LiteralPath $nuget
if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'O=Microsoft Corporation') {
    throw 'NuGet executable must have a valid Microsoft signature.'
}
& $nuget restore (Join-Path $engine 'src\engine.sln') -NonInteractive -Verbosity quiet
if ($LASTEXITCODE -ne 0) { throw 'Engine NuGet restore failed' }
& $nuget install Microsoft.Windows.SDK.CPP.x64 -Version 10.0.19041.5 -OutputDirectory $toolFolder -Source https://api.nuget.org/v3/index.json -NonInteractive -Verbosity quiet
if ($LASTEXITCODE -ne 0) { throw 'Local Windows SDK acquisition failed' }
foreach ($name in @('xray.user.props','xray_project_configuration.user.props')) {
    $source = Join-Path $PSScriptRoot "engine-config\$name"
    $destination = Join-Path $engine "src\$name"
    if (Test-Path -LiteralPath $destination) {
        if ((Get-FileHash -LiteralPath $source).Hash -ne (Get-FileHash -LiteralPath $destination).Hash) {
            throw "Existing engine settings differ; refusing to overwrite: $destination"
        }
    } else { Copy-Item -LiteralPath $source -Destination $destination }
}
Write-Output 'OpenXRay sources, submodules, dependencies and local Windows SDK prepared.'
$patch = Join-Path $root 'patches\openxray-vr.patch'
if (Test-Path -LiteralPath $patch) {
    & git -C $engine apply --reverse --check $patch 2>$null
    if ($LASTEXITCODE -ne 0) {
        & git -C $engine apply --check $patch
        if ($LASTEXITCODE -ne 0) { throw 'VR engine patch conflicts with local changes; checkout was preserved.' }
        & git -C $engine apply $patch
        if ($LASTEXITCODE -ne 0) { throw 'Failed to apply VR engine patch' }
    }
    Write-Output 'VR engine patch is present.'
}
