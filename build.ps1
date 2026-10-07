$ErrorActionPreference = 'Stop'

$projectRoot = $PSScriptRoot
$cDirectory = Join-Path $projectRoot 'core\c'
$cppIncludeDirectory = Join-Path $projectRoot 'core\cpp\include'
$cppSourceDirectory = Join-Path $projectRoot 'core\cpp\src'
$buildDirectory = Join-Path $projectRoot 'build'

function Find-Compiler([string]$name) {
    $available = Get-Command $name -ErrorAction SilentlyContinue
    if ($available) { return $available.Source }

    $fallback = Join-Path 'C:\MinGW\bin' ($name + '.exe')
    if (Test-Path -LiteralPath $fallback) { return $fallback }

    throw "Could not find $name. Install MinGW-w64 and add its bin folder to PATH."
}

$gcc = Find-Compiler 'gcc'
$gxx = Find-Compiler 'g++'
New-Item -ItemType Directory -Force -Path $buildDirectory | Out-Null

$objectFiles = @()
$cSource = Join-Path $cDirectory 'drone_structures.c'
$cObject = Join-Path $buildDirectory 'drone_structures.o'
& $gcc -std=c11 -Wall -Wextra "-I$cDirectory" -c $cSource -o $cObject
if ($LASTEXITCODE -ne 0) { throw "C compilation failed (exit $LASTEXITCODE)." }
$objectFiles += $cObject

$executable = Join-Path $buildDirectory 'drone-demo.exe'
& $gxx -std=c++17 -Wall -Wextra "-I$cDirectory" "-I$cppIncludeDirectory" `
    (Join-Path $cppSourceDirectory 'dispatch_manager.cpp') `
    (Join-Path $cppSourceDirectory 'main.cpp') `
    @objectFiles -o $executable
if ($LASTEXITCODE -ne 0) { throw "C++ compilation or linking failed (exit $LASTEXITCODE)." }

Write-Host "Build complete: $executable"
