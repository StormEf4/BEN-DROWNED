# BEN DROWNED - Windows setup.
#
# Paste this into PowerShell (no admin needed unless WSL still has to be installed):
#
#   irm https://raw.githubusercontent.com/StormEf4/BEN-DROWNED/HEAD/windows/BEN-Setup.ps1 | iex
#
# or double-click windows\BEN-Setup.cmd in a downloaded copy of the project.
#
# What it does (safe to run again at any time; it picks up where it left off):
#   1. Installs WSL2 + Ubuntu if missing (needs one restart).
#   2. Asks you to pick your Majora's Mask (USA) ROM.
#   3. Downloads the project into your Ubuntu home folder (or updates it).
#   4. Runs `./ben setup`, which installs build tools, extracts assets and builds the hack.
#   5. Puts two shortcuts on your desktop: "BEN DROWNED - Update & Build" and "BEN DROWNED - Play".
#
# Written for Windows PowerShell 5.1 (built into Windows 10/11). Kept free of `exit` so that running it through
# `iex` never closes your PowerShell window.

$BenRepoUrl = 'https://github.com/StormEf4/BEN-DROWNED.git'
$BenFolder = 'BEN-DROWNED'

function Write-BenStep([string]$Text) { Write-Host ''; Write-Host "==> $Text" -ForegroundColor Cyan }
function Write-BenOk([string]$Text) { Write-Host "  [ok] $Text" -ForegroundColor Green }
function Write-BenWarn([string]$Text) { Write-Host "  [!] $Text" -ForegroundColor Yellow }
function Write-BenFail([string]$Text) { Write-Host ''; Write-Host "  [x] $Text" -ForegroundColor Red }

# Names of installed WSL distributions. `wsl -l -q` prints UTF-16, so strip the NUL bytes.
function Get-BenDistros {
    $env:WSL_UTF8 = '1' # newer WSL versions then print plain UTF-8
    $raw = & wsl.exe -l -q 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $raw) { return @() }
    return @($raw | ForEach-Object { ($_ -replace "`0", '').Trim() } | Where-Object { $_ -ne '' })
}

function Find-BenUbuntu {
    if ($env:BEN_DISTRO) { return $env:BEN_DISTRO }
    $ubuntu = Get-BenDistros | Where-Object { $_ -like 'Ubuntu*' } | Select-Object -First 1
    return $ubuntu
}

function Install-BenWsl {
    Write-BenStep 'Installing WSL2 and Ubuntu'
    Write-Host '  Windows will ask for administrator permission.'
    try {
        $proc = Start-Process -FilePath 'wsl.exe' -ArgumentList '--install', '-d', 'Ubuntu' -Verb RunAs -Wait -PassThru
    } catch {
        Write-BenFail 'Administrator permission was refused, so WSL could not be installed.'
        return
    }
    if ($proc.ExitCode -ne 0) {
        Write-BenFail "WSL install failed (code $($proc.ExitCode)). Check that virtualization is enabled in your BIOS."
        return
    }
    Write-Host ''
    Write-Host 'Almost there:' -ForegroundColor Green
    Write-Host '  1. Restart your PC.'
    Write-Host '  2. Open "Ubuntu" from the Start menu once and choose a username and password.'
    Write-Host '  3. Run this setup again. It continues from where it stopped.'
}

function Select-BenRom {
    Write-BenStep 'Choose your Majora''s Mask ROM'
    Write-Host '  It must be the USA Nintendo 64 version (.z64, .n64 or .v64).'
    Write-Host '  Cancel to let the setup search your Downloads, Desktop and Documents folders instead.'
    try {
        Add-Type -AssemblyName System.Windows.Forms
        $dialog = New-Object System.Windows.Forms.OpenFileDialog
        $dialog.Title = 'Select your Majora''s Mask (USA) N64 ROM'
        $dialog.Filter = 'N64 ROMs (*.z64;*.n64;*.v64)|*.z64;*.n64;*.v64|All files (*.*)|*.*'
        $dialog.InitialDirectory = [Environment]::GetFolderPath('UserProfile') + '\Downloads'
        if ($dialog.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) { return $dialog.FileName }
    } catch {
        Write-BenWarn 'Could not open a file picker; the setup will search for the ROM itself.'
    }
    return $null
}

# Convert a Windows path to the path Ubuntu sees (C:\Users\me\x.z64 -> /mnt/c/Users/me/x.z64).
function ConvertTo-BenWslPath([string]$Distro, [string]$WindowsPath) {
    $converted = & wsl.exe -d $Distro --exec wslpath -a -u $WindowsPath 2>$null
    if ($LASTEXITCODE -eq 0 -and $converted) { return ($converted | Select-Object -First 1).Trim() }
    if ($WindowsPath -match '^([A-Za-z]):\\(.*)$') {
        return '/mnt/' + $Matches[1].ToLower() + '/' + ($Matches[2] -replace '\\', '/')
    }
    return $null
}

function Invoke-BenWsl([string]$Distro, [string[]]$Arguments) {
    & wsl.exe -d $Distro --cd '~' --exec @Arguments
    return ($LASTEXITCODE -eq 0)
}

function New-BenShortcuts([string]$Distro) {
    try {
        $desktop = [Environment]::GetFolderPath('Desktop')
        $shell = New-Object -ComObject WScript.Shell
        $wsl = Join-Path $env:WINDIR 'System32\wsl.exe'
        $items = @(
            @{ Name = 'BEN DROWNED - Update & Build'; Command = 'update' },
            @{ Name = 'BEN DROWNED - Play'; Command = 'run' }
        )
        foreach ($item in $items) {
            $lnk = $shell.CreateShortcut((Join-Path $desktop ($item.Name + '.lnk')))
            $lnk.TargetPath = Join-Path $env:WINDIR 'System32\cmd.exe'
            $lnk.Arguments = "/c `"`"$wsl`" -d $Distro --cd ~ --exec $BenFolder/ben $($item.Command) & pause`""
            $lnk.WorkingDirectory = [Environment]::GetFolderPath('UserProfile')
            $lnk.Save()
        }
        Write-BenOk 'Desktop shortcuts: "BEN DROWNED - Update & Build" and "BEN DROWNED - Play"'
    } catch {
        Write-BenWarn "Could not create desktop shortcuts: $($_.Exception.Message)"
    }
}

function Invoke-BenSetup {
    Write-Host 'BEN DROWNED - Windows setup' -ForegroundColor White

    Write-BenStep 'Checking WSL (the Linux environment the build runs in)'
    if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
        Write-BenFail 'This version of Windows has no WSL. Windows 10 version 2004 or newer, or Windows 11, is required.'
        return
    }
    $distro = Find-BenUbuntu
    if (-not $distro) {
        Install-BenWsl
        return
    }
    if (-not (Invoke-BenWsl $distro @('true'))) {
        Write-BenFail "Ubuntu ($distro) is installed but didn't start. Open Ubuntu from the Start menu once to finish its setup, then run this again."
        return
    }
    Write-BenOk "Using $distro"

    $rom = Select-BenRom
    $romWsl = $null
    if ($rom) {
        $romWsl = ConvertTo-BenWslPath $distro $rom
        Write-BenOk "ROM: $rom"
    }

    Write-BenStep 'Getting the project'
    if (Invoke-BenWsl $distro @('test', '-d', "$BenFolder/.git")) {
        if (-not (Invoke-BenWsl $distro @('git', '-C', $BenFolder, 'pull', '--ff-only'))) {
            Write-BenWarn 'Could not update (local changes?). Continuing with the version you have.'
        }
    } elseif (-not (Invoke-BenWsl $distro @('git', 'clone', $BenRepoUrl, $BenFolder))) {
        Write-BenFail 'Downloading the project failed. Check your internet connection and run this again.'
        return
    }
    Write-BenOk "Project folder: \\wsl$\$distro\home\<you>\$BenFolder"

    Write-BenStep 'Running the build setup inside Ubuntu (first time takes 10-30 minutes)'
    Write-Host '  You may be asked for your Ubuntu password so it can install build tools.'
    $setupArgs = @("$BenFolder/ben", 'setup')
    if ($romWsl) { $setupArgs += $romWsl }
    if (-not (Invoke-BenWsl $distro $setupArgs)) {
        Write-BenFail 'Setup stopped (see the message above). Fix that, then run this again. It resumes where it stopped.'
        return
    }

    New-BenShortcuts $distro
    $out = Join-Path ([Environment]::GetFolderPath('UserProfile')) $BenFolder
    Write-Host ''
    Write-Host "All set. Your ROM is in $out" -ForegroundColor Green
    if (Test-Path $out) { Start-Process explorer.exe $out }
}

Invoke-BenSetup
