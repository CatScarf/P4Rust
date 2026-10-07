# Print and execute every native command with consistent exit-code handling.
function Invoke-Native([string]$Program, [string[]]$Arguments, [switch]$Capture) {
    Write-Host ('> ' + $Program + ' ' + ($Arguments -join ' '))
    if ($Capture) {
        $output = & $Program @Arguments 2>&1
    } else {
        & $Program @Arguments
    }
    if ($LASTEXITCODE -ne 0) { throw "Failed to execute $Program (exit code $LASTEXITCODE): $output" }
    if ($Capture) { return $output }
}
