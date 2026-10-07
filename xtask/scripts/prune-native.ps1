param(
    [Parameter(Mandatory)][string]$LibraryDirectory,
    [string]$Objcopy = 'llvm-objcopy'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'command.ps1')
$libraries = @('p4rust_bridge', 'libclient', 'libp4script_cstub', 'librpc', 'libsupp', 'libssl', 'libcrypto')
$systemLibraries = @('ws2_32', 'advapi32', 'crypt32', 'user32', 'shell32', 'ole32', 'gdi32')
$env:VSLANG = '1033'

# Remove only debug sections while preserving COFF symbols and relocations.
function Remove-Debug([string]$Library) {
    $source = Join-Path $LibraryDirectory "$Library.lib"
    $stripped = Join-Path $LibraryDirectory "$Library-stripped.lib"
    Invoke-Native $Objcopy -Arguments @('--strip-debug', $source, $stripped)
    Move-Item -LiteralPath $stripped -Destination $source -Force
}

# Retain the complete object closure of the versioned C ABI, including cold paths.
function Remove-Unreachable([string]$Library, [string[]]$Trace) {
    $required = @{}
    foreach ($line in $Trace) {
        if ($line -match ('Loaded ' + [regex]::Escape($Library) + '\.lib\(([^)]+)\)')) {
            $required[$Matches[1]] = $true
        }
    }
    if ($required.Count -eq 0) { throw "Failed to discover any reachable objects in $Library" }
    $source = Join-Path $LibraryDirectory "$Library.lib"
    $members = Invoke-Native 'lib.exe' -Arguments @('/NOLOGO', '/LIST', $source) -Capture
    $unreachable = @($members | Where-Object { -not $required.ContainsKey([IO.Path]::GetFileName($_.Trim())) })
    $output = Join-Path $LibraryDirectory "$Library-pruned.lib"
    $response = Join-Path $LibraryDirectory "$Library-prune.rsp"
    $arguments = @('/NOLOGO', '/BREPRO', ('"/OUT:' + $output + '"'), ('"' + $source + '"'))
    $arguments += @($unreachable | ForEach-Object { '"/REMOVE:' + $_.Trim() + '"' })
    $arguments | Set-Content -LiteralPath $response -Encoding ascii
    Invoke-Native 'lib.exe' -Arguments @("@$response")
    Move-Item -LiteralPath $output -Destination $source -Force
    [pscustomobject]@{ library = $Library; total = $members.Count; retained = $members.Count - $unreachable.Count; removed = $unreachable.Count }
}

foreach ($library in $libraries) { Remove-Debug $library }
$probe = Join-Path $LibraryDirectory 'dependency-probe.dll'
$arguments = @('/NOLOGO', '/DLL', '/INCREMENTAL:NO', '/OPT:NOREF', '/VERBOSE', '/EXPORT:p4rust_abi_version', '/EXPORT:p4rust_execute_v1', '/EXPORT:p4rust_execute_controlled_v1', "/OUT:$probe", "/LIBPATH:$LibraryDirectory")
$arguments += @($libraries | ForEach-Object { "$_.lib" })
$arguments += @($systemLibraries | ForEach-Object { "$_.lib" })
$trace = Invoke-Native 'link.exe' -Arguments $arguments -Capture
$trace | Set-Content (Join-Path $LibraryDirectory 'dependency-trace.txt') -Encoding utf8NoBOM
$inventory = @()
foreach ($library in @('libclient', 'libp4script_cstub', 'librpc', 'libsupp')) {
    $inventory += Remove-Unreachable $library $trace
}
Invoke-Native 'link.exe' -Arguments $arguments -Capture | Set-Content (Join-Path $LibraryDirectory 'pruned-link.log')
$inventory | Format-Table -AutoSize
