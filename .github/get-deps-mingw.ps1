$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$sdlVersion = "2.0.16"

$sdlArchive = "SDL2-devel-$sdlVersion-mingw.tar.gz"

$sdlUrl = "https://www.libsdl.org/release/$sdlArchive"

"Downloading $sdlUrl" | Write-Host
curl.exe -sSLfO $sdlUrl
if ($LASTEXITCODE) { "Download failed" | Write-Host: exit 1 }

$target = Join-Path $pwd.Drive.Root "opt/local/x86_64-w64-mingw32"
mkdir $target

tar xf $sdlArchive
gci ".\SDL2-$sdlVersion\x86_64-w64-mingw32" | cp -dest $target -recurse -force
mv ".\SDL2-$sdlVersion" ".\SDL2"

