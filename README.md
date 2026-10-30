# NWindow.dll do Lineage II C2 — source reconstruída em C++

**Português** · [English](README.en.md) · [Русский](README.ru.md)

Reconstrução, função por função, do `NWindow.dll` do cliente Lineage II **Chronicle 2 (Age of Splendor)**: a DLL
nativa da interface (janelas, controles, HTML das NPCs, IME, chat, console, MSN). O alvo é o build original de
03/03/2005 (TimeDateStamp `0x4227239B`, Visual C++ .NET 2002 `13.00.9466`, md5 `0111f657…`, 225 classes nativas,
227 exports, 7.117 funções). É o fork do [projeto do C1](https://github.com/luannbr/Lineage2_Interface_source_C1_-Nwindow.dll-):
as mesmas ferramentas e o mesmo método, com a source portada para a imagem do C2 (`tools/port/`). A mesma source
compila de dois jeitos:

| Alvo CMake | Arquivo gerado | O que é |
|---|---|---|
| `NWindowDecomp` | `build/bin/Release/NWindowDecomp.dll` | **modo hook**: roda ao lado do `NWindow.dll` original e troca cada função já reimplementada por um `jmp` para a nossa |
| `NWindowStandalone` | `build/standalone/Release/NWindow.dll` | **standalone**: o `NWindow.dll` inteiro, só com o nosso código; é o que os juízes comparam com o original |

Não há camada de recursos novos (a "fase 6" do C1, `NWindowL2`) neste repositório: veja
[docs/MODIFICAR_INTERFACE.md](docs/MODIFICAR_INTERFACE.md) para saber como acrescentar uma.

## Como funciona

**Modo hook.** O instalador (`tools/install_test.py`) acrescenta ao `NWindow.dll` original um import da
`NWindowDecomp.dll`. Ela carrega antes de qualquer código do original rodar e põe um `jmp` na entrada de cada
função que já reimplementamos (um `NW_HOOK_*` ao lado de cada definição, `src/re/NwHook.h`). O resto continua
sendo o código original. Qualquer função pode ser desligada no `NWindowDecomp.ini` (`[Hooks] Disable=`) para
isolar um bug.

**Standalone (fase 5).** Com `NW_HOOK_MODE=0` os macros de hook ficam vazios e a source gera o `NWindow.dll`
sozinha: as 225 `UClass` (`IMPLEMENT_CLASS`, `src/standalone/Classes.gen.cpp`), as globais com os valores do
original (`NwGlobals.cpp`), os 227 exports com os ordinais do original (`NWindow.def`) e os 7 cursores coloridos.
A engine (`Core.dll`/`Engine.dll`) carrega o nosso DLL como carregava o original.

Duas diferenças assumidas em relação ao original (detalhes em [docs/notas/C2_Fase5.md](docs/notas/C2_Fase5.md)):
- a biblioteca de terceiros nProtect GameGuard, ligada estaticamente no original (85 funções), **não faz parte
  desta source**: no standalone os três pontos de entrada que o jogo chama são funções vazias documentadas
  (`include/nwindow/NGameGuard.h`), como no cliente remendado que serviu de referência;
- o NKeyCrypt (`npkcrypt.dll`, importado por ordinal no original) é carregado em tempo de execução
  (`LoadLibrary` + ordinal, `src/nwindow/NKeyCrypt.cpp`): o standalone funciona com ou sem essa DLL.

O standalone escreve dois arquivos de diagnóstico ao lado do `l2.exe`: `NWindow_crash.log` (só quando o
processo falha: registradores e pilha como módulo+RVA, `python tools/port/map_addr.py <rva>` dá os nomes) e
`NWindow_trace.log` (eventos do clan, `NW_TRACE`).

## Estado

- Modo hook: **3.451 de 4.652** funções a reimplementar estão feitas (**92,08 %** dos bytes de código); o
  verificador aceita todas (diferença só com explicação em `db/verify_notes.csv`), `regcheck` limpo, nenhum hook
  alcança um corpo `int3`. Sem hook, com motivo: 416 de convenção (*scalar deleting destructors*,
  `InternalConstructor`) e 686 que o compilador gera no standalone (inicialização estática, `DllMain`). Sem hook e
  sem motivo: 99, das quais 85 são a biblioteca nProtect e as suas três chamadoras (fora do standalone) e 14 são
  corpos minúsculos dobrados pelo linker. Contagem viva: [docs/PROGRESSO.md](docs/PROGRESSO.md) (gerado por
  `tools/build_db.py`).
- Standalone: linka, passa em `verify_standalone.py` (PE, imports, recursos, 227 exports, 225 classes, 227
  vtables), no smoke (a saída é a do original, linha a linha), em `gen_globals.py --check` (82 globais iguais) e
  foi usado no jogo contra um servidor de C2 (clan e lojas privadas conferidos numa sessão real:
  [docs/notas/C2_Pacotes.md](docs/notas/C2_Pacotes.md)).
- Ainda não feito: o `difftest` (os casos chamam o original por endereços do C1 e `tools/difftest.py` recusa a
  imagem do C2) e o `db/verified.csv` (nenhuma função marcada como testada no jogo uma a uma).
- As notas técnicas (`docs/`) estão em português; os comentários de código, em inglês.

## Requisitos

- Windows, **Visual Studio 2022** com C++ para x86 (MSVC v143), **CMake ≥ 3.20**;
- **Python 3** com `pefile` e `capstone` (`pip install pefile capstone`): usado pelo build standalone (extração dos
  cursores) e por todas as ferramentas; `unicorn` para os testes diferenciais de `tools/port/emu/`;
- **o seu próprio cliente Lineage II C2**. Nenhum arquivo do jogo vem no repositório: copie para `orig/` os
  arquivos listados em [orig/README.md](orig/README.md) (o `NWindow.dll` tem de ter o md5 de lá);
- para os juízes (`verify_hooks.py`, `regcheck.py`, `verify_standalone.py`, `build_db.py`, `tools/port/*`): o
  export do Ghidra em `analysis/ghidra/`, recriado por `bash tools/ghidra/run_full.sh` (Ghidra 11.3.2 e JDK 21;
  caminhos em `NWD_GHIDRA_HOME` e `GHIDRA_JAVA_HOME`).

## Compilar

Num *Developer Command Prompt* x86 do VS 2022 (ou depois de `vcvars32.bat`), na raiz do repositório:

```bat
cmake -S . -B build -A Win32 -DNWD_GAME_SOURCES=ON
cmake --build build --config Release
cmake --build build --config Release --target NWindowStandalone
```

`-DNWD_GAME_SOURCES=ON` compila a source do jogo (`src/nwindow`). Sem ela (o padrão do `CMakeLists.txt`) o
`NWindowDecomp` tem só o runtime do modo hook e nenhuma função trocada: serve para provar a cadeia de instalação.
O primeiro `--build` gera o DLL do modo hook e as ferramentas de teste (`nwd_smoke`, `nwd_difftest`) e não precisa
de `orig/`; o `NWindowStandalone` precisa de `orig/NWindow.dll`. A mensagem `'pwsh.exe' não é reconhecido` no fim
de cada projeto vem do ambiente do Visual Studio e não faz o build falhar.

## Instalar e testar

Use uma **cópia** da pasta `system` do cliente, nunca a que você usa para jogar
(`xcopy /E /I /Y "<client>\system" "<client>\system_nwtest"`).

**Modo hook:**

```bat
python tools\install_test.py --system "<client>\system_nwtest"
python tools\install_test.py --system "<client>\system_nwtest" --restore
```

O primeiro grava um `NWindow.dll` refeito a partir de `orig/NWindow.dll` com o import a mais, a `NWindowDecomp.dll`
e um `NWindowDecomp.ini` (o `NWindow.dll` que estava lá fica como `NWindow.dll.pre-decomp-bak`); o segundo devolve o
que estava lá. O `NWindowDecomp.log` dessa pasta lista cada hook instalado. No `.ini`: `[Hooks] Enabled=0` deixa
todo o código original; `Disable=` aceita um endereço (`1000cc40`), um método (`NCGDevice::DrawText`) ou uma classe
(`NCGDevice::*`); `[Debug] Marker=1` desenha marcas que só o nosso código desenha (linha amarela no topo, `#` na
senha), para conferir que os hooks estão ativos.

**Standalone:** feche o jogo, guarde o `NWindow.dll` da pasta de teste e copie por cima dele o
`build\standalone\Release\NWindow.dll`. Só esse arquivo. Para voltar, ponha o original de volta.

**Ciclo completo** (para no primeiro passo que falhar):

```bat
set NWD_CLIENT_SYSTEM=<client>\system_nwtest
python tools\cycle.py --game-source --no-install
python tools\cycle.py --game-source --standalone --no-install
tools\port\cycle.bat
```

`cycle.py` configura e compila `build/`, instala numa cópia mínima em `build/smoke_system` (copia da pasta de teste
`Core.dll`, `Engine.dll`, as suas dependências e `npkcrypt.dll`/`npkpdb.dll`), carrega fora do jogo, verifica,
roda o `regcheck` e regenera o banco; `--standalone` acrescenta os passos do standalone; sem `--no-install` instala
também na pasta de teste (o jogo dessa pasta tem de estar fechado). `tools\port\cycle.bat` é o ciclo que cada
mudança tem de passar: `cycle.py --game-source --no-install` e depois `verify_data.py` e `check_breakers.py` (ele
mesmo chama o `vcvars32.bat` do VS 2022 Community).

**Caminhos da máquina.** Nenhuma ferramenta tem um caminho da máquina do autor: uma pasta fora do repositório vem
da opção de linha de comando ou de uma destas variáveis (a opção vence; a ferramenta que precisa de uma e não tem
nenhuma para e diz qual):

| Variável | Aponta para | Usada por |
|---|---|---|
| `NWD_CLIENT_SYSTEM` | a pasta `system` do cliente de teste (uma cópia: a instalação escreve nela) | `--system` padrão de `install_test.py`, `cycle.py`, `smoke_standalone.py`, `difftest.py`; `verify_hooks.py --log`; `ue2_layouts.py` (`Engine.u`, `Core.u`) |
| `NWD_GHIDRA_HOME` | a instalação do Ghidra (com `support/analyzeHeadless.bat`; `GHIDRA` ainda tem precedência) | `tools/ghidra/*.sh`, `tools/c2/ghidra_census.sh` |
| `NWD_C1_ROOT` | uma cópia do [projeto do C1](https://github.com/luannbr/Lineage2_Interface_source_C1_-Nwindow.dll-) com o seu `orig/` e o seu `analysis/ghidra/` | `tools/port/*` (o porte C1 → C2), `tools/c2/compare_nwindow.py`, `tools/ue2_console_slots.py`, `ue2_gamedata_regions.py`, `ue2_ret_diff.py`, `ue2_vtable_diff.py` |
| `NWD_C2_SERVER` | a pasta de deploy de um servidor (páginas em `gameserver/data/html-en`) | `difftest.py`, conjunto de páginas `server` (pulado sem ela) |

## Os juízes

| Juiz | Comando | O que prova |
|---|---|---|
| smoke | `build\smoke\Release\nwd_smoke.exe build\smoke_system` | o `NWindow.dll` remendado carrega fora do jogo e todo hook dá `ok` (`NWindowDecomp.log`) |
| verify | `python tools/verify_hooks.py --log build/smoke_system/NWindowDecomp.log` | cada função nossa contra a original no assembly: bytes desempilhados no `ret`, slots de vtable e imports chamados; diferença aceita só com explicação em `db/verify_notes.csv` |
| regcheck | `python tools/regcheck.py --game-src` | nenhum chamador original depende de um registrador que a nossa função pode mudar (o original foi compilado com `/GL`) |
| verify_data | `python tools/port/verify_data.py` | os dados que cada função toca (constantes, offsets de campo, strings) iguais aos do original; diferenças olhadas em `db/verify_data_notes.csv` |
| breakers | `python tools/port/check_breakers.py` | nenhum hook alcança uma função cujo corpo é `int3` (`NW_UNPORTED`) |
| emu | `python tools/port/emu/t_<nome>.py ...` (ex.: `t_nkeycrypt.py 1500 1`) | a função original e a nossa rodam lado a lado no unicorn sobre entradas aleatórias e escrevem o mesmo |
| standalone: declarações | `python tools/verify_standalone.py --checks source` | os nossos `NW_DECLARE_CLASS` / `NW_CHECK_VTABLE` contra a tabela de classes original |
| standalone: verify | `python tools/verify_standalone.py --dll build/standalone/Release/NWindow.dll` | PE, imports, recursos, exports, classes e vtables iguais aos do original, sem rodar |
| standalone: smoke | `python tools/smoke_standalone.py --dll build/standalone/Release/NWindow.dll` | carregado como a engine carrega: classes registradas, `ConsoleWnd`/`L2Font` construídos, saída igual à do original |
| db | `python tools/build_db.py --game-src` | regenera `db/functions.csv` e `docs/PROGRESSO.md` |

## Pastas

```
orig/          binários do seu cliente (fora do git; md5 em orig/README.md)
analysis/      fatos extraídos do original (classes, EH, vtables; as do Core/Engine; layouts das classes do Engine); analysis/ghidra/ = export do Ghidra (gerado, fora do git)
db/            banco de funções (functions.csv gerado) + nomes, notas e verificações dados à mão; mapas do porte C1 → C2
include/ue2/   declarações do Core.dll/Engine.dll do C2, conferidas contra os exports
include/nwindow/, src/nwindow/    a source decompilada, uma classe por arquivo
src/re/        runtime do modo hook (instalação dos hooks, log, ini, compatibilidade do CRT)
src/standalone/                   IMPLEMENT_CLASS / IMPLEMENT_PACKAGE, globais, NWindow.def, GameGuard vazio, logs de diagnóstico
tests/         saídas de referência (golden, ainda as do C1) e listas do standalone
third_party/ue_import/            .def dos exports do Core/Engine (viram .lib no build)
tools/         análise, porte C1 → C2 (tools/port), instalação, juízes, ciclo
docs/          plano, convenções, progresso e notas técnicas (português)
```

## Guias

- [docs/PLANO.md](docs/PLANO.md) — plano e método (herdado do C1); [docs/PLANO_C2.md](docs/PLANO_C2.md) — o que é do C2
  (constantes da imagem, ferramentas); [docs/CONVENCOES.md](docs/CONVENCOES.md) — regras de código e armadilhas
- [docs/notas/C2_Porte.md](docs/notas/C2_Porte.md) — o porte C1 → C2; [docs/notas/C2_Fase5.md](docs/notas/C2_Fase5.md) — o standalone
- [docs/MODIFICAR_INTERFACE.md](docs/MODIFICAR_INTERFACE.md) — como modificar a interface (barra de atalhos, botão, janela, cor de chat, linha de tooltip)
  · [English](docs/MODIFY_INTERFACE.md) · [Русский](docs/MODIFY_INTERFACE.ru.md)
- [docs/LLM.md](docs/LLM.md) — guia para agentes de IA que forem trabalhar neste código (em inglês)

## Nota legal

Projeto independente de engenharia reversa, para compatibilidade e preservação. Não é afiliado, autorizado nem
endossado pela NCSoft. *Lineage II* é marca registrada da NCSoft Corporation. **Nenhum arquivo original do
jogo está incluído** (nem DLLs, nem executáveis, nem pacotes de dados): cada usuário fornece o próprio cliente.

## Licença

[GNU General Public License v3.0](LICENSE).
