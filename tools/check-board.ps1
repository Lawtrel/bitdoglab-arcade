$ErrorActionPreference='Stop'
$arcadeRoot=Split-Path -Parent $PSScriptRoot
$pythonExe=$null
$localPath=Join-Path $arcadeRoot 'local-settings.json'
if (Test-Path -LiteralPath $localPath) {$local=Get-Content -LiteralPath $localPath -Raw | ConvertFrom-Json;$pythonExe=$local.python}
if (!$pythonExe) {$pythonExe=(Get-Command python -ErrorAction Stop).Source}
$folder=Join-Path $arcadeRoot 'work'
New-Item -ItemType Directory -Path $folder -Force | Out-Null
& $pythonExe -X utf8 (Join-Path $PSScriptRoot 'check_board.py') --out (Join-Path $folder 'runtime-check.json')
if ($LASTEXITCODE -ne 0) {throw 'A consulta falhou; confira o OLED e feche outros monitores seriais.'}
