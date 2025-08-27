param (
    [Parameter(Mandatory=$true)]
    [string]$Version
)

# A small script to build and package About This PC for Windows.
$Author = "Calebh101"

if ($env:OS -ne "Windows_NT") {
    Write-Host "This script must be run on Windows."
    exit 1
}

$WindowsDir = Split-Path -Path $MyInvocation.MyCommand.Path -Parent
$ParentDir = Split-Path -Path $WindowsDir -Parent
$OutputDir = "$ParentDir\Output\windows"
$AppDataDir = "$env:LOCALAPPDATA\AboutThisPC"

if (Test-Path $OutputDir) {
    Remove-Item $OutputDir -Recurse -Force
}

New-Item -Path $OutputDir -ItemType Directory
Write-Output "Creating About This PC version $Version..."
Set-Location -Path "$WindowsDir\AboutThisPC"
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

function Build {
    param(
        [string]$Arch,
        [string]$Target
    )

    $directory="$OutputDir\$Target-Raw"
    $ServicePath="$ParentDir\windows-service\build"
    Write-Output "Building application for $Target..."
    & dotnet publish -c Release -r "$Target" --self-contained true -o "$directory"

    if (Test-Path $ServicePath) {
        Remove-Item "$ServicePath\*" -Recurse -Force
    } else {
        New-Item -Path "$ServicePath" -ItemType Directory
    }

    if (Test-Path "$ParentDir\windows-service\rsrc.syso") {
        Remove-Item "$ParentDir\windows-service\rsrc.syso" -Force
    }

    Write-Output "Creating archive for $directory..."
    Compress-Archive -Path "$directory\*" -DestinationPath "$OutputDir\$Target-Raw-Archive"

    Write-Output "Creating service application..."
    Copy-Item -Path "$OutputDir\$Target-Raw-Archive.zip" -Destination "$ServicePath\archive.zip"
    Copy-Item -Path "$WindowsDir\AboutThisPC\Assets\appicon.ico" -Destination "$ServicePath\icon.ico"

    Set-Location -Path "$ParentDir\windows-service"
    $env:GOOS = "windows"
    $env:GOARCH = $Arch

    # `-H=windowsgui` needs to be present in the flags to prevent the console from showing up, but it also prevents logging.
    & rsrc -arch $Arch -ico build/icon.ico -o rsrc.syso
    & go build -ldflags "-X 'main.Version=$Version' -H=windowsgui" -o build/app.exe
    & go build -ldflags "-X 'main.Version=$Version' -X 'main.IsConsole=1'" -o build/app-cli.exe

    $directory="$OutputDir\$Target-Results"
    $ArchivePath="$ParentDir\Output\AboutThisPC-$Version-$Target.zip"

    Set-Location -Path "$WindowsDir\AboutThisPC"
    Write-Output "Creating result for $directory..."

    if (-not (Test-Path $directory)) {
        New-Item -Path $directory -ItemType Directory
    }

    Copy-Item -Path "$ServicePath\app.exe" -Destination "$directory\AboutThisPC.exe"
    Copy-Item -Path "$ServicePath\app-cli.exe" -Destination "$directory\AboutThisPC-Debug.exe"
    Copy-Item -Path "$OutputDir\shortcut.lnk" -Destination "$directory\About This PC.lnk"
    Copy-Item -Path "$ParentDir\README.md" -Destination "$directory\README.md"
    Copy-Item -Path "$ParentDir\LICENSE.md" -Destination "$directory\LICENSE.md"
    Copy-Item -Path "$ParentDir\SECURITY.md" -Destination "$directory\SECURITY.md"
    Copy-Item -Path "$ParentDir\CONTRIBUTING.md" -Destination "$directory\CONTRIBUTING.md"
    Copy-Item -Path "$ParentDir\CODE_OF_CONDUCT.md" -Destination "$directory\CODE_OF_CONDUCT.md"
    Copy-Item -Path "$ParentDir\INSTALLING.md" -Destination "$directory\INSTALLING.md"

    Write-Output "Creating archive at $ArchivePath..."
    Compress-Archive -Force -Path "$directory\*" -DestinationPath "$ArchivePath"
}

function Shortcut() {
    $shortcutPath = "$OutputDir\shortcut.lnk"
    $targetPath = "$AppDataDir\AboutThisPC-Service.exe"
    Write-Output "Building shortcut for $targetPath..."

    $wsh = New-Object -ComObject WScript.Shell
    $shortcut = $wsh.CreateShortcut($shortcutPath)
    $shortcut.TargetPath = $targetPath
    $shortcut.Description = "About This PC $Version by $Author"
    $shortcut.IconLocation = "$AppDataDir\Assets\appicon.ico"
    $shortcut.Save()
}

Shortcut
Build -Arch "amd64" -Target "win-x64"
Build -Arch "arm64" -Target "win-arm64"

Set-Location -Path "$ParentDir"
$stopwatch.Stop()
Write-Output "All jobs done at $((Get-Date).ToString("MM/dd HH:mm:ss")) ($($stopwatch.Elapsed) elapsed)! Version: $Version"