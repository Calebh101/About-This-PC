param (
    [Parameter(Mandatory=$true)]
    [string]$Version,
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$RemainingArgs
)

$ServiceDir = $PSScriptRoot
Set-Location $ServiceDir

$GoArgs = $RemainingArgs | Where-Object { $_ -like '--*' }
Write-Output "Running service with version $Version and args '$GoArgs'..."
& go run -ldflags="-X 'main.Version=$Version' -X 'main.IsConsole=1'" .