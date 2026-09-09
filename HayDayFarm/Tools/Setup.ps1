<#
.SYNOPSIS
    Builds the HayDayFarm editor target, creates the bootstrap level, and
    opens the project.

.DESCRIPTION
    Automates the mechanical part of a first run. It does NOT install
    anything -- Unreal Engine and Visual Studio must already be present.

    Run it from anywhere; it locates the project relative to its own path.

.PARAMETER EnginePath
    Root of the Unreal install, e.g. "C:\Program Files\Epic Games\UE_5.4".
    Auto-detected from the registry and the usual install locations when
    omitted.

.PARAMETER SkipBuild
    Skip compiling. Useful when only regenerating the level.

.PARAMETER SkipLevel
    Skip creating Content/Levels/L_FarmBlockout.

.PARAMETER NoLaunch
    Build (and generate the level) but do not open the editor.

.EXAMPLE
    .\Tools\Setup.ps1

.EXAMPLE
    .\Tools\Setup.ps1 -EnginePath "D:\UE_5.5" -NoLaunch

.NOTES
    UNVERIFIED: written without an Unreal installation to test against.
    See KNOWN_ISSUES.md KI-12. If a step fails, NEXT_TASK.md Task A has the
    equivalent manual instructions.
#>

[CmdletBinding()]
param(
    [string]$EnginePath,
    [switch]$SkipBuild,
    [switch]$SkipLevel,
    [switch]$NoLaunch
)

$ErrorActionPreference = 'Stop'

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$UProject    = Join-Path $ProjectRoot 'HayDayFarm.uproject'
$LevelScript = Join-Path $PSScriptRoot 'generate_bootstrap_level.py'
$TargetName  = 'HayDayFarmEditor'

function Write-Step  ($m) { Write-Host "`n==> $m" -ForegroundColor Cyan }
function Write-Ok    ($m) { Write-Host "    OK  $m" -ForegroundColor Green }
function Write-Warn2 ($m) { Write-Host "    !   $m" -ForegroundColor Yellow }

function Fail ($message, $hint) {
    Write-Host "`nFAILED: $message" -ForegroundColor Red
    if ($hint) { Write-Host "        $hint" -ForegroundColor Yellow }
    exit 1
}

function Find-UnrealEngine {
    # 1. Launcher installs record themselves in the registry.
    $registryRoots = @(
        'HKLM:\SOFTWARE\EpicGames\Unreal Engine',
        'HKLM:\SOFTWARE\WOW6432Node\EpicGames\Unreal Engine'
    )
    foreach ($root in $registryRoots) {
        if (-not (Test-Path $root)) { continue }
        # Newest version first, so 5.5 wins over 5.4 if both are installed.
        $versions = Get-ChildItem $root | Sort-Object PSChildName -Descending
        foreach ($v in $versions) {
            $dir = (Get-ItemProperty $v.PSPath -ErrorAction SilentlyContinue).InstalledDirectory
            if ($dir -and (Test-Path $dir)) { return $dir }
        }
    }

    # 2. Default install locations.
    foreach ($drive in @('C:', 'D:', 'E:')) {
        $base = Join-Path $drive 'Program Files\Epic Games'
        if (-not (Test-Path $base)) { $base = Join-Path $drive 'Epic Games' }
        if (-not (Test-Path $base)) { continue }

        $candidates = Get-ChildItem $base -Directory -Filter 'UE_5.*' -ErrorAction SilentlyContinue |
                      Sort-Object Name -Descending
        foreach ($c in $candidates) {
            if (Test-Path (Join-Path $c.FullName 'Engine\Binaries\Win64\UnrealEditor.exe')) {
                return $c.FullName
            }
        }
    }
    return $null
}

# --- Locate the project ----------------------------------------------------

Write-Step 'Locating project'
if (-not (Test-Path $UProject)) {
    Fail "HayDayFarm.uproject not found at $UProject" `
         'Run this script from inside the HayDayFarm folder (Tools\Setup.ps1).'
}
Write-Ok $UProject

$declaredVersion = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
Write-Ok "Project targets Unreal $declaredVersion"

# --- Locate the engine -----------------------------------------------------

Write-Step 'Locating Unreal Engine'
if (-not $EnginePath) { $EnginePath = Find-UnrealEngine }

if (-not $EnginePath) {
    Fail 'No Unreal Engine installation found.' `
         'Install it from the Epic Games Launcher, or pass -EnginePath "C:\Program Files\Epic Games\UE_5.4".'
}

$Editor    = Join-Path $EnginePath 'Engine\Binaries\Win64\UnrealEditor.exe'
$EditorCmd = Join-Path $EnginePath 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$BuildBat  = Join-Path $EnginePath 'Engine\Build\BatchFiles\Build.bat'

if (-not (Test-Path $Editor)) {
    Fail "$EnginePath does not look like an Unreal install (no UnrealEditor.exe)." `
         'Point -EnginePath at the folder that contains Engine\.'
}
Write-Ok $EnginePath

$installedVersion = (Split-Path $EnginePath -Leaf) -replace '^UE_', ''
if ($installedVersion -and $declaredVersion -and $installedVersion -ne $declaredVersion) {
    Write-Warn2 "Project says $declaredVersion but this engine looks like $installedVersion."
    Write-Warn2 'Unreal will offer to convert the project. That is fine, but to avoid'
    Write-Warn2 'the prompt, see NEXT_TASK.md Task A for the three-line version change.'
}

# --- Static check ----------------------------------------------------------

Write-Step 'Static validation'
$python = Get-Command python -ErrorAction SilentlyContinue
if ($python) {
    & $python.Source (Join-Path $PSScriptRoot 'validate_project.py')
    if ($LASTEXITCODE -ne 0) {
        Fail 'validate_project.py reported errors.' 'Fix those before building -- they are cheaper to diagnose than UHT errors.'
    }
} else {
    Write-Warn2 'python not on PATH; skipping the static check (optional).'
}

# --- Build -----------------------------------------------------------------

if (-not $SkipBuild) {
    Write-Step "Building $TargetName (Win64 Development)"
    Write-Host '    First build compiles the engine headers too -- expect 10-30 minutes.' -ForegroundColor DarkGray

    & $BuildBat $TargetName Win64 Development -Project="$UProject" -WaitMutex -FromMsBuild
    if ($LASTEXITCODE -ne 0) {
        Fail 'Compilation failed.' `
             'Read the first error, not the last. KNOWN_ISSUES.md KI-01 lists the three lines most likely to break.'
    }
    Write-Ok 'Build succeeded'
} else {
    Write-Warn2 'Skipping build (-SkipBuild)'
}

# --- Level -----------------------------------------------------------------

if (-not $SkipLevel) {
    $levelAsset = Join-Path $ProjectRoot 'Content\Levels\L_FarmBlockout.umap'
    if (Test-Path $levelAsset) {
        Write-Step 'Level already exists; leaving it alone'
        Write-Ok $levelAsset
    } else {
        Write-Step 'Creating Content/Levels/L_FarmBlockout'
        & $EditorCmd "$UProject" -run=pythonscript -script="$LevelScript" -unattended -nopause
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path $levelAsset)) {
            Write-Warn2 'Level generation did not produce a .umap (KNOWN_ISSUES.md KI-02).'
            Write-Warn2 'Create it by hand instead: place an AFarmWorldBootstrap at the origin'
            Write-Warn2 'and a PlayerStart at (0, 0, 120), then save as Content/Levels/L_FarmBlockout.'
        } else {
            Write-Ok $levelAsset
        }
    }
} else {
    Write-Warn2 'Skipping level generation (-SkipLevel)'
}

# --- Launch ----------------------------------------------------------------

if (-not $NoLaunch) {
    Write-Step 'Opening the editor'
    Write-Host '    Press Alt+P (or the Play button) to walk around.' -ForegroundColor DarkGray
    Start-Process -FilePath $Editor -ArgumentList "`"$UProject`""
    Write-Ok 'Editor launching'
} else {
    Write-Warn2 'Not launching (-NoLaunch)'
}

Write-Host "`nDone. See NEXT_TASK.md for what to check once you are in game.`n" -ForegroundColor Cyan
