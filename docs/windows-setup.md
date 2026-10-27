# Exemplos de configuração local (não versionar seus caminhos pessoais)

Crie `local-settings.json` na raiz caso as ferramentas não estejam no PATH:

```json
{
  "sdk": "C:/pico/pico-sdk",
  "toolchain": "C:/pico/toolchain",
  "cmake": "C:/tools/cmake/bin/cmake.exe",
  "ninja": "C:/tools/ninja.exe",
  "python": "C:/Python312/python.exe",
  "stage": "C:/pico/arcade-builds"
}
```

São caminhos de exemplo. A configuração do computador de desenvolvimento fica fora do Git. O build público e os previews são gerados no GitHub Actions, sem depender desta configuração.
