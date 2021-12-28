$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$sdlVersion = "2.0.18"

$sdlArchive = "SDL2-devel-$sdlVersion-VC.zip"

$sdlUrl = "https://www.libsdl.org/release/$sdlArchive"

"Downloading $sdlUrl" | Write-Host
curl.exe -sSLfO $sdlUrl
if ($LASTEXITCODE) { "Download failed" | Write-Host: exit 1 }

$env:GITHUB_PATH

#$target = Join-Path $pwd.Drive.Root "deps/SDL"
$target = $env:GITHUB_PATH + "/deps"
mkdir $target

Expand-Archive -Path $sdlArchive -DestinationPath $target -PassThru

# Remove version string from path to help CMake
$extractTarget = Join-Path $target "SDL2-" $sdlVersion
$newTarget = Join-Path $target "sdl2"
Move-Item -Path $extractTarget -Destination $newTarget -PassThru
