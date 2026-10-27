param([switch]$Resume,[string]$BuildName=('build-'+(Get-Date -Format 'yyyyMMdd-HHmmss')),[string]$SdkPath=$env:PICO_SDK_PATH,[string]$ToolchainPath=$env:PICO_TOOLCHAIN_PATH)
$ErrorActionPreference='Stop'
$PSNativeCommandUseErrorActionPreference=$false
$arcadeRoot=Split-Path -Parent $PSScriptRoot
$workspaceRoot=Split-Path -Parent $arcadeRoot
$localConfig=$null
$localPath=Join-Path $arcadeRoot 'local-settings.json'
if (Test-Path -LiteralPath $localPath) {$localConfig=Get-Content -LiteralPath $localPath -Raw | ConvertFrom-Json}
function Resolve-Tool([string]$Name,[string]$Configured) {
  if ($Configured -and (Test-Path -LiteralPath $Configured)) {return $Configured}
  $command=Get-Command $Name -ErrorAction SilentlyContinue
  if ($command) {return $command.Source}
  throw "Ferramenta ausente: $Name. Defina PATH ou local-settings.json."
}
if (!$SdkPath) {$SdkPath=$localConfig.sdk}
if (!$ToolchainPath) {$ToolchainPath=$localConfig.toolchain}
if (!$ToolchainPath) {$gcc=Resolve-Tool 'arm-none-eabi-gcc' ''; $ToolchainPath=Split-Path -Parent (Split-Path -Parent $gcc)}
$compilerBin=Join-Path $ToolchainPath 'bin'
$cmake=Resolve-Tool 'cmake' $localConfig.cmake
$ninja=Resolve-Tool 'ninja' $localConfig.ninja
$pythonExe=Resolve-Tool 'python' $localConfig.python
foreach ($item in @($cmake,$ninja,$pythonExe,(Join-Path $compilerBin 'arm-none-eabi-g++.exe'),(Join-Path $SdkPath 'external\pico_sdk_import.cmake'))) {if (!(Test-Path -LiteralPath $item)) {throw "Ferramenta ausente: $item"}}
if ($BuildName -notmatch '^[a-zA-Z0-9_-]+$') {throw 'Nome de build invalido'}
$stageRoot=$localConfig.stage
if (!$stageRoot) {$stageRoot=Join-Path $arcadeRoot 'work\pico'}
if ($Resume) {
  $candidate=Get-ChildItem -LiteralPath $stageRoot -Directory -Filter 'build-*' | Where-Object {Test-Path -LiteralPath (Join-Path $_.FullName 'CMakeCache.txt')} | Sort-Object LastWriteTime -Descending | Select-Object -First 1
  if (!$candidate) {throw 'Nao ha build anterior para retomar.'}
  $BuildName=$candidate.Name
  $cache=Get-Content -LiteralPath (Join-Path $candidate.FullName 'CMakeCache.txt') -Raw
  if ($cache -notmatch 'CMAKE_PROJECT_NAME:STATIC=bitdoglab_arcade') {throw 'Projeto do build nao corresponde ao Arcade.'}
} else {
  if (Test-Path -LiteralPath (Join-Path $stageRoot $BuildName)) {throw 'Build ja existe. Nada foi substituido.'}
}
$source=Join-Path $stageRoot "source-$BuildName"
$build=Join-Path $stageRoot $BuildName
New-Item -ItemType Directory -Path $source,$build -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $arcadeRoot 'CMakeLists.txt') -Destination $source -Force
New-Item -ItemType Directory -Path (Join-Path $source 'src') -Force | Out-Null
$sourceBase=Join-Path $arcadeRoot 'src'
Get-ChildItem -LiteralPath $sourceBase -File -Recurse | ForEach-Object {
  $relative=$_.FullName.Substring($sourceBase.Length).TrimStart('\')
  $destination=Join-Path (Join-Path $source 'src') $relative
  New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
  Copy-Item -LiteralPath $_.FullName -Destination $destination -Force
}
New-Item -ItemType Directory -Path (Join-Path $source 'tools') -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'bin_to_uf2.py') -Destination (Join-Path $source 'tools') -Force
$env:PATH="$compilerBin;$env:PATH"
$taskTemp=Join-Path $stageRoot 'temp'
New-Item -ItemType Directory -Path $taskTemp -Force | Out-Null
$env:TMPDIR=$taskTemp;$env:TEMP=$taskTemp;$env:TMP=$taskTemp
Write-Host 'Compilando 8 jogos no mesmo firmware. Nenhum arquivo do TinyML sera alterado.'
$ErrorActionPreference='Continue'
& $pythonExe -X utf8 (Join-Path $PSScriptRoot 'run_logged.py') (Join-Path $build 'configure.log') $cmake -S $source -B $build -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" '-DPICO_BOARD=pico_w' "-DPICO_SDK_PATH=$SdkPath" "-DPython3_EXECUTABLE=$pythonExe" '-DCMAKE_BUILD_TYPE=Release' '-DCMAKE_OBJECT_PATH_MAX=200'
$configureCode=$LASTEXITCODE
$ErrorActionPreference='Stop'
if ($configureCode -ne 0) {throw "Configuracao falhou. Consulte $build\configure.log"}
$ErrorActionPreference='Continue'
& $pythonExe -X utf8 (Join-Path $PSScriptRoot 'run_logged.py') (Join-Path $build 'build.log') $cmake --build $build --target bitdoglab_arcade --parallel 6
$buildCode=$LASTEXITCODE
$ErrorActionPreference='Stop'
if ($buildCode -ne 0) {throw "Compilacao falhou. Consulte $build\build.log"}
$dest=Join-Path $arcadeRoot "dist\$BuildName"
if (Test-Path -LiteralPath $dest) {$dest=Join-Path $arcadeRoot ("dist\"+$BuildName+"-revision-"+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
New-Item -ItemType Directory -Path $dest -Force | Out-Null
foreach ($name in @('bitdoglab_arcade.uf2','bitdoglab_arcade.bin','bitdoglab_arcade.elf','bitdoglab_arcade.elf.map','configure.log','build.log')) { $file=Join-Path $build $name;if (Test-Path -LiteralPath $file) {Copy-Item -LiteralPath $file -Destination $dest} }
& $pythonExe -X utf8 (Join-Path $PSScriptRoot 'audit.py') $dest
if ($LASTEXITCODE -ne 0) {throw 'Auditoria falhou. Nao grave este build.'}
$sourceManifest=@{}
Get-ChildItem -LiteralPath (Join-Path $source 'src') -File -Recurse | ForEach-Object { $sourceManifest[$_.FullName.Substring($source.Length+1).Replace('\','/')]=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLower() }
$sourceManifest | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $dest 'source-hashes.json') -Encoding utf8
@{folder=$dest;built_utc=[DateTime]::UtcNow.ToString('o')} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $arcadeRoot 'dist\latest.json') -Encoding utf8
Write-Host "UF2_ARCADE_PRONTO: $dest"
Write-Host 'Ainda nao foi gravado. O autoteste de logica so executa ao iniciar a placa.'
