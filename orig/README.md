# Binários de referência (fora do git) / Reference binaries (not in git)

**PT.** Esta pasta não vem com nenhum arquivo do jogo: nenhum binário da NCSoft é publicado. Copie os
arquivos abaixo do **seu próprio** cliente Lineage II C2 (pasta `system`) para cá. O build do modo hook
(`NWindowDecomp`) não precisa deles; o `NWindowStandalone` precisa de `orig/NWindow.dll` (os 7 cursores
coloridos são extraídos dele na compilação); as ferramentas de verificação precisam do `NWindow.dll`, do
`Core.dll` e do `Engine.dll`. O `NWindow.dll` tem de ter o md5 da tabela.

**EN.** This folder ships without any game file: no NCSoft binary is published. Copy the files below
from **your own** Lineage II C2 client (its `system` folder) into it. The hook-mode build
(`NWindowDecomp`) does not need them; `NWindowStandalone` needs `orig/NWindow.dll` (the 7 colour cursors
are extracted from it at build time); the verification tools need `NWindow.dll`, `Core.dll` and
`Engine.dll`. The `NWindow.dll` must have the md5 in the table.

| Arquivo / File | md5 | TimeDateStamp (PE) | Uso / Use |
|---|---|---|---|
| `NWindow.dll` | `0111f6578f6482f73980f6268221c03e` | `0x4227239B` (03/03/2005) | **o** binário que o projeto reconstrói (1.835.028 bytes). Toda ferramenta confere o md5 (`tools/nwd/image.py: STOCK_MD5`). |
| `Core.dll` | `d0c235c7be3740b4c716ec557d133afa` | `0x422722DE` (03/03/2005) | declarações em `include/ue2`, `third_party/ue_import/Core.def` |
| `Engine.dll` | `684f94af9fb5fcd8244131430a638c21` | `0x42272361` (03/03/2005) | idem (`Engine.def`); tem mais de 8.192 exports (9.050): a tabela é lida à mão (`tools/engine_vtables.py`, `tools/gen_import_defs.py`) |
| `npkcrypt.dll` | `75ded44487e3ee23c59de89aa7e7d836` | `0x456562BF` (23/11/2006) | o `NWindow.dll` original importa 8 funções dele **por ordinal**: sem ele o original não carrega fora do jogo |
| `WinDrv.dll` | `768e8e9f11adf48cd005ef6f22d09611` | `0x42270ECA` | só consulta (virtuais do `UViewport`) |
| `Window.dll` | `dbf3d747603daafd5313f8df641e392b` | `0x42270D45` | só consulta |
| `ALAudio.dll` | `7ccdafb9d0151b0edcd0450b60ed1bdf` | `0x42270FF6` | `UALAudioSubsystem`, para as puras do `UAudioSubsystem` (`tools/gen_engine_decls.py`) |
| `D3DDrv.dll` | `f6d0760994743417644bee9148304a29` | `0x42270D87` | `UD3DRenderDevice`, para as puras do `URenderDevice` |
| `IpDrv.dll` | `345b07e8ab06490995a3fc45ec65d0be` | `0x42270EC2` | só consulta |
| `l2.exe` | `411577989f28da8b8ae30d00ea1e4fa1` | `0x42272379` | só consulta |

O cliente do C2 **não tem `Network.dll`** (o do C1 tinha): o código de rede está no `Engine.dll` /
`IpDrv.dll`, e `tools/engine_vtables.py` só lê uma `Network.dll` quando ela existe em `orig/`.

O projeto do C1 (de onde este foi derivado) usa outro `NWindow.dll` (md5 `2dacb6ebcbb3bbe22cec2b296231dc08`,
`0x40DBD452`, 25/06/2004): não serve aqui.

## `NWindow.dll` e `nwindow.dll.BAK`

O cliente usado pelo autor tem também um `nwindow.dll.BAK` (md5 `d3e849a857bc14fc7bb99f03d63022d9`): mesmo
tamanho e mesmo PE, **24 bytes diferentes em 8 lugares** (offsets de arquivo `0xC78F-0xC792`,
`0xC7BB-0xC7BE`, `0xC7D3-0xC7D6`, `0x23952`, `0x25240-0x25243`, `0x62CC8`, `0x6BDD9`, `0xFFB60-0xFFB63`,
todos em `.text`). O `NWindow.dll` da tabela (o que roda no cliente) é o remendado; o `.BAK` é o anterior.
O projeto reconstrói o que roda. Se o seu `NWindow.dll` tem outro md5, as ferramentas recusam a imagem.

## Para recriar a pasta / To fill the folder

Na raiz do repositório / at the repository root (`<client>` = a pasta do seu cliente C2):

```bat
set C2=<client>\system
for %f in (NWindow Core Engine npkcrypt WinDrv Window ALAudio D3DDrv IpDrv) do copy "%C2%\%f.dll" orig\
copy "%C2%\l2.exe" orig\
```

A pasta de teste (para `tools/install_test.py`, `tools/cycle.py`, `tools/smoke_standalone.py`) é uma
**cópia** da `system` inteira; nunca use a pasta com que você joga. Aponte `NWD_CLIENT_SYSTEM` para ela:

```bat
xcopy /E /I /Y "%C2%" "<client>\system_nwtest"
set NWD_CLIENT_SYSTEM=<client>\system_nwtest
```
