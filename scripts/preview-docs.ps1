param(
    [ValidateRange(1024, 65535)]
    [int]$Port = 8765
)

$ErrorActionPreference = 'Stop'
$siteRoot = Join-Path (Split-Path -Parent $PSScriptRoot) 'docs/site-docs'
Write-Host "Atlas preview: http://127.0.0.1:$Port/ (Ctrl+C to stop)"
python -m http.server $Port --bind 127.0.0.1 --directory $siteRoot
exit $LASTEXITCODE
