param(
    [Parameter(Mandatory)][string]$Version,
    [Parameter(Mandatory)][string]$InputDirectory,
    [Parameter(Mandatory)][string]$OutputDirectory
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'command.ps1')
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$targets = (Get-Content (Join-Path $root 'sdk/archives.json') -Raw | ConvertFrom-Json).directory |
    ForEach-Object { ($_ -split '/')[-1] }
if ($targets.Count -ne 6) { throw 'Release requires exactly six maintained SDK targets' }
$stage = Join-Path $OutputDirectory ('stage-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $stage -Force | Out-Null
foreach ($target in $targets) {
    $source = Join-Path $InputDirectory "p4rust-$target/p4rust-$Version.crate"
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { throw "Missing platform package: $target" }
    if ((Get-Item -LiteralPath $source).Length -ge 10000000) { throw "Oversized platform package: $target" }
    $entries = @(Invoke-Native 'tar' -Arguments @('-tzf', $source) -Capture)
    $prefix = "p4rust-$Version/native/lib/$target/"
    if (-not ($entries | Where-Object { $_.StartsWith($prefix) })) { throw "Platform package has no matching native libraries: $target" }
    if ($entries | Where-Object { $_ -match '/native/lib/' -and -not $_.StartsWith($prefix) -and -not $_.EndsWith('/native/lib/') }) {
        throw "Platform package contains another target: $target"
    }
    $destination = Join-Path $stage $target
    New-Item -ItemType Directory -Path $destination | Out-Null
    Copy-Item -LiteralPath $source -Destination $destination
}
$archive = Join-Path $OutputDirectory "p4rust-$Version.zip"
Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $archive -Force
Write-Output $archive
