param([string]$ServerEndpoint = '')
$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$target = $env:P4RUST_TARGET
if (-not $target) {
    $target = ((rustc -vV | Select-String '^host: ').Line -replace '^host: ', '')
    if ($LASTEXITCODE -ne 0) { throw 'Failed to read Rust host' }
}
$fixture = Join-Path $root ('temp/release-' + [Guid]::NewGuid().ToString('N'))
$vendor = Join-Path $fixture 'vendor'
$consumer = Join-Path $fixture 'consumer'
New-Item -ItemType Directory -Path $vendor, $consumer, (Join-Path $consumer '.cargo') | Out-Null

# Stop immediately when an external verification command fails.
function Confirm-Command([string]$Operation) {
    if ($LASTEXITCODE -ne 0) { throw "Failed to $Operation (exit $LASTEXITCODE)" }
}

# Generate Cargo directory-source checksums from the actual packaged files.
function Write-Checksum([string]$Directory, [string]$Archive) {
    $files = @{}
    foreach ($file in Get-ChildItem -LiteralPath $Directory -File -Recurse) {
        $relative = [IO.Path]::GetRelativePath($Directory, $file.FullName).Replace('\', '/')
        if ($relative -ne '.cargo-checksum.json') {
            $files[$relative] = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        }
    }
    $checksum = @{ files = $files; package = (Get-FileHash -LiteralPath $Archive -Algorithm SHA256).Hash.ToLowerInvariant() }
    $checksum | ConvertTo-Json -Depth 5 -Compress | Set-Content (Join-Path $Directory '.cargo-checksum.json') -Encoding utf8NoBOM
}

# Verify TARGET selection and reject an incompatible CRT before linker metadata is emitted.
function Confirm-TargetGuards([string]$BuildScript) {
    $previous = @{}
    foreach ($key in @('TARGET', 'HOST', 'CARGO_CFG_TARGET_FEATURE')) { $previous[$key] = [Environment]::GetEnvironmentVariable($key, 'Process') }
    try {
        $env:HOST = 'x86_64-pc-windows-msvc'
        $env:TARGET = 'i686-pc-windows-msvc'
        $env:CARGO_CFG_TARGET_FEATURE = ''
        $output = & $BuildScript 2>&1 | Out-String
        if ($LASTEXITCODE -eq 0 -or $output -notmatch 'unavailable for i686-pc-windows-msvc') { throw 'Unsupported TARGET guard did not reject cross-compilation' }
        if ($target.EndsWith('windows-msvc')) {
            $env:TARGET = $target
            $env:CARGO_CFG_TARGET_FEATURE = 'crt-static'
            $output = & $BuildScript 2>&1 | Out-String
            if ($LASTEXITCODE -eq 0 -or $output -notmatch 'shared MSVC CRT') { throw 'Static CRT guard did not reject incompatible libraries' }
        }
        'Unsupported TARGET and crt-static guards passed.' | Set-Content (Join-Path $fixture 'target-guards.txt') -Encoding utf8NoBOM
    } finally {
        foreach ($key in $previous.Keys) { [Environment]::SetEnvironmentVariable($key, $previous[$key], 'Process') }
    }
}

Push-Location $root
try {
    cargo package -p p4rust --offline --allow-dirty --target $target *> (Join-Path $fixture 'package.log')
    Confirm-Command 'package and verify the release'
    $metadata = cargo metadata --no-deps --format-version 1 | ConvertFrom-Json
    Confirm-Command 'read workspace package metadata'
    $packages = $metadata.packages | Where-Object { $_.name -eq 'p4rust' }
    $sizes = @()
    foreach ($package in $packages) {
        $name = "$($package.name)-$($package.version)"
        $archive = Join-Path $metadata.target_directory "package/$name.crate"
        $bytes = (Get-Item -LiteralPath $archive).Length
        if ($bytes -ge 10000000) { throw "Release package exceeds the limit: $name ($bytes bytes)" }
        tar -xzf $archive -C $vendor
        Confirm-Command "extract $name"
        Write-Checksum (Join-Path $vendor $name) $archive
        $sizes += [pscustomobject]@{ name = $name; bytes = $bytes }
    }
    $sizes | ConvertTo-Json | Set-Content (Join-Path $fixture 'package-sizes.json') -Encoding utf8NoBOM
    $version = $packages[0].version
    @"
[package]
name = "p4rust-external-consumer"
version = "0.0.0"
edition = "2024"
[workspace]
[dependencies]
p4rust = "=$version"
"@ | Set-Content (Join-Path $consumer 'Cargo.toml') -Encoding utf8NoBOM
    @'
[source.crates-io]
replace-with = "release-fixture"
[source.release-fixture]
directory = "../vendor"
[net]
offline = true
'@ | Set-Content (Join-Path $consumer '.cargo/config.toml') -Encoding utf8NoBOM
    New-Item -ItemType Directory -Path (Join-Path $consumer 'src') | Out-Null
    @'
// Exercise the packaged safe API from a completely independent Cargo workspace.
fn main() -> Result<(), Box<dyn std::error::Error>> {
    let endpoint = std::env::args().nth(1).unwrap_or("127.0.0.1:1".to_owned());
    let client = p4rust::Client::new(p4rust::Config::new(&endpoint, "tester", "test-client"))
        .map_err(|error| format!("Failed to initialize external client: {error:#}"))?;
    match client.run("info", &[]) {
        Ok(output) => println!("External consumer received {} records", output.records.len()),
        Err(error) if endpoint == "127.0.0.1:1" => println!("External consumer propagated: {error:#}"),
        Err(error) => return Err(format!("Failed to query external server: {error:#}").into()),
    }
    Ok(())
}
'@ | Set-Content (Join-Path $consumer 'src/main.rs') -Encoding utf8NoBOM
    $trapName = if ($IsWindows) { 'native-compiler-trap.cmd' } else { 'native-compiler-trap.sh' }
    $trap = Join-Path $fixture $trapName
    $probe = Join-Path $fixture 'native-compiler-invocations.txt'
    if ($IsWindows) {
        "@echo off`r`necho native compiler invoked >> `"$probe`"`r`nexit /b 97" | Set-Content $trap -Encoding ascii
    } else {
        "#!/bin/sh`necho invoked >> '$probe'`nexit 97" | Set-Content $trap -Encoding ascii
        chmod +x $trap
        Confirm-Command 'make the native compiler trap executable'
    }
    $suffix = $target.Replace('-', '_')
    $variables = @('CC', 'CXX', 'AR', "CC_$suffix", "CXX_$suffix", "AR_$suffix")
    $previous = @{}
    foreach ($variable in $variables) {
        $previous[$variable] = [Environment]::GetEnvironmentVariable($variable, 'Process')
        [Environment]::SetEnvironmentVariable($variable, $trap, 'Process')
    }
    Push-Location $consumer
    try {
        cargo build --offline -vv --target $target --target-dir (Join-Path $fixture 'target') *> (Join-Path $fixture 'consumer-build.log')
        Confirm-Command 'build an offline external consumer'
        cargo tree --offline | Set-Content (Join-Path $fixture 'consumer-tree.txt') -Encoding utf8NoBOM
        Confirm-Command 'inspect external consumer dependencies'
        $tree = Get-Content (Join-Path $fixture 'consumer-tree.txt') -Raw
        if (@($tree -split '\r?\n' | Where-Object { $_ -match ' v[0-9]' }).Count -ne 2) { throw 'Public crate must have no dependencies' }
        if (Test-Path -LiteralPath $probe) { throw 'Consumer attempted native compilation' }
        $scriptName = if ($IsWindows) { 'build-script-build.exe' } else { 'build-script-build' }
        $buildScript = Get-ChildItem (Join-Path $fixture "target/debug/build/p4rust-*/$scriptName") | Select-Object -First 1
        if (-not $buildScript) { throw 'Failed to locate packaged Rust build script' }
        Confirm-TargetGuards $buildScript.FullName
        $programName = if ($IsWindows) { 'p4rust-external-consumer.exe' } else { 'p4rust-external-consumer' }
        $exe = Join-Path $fixture "target/$target/debug/$programName"
        if ($ServerEndpoint) { & $exe $ServerEndpoint *> (Join-Path $fixture 'consumer-run.log') }
        else { & $exe *> (Join-Path $fixture 'consumer-run.log') }
        Confirm-Command 'run the packaged external consumer'
    } finally {
        Pop-Location
        foreach ($variable in $variables) { [Environment]::SetEnvironmentVariable($variable, $previous[$variable], 'Process') }
    }
    $sizes | Format-Table -AutoSize
    Write-Output "Release verification passed. Evidence: $fixture"
} finally { Pop-Location }
