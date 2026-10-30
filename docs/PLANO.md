# Plano e método

Este plano é o do projeto do C1, apontado para o `NWindow.dll` do C2 (Interlude). O que é específico do
C2 — constantes da imagem, o que foi retargetado, o que falta — está em **[PLANO_C2.md](PLANO_C2.md)**.

## O binário

| Fato | Valor |
|---|---|
| Arquivo | `NWindow.dll` stock C2, 1.835.028 bytes, md5 `0111f657…`, TimeDateStamp `0x4227239B` (03/03/2005) |
| Compilador | **Visual C++ .NET 2002 (7.0, cl 13.00.9466)**, CRT estático (libcmt 7.0), linker 7.0 (Rich header: o mesmo do C1) |
| Código | `.text` 0x105020 bytes (0x10001000–0x10106020); ImageBase `0x10000000`, sem ASLR |
| Exports | 227: `GPackage`, `_DllMain@12`, 225 `autoclass*` |
| Imports | 452: Core.dll 161, Engine.dll 98, KERNEL32 109, ADVAPI32 29, IMM32 16, USER32 15, WS2_32 12, `npkcrypt.dll` 8 (por ordinal), VERSION 3, WINMM 1 |
| RTTI | não (compilado com /GR-) |
| Funções | 7.117 no Ghidra: 3.976 do jogo, 685 inicializadores estáticos, 1.861 funclets de EH, 515 do CRT, 79 thunks, 1 fragmento |
| A decompilar | **4.661 funções, 917.473 bytes** (jogo + inicializadores) |

De onde vêm os nomes (nenhum PDB existe):
- **225 classes** com tamanho exato, classe-pai, construtor e vtable: dos inicializadores do
  `IMPLEMENT_CLASS` (`tools/extract_classes.py` → `analysis/classes.md`);
- **375 funções** com o nome original: as strings dos macros `guard()` do UE2, achadas pelas
  tabelas de exceção (`tools/eh_scan.py`);
- slots de vtable: o nome dado a um slot (ex.: 55 = `OnPaint`) vale para todas as classes filhas;
- as vtables da engine (`UCanvas`, `UObject`…) com assinatura completa, lidas do próprio
  `Engine.dll`/`Core.dll` (`tools/engine_vtables.py`);
- **o projeto do C1**: cada função do C2 que o `tools/port/match.py` casou com uma do C1 herda o nome
  (`db/fmap.csv`), com a menor prioridade de todas (o `tools/build_db.py` grava a origem de cada nome na
  coluna `name_src`).

## Método: modo hook, função por função

Mesmo caminho de projetos como re3 e OpenRCT2: o executável original continua rodando e cada função
reimplementada o substitui no lugar, até não sobrar nada do original.

1. **Escolher** funções `todo` em `db/functions.csv` (de preferência uma classe inteira, das
   folhas para a base ou das mais usadas).
2. **Ler** `analysis/ghidra/decomp/<va>.c` e **conferir no assembly** `analysis/ghidra/asm/<va>.asm`:
   o decompilador do Ghidra erra número de parâmetros (conferir o `ret N`), confunde
   parâmetros com endereço de retorno e não sabe os slots de vtable.
3. **Escrever** em C++ no estilo UE2 em `src/nwindow/<Classe>.cpp`, com o endereço em comentário e o
   `NW_HOOK_*` logo abaixo de cada função. Layout de classe com `NW_ASSERT_OFFSET/SIZE`.
4. **`python tools/cycle.py`**: compila, instala, carrega fora do jogo (todo hook tem de dar `ok`),
   e o `verify_hooks.py` compara com o original: bytes desempilhados no `ret`, slots de vtable
   chamados e imports chamados. Toda diferença tem de ser entendida; as que não são bug vão
   para `db/verify_notes.csv` com a explicação.
5. **Testar no jogo** com o System de teste; passando, registrar em `db/verified.csv`.
6. Um bug no jogo? Desligar hooks no `NWindowDecomp.ini` (`Disable=NCGDevice::*`, um endereço,
   um método) até isolar a função.

O que o verificador já pegou na primeira classe (no C1): o MSVC põe **sobrecargas virtuais em ordem
inversa** na vtable. `UCanvas::DrawStretchedTex(5 args)` está no slot 35 e a de 9 args no 34;
declaradas na ordem natural, o nosso código chamava o slot errado.

## Fases

| Fase | Conteúdo | Estado no C2 |
|---|---|---|
| 0 Fundação | pasta, análise, banco, build, hooks, instalador, smoke test, verificador | **feito (06/10/2026)**, com a source de jogo vazia |
| 1 Base da UI | `NCObject`, `NCGDevice`; `NCCmdTarget`, `NCWnd` (0x98, 67 slots), `NControl` (0x1A4, 68 slots), `NCVirtualWndMain` (despacho de mensagens), `NL2Font`, `NHtmlAnalyzer` | a source vem do `tools/port` (C1 → C2) |
| 2 Controles | `NCButton`, `NCEditBox`, `NCListBox`, `NCScrollWnd`, `NCComboBox`, `NCTabControl`, `NCSliderBar`, `NCProgressCtrl`, `NCListCtrl`, `NCMultiEdit`, `NCHtmlViewer`… e os novos (`NCTree`, `NCTreeCtrl`, `NCSliderCtrl`, `NCThumbButton`…) | |
| 3 Janelas | ~110 classes `NC*Wnd` do C1 + as 47 novas do C2 (receitas, macros, henna, manor, MSN, GM snoop…) | |
| 4 Console e sistemas | `NConsoleWnd` (0x35B8 bytes, 335 slots: os callbacks de rede que o Engine chama), IME (5 variantes, com a tailandesa), HTML | |
| 5 Standalone | `IMPLEMENT_CLASS` das 225 classes, globais e inicializadores próprios, CRT moderno, o import por ordinal da `npkcrypt.dll`; a source gera `NWindow.dll` sozinha e substitui o original | pendências em [PLANO_C2.md](PLANO_C2.md) |
| 6 Evolução | features novas (a camada do C1 — hover de item, combat log, craft — não vale no C2 como está) | fora do escopo por ora |

Na fase 5 não pode sobrar nenhum `NW_STOCK`, `nw::StockCall` ou `nw::Field` — são andaimes do
modo hook para falar com código ainda não decompilado.

## Pendências de ferramenta

- Renomear no projeto Ghidra (`ghidra/census/project`, `tools/ghidra/run_names_export.sh`) a partir do
  `db/functions.csv` e reexportar: o decompilador melhora muito com nomes e tipos.
- Compilação *matching* (byte a byte) exige o VC 7.0 (13.00.9466), que não está instalado. Por isso
  o código decompilado evita C++ moderno: dá para tentar mais tarde.
