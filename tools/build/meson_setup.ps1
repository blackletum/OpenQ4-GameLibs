$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Get-VsDevCmdPath {
    if ($env:VSINSTALLDIR) {
        $candidate = Join-Path $env:VSINSTALLDIR "Common7\Tools\VsDevCmd.bat"
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "Could not locate vswhere.exe."
    }

    $component = "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
    $installPathRaw = & $vswhere -latest -prerelease -products * -requires $component -property installationPath
    $installPath = if ($null -eq $installPathRaw) { "" } else { "$installPathRaw".Trim() }
    if ([string]::IsNullOrWhiteSpace($installPath)) {
        throw "No Visual Studio installation with C++ tools was found."
    }

    $vsDevCmd = Join-Path $installPath "Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $vsDevCmd)) {
        throw "Could not locate VsDevCmd.bat at '$vsDevCmd'."
    }

    return $vsDevCmd
}

function Quote-CmdArg([string]$Value) {
    if ($Value -match '[\s"&<>|()]') {
        return '"' + ($Value -replace '"', '\"') + '"'
    }
    return $Value
}

function Invoke-Meson {
    param(
        [string[]]$MesonArgs,
        [string]$VsDevCmdPath
    )

    if ([string]::IsNullOrWhiteSpace($VsDevCmdPath)) {
        & meson @MesonArgs
        return
    }

    $mesonCmd = "meson " + (($MesonArgs | ForEach-Object { Quote-CmdArg $_ }) -join " ")
    $fullCmd = 'call "' + $VsDevCmdPath + '" -arch=x86 -host_arch=x64 >nul && set CC=cl && set CXX=cl && ' + $mesonCmd
    & $env:ComSpec /d /c $fullCmd
}

function Get-BuildDirInfo {
    param(
        [string[]]$MesonArgs,
        [string]$DefaultBuildDir
    )

    $result = [PSCustomObject]@{
        BuildDir = $DefaultBuildDir
        HasExplicit = $false
    }

    for ($i = 0; $i -lt $MesonArgs.Length; $i++) {
        $arg = $MesonArgs[$i]
        if ($arg -eq "-C" -and ($i + 1) -lt $MesonArgs.Length) {
            $result.BuildDir = $MesonArgs[$i + 1]
            $result.HasExplicit = $true
            break
        }

        if ($arg.StartsWith("-C") -and $arg.Length -gt 2) {
            $result.BuildDir = $arg.Substring(2)
            $result.HasExplicit = $true
            break
        }
    }

    $result.BuildDir = [System.IO.Path]::GetFullPath($result.BuildDir)
    return $result
}

function Test-MesonBuildDirectory {
    param([string]$BuildDir)

    $coreData = Join-Path $BuildDir "meson-private\coredata.dat"
    $ninjaFile = Join-Path $BuildDir "build.ninja"
    return (Test-Path $coreData) -and (Test-Path $ninjaFile)
}

function Copy-GameLibrariesToInstallGameDir {
    param(
        [string]$BuildDir,
        [string]$RepoRoot
    )

    $installGameDir = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot "..\OpenQ4\.install\baseoq4"))
    New-Item -Path $installGameDir -ItemType Directory -Force | Out-Null

    $binaries = @(
        "game-sp_x86.dll",
        "game-mp_x86.dll"
    )

    foreach ($binary in $binaries) {
        $sourcePath = Join-Path $BuildDir (Join-Path "src" $binary)
        if (-not (Test-Path $sourcePath)) {
            throw "Expected build output '$sourcePath' was not found."
        }

        $destinationPath = Join-Path $installGameDir $binary
        Copy-Item -Path $sourcePath -Destination $destinationPath -Force
    }

    Write-Host "Copied game libraries to '$installGameDir'."
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $scriptDir "..\.."))
$defaultBuildDir = Join-Path $repoRoot "builddir"

$vsDevCmd = $null
if ($null -eq (Get-Command cl -ErrorAction SilentlyContinue)) {
    $vsDevCmd = Get-VsDevCmdPath
}

$effectiveArgs = @($args)
if ($effectiveArgs.Count -eq 0) {
    throw "No Meson arguments were provided to meson_setup.ps1."
}

$isCompileCommand = $effectiveArgs[0] -eq "compile"

if ($effectiveArgs.Length -gt 0 -and ($effectiveArgs[0] -eq "compile" -or $effectiveArgs[0] -eq "install")) {
    $buildInfo = Get-BuildDirInfo -MesonArgs $effectiveArgs -DefaultBuildDir $defaultBuildDir

    if (-not (Test-MesonBuildDirectory $buildInfo.BuildDir)) {
        Write-Host "Meson build directory '$($buildInfo.BuildDir)' is missing or invalid. Running meson setup..."
        $setupArgs = @(
            "setup",
            "--wipe",
            $buildInfo.BuildDir,
            $repoRoot,
            "--backend",
            "ninja",
            "--buildtype=release",
            "--vsenv"
        )
        Invoke-Meson -MesonArgs $setupArgs -VsDevCmdPath $vsDevCmd
        $setupCode = [int]$LASTEXITCODE
        if ($setupCode -ne 0) {
            exit $setupCode
        }
    }

    if (-not $buildInfo.HasExplicit) {
        $remainingArgs = @()
        if ($effectiveArgs.Length -gt 1) {
            $remainingArgs = $effectiveArgs[1..($effectiveArgs.Length - 1)]
        }
        $effectiveArgs = @($effectiveArgs[0], "-C", $buildInfo.BuildDir) + $remainingArgs
    }
}

Invoke-Meson -MesonArgs $effectiveArgs -VsDevCmdPath $vsDevCmd
$exitCode = [int]$LASTEXITCODE

if ($exitCode -eq 0 -and $isCompileCommand -and $env:OPENQ4_COPY_GAMELIBS_TO_INSTALL -eq "1") {
    $compileBuildInfo = Get-BuildDirInfo -MesonArgs $effectiveArgs -DefaultBuildDir $defaultBuildDir
    Copy-GameLibrariesToInstallGameDir -BuildDir $compileBuildInfo.BuildDir -RepoRoot $repoRoot
}

exit $exitCode
