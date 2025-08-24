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
    Write-Output "Building application for $Target..."
    dotnet publish -c Release -r "$Target" --self-contained true -o "$directory"

    Write-Output "Creating archive for $directory..."
    Compress-Archive -Path "$directory\*" -DestinationPath "$OutputDir\$Target-Raw-Archive"

    Write-Output "Creating service application..."
    Copy-Item -Path "$OutputDir\$Target-Raw-Archive.zip" -Destination "$ParentDir\windows-service\build\archive.zip"
    Copy-Item -Path "$WindowsDir\AboutThisPC\Assets\appicon.png" -Destination "$ParentDir\windows-service\build\icon.png"

    Set-Location -Path "$ParentDir\windows-service"
    $env:GOOS = "windows"
    $env:GOARCH = $Arch
    & go build -ldflags "-H=windowsgui -X 'main.Version=$version'" -o build/app.exe # `-H=windowsgui` needs to be present in the flags to prevent the console from showing up
    Set-Location -Path "$WindowsDir\AboutThisPC"

    $directory="$OutputDir\$Target-Results"
    $ArchivePath="$ParentDir\Output\AboutThisPC-$version-$Target.zip"
    Write-Output "Creating result for $directory..."

    if (-not (Test-Path $directory)) {
        New-Item -Path $directory -ItemType Directory
    }

    Copy-Item -Path "$ParentDir\windows-service\build\app.exe" -Destination "$directory\AboutThisPC.exe"
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
Write-Output "All jobs done after $($stopwatch.Elapsed)! Version: $version"