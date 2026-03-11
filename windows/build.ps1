param (
    [Parameter(Mandatory=$true)]
    [string]$Version,
    [switch]$SkipSigning,
    [switch]$BuildDebug
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

function CheckCert {
    if ($SkipSigning) {
        Write-Host "Skipping certificate checks"
        return
    }

    $Cert = Get-ChildItem -Path Cert:\CurrentUser\My -CodeSigningCert | Select-Object -First 1

    if ($null -eq $Cert) {
        Write-Error "No code signing certificate found."
        exit 1
    }

    try {
        $ReadStore = [System.Security.Cryptography.X509Certificates.X509Store]::new("Root", "CurrentUser")
        $ReadStore.Open("ReadOnly")
        $existing = $ReadStore.Certificates | Where-Object { $_.Thumbprint -eq $Cert.Thumbprint }
        $ReadStore.Close()

        if (-not $existing) {
            $isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
            if (-not $isAdmin) {
                Write-Error "Certificate is not trusted. Re-run this script as Administrator to trust it, or use -SkipSigning."
                exit 1
            }
            $WriteStore = [System.Security.Cryptography.X509Certificates.X509Store]::new("Root", "LocalMachine")
            $WriteStore.Open("ReadWrite")
            Write-Output "Adding certificate to trusted root store..."
            $WriteStore.Add($Cert)
            $WriteStore.Close()
        }
    } catch {
        Write-Error "Failed to update certificate store: $_"
        exit 1
    }
}

CheckCert
New-Item -Path $OutputDir -ItemType Directory
Write-Output "Creating About This PC version $Version..."
Set-Location -Path "$WindowsDir\AboutThisPC"
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

# New-SelfSignedCertificate -Subject "CN=AboutThisPC Code Signing" -Type CodeSigningCert -CertStoreLocation "Cert:\CurrentUser\My" -KeyUsage DigitalSignature -NotAfter (Get-Date).AddYears(3)

function Sign {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Path
    )

    if ($SkipSigning) {
        Write-Host "Skipping code signing"
        return
    }

    $Cert = Get-ChildItem -Path Cert:\CurrentUser\My -CodeSigningCert | Select-Object -First 1
    Write-Output $Cert

    if ($null -eq $Cert) {
        Write-Error "No code signing certificate found."
        return
    }

    Set-AuthenticodeSignature -FilePath "$Path" -Certificate $Cert -HashAlgorithm SHA256 -TimestampServer "http://timestamp.digicert.com"
    Get-AuthenticodeSignature -FilePath "$Path"
}

function Build {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Arch,
        [Parameter(Mandatory=$true)]
        [string]$Target
    )

    $directory="$OutputDir\$Target-Raw"
    $ServicePath="$ParentDir\windows-service\build"
    $CompanionPath="$ParentDir\companion\build\windows\x64\runner\Release"

    Write-Output "Building application for $Target..."
    & dotnet publish -c Release -r "$Target" --self-contained true -o "$directory"
    Sign -Path "$directory\AboutThisPC.exe"

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

    Copy-Item -Path "$OutputDir\$Target-Raw-Archive.zip" -Destination "$ServicePath\archive.zip"
    Copy-Item -Path "$WindowsDir\AboutThisPC\Assets\appicon.ico" -Destination "$ServicePath\icon.ico"

    Write-Output "Creating service application..."
    Set-Location -Path "$ParentDir\windows-service"
    $env:GOOS = "windows"
    $env:GOARCH = $Arch

    # `-H=windowsgui` needs to be present in the flags to prevent the console from showing up, but it also prevents logging.
    & rsrc -arch $Arch -ico build/icon.ico -o rsrc.syso
    & go build -ldflags "-X 'main.Version=$Version' -H=windowsgui" -o build\app.exe
    if ($BuildDebug) {& go build -ldflags "-X 'main.Version=$Version' -X 'main.IsConsole=1'" -o build\app-cli.exe}

    Sign -Path "$ServicePath\app.exe"
    if ($BuildDebug) {Sign -Path "$ServicePath\app-cli.exe"}

    $directory="$OutputDir\$Target-Results"
    $ArchivePath="$ParentDir\Output\AboutThisPC-$Version-$Target.zip"

    Set-Location -Path "$WindowsDir\AboutThisPC"
    Write-Output "Creating result for $directory..."

    if (-not (Test-Path $directory)) {
        New-Item -Path $directory -ItemType Directory
    }

    Copy-Item -Path "$ServicePath\app.exe" -Destination "$directory\AboutThisPC.exe"
    if ($BuildDebug) {Copy-Item -Path "$ServicePath\app-cli.exe" -Destination "$directory\AboutThisPC-Debug.exe"}
    Copy-Item -Path "$CompanionPath\*" -Destination "$directory\" -Recurse
    Rename-Item -Path "$directory\companion.exe" -NewName "$directory\AboutThisPC-Launcher.exe"
    #Copy-Item -Path "$OutputDir\shortcut.lnk" -Destination "$directory\About This PC.lnk"
    Copy-Item -Path "$ParentDir\README.md" -Destination "$directory\README.md"
    Copy-Item -Path "$ParentDir\LICENSE.md" -Destination "$directory\LICENSE.md"
    Copy-Item -Path "$ParentDir\SECURITY.md" -Destination "$directory\SECURITY.md"
    Copy-Item -Path "$ParentDir\CONTRIBUTING.md" -Destination "$directory\CONTRIBUTING.md"
    Copy-Item -Path "$ParentDir\CODE_OF_CONDUCT.md" -Destination "$directory\CODE_OF_CONDUCT.md"
    Copy-Item -Path "$ParentDir\INSTALLING.md" -Destination "$directory\INSTALLING.md"
    Set-Content -Path "$directory\version" -Value "$Version"

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

#Shortcut
Write-Output "Building companion app..."
Set-Location -Path "$ParentDir\companion"
& flutter build windows
Set-Location -Path "$ParentDir"
Set-Location -Path "$WindowsDir\AboutThisPC"

Build -Arch "amd64" -Target "win-x64"
Build -Arch "arm64" -Target "win-arm64"

Set-Location -Path "$ParentDir"
$stopwatch.Stop()
Write-Output "All jobs done at $((Get-Date).ToString("MM/dd HH:mm:ss")) ($($stopwatch.Elapsed) elapsed)! Version: $Version"