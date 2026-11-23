$ErrorActionPreference = 'Stop'

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$OutDir = Join-Path $Root 'build'
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$CCompiler = Get-Command gcc -ErrorAction SilentlyContinue
$CppCompiler = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $CCompiler -or -not $CppCompiler) {
    throw 'gcc and g++ were not found on PATH. Install a working GCC/G++ toolchain and reopen PowerShell.'
}

$Executable = Join-Path $OutDir 'drone-phase2.exe'
$CSourceFiles = @(
    'priority_queue.c',
    'pending_queue.c',
    'zone_graph.c',
    'record_support.c'
)
$CObjects = @()

foreach ($CSource in $CSourceFiles) {
    $ObjectName = [System.IO.Path]::GetFileNameWithoutExtension($CSource) + '.o'
    $ObjectPath = Join-Path $OutDir $ObjectName
    & $CCompiler.Source -std=c11 -Wall -Wextra -pedantic -I. -c $CSource -o $ObjectPath
    if ($LASTEXITCODE -ne 0) { throw "C compilation failed for $CSource." }
    $CObjects += $ObjectPath
}

& $CppCompiler.Source -std=c++17 -Wall -Wextra -pedantic -I. -Icpp/include `
    cpp/src/models.cpp cpp/src/dispatch_manager.cpp cpp/src/main.cpp @CObjects -o $Executable
if ($LASTEXITCODE -ne 0) { throw 'C++ compilation or linking failed.' }

Write-Host "Built $Executable"
