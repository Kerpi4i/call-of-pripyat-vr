param(
    [ValidateRange(-0.20,0.30)][double]$Height=0,
    [ValidateRange(-0.15,0.30)][double]$Forward=0,
    [ValidateRange(0.75,1.4)][double]$Width=1,
    [ValidateRange(-45,45)][double]$Yaw=0,
    [switch]$Seated
)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if(Get-Process xrEngine -ErrorAction SilentlyContinue) { throw 'Close the game before changing equipment fit.' }
$source=Join-Path $root 'assets\gamedata\configs\copvr_equipment.ltx'
$destination=Join-Path $root 'game-vr\gamedata\configs\copvr_equipment.ltx'
$culture=[Globalization.CultureInfo]::InvariantCulture
# Read pristine authored defaults each time: repeated adjustments never accumulate.
$lines=foreach($line in Get-Content -LiteralPath $source) {
    if($line -match '^frame_position\s*=') {
        $lift=$Height+$(if($Seated){0.12}else{0})
        'frame_position = 0, '+$lift.ToString('0.####',$culture)+', '+$Forward.ToString('0.####',$culture)
    } elseif($line -match '^frame_rotation\s*=') {
        'frame_rotation = '+$Yaw.ToString('0.####',$culture)+', 0, 0'
    } elseif($line -match '^(left_hip|right_hip|left_shoulder|right_shoulder|belt_\d+)\s*=\s*([^,]+),(.+)$') {
        $fitKey=$Matches[1]; $fitX=[double]::Parse($Matches[2],$culture); $fitRest=$Matches[3]
        $fitKey+' = '+($fitX*$Width).ToString('0.####',$culture)+','+$fitRest
    } else { $line }
}
New-Item -ItemType Directory -Force (Split-Path -Parent $destination) | Out-Null
if(Test-Path -LiteralPath $destination) { Copy-Item -LiteralPath $destination -Destination ($destination+'.bak') -Force }
$lines | Set-Content -LiteralPath $destination -Encoding ascii
Write-Output "Equipment fit saved: $destination"
Write-Output 'No game launched. Height/Forward are metres; Width is a factor; Yaw is degrees.'
