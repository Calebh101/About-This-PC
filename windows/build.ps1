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
    param($Target)
    $directory="$OutputDir\$Target-Raw"

    Write-Output "Building application for $Target..."
    dotnet publish -c Release -r "$Target" --self-contained true -o "$directory"

    Write-Output "Creating archive for $directory..."
    Compress-Archive -Path "$directory\*" -DestinationPath "$OutputDir\$Target-Raw-Archive"

    $directory="$OutputDir\$Target-Results"
    $ArchivePath="$ParentDir\Output\AboutThisPC-$version-$Target.zip"
    Write-Output "Creating result for $directory..."

    if (-not (Test-Path $directory)) {
        New-Item -Path $directory -ItemType Directory
    }

    Copy-Item -Path "$OutputDir\$Target-Raw-Archive.zip" -Destination "$directory\AboutThisPC-Package.zip"
    Copy-Item -Path "$ParentDir\README.md" -Destination "$directory\README.md"
    Copy-Item -Path "$ParentDir\LICENSE.md" -Destination "$directory\LICENSE.md"
    Copy-Item -Path "$ParentDir\SECURITY.md" -Destination "$directory\SECURITY.md"
    Copy-Item -Path "$ParentDir\CONTRIBUTING.md" -Destination "$directory\CONTRIBUTING.md"
    Copy-Item -Path "$ParentDir\CODE_OF_CONDUCT.md" -Destination "$directory\CODE_OF_CONDUCT.md"

    if (Test-Path $OutputDir) {
        Remove-Item $OutputDir -Recurse -Force
    }

    if (Test-Path $ArchivePath) {
        Remove-Item $ArchivePath -Recurse -Force
    }

    Write-Output "Creating archive at $ArchivePath..."
    Compress-Archive -Path "$directory\*" -DestinationPath "$ArchivePath"
}

Build -Target "win-x64"
Build -Target "win-arm64"

$stopwatch.Stop()
Write-Output "All jobs done after $($stopwatch.Elapsed)! Version: $version"