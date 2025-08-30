param (
    [Parameter(Mandatory=$true)]
    [string]$Version,
    [Parameter()]
    [string]$Architecture,
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$RemainingArgs
)

$ServiceDir = $PSScriptRoot
$ParentDir = (Get-Item $ServiceDir).Parent.FullName
$BuildDir = "$ServiceDir\build"

Write-Output "Starting runner with version $Version..."
Set-Location $ServiceDir

Write-Output "Copying required build files..."
Copy-Item "$ParentDir\Output\windows\win-$Architecture-Raw-Archive.zip" "$BuildDir\archive.zip"
Copy-Item "$ParentDir\Output\windows\win-$Architecture-Raw\Assets\appicon.ico" "$BuildDir\icon.ico"

Write-Output "Running service with args '$RemainingArgs'..."
& go run -ldflags="-X 'main.Version=$Version' -X 'main.IsConsole=1'" . $RemainingArgs