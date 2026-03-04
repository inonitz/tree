param(
    [Parameter(Mandatory=$true, ParameterSetName="Build")]
    [ValidateSet("debug", "debug_perf", "release", "release_dbginfo", "release_perf")]
    [string]$BuildType,

    [Parameter(Mandatory=$true, ParameterSetName="Build")]
    [ValidateSet("shared", "static")]
    [string]$LinkType,

    [Parameter(Mandatory=$true, ParameterSetName="Build")]
    [ValidateSet("cleanbuild", "configure", "build", "runtests", "runbench")]
    [string]$Action,

    [Parameter(Mandatory=$false, ParameterSetName="Build")]
    [switch]$DryRun,

    [Parameter(Mandatory=$true, ParameterSetName="Help")]
    [Alias("h", "-Help")]
    [switch]$Help
)


# --- Helper Functions ---
function Show-CustomHelp {
    Write-Host "Usage: .\build.ps1 -BuildType <type> -LinkType <link> -Action <action> [-DryRun]" -ForegroundColor Cyan
    Write-Host "Usage: .\build.ps1 -Help" -ForegroundColor Cyan
    Write-Host "`nArguments:"
    Write-Host "  -BuildType   : debug, release, release_dbginfo, debug_perf, release_perf"
    Write-Host "  -LinkType    : shared, static"
    Write-Host "  -Action      : cleanbuild, configure, build, runtests, runbench"
}


function Run-Command {
    param([string]$Description, [scriptblock]$Command)
    if ($DryRun) {
        Write-Host "[DRY-RUN] Would execute: $Description" -ForegroundColor Yellow
    } else {
        try {
            & $Command
        } catch {
            Write-Error "Execution failed: $_"
            exit $LASTEXITCODE
        }
    }
}


# --- Initialization ---

if ($PSCmdlet.ParameterSetName -eq "Help") {
    Show-CustomHelp
    exit 0
}

$ErrorActionPreference = "Stop"
$PROJECT_NAME = "all"
$CMAKE_ROOT_BUILD_DIR = "build"
$CMAKE_ARGLIST = @(
    "-DCMAKE_C_COMPILER=clang",
    "-DCMAKE_CXX_COMPILER=clang++",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=1",
    "-DCMAKE_COLOR_DIAGNOSTICS=ON",
    "-DENABLE_AND_BUILD_TESTS=ON"
)

# --- Mapping logic ---

switch ($BuildType) {
    "debug"           { $CMAKE_ARGLIST += "-DCMAKE_BUILD_TYPE=Debug" }
    "debug_perf"      { $CMAKE_ARGLIST += "-DCMAKE_BUILD_TYPE=Debug" }
    "release"         { $CMAKE_ARGLIST += "-DCMAKE_BUILD_TYPE=Release" }
    "release_dbginfo" { $CMAKE_ARGLIST += "-DCMAKE_BUILD_TYPE=RelWithDbgInfo" }
    "release_perf"    { $CMAKE_ARGLIST += "-DCMAKE_BUILD_TYPE=Release" }
}
$CMAKE_ARGLIST += $( If ($LinkType -eq "shared") { "-DBUILD_SHARED_LIBS=1" } Else { "-DBUILD_SHARED_LIBS=0" } )


# Constructing paths
$CMAKE_FINAL_BUILD_DIR = Join-Path $CMAKE_ROOT_BUILD_DIR (Join-Path $BuildType $LinkType)

Write-Host "Out-of-source Target Build Directory: '$CMAKE_FINAL_BUILD_DIR'" -ForegroundColor Blue
Write-Host "Arguments: $BuildType $LinkType $Action"

# --- Execution ---

# 1. Directory Setup
if (-not (Test-Path $CMAKE_ROOT_BUILD_DIR)) {
    Run-Command "mkdir $CMAKE_ROOT_BUILD_DIR" { New-Item -ItemType Directory -Path $CMAKE_ROOT_BUILD_DIR | Out-Null }
}

if ($Action -eq "cleanbuild") {
    if (Test-Path $CMAKE_FINAL_BUILD_DIR) {
        Run-Command "Remove $CMAKE_FINAL_BUILD_DIR" { Remove-Item -Recurse -Force $CMAKE_FINAL_BUILD_DIR }
    }
}


# 2. Configure
if ($Action -eq "configure" -or $Action -eq "cleanbuild") {
    $CMAKE_ARGLIST += "-DGIT_SUBMODULE=ON"
    Run-Command "mkdir $CMAKE_FINAL_BUILD_DIR" { New-Item -ItemType Directory -Path $CMAKE_FINAL_BUILD_DIR -Force | Out-Null }
    Run-Command "CMake Configure" { cmake -S . -B $CMAKE_FINAL_BUILD_DIR -G "Ninja" $CMAKE_ARGLIST }
}


# 3. Build
if ($Action -eq "build") {
    if (-not $DryRun) { Push-Location $CMAKE_FINAL_BUILD_DIR }
    
    # Sync compile_commands.json as per original script
    if (Test-Path "compile_commands.json") {
        Run-Command "Update compile_commands.json" { Copy-Item "compile_commands.json" "../../compile_commands.json" -Force }
    }
    
    Run-Command "Ninja Build" { ninja $PROJECT_NAME }
    if (-not $DryRun) { Pop-Location }
}


# 4. Run
if ($Action -eq "runtests") {
    if (-not $DryRun) { Push-Location $CMAKE_FINAL_BUILD_DIR }
    Run-Command "Ninja Run" { ninja run_test_treelib }
    if (-not $DryRun) { Pop-Location }
}

if ($Action -eq "runbench") {
    if (-not $DryRun) { Push-Location $CMAKE_FINAL_BUILD_DIR }
    Run-Command "Ninja Run" { ninja run_benchmark_treelib }
    if (-not $DryRun) { Pop-Location }
}