param([string[]]$IdfArgs = @('build'))
$ErrorActionPreference = 'Stop'
if (-not $env:IDF_PATH) { $env:IDF_PATH = 'C:/Espressif/frameworks/esp-idf-v5.5.5' }
$env:PATH = 'C:/Espressif/python_env/idf5.5_py3.11_env/Scripts;' + $env:PATH
. "$env:IDF_PATH/export.ps1"
idf.py @IdfArgs
exit $LASTEXITCODE
