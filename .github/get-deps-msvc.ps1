$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$sdlVersion = "2.0.18"

$sdlArchive = "SDL2-devel-$sdlVersion-VC.zip"

$sdlUrl = "https://www.libsdl.org/release/$sdlArchive"

"Downloading $sdlUrl" | Write-Host
curl.exe -sSLfO $sdlUrl
if ($LASTEXITCODE) { "Download failed" | Write-Host: exit 1 }

$env:GITHUB_PATH
$myLocation = Get-Location

$target = Join-Path $myLocation "deps"
mkdir $target

Expand-Archive -Path $sdlArchive -DestinationPath $target

# Remove version string from path to help CMake
$extractTarget = Join-Path $target "SDL2-$sdlVersion"
$newTarget = Join-Path $target "sdl2"
Move-Item -Path $extractTarget -Destination $newTarget

Get-ChildItem -Path $target -Recurse