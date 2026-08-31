$ErrorActionPreference = "Stop"

$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$DependencyRoot = Join-Path $RepositoryRoot ".dependencies"
$VcpkgRoot = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { Join-Path $DependencyRoot "vcpkg" }
$Baseline = "63feddf004cc39169a8b0b7f79c2eba5065d6140"

if (-not (Test-Path (Join-Path $VcpkgRoot ".git"))) {
    New-Item -ItemType Directory -Force -Path $DependencyRoot | Out-Null
    git clone https://github.com/microsoft/vcpkg.git $VcpkgRoot
}

git -C $VcpkgRoot fetch --depth 1 origin $Baseline
git -C $VcpkgRoot checkout --detach $Baseline
& (Join-Path $VcpkgRoot "bootstrap-vcpkg.bat") -disableMetrics

Write-Host "vcpkg is ready at $VcpkgRoot"
Write-Host "Run: `$env:VCPKG_ROOT='$VcpkgRoot'"
Write-Host "Then: cmake --preset debug; cmake --build --preset debug; ctest --preset debug"
