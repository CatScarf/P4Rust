param(
    [Parameter(Mandatory)][string]$LibraryDirectory,
    [string]$Objcopy = 'llvm-objcopy'
)
$ErrorActionPreference = 'Stop'
$libraries = @('p4rust_bridge', 'libclient', 'libp4script_cstub', 'librpc', 'libsupp', 'libssl', 'libcrypto')
$systemLibraries = @('ws2_32', 'advapi32', 'crypt32', 'user32', 'shell32', 'ole32', 'gdi32')
$env:VSLANG = '1033'

# Remove only debug sections while preserving COFF symbols and relocations.
function Remove-Debug([string]$Library) {
    $source = Join-Path $LibraryDirectory "$Library.lib"
    $stripped = Join-Path $LibraryDirectory "$Library-stripped.lib"
    & $Objcopy --strip-debug $source $stripped
    if ($LASTEXITCODE -ne 0) { throw "Failed to strip debug sections from $Library" }
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
    $members = & lib.exe /NOLOGO /LIST $source
    if ($LASTEXITCODE -ne 0) { throw "Failed to enumerate archive $Library" }
    $unreachable = @($members | Where-Object { -not $required.ContainsKey([IO.Path]::GetFileName($_.Trim())) })
    $output = Join-Path $LibraryDirectory "$Library-pruned.lib"
    $response = Join-Path $LibraryDirectory "$Library-prune.rsp"
    $arguments = @('/NOLOGO', '/BREPRO', ('"/OUT:' + $output + '"'), ('"' + $source + '"'))
    $arguments += @($unreachable | ForEach-Object { '"/REMOVE:' + $_.Trim() + '"' })
    $arguments | Set-Content -LiteralPath $response -Encoding ascii
    & lib.exe "@$response"
    if ($LASTEXITCODE -ne 0) { throw "Failed to prune unreachable archive objects in $Library" }
    Move-Item -LiteralPath $output -Destination $source -Force
    [pscustomobject]@{ library = $Library; total = $members.Count; retained = $members.Count - $unreachable.Count; removed = $unreachable.Count }
}

foreach ($library in $libraries) { Remove-Debug $library }
$probe = Join-Path $LibraryDirectory 'dependency-probe.dll'
$arguments = @('/NOLOGO', '/DLL', '/INCREMENTAL:NO', '/OPT:NOREF', '/VERBOSE', '/EXPORT:p4rust_abi_version', '/EXPORT:p4rust_execute_v1', '/EXPORT:p4rust_execute_controlled_v1', "/OUT:$probe", "/LIBPATH:$LibraryDirectory")
$arguments += @($libraries | ForEach-Object { "$_.lib" })
$arguments += @($systemLibraries | ForEach-Object { "$_.lib" })
$trace = & link.exe @arguments 2>&1
if ($LASTEXITCODE -ne 0) { throw "Failed to link the complete C ABI dependency probe: $trace" }
$trace | Set-Content (Join-Path $LibraryDirectory 'dependency-trace.txt') -Encoding utf8NoBOM
$inventory = @()
foreach ($library in @('libclient', 'libp4script_cstub', 'librpc', 'libsupp')) {
    $inventory += Remove-Unreachable $library $trace
}
& link.exe @arguments *> (Join-Path $LibraryDirectory 'pruned-link.log')
if ($LASTEXITCODE -ne 0) { throw 'Failed to link the C ABI after object pruning' }
$inventory | Format-Table -AutoSize
