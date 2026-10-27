param([switch]$RestoreArcadeV1,[switch]$RestoreArcadeV2)
$ErrorActionPreference='Stop'
$arcadeRoot=Split-Path -Parent $PSScriptRoot
if ($RestoreArcadeV1) {
  $uf2=Join-Path $arcadeRoot 'firmware\arcade-v1.uf2'
  $expected='8817f5ec014c4e8d5ae002c6d5eceefe054c44f866f685ae85ea1cf8b96c9cb5'
} elseif ($RestoreArcadeV2) {
  $uf2=Join-Path $arcadeRoot 'firmware\arcade-v2.uf2'
  $expected='97de177555a5b7f03bf3ee07623082f71c00ca97f8ad725b2b2d62d67de4bbb6'
} else {
  $pointer=Join-Path $arcadeRoot 'dist\latest.json'
  $folder=Join-Path $arcadeRoot 'firmware'
  if (Test-Path -LiteralPath $pointer) {
    $latest=Get-Content -LiteralPath $pointer -Raw | ConvertFrom-Json
    $candidate=Join-Path $latest.folder 'audit.json'
    if (Test-Path -LiteralPath $candidate) {
      $candidateReport=Get-Content -LiteralPath $candidate -Raw | ConvertFrom-Json
      if ($candidateReport.app -eq 'bitdoglab_arcade_v3') {$folder=$latest.folder}
    }
  }
  $report=Get-Content -LiteralPath (Join-Path $folder 'audit.json') -Raw | ConvertFrom-Json
  if ($report.app -ne 'bitdoglab_arcade_v3' -or $report.games -ne 9) {throw 'Primeiro compile ou baixe o firmware V3 auditado.'}
  $uf2=Join-Path $folder 'bitdoglab_arcade.uf2';$expected=$report.uf2_sha256
}
if ((Get-FileHash -LiteralPath $uf2 -Algorithm SHA256).Hash.ToLower() -ne $expected) {throw 'O UF2 mudou. Nada foi gravado.'}
$drives=@(Get-PSDrive -PSProvider FileSystem | Where-Object {Test-Path -LiteralPath (Join-Path $_.Root 'INFO_UF2.TXT') -ErrorAction SilentlyContinue} | Where-Object {(Get-Content -LiteralPath (Join-Path $_.Root 'INFO_UF2.TXT') -Raw) -match '(?m)^Board-ID:\s*RPI-RP2\s*$'})
if ($drives.Count -ne 1) {throw 'Conecte uma unica BitDogLab RP2040 em BOOTSEL (unidade RPI-RP2). Nada foi gravado.'}
Write-Host 'Substituindo o programa atual da placa pelo UF2 selecionado e verificado.'
Copy-Item -LiteralPath $uf2 -Destination (Join-Path $drives[0].Root ([IO.Path]::GetFileName($uf2)))
Write-Host 'UF2_COPIADO. A unidade deve desaparecer e a placa reiniciar.'
if (!$RestoreArcadeV1 -and !$RestoreArcadeV2) {Write-Host 'Solte o joystick ao ligar. O OLED deve mostrar BITDOG ARCADE e depois o menu de 9 jogos.'}
Write-Host 'A copia USB nao comprova que OLED, matriz e controles funcionaram. Confira na placa.'
