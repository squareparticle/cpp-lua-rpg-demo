[CmdletBinding()]
param(
    [string] $BuildDirectory = "build"
)

$ErrorActionPreference = "Stop"

$vswhere = Join-Path ${env:ProgramFiles(x86)} `
    "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw "Visual Studio Installer's vswhere.exe was not found."
}

$visualStudioPath = & $vswhere `
    -latest `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath
if ($LASTEXITCODE -ne 0 -or -not $visualStudioPath) {
    throw "A Visual Studio installation with the x64 C++ tools was not found."
}

$vcvars = Join-Path $visualStudioPath "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path -LiteralPath $vcvars)) {
    throw "vcvars64.bat was not found under $visualStudioPath."
}

$developerEnvironment = & $env:ComSpec /d /s /c `
    "call `"$vcvars`" >nul && set"
if ($LASTEXITCODE -ne 0) {
    throw "The Visual Studio x64 developer environment could not be loaded."
}

foreach ($entry in $developerEnvironment) {
    if ($entry -match "^([^=]+)=(.*)$") {
        Set-Item -LiteralPath "Env:$($matches[1])" -Value $matches[2]
    }
}

$cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
if ($cmakeCommand) {
    $cmake = $cmakeCommand.Source
}
else {
    $cmake = Join-Path $visualStudioPath `
        "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
}

if (-not (Test-Path -LiteralPath $cmake)) {
    throw "CMake was not found on PATH or in the Visual Studio installation."
}

& $cmake --build $BuildDirectory
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
