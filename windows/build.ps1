# A small script to build the Windows version of AboutThisPC.
$version = "0.0.0A-R4"

$WindowsDir = Split-Path -Path $MyInvocation.MyCommand.Path -Parent
$ParentDir = Split-Path -Path $WindowsDir -Parent
$OutputDir = "$ParentDir\Output\windows"

if (Test-Path $OutputDir) {
    Remove-Item $OutputDir -Recurse -Force
}

Write-Output "Creating About This PC version $version..."
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

    # `-H=windowsgui` needs to be present in the flags to prevent the console from showing up
    & rsrc -arch $Arch -ico build/icon.ico -o rsrc.syso
    & go build -ldflags "-X 'main.Version=$version'" -o build/app.exe

    $directory="$OutputDir\$Target-Results"
    $ArchivePath="$ParentDir\Output\AboutThisPC-$version-$Target.zip"

    Set-Location -Path "$WindowsDir\AboutThisPC"
    Write-Output "Creating result for $directory..."

    if (-not (Test-Path $directory)) {
        New-Item -Path $directory -ItemType Directory
    }

    Copy-Item -Path "$ServicePath\app.exe" -Destination "$directory\AboutThisPC.exe"
    Copy-Item -Path "$ParentDir\README.md" -Destination "$directory\README.md"
    Copy-Item -Path "$ParentDir\LICENSE.md" -Destination "$directory\LICENSE.md"
    Copy-Item -Path "$ParentDir\SECURITY.md" -Destination "$directory\SECURITY.md"
    Copy-Item -Path "$ParentDir\CONTRIBUTING.md" -Destination "$directory\CONTRIBUTING.md"
    Copy-Item -Path "$ParentDir\CODE_OF_CONDUCT.md" -Destination "$directory\CODE_OF_CONDUCT.md"

    Write-Output "Creating archive at $ArchivePath..."
    Compress-Archive -Force -Path "$directory\*" -DestinationPath "$ArchivePath"
}

Build -Arch "amd64" -Target "win-x64"
Build -Arch "arm64" -Target "win-arm64"

Set-Location -Path "$ParentDir"
$stopwatch.Stop()
Write-Output "All jobs done at $((Get-Date).ToString("MM/dd HH:mm:ss")) ($($stopwatch.Elapsed) elapsed)! Version: $version"