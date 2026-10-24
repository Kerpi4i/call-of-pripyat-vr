$ErrorActionPreference = 'Stop'
$folder = Join-Path $PSScriptRoot 'toolchain'
$archive = Join-Path $folder 'zig.zip'
$expected = '3a0ed1e8799a2f8ce2a6e6290a9ff22e6906f8227865911fb7ddedc3cc14cb0c'
New-Item -ItemType Directory -Force $folder | Out-Null
if (!(Test-Path -LiteralPath $archive)) {
    Invoke-WebRequest 'https://ziglang.org/download/0.15.2/zig-x86_64-windows-0.15.2.zip' -OutFile $archive
}
if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) {
    throw 'Portable compiler checksum mismatch.'
}
if (!(Test-Path -LiteralPath (Join-Path $folder 'zig-x86_64-windows-0.15.2\zig.exe'))) {
    Expand-Archive -LiteralPath $archive -DestinationPath $folder -Force
}
Write-Output 'Portable Zig 0.15.2 ready. No global PATH or registry changes.'
