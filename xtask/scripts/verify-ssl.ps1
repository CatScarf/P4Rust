param(
    [string]$ServerExecutable,
    [ValidateSet('RSA', 'EC')][string]$Certificate = 'RSA',
    [ValidateSet('12', '13')][string]$TlsVersion = '13',
    [ValidateSet('DEFAULT', 'CAMELLIA256-SHA')][string]$CipherList = 'DEFAULT',
    [string]$ExternalConsumerExecutable = ''
)
$ErrorActionPreference = 'Stop'
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not $ServerExecutable) { $ServerExecutable = Join-Path $root 'temp/p4d.exe' }
$fixture = Join-Path $root ('temp/ssl-' + $Certificate + '-' + $TlsVersion + '-' + [Guid]::NewGuid().ToString('N'))
$ssl = Join-Path $fixture 'certificates'
New-Item -ItemType Directory -Path $fixture, $ssl | Out-Null
$variables = @('P4SSLDIR', 'P4TRUST', 'P4TICKETS', 'P4CONFIG', 'P4RUST_TEST_PORT', 'P4RUST_TEST_FINGERPRINT')
$previous = @{}
foreach ($variable in $variables) { $previous[$variable] = [Environment]::GetEnvironmentVariable($variable, 'Process') }
$server = $null
Push-Location $root
try {
    $env:P4SSLDIR = $ssl
    $env:P4TRUST = Join-Path $fixture 'trust.txt'
    $env:P4TICKETS = Join-Path $fixture 'tickets.txt'
    $env:P4CONFIG = Join-Path $fixture 'client.p4config'
    "ssl.client.cipher.list=$CipherList" | Set-Content $env:P4CONFIG -Encoding ascii
    $config = Join-Path $ssl 'openssl.cnf'
    "[req]`ndistinguished_name=dn`nprompt=no`n[dn]`nCN=P4Rust-Disposable-Fixture" | Set-Content $config -Encoding ascii
    $keyArguments = if ($Certificate -eq 'RSA') { @('-newkey', 'rsa:3072') } else { @('-newkey', 'ec', '-pkeyopt', 'ec_paramgen_curve:P-256') }
    openssl req -x509 -nodes @keyArguments -config $config -keyout (Join-Path $ssl 'privatekey.txt') -out (Join-Path $ssl 'certificate.txt') -days 2 -subj '/CN=P4Rust-Disposable-Fixture' *> (Join-Path $fixture 'certificate.log')
    if ($LASTEXITCODE -ne 0) { throw 'Failed to generate disposable SSL certificate' }
    $fingerprint = & $ServerExecutable -r $fixture -Gf 2>&1
    if ($LASTEXITCODE -ne 0) { throw "Failed to read SSL fingerprint: $fingerprint" }
    $env:P4RUST_TEST_FINGERPRINT = ($fingerprint | Select-String '(?:[0-9A-Fa-f]{2}:){19}[0-9A-Fa-f]{2}').Matches.Value
    if (-not $env:P4RUST_TEST_FINGERPRINT) { throw "Failed to parse SSL fingerprint: $fingerprint" }
    $listener = [Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback, 0)
    $listener.Start()
    $port = $listener.LocalEndpoint.Port
    $listener.Stop()
    $endpoint = "ssl:127.0.0.1:$port"
    $env:P4RUST_TEST_PORT = $endpoint
    $server = Start-Process -FilePath $ServerExecutable -ArgumentList @('-r', $fixture, '-p', $endpoint, '-v', "ssl.cipher.list=$CipherList", '-v', "ssl.tls.version.min=$TlsVersion", '-v', "ssl.tls.version.max=$TlsVersion", '-L', (Join-Path $fixture 'server.log'), '-q') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $fixture 'stdout.log') -RedirectStandardError (Join-Path $fixture 'stderr.log')
    $ready = $false
    for ($attempt = 0; $attempt -lt 50; $attempt++) {
        if ($server.HasExited) { throw 'Failed to start isolated SSL server' }
        $socket = [Net.Sockets.TcpClient]::new()
        try { $socket.Connect('127.0.0.1', $port); $ready = $true; break }
        catch { Start-Sleep -Milliseconds 100 }
        finally { $socket.Dispose() }
    }
    if (-not $ready) { throw 'Failed to wait for SSL server readiness' }
    cargo test -p p4rust client_tests::ssl_server_commands -- --ignored --exact *> (Join-Path $fixture 'test.log')
    if ($LASTEXITCODE -ne 0) { throw "Failed SSL validation; see $fixture/test.log" }
    if ($ExternalConsumerExecutable) {
        & $ExternalConsumerExecutable $endpoint *> (Join-Path $fixture 'external-consumer.log')
        if ($LASTEXITCODE -ne 0) { throw "Failed packaged SSL consumer; see $fixture/external-consumer.log" }
    }
    Write-Output "SSL $Certificate TLS $TlsVersion $CipherList validation passed. Evidence: $fixture"
} finally {
    if ($server -and -not $server.HasExited) { Stop-Process -Id $server.Id }
    foreach ($variable in $variables) { [Environment]::SetEnvironmentVariable($variable, $previous[$variable], 'Process') }
    Pop-Location
}
