# A small script to build the Windows version of AboutThisPC.
$version = "0.0.0A-R4"

$WindowsDir = Split-Path -Path $MyInvocation.MyCommand.Path -Parent
$ParentDir = Split-Path -Path $WindowsDir -Parent
$OutputDir = "$ParentDir\Output\windows"

Write-Output "Creating About This PC version $version..."
Set-Location -Path "$WindowsDir\AboutThisPC"

function GetSed {
    param($Target)
    $TargetDir="$OutputDir\$Target-Exe"

    $content = @"
[Version]
Class=IEXPRESS
SEDVersion=3

[Options]
PackagePurpose=InstallApp
ShowInstallProgramWindow=1
HideExtractAnimation=0
UseLongFileName=1
OverwriteInstallFiles=2
TargetName=${TargetDir}\AboutThisPC-${Target}.exe
FriendlyName=AboutThisPC ${Target}
CreateCAB=0
InstallPrompt=%NoPrompt%
DisplayLicense=%No%
FinishMessage=%No%

[SourceFiles]
SourceFiles0=${OutputDir}\${Target}-Raw

[SourceFiles0]
*.*=*

[DestinationDirs]
DefaultDir=C:\Temp\Extracted

[InstallExecute]
AboutThisPC.exe
"@

    $path = "C:\Temp\AboutThisPC.sed"
    $directory = Split-Path $path -Parent

    if (-not (Test-Path $directory)) {
        New-Item -ItemType Directory -Path $directory -Force
    }

    if (-not (Test-Path $TargetDir)) {
        New-Item -ItemType Directory -Path $TargetDir -Force
    }

    $content | Out-File -FilePath $path -Encoding ASCII
    return $path;
}

function Build {
    param($Target)
    $sedPath=GetSed -Target "$Target"

    Write-Output "Building application for $Target..."
    #dotnet publish -c Release -r "$Target" --self-contained true -o "$OutputDir\$Target-Raw"

    Write-Output "Creating executable for path $sedPath..."
    & "iexpress.exe" /N /Q /M "$sedPath"
}

Build -Target "win-x64"
Build -Target "win-arm64"