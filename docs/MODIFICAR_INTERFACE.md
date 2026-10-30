# Como modificar a interface

**Português** · [English](MODIFY_INTERFACE.md) · [Русский](MODIFY_INTERFACE.ru.md)

Guia prático para mudar a interface do C2 nesta source: como as janelas são montadas, como acrescentar uma camada
de recursos (este repositório **não tem uma**) e receitas passo a passo (barra de atalhos nova, botão, janela, cor
de chat, linha de tooltip).

**Leia primeiro:** aqui só existem os dois alvos fiéis, `NWindowDecomp` (modo hook) e `NWindowStandalone`. Os dois
têm de continuar iguais ao original, porque é isso que os juízes comparam. O projeto do C1 resolveu isso com uma
"fase 6": um terceiro alvo (`NWindowL2`, `NW_FEATURES=1`) e o código novo em `src/features/`; no C2 essa camada
ainda não foi feita. As receitas abaixo dizem como fazê-la sobre as classes que já existem; nenhuma delas foi
implementada nem testada **no C2**. Os blocos `#if NW_FEATURES` que aparecem em alguns arquivos (`NCShortCutWnd.cpp`,
`NConsoleWndEvents.cpp`, `NCChatWnd.cpp`...) são pontos de chamada herdados do C1: os headers que eles incluem não
estão aqui e os endereços e layouts daqueles recursos são os do C1.

## 1. Como as janelas são montadas

### A hierarquia

| Classe | Arquivo | Papel |
|---|---|---|
| `NCObject : UObject` | `include/nwindow/NCObject.h` | base de tudo; `FindTexture(TEXT("Pacote.Grupo.Nome"))` carrega uma textura; `SysString(i)`, `PlayInterfaceSound(id)` |
| `NCWnd` (0x98 bytes) | `include/nwindow/NCWnd.h` | toda janela: posição (`LocalX/Y/W/H` relativos ao pai, `ScreenX/Y`, `Width`, `Height` absolutos), estilo, filhos, foco, barra de título (`FrameCtrl`), ícone minimizado, `ToolTipSkin` (novo no C2) |
| `NControl : NCWnd` (0x1A4) | `include/nwindow/NControl.h` | todo controle; tem `ID` e avisa o pai com `NWM_COMMAND` (wParam = `ID`) |
| `NCButton`, `NCExButton`, `NCPushButton`, `NCRadioButton` | `include/nwindow/NCButton.h` | botões com textura normal/apertada (`SetNormalTex`, `SetPushedTex`, `SetText`, `SetTextID`) |
| `NCEditBox`, `NCListBox`, `NCScrollWnd`, `NCComboBox`, `NCTabControl`, `NCSliderBar`, `NCProgressCtrl`, `NCListCtrl`, `NCMultiEdit`, `NCHtmlViewer`... | `include/nwindow/` | os outros controles |
| `NC*Wnd` | `include/nwindow/` | as janelas do jogo (inventário, chat, barra de atalhos, status, clan, lojas...) |
| `NCVirtualWndMain` | `include/nwindow/NCVirtualWndMain.h` | dono de todas as janelas: registro, ordem Z, teste de clique, foco, despacho de mensagens, cursores |
| `NConsoleWnd : UL2ConsoleWnd` | `include/nwindow/NConsoleWnd.h` | o console da engine (0x35B8 bytes, 335 slots: eventos da rede e da UI); `GConsole` |
| `NCConsole` | `include/nwindow/NCConsole.h` | a **área de trabalho** do jogo (`GConsole->GetDesktop()`): pai de toda janela do jogo, com um ponteiro para cada uma (`ShortCutWnd`, `ChatWnd`, `InventoryWnd`, `SystemMenuWnd`...) |

### Criar uma janela ou um controle

O padrão do código original (`NCSystemMenuWnd::OnCreate` em `src/nwindow/NCSystemMenuWnd.cpp`):

```cpp
NCButton* Button = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
Button->ID = NID_SYSMENU_HELP;          // o que chega no NWM_COMMAND do pai
Button->SetWindowRect(86, 61, 67, 19);  // relativo ao pai
CreateChild(Button, NWS_VISIBLE);       // registra, torna a posição absoluta, manda WM_CREATE -> OnCreate
Button->SetTextID(0x91);                // texto da tabela SysString do cliente
```

- O `new` de uma classe do pacote é o `operator new` do UE2 (o objeto é um `UObject`, alocado com o tamanho da
  `UClass` registrada).
- `CreateChild(filho, estilo)` (`NCWnd`, inline) = `AddChildWindow` + `Entry` + posição de tela + `WM_CREATE`. A área
  de trabalho usa também as variantes ocultas `CreateHiddenChild` / `CreateChildHidden` (`src/nwindow/NCConsole.cpp`:
  `AddChildWindow`, `HideWindow`, depois o anexo).
- Estilos (`NWS_*` em `NCWnd.h`): `NWS_VISIBLE`, `NWS_TOPMOST` (lista separada, por cima), `NWS_TITLEBAR`
  (`NCWnd::OnCreate` faz uma barra de título `NCFrameCtrl`), `NWS_MINIMIZABLE`, `NWS_NOCLOSE`, `NWS_MODAL` (novo no C2).
- Destruir: no `OnDestroy` do dono, `RemoveChildWindow(Filho->Entry); delete Filho;` (ver
  `NCSystemMenuWnd::OnDestroy`). `NCWnd::OnDestroy` só destrói a barra de título, o ícone e o `ToolTipSkin`.

### Mensagens, pintura, tooltip

- Tudo chega por `WindowProc(Msg, wParam, lParam)` (vslot 25) com números de mensagem do Win32; o `NCWnd` despacha
  para as virtuais: `OnCreate` (36), `OnDestroy` (37), `OnPaint(NCGDevice*)` (55), `OnDrawToolTip` (56),
  `OnKeyDown`, `OnMouseMove`, `OnLButtonDown/Up`, `OnRButtonDown/Up`, `OnMove` (`WM_MOVE`), `OnMouseWheel`.
- Um controle avisa o pai com `NWM_COMMAND` (`0x501`), `wParam` = `ID` do controle. O pai trata no seu `WindowProc`
  (exemplo: `NCSystemMenuWnd::WindowProc`).
- A pintura (`OnPaint`) usa o `NCGDevice` (`include/nwindow/NCGDevice.h`, as assinaturas do C2 no comentário do
  topo): `SetClip`, `BeginPaint(ScreenX, ScreenY)`, `DrawTexture`, `DrawRect`, `DrawText`, `EndPaint`,
  `ReleaseClip`. As texturas são carregadas uma vez
  no `OnCreate` com `FindTexture` (pacotes `L2UI.*` do cliente).
- Tooltip: `OnDrawToolTip(NCGDevice*)` da janela sob o mouse, chamado depois da pintura.

### Console e área de trabalho

O `NConsoleWnd` (o objeto que a engine cria a partir do `L2.ini`) recebe os eventos do servidor e do jogo (seus
335 slots, divididos em `src/nwindow/NConsoleWnd*.cpp` por assunto) e é dono da área de trabalho `NCConsole`.
`NCConsole::OnCreate` cria toda janela do jogo (posições a partir do tamanho da tela) e `NCConsole::OnDestroy` as
destrói. `NConsoleWnd::Tick` (`NConsoleWndInput.cpp`) roda a cada quadro.

## 2. Como acrescentar uma camada de recursos (não existe ainda)

O jeito que o C1 usou e que mantém os juízes válidos:

1. **Um alvo novo no `CMakeLists.txt`**, sem mexer nos dois fiéis. A função `nwd_standalone_dll_of(objs alvo nome
   pasta)` já existe para isso: crie uma biblioteca `OBJECT` com as mesmas fontes do `nwd_standalone_objs` mais
   `src/features/*.cpp`, as mesmas opções (`${NWD_CXX_FLAGS} /Ob1`, `NW_HOOK_MODE=0`) e `NW_FEATURES=1`, e chame
   `nwd_standalone_dll_of(nwd_l2_objs NWindowL2 NWindow "${CMAKE_BINARY_DIR}/l2")`.
2. **Os pontos de chamada herdados.** Com `NW_FEATURES=1` os blocos `#if NW_FEATURES` que já estão em `src/nwindow`
   passam a compilar e incluem `features/*.h` que não existem aqui. Apague os que você não vai portar; os que
   quiser, porte do [projeto do C1](https://github.com/luannbr/Lineage2_Interface_source_C1_-Nwindow.dll-)
   (`src/features/`) corrigindo endereços e layouts para o C2 (nunca valem os do C1).
3. **O código do recurso** em `include/features/<Recurso>.h` + `src/features/<Recurso>.cpp`, em `namespace nwf` e
   `#if NW_FEATURES`. No código decompilado entra só o ponto de chamada, num bloco `#if NW_FEATURES` com o comentário
   `// phase 6: <Recurso> ...`; sem o bloco, o código tem de ler igual ao original.
4. **Estado** em globais do `.cpp` do recurso, nunca em campos novos de uma classe original (o tamanho dela está
   registrado na engine). **Configurações** no registro (`HKCU\Software\...`), nunca em arquivo ao lado da DLL.
5. **Pontos de entrada úteis**: `NCConsole::OnCreate`/`OnDestroy` (criar/destruir janelas), `NConsoleWnd::Tick`
   (a cada quadro), o `WindowProc`/`OnPaint`/`OnDrawToolTip`/`OnLButtonUp` de uma janela, `NConsoleWnd::Say2` (chat).
   O `DllMain` do standalone está em `src/standalone/NWindowPackage.cpp`.
6. **Classe nova** (janela com campos próprios): no C1 é `NW_DECLARE_L2_CLASS` / `NW_IMPLEMENT_L2_CLASS`
   (`include/features/NwL2Class.h` do projeto do C1, a portar) mais uma linha em `tests/standalone/l2_extras.csv`;
   `python tools/verify_standalone.py --dll build/l2/Release/NWindow.dll --l2` aceita exatamente essas classes a mais.
7. **Prova**: o ciclo do modo hook e do standalone continua verde (`tools\port\cycle.bat`,
   `python tools\cycle.py --game-source --standalone --no-install`), o alvo novo passa no `verify_standalone.py --l2`,
   e o teste no jogo é feito numa cópia da pasta `system`.

## 3. Receita: uma barra de atalhos nova (como no C5 / Interlude)

Não implementada no C2. A ideia (a do C5, já feita como experimento no C1) cabe nas classes do C2 sem mudar
protocolo, servidor nem texturas:

**A ideia:** a barra extra **não tem slots próprios**. É mais uma instância de `NCShortCutWnd` mostrando outra
página da mesma tabela de atalhos (`Cols × Pages`, 10 × 10 na barra principal). O slot de um atalho é
`página × 10 + posição`, um número que o servidor guarda e devolve; uma barra que só mostra outra página não muda
nada no protocolo. A classe mantém o tamanho (0x208) e nenhuma `UClass` nova é preciso.

**Passos:**

1. **Arquivos do recurso:** `include/features/ShortCutBars.h` / `src/features/ShortCutBars.cpp` (`namespace nwf`,
   `#if NW_FEATURES`): `ShortCutBars_Create(NCWnd* Desktop, NCShortCutWnd* Main)`, `ShortCutBars_Destroy`,
   `ShortCutBars_OnTick`, `ShortCutBars_OnCommand`... As barras e o botão ficam em globais do `.cpp`.
2. **Criar** em `NCConsole::OnCreate`, logo depois da barra principal (`ShortCutWnd = new NCShortCutWnd(10, 10, 32);
   SetWindowRect(Width - 45, Height / 2 - 218, 44, 437); CreateChildHidden(this, ShortCutWnd, 0x1022);`): para cada
   barra extra, `new NCShortCutWnd(Main->Cols, Main->Pages, Main->IconSize)`, `SetWindowRect` e o mesmo registro
   oculto. O `OnCreate` original da barra aloca tabelas próprias (`Items = new NShortCutItem[Cols * Pages]`,
   `Keys = new NShortCutKey[...]`): libere-as (`delete[]`) e aponte para as da principal (`Bar->Items = Main->Items`,
   `Bar->Keys = Main->Keys`). `Bar->Page` = a primeira página mostrada. Esconda o `RotateButton` das extras.
3. **Destruir** em `NCConsole::OnDestroy`, **antes** de `DestroyChild(this, ShortCutWnd)`: nas extras ponha
   `Items` e `Keys` em `NULL` (são da principal; senão o `OnDestroy` original as libera duas vezes), depois
   `RemoveChildWindow(Bar->Entry)` e `delete Bar`.
4. **Seguir a principal** a cada quadro (do tick do recurso): mesma orientação (`bHorizontal`, e `UpdateLayout()`
   — vslot 68 — quando ela muda; o C2 tem também o layout de game pad, `NID_SHORTCUT_PAD`), posição ao lado da
   principal com `SetWindowRect`, visível só enquanto ela estiver.
5. **Botão de expandir** (opcional): um `NCPushButton` filho da principal, com um `ID` depois dos originais (os da
   barra vão até `0xFF000009`), tratado no ramo `NWM_COMMAND` de `NCShortCutWnd::WindowProc`.
6. **Arrastar entre barras:** em `NCShortCutWnd::OnLButtonUp`, o ramo original que **apaga** um atalho solto fora da
   barra (`GConsole->RequestShortCutDel`) confere antes se o ponto está sobre outra barra e, se estiver, move o
   atalho para o slot dali (registro + remoção pelo protocolo original).
7. **Arrastar a alça de uma extra move o grupo:** um `WM_MOVE` de uma barra extra é repassado à principal
   (`NCWnd::Snap` puxa o grupo; acerte o ponto de pega da `FrameCtrl` da principal).

O que só o jogo mostra: posição, arte (o fundo da barra tem o ícone do botão de girar pintado na textura) e o
arraste. O filtro de pacotes do servidor pode descartar registros de atalho enviados rápido demais.

## 4. Receita: um botão novo numa janela existente

Exemplo: mais um botão no menu do sistema (`NCSystemMenuWnd`, que no C2 tem seis botões numa coluna).

1. `include/features/MyButton.h` / `src/features/MyButton.cpp`: `MyButton_Create(NCWnd* Owner)`,
   `MyButton_OnCommand(NCWnd* Owner, WPARAM ID)`, `MyButton_Destroy(NCWnd* Owner)`; o ponteiro do botão numa global.
2. Criar no fim de `NCSystemMenuWnd::OnCreate` (`#if NW_FEATURES` / `// phase 6: MyButton` / `nwf::MyButton_Create(this);`).
   Dentro: `new NCButton(normal, apertada, 0)`, um `ID` que nenhum controle da janela usa (ver o enum `NID_SYSMENU_*`,
   `0xF0F006..0xF0F023`), `SetWindowRect` dentro da área da janela (os botões estão em x 86, de 23 em 23 pixels a
   partir de y 38), `Owner->CreateChild(Button, NWS_VISIBLE)`, `SetText`.
3. Tratar o clique no ramo `NWM_COMMAND` de `NCSystemMenuWnd::WindowProc`:
   `if (nwf::MyButton_OnCommand(this, wParam)) return 1;` (em `#if NW_FEATURES`).
4. Destruir em `NCSystemMenuWnd::OnDestroy`, antes de `NCWnd::OnDestroy()`: `RemoveChildWindow(Button->Entry);
   delete Button;`.
5. Se a janela tiver de crescer: o `SetWindowRect(Width - 290, 250, 240, 192)` em `NCConsole::OnCreate` e a pintura
   do fundo no `OnPaint` — sempre em `#if NW_FEATURES`, com o ramo original intacto.

## 5. Receita: uma janela nova

1. Uma janela com campos ou virtuais próprios é uma classe nova, registrada só no alvo de recursos (seção 2,
   item 6): `class NCMyWnd : public NCWnd { ... };`.
2. Sobrescreva as virtuais que precisar: `OnCreate` (chame `NCWnd::OnCreate()` primeiro, carregue texturas com
   `FindTexture`, crie a barra de título e os controles), `OnDestroy` (destrua os filhos, depois `NCWnd::OnDestroy()`),
   `WindowProc` (`NWM_COMMAND`), `OnPaint`.
3. Crie-a na área de trabalho quando precisar (`SetWindowRect(0, 0, W, H)`, `GConsole->GetDesktop()` como pai em
   `CreateChild(janela, NWS_VISIBLE)`, `HideWindow()`) e mostre com `ShowWindow()` + `Desktop->BringChildToTop(Entry)`.
4. Modelos no código: `NCSystemMenuWnd` (barra de título decorada `NCFrameCtrl(1)`, botões), as janelas de receita
   do C2 (`NCRecipeBookWnd`, `NCRecipeShopWnd`, listas com rolagem) e, no projeto do C1, `src/features/craft/`.

## 6. Receita: uma cor de chat nova

O servidor não manda cor: manda o **tipo** do chat (`Say2`), e o cliente escolhe a cor em `NConsoleWnd::Say2`
(`src/nwindow/NConsoleWndEvents.cpp`). O C2 colore os tipos 1 (grito, `0xFFFF7200`), 2 (tell, `0xFFFF00FF`), 3
(party, `0xFF00FF00`), 4 (clan, `0xFF7D77FF`), 6/7 (petição, `0xFF80FFFF`), 8 (comércio, `0xFFEAA5F5`), 9 (aliança,
`0xFF77FF99`) e 10 (anúncio); o resto cai no último `else`, branco.

1. No último `else` de `Say2`, em `#if NW_FEATURES`, pergunte ao recurso (`nwf::ChatColors_Find(Type, &Color)` no C1:
   o ponto de chamada já está lá) e chame `GetDesktop()->Say(ObjectID, Name, Text, Color, 0)`.
2. A linha continua indo para o chat como uma normal (aba All, filtro Normal). Não reaproveite tipos com efeito
   colateral (o TELL põe quem falou na lista de tell).
3. Mudar a cor de um tipo que o C2 **já** colore é mudar um ramo fiel: faça em `#if NW_FEATURES` dentro de `Say2`,
   com a cor original no ramo `#else`.

## 7. Receita: uma linha nova no tooltip de item

O tooltip completo do item é desenhado pelas rotinas de `include/nwindow/NItemToolTip.h`, que as grades de item
incluem: `SetTexts`/`SetStoreTexts` preenchem `ToolTipName` e `ToolTipText` (o texto que **dimensiona** a caixa) e
`DrawLines` desenha as linhas, de 13 em 13 pixels (`DrawTypeLines`, depois `DrawTailLines`, que termina com a
descrição).

1. Acrescente a linha nos dois lugares, em `#if NW_FEATURES`: no texto que dimensiona (para a caixa crescer) e no
   desenho (por exemplo em `DrawTailLines`, antes da descrição, com `DrawLabel`/`DrawValue` e `Y += 13`).
2. As rotinas são templates instanciados por cada grade com parâmetros diferentes (inventário, paperdoll, troca,
   armazém, pet, lojas privadas): confira cada cópia que mudar.
3. Para o tooltip de outra janela: o seu `OnDrawToolTip`; um ponto dá a altura a mais, outro desenha a linha.

## 8. O ciclo de compilação e teste

```bat
set NWD_CLIENT_SYSTEM=<client>\system_nwtest
tools\port\cycle.bat
python tools\cycle.py --game-source --standalone --no-install
```

1. Compile sem erro e sem warning novo.
2. Rode os dois ciclos: eles provam que o modo hook e o standalone não mudaram (os juízes comparam com o original).
3. Teste no jogo: feche o jogo, guarde o `NWindow.dll` de uma **cópia** da pasta `system` e copie por cima dele o DLL
   que você montou. O ciclo não desenha a interface: posição, arte e arraste só aparecem no jogo.
4. Para diagnosticar um crash do standalone: `NWindow_crash.log` ao lado do `l2.exe` e
   `python tools\port\map_addr.py --log NWindow_crash.log`.

## 9. Regras de segurança

- Nunca acrescente campos nem slots virtuais novos a uma classe original: o tamanho dela está registrado na engine
  e o código original indexa nela e na vtable. Sobrescrever um slot que a classe já herda é permitido. Estado de
  recurso vai em globais do `.cpp` ou numa classe nova do alvo de recursos.
- No código decompilado, só o ponto de chamada, em `#if NW_FEATURES` com `// phase 6: ...`; o ramo original fica
  como está.
- Um remendo em tempo de execução de outro módulo confere os bytes antes e desliga o recurso com uma linha de log
  quando eles não batem.
- Mesmo protocolo do cliente original: o servidor não muda; respeite os limites do filtro de pacotes dele.
- Teste numa cópia do cliente, nunca na pasta em que você joga; nenhum binário do jogo entra no repositório.
- Commits pequenos, um assunto cada, com o ciclo verde.
