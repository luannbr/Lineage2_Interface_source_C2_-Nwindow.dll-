# Как изменять интерфейс

[Português](MODIFICAR_INTERFACE.md) · [English](MODIFY_INTERFACE.md) · **Русский**

Практическое руководство по изменению интерфейса C2 в этом исходном коде: как устроены окна, как добавить слой
новых возможностей (в этом репозитории его **нет**) и пошаговые рецепты (новая панель ярлыков, кнопка, окно, цвет
чата, строка подсказки).

**Сначала прочтите:** здесь есть только две «верные» цели — `NWindowDecomp` (режим хуков) и `NWindowStandalone`. Обе
должны оставаться равными оригиналу, потому что именно это сравнивают «судьи». Проект C1 решил это «фазой 6»:
третьей целью (`NWindowL2`, `NW_FEATURES=1`) и новым кодом в `src/features/`; в C2 этот слой ещё не создан. Рецепты
ниже объясняют, как построить его поверх существующих классов; ни один из них **в C2** не реализован и не проверен.
Блоки `#if NW_FEATURES`, которые встречаются в некоторых файлах (`NCShortCutWnd.cpp`, `NConsoleWndEvents.cpp`,
`NCChatWnd.cpp`...), — точки вызова, унаследованные от C1: включаемых ими заголовков здесь нет, а адреса и раскладки
тех возможностей — от C1.

## 1. Как устроены окна

### Иерархия

| Класс | Файл | Роль |
|---|---|---|
| `NCObject : UObject` | `include/nwindow/NCObject.h` | основа всего; `FindTexture(TEXT("Package.Group.Name"))` загружает текстуру; `SysString(i)`, `PlayInterfaceSound(id)` |
| `NCWnd` (0x98 байт) | `include/nwindow/NCWnd.h` | любое окно: положение (`LocalX/Y/W/H` относительно родителя, `ScreenX/Y`, `Width`, `Height` абсолютные), стиль, дочерние окна, фокус, заголовок (`FrameCtrl`), значок свёрнутого окна, `ToolTipSkin` (новое в C2) |
| `NControl : NCWnd` (0x1A4) | `include/nwindow/NControl.h` | любой элемент управления; имеет `ID` и сообщает родителю через `NWM_COMMAND` (wParam = `ID`) |
| `NCButton`, `NCExButton`, `NCPushButton`, `NCRadioButton` | `include/nwindow/NCButton.h` | кнопки с обычной и нажатой текстурой (`SetNormalTex`, `SetPushedTex`, `SetText`, `SetTextID`) |
| `NCEditBox`, `NCListBox`, `NCScrollWnd`, `NCComboBox`, `NCTabControl`, `NCSliderBar`, `NCProgressCtrl`, `NCListCtrl`, `NCMultiEdit`, `NCHtmlViewer`... | `include/nwindow/` | остальные элементы управления |
| `NC*Wnd` | `include/nwindow/` | окна игры (инвентарь, чат, панель ярлыков, статус, клан, лавки...) |
| `NCVirtualWndMain` | `include/nwindow/NCVirtualWndMain.h` | владелец всех окон: регистрация, порядок по Z, проверка попадания, фокус, рассылка сообщений, курсоры |
| `NConsoleWnd : UL2ConsoleWnd` | `include/nwindow/NConsoleWnd.h` | консоль движка (0x35B8 байт, 335 слотов: события сети и UI); `GConsole` |
| `NCConsole` | `include/nwindow/NCConsole.h` | игровой **рабочий стол** (`GConsole->GetDesktop()`): родитель всех игровых окон, с указателем на каждое (`ShortCutWnd`, `ChatWnd`, `InventoryWnd`, `SystemMenuWnd`...) |

### Создание окна или элемента управления

Образец из оригинального кода (`NCSystemMenuWnd::OnCreate` в `src/nwindow/NCSystemMenuWnd.cpp`):

```cpp
NCButton* Button = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
Button->ID = NID_SYSMENU_HELP;          // то, что придёт в NWM_COMMAND родителя
Button->SetWindowRect(86, 61, 67, 19);  // относительно родителя
CreateChild(Button, NWS_VISIBLE);       // регистрирует, делает положение абсолютным, шлёт WM_CREATE -> OnCreate
Button->SetTextID(0x91);                // текст из таблицы SysString клиента
```

- `new` для класса пакета — это `operator new` UE2 (объект — `UObject`, память выделяется по размеру
  зарегистрированного `UClass`).
- `CreateChild(child, style)` (`NCWnd`, inline) = `AddChildWindow` + `Entry` + экранное положение + `WM_CREATE`.
  Рабочий стол использует ещё скрытые варианты `CreateHiddenChild` / `CreateChildHidden`
  (`src/nwindow/NCConsole.cpp`: `AddChildWindow`, `HideWindow`, затем присоединение).
- Стили (`NWS_*` в `NCWnd.h`): `NWS_VISIBLE`, `NWS_TOPMOST` (отдельный список, поверх), `NWS_TITLEBAR`
  (`NCWnd::OnCreate` создаёт заголовок `NCFrameCtrl`), `NWS_MINIMIZABLE`, `NWS_NOCLOSE`, `NWS_MODAL` (новое в C2).
- Уничтожение: в `OnDestroy` владельца `RemoveChildWindow(Child->Entry); delete Child;` (см.
  `NCSystemMenuWnd::OnDestroy`). `NCWnd::OnDestroy` уничтожает только заголовок, значок и `ToolTipSkin`.

### Сообщения, отрисовка, подсказки

- Всё приходит через `WindowProc(Msg, wParam, lParam)` (vslot 25) с номерами сообщений Win32; `NCWnd` распределяет
  их по виртуальным методам: `OnCreate` (36), `OnDestroy` (37), `OnPaint(NCGDevice*)` (55), `OnDrawToolTip` (56),
  `OnKeyDown`, `OnMouseMove`, `OnLButtonDown/Up`, `OnRButtonDown/Up`, `OnMove` (`WM_MOVE`), `OnMouseWheel`.
- Элемент управления сообщает родителю через `NWM_COMMAND` (`0x501`), `wParam` = `ID` элемента. Родитель
  обрабатывает это в своём `WindowProc` (пример: `NCSystemMenuWnd::WindowProc`).
- Отрисовка (`OnPaint`) использует `NCGDevice` (`include/nwindow/NCGDevice.h`, сигнатуры C2 — в комментарии в
  начале файла): `SetClip`, `BeginPaint(ScreenX, ScreenY)`, `DrawTexture`, `DrawRect`, `DrawText`, `EndPaint`,
  `ReleaseClip`. Текстуры загружаются один раз в `OnCreate` через `FindTexture` (пакеты клиента `L2UI.*`).
- Подсказка: `OnDrawToolTip(NCGDevice*)` окна под курсором, вызывается после отрисовки.

### Консоль и рабочий стол

`NConsoleWnd` (объект, который движок создаёт по `L2.ini`) получает события сервера и игры (его 335 слотов
разнесены по темам в `src/nwindow/NConsoleWnd*.cpp`) и владеет рабочим столом `NCConsole`. `NCConsole::OnCreate`
создаёт все окна игры (положения считаются от размера экрана), а `NCConsole::OnDestroy` их уничтожает.
`NConsoleWnd::Tick` (`NConsoleWndInput.cpp`) выполняется каждый кадр.

## 2. Как добавить слой возможностей (его пока нет)

Способ, которым это сделано в C1 и при котором «судьи» остаются в силе:

1. **Новая цель в `CMakeLists.txt`**, не трогая две «верные». Для этого уже есть функция
   `nwd_standalone_dll_of(objs target name dir)`: создайте библиотеку `OBJECT` с теми же исходниками, что у
   `nwd_standalone_objs`, плюс `src/features/*.cpp`, с теми же опциями (`${NWD_CXX_FLAGS} /Ob1`, `NW_HOOK_MODE=0`) и
   `NW_FEATURES=1`, и вызовите `nwd_standalone_dll_of(nwd_l2_objs NWindowL2 NWindow "${CMAKE_BINARY_DIR}/l2")`.
2. **Унаследованные точки вызова.** С `NW_FEATURES=1` блоки `#if NW_FEATURES`, уже имеющиеся в `src/nwindow`, начнут
   компилироваться и включать файлы `features/*.h`, которых здесь нет. Удалите те, что не будете переносить; нужные
   перенесите из [проекта C1](https://github.com/luannbr/Lineage2_Interface_source_C1_-Nwindow.dll-)
   (`src/features/`), исправив адреса и раскладки под C2 (адреса C1 здесь никогда не годятся).
3. **Код возможности** — в `include/features/<Feature>.h` + `src/features/<Feature>.cpp`, в `namespace nwf` и
   `#if NW_FEATURES`. В декомпилированный код попадает только точка вызова, в блоке `#if NW_FEATURES` с комментарием
   `// phase 6: <Feature> ...`; без этого блока код должен читаться так же, как оригинал.
4. **Состояние** — в глобальных переменных `.cpp` возможности, никогда не в новых полях оригинального класса (его
   размер зарегистрирован в движке). **Настройки** — в реестре (`HKCU\Software\...`), никогда не в файле рядом с DLL.
5. **Полезные точки входа**: `NCConsole::OnCreate`/`OnDestroy` (создание/уничтожение окон), `NConsoleWnd::Tick`
   (каждый кадр), `WindowProc`/`OnPaint`/`OnDrawToolTip`/`OnLButtonUp` окна, `NConsoleWnd::Say2` (чат). `DllMain`
   standalone находится в `src/standalone/NWindowPackage.cpp`.
6. **Новый класс** (окно со своими полями): в C1 это `NW_DECLARE_L2_CLASS` / `NW_IMPLEMENT_L2_CLASS`
   (`include/features/NwL2Class.h` проекта C1, его нужно перенести) плюс строка в `tests/standalone/l2_extras.csv`;
   `python tools/verify_standalone.py --dll build/l2/Release/NWindow.dll --l2` принимает ровно эти дополнительные
   классы.
7. **Доказательство**: циклы режима хуков и standalone остаются «зелёными» (`tools\port\cycle.bat`,
   `python tools\cycle.py --game-source --standalone --no-install`), новая цель проходит `verify_standalone.py --l2`,
   а проверка в игре делается на копии папки `system`.

## 3. Рецепт: новая панель ярлыков (как в C5 / Interlude)

В C2 не реализовано. Идея (из C5, уже сделанная в C1 как эксперимент) ложится на классы C2 без изменения протокола,
сервера и текстур:

**Идея:** дополнительная панель **не владеет слотами**. Это ещё один экземпляр `NCShortCutWnd`, показывающий другую
страницу той же таблицы ярлыков (`Cols × Pages`, 10 × 10 у основной панели). Слот ярлыка — `страница × 10 + позиция`,
число, которое сервер хранит и возвращает; панель, которая лишь показывает другую страницу, ничего не меняет в
протоколе. Класс сохраняет свой размер (0x208), новый `UClass` не нужен.

**Шаги:**

1. **Файлы возможности:** `include/features/ShortCutBars.h` / `src/features/ShortCutBars.cpp` (`namespace nwf`,
   `#if NW_FEATURES`): `ShortCutBars_Create(NCWnd* Desktop, NCShortCutWnd* Main)`, `ShortCutBars_Destroy`,
   `ShortCutBars_OnTick`, `ShortCutBars_OnCommand`... Панели и кнопка хранятся в глобальных переменных `.cpp`.
2. **Создание** в `NCConsole::OnCreate`, сразу после основной панели (`ShortCutWnd = new NCShortCutWnd(10, 10, 32);
   SetWindowRect(Width - 45, Height / 2 - 218, 44, 437); CreateChildHidden(this, ShortCutWnd, 0x1022);`): для каждой
   дополнительной панели `new NCShortCutWnd(Main->Cols, Main->Pages, Main->IconSize)`, `SetWindowRect` и та же скрытая
   регистрация. Оригинальный `OnCreate` панели выделяет собственные таблицы (`Items = new NShortCutItem[Cols * Pages]`,
   `Keys = new NShortCutKey[...]`): освободите их (`delete[]`) и укажите на таблицы основной (`Bar->Items =
   Main->Items`, `Bar->Keys = Main->Keys`). `Bar->Page` — первая показываемая страница. Скройте `RotateButton`
   дополнительных панелей.
3. **Уничтожение** в `NCConsole::OnDestroy`, **до** `DestroyChild(this, ShortCutWnd)`: у дополнительных панелей
   обнулите `Items` и `Keys` (они принадлежат основной; иначе оригинальный `OnDestroy` освободит их дважды), затем
   `RemoveChildWindow(Bar->Entry)` и `delete Bar`.
4. **Следовать за основной панелью** каждый кадр (из тика возможности): та же ориентация (`bHorizontal`, и
   `UpdateLayout()` — vslot 68 — при её смене; в C2 есть ещё раскладка геймпада, `NID_SHORTCUT_PAD`), положение рядом
   с основной через `SetWindowRect`, видимость — только пока видна основная.
5. **Кнопка разворота** (необязательно): `NCPushButton` — дочернее окно основной панели, с `ID` после
   оригинальных (у панели они доходят до `0xFF000009`), обрабатывается в ветке `NWM_COMMAND` метода
   `NCShortCutWnd::WindowProc`.
6. **Перетаскивание между панелями:** в `NCShortCutWnd::OnLButtonUp` оригинальная ветка, которая **удаляет** ярлык,
   брошенный за пределами панели (`GConsole->RequestShortCutDel`), сначала проверяет, не над другой ли панелью точка,
   и если да — переносит ярлык в слот там (регистрация + удаление по оригинальному протоколу).
7. **Перетаскивание ручки дополнительной панели двигает группу:** `WM_MOVE` дополнительной панели передаётся
   основной (`NCWnd::Snap` тянет группу; задайте соответствующую точку захвата у `FrameCtrl` основной панели).

Что видно только в игре: положение, графика (на фоне панели в текстуре нарисован значок кнопки поворота) и
перетаскивание. Фильтр пакетов сервера может отбрасывать регистрации ярлыков, отправленные слишком быстро.

## 4. Рецепт: новая кнопка в существующем окне

Пример: ещё одна кнопка в системном меню (`NCSystemMenuWnd`, в C2 — шесть кнопок в один столбец).

1. `include/features/MyButton.h` / `src/features/MyButton.cpp`: `MyButton_Create(NCWnd* Owner)`,
   `MyButton_OnCommand(NCWnd* Owner, WPARAM ID)`, `MyButton_Destroy(NCWnd* Owner)`; указатель на кнопку — в глобальной
   переменной.
2. Создайте её в конце `NCSystemMenuWnd::OnCreate` (`#if NW_FEATURES` / `// phase 6: MyButton` /
   `nwf::MyButton_Create(this);`). Внутри: `new NCButton(normal, pushed, 0)`, `ID`, который не использует ни один
   элемент этого окна (см. перечисление `NID_SYSMENU_*`, `0xF0F006..0xF0F023`), `SetWindowRect` внутри окна (кнопки
   стоят на x 86, через 23 пикселя начиная с y 38), `Owner->CreateChild(Button, NWS_VISIBLE)`, `SetText`.
3. Обработайте нажатие в ветке `NWM_COMMAND` метода `NCSystemMenuWnd::WindowProc`:
   `if (nwf::MyButton_OnCommand(this, wParam)) return 1;` (в `#if NW_FEATURES`).
4. Уничтожьте её в `NCSystemMenuWnd::OnDestroy`, до `NCWnd::OnDestroy()`: `RemoveChildWindow(Button->Entry);
   delete Button;`.
5. Если окно должно вырасти: `SetWindowRect(Width - 290, 250, 240, 192)` в `NCConsole::OnCreate` и отрисовка фона в
   его `OnPaint` — всегда в `#if NW_FEATURES`, оригинальная ветка остаётся нетронутой.

## 5. Рецепт: новое окно

1. Окно со своими полями или виртуальными методами — это новый класс, регистрируемый только в цели возможностей
   (раздел 2, пункт 6): `class NCMyWnd : public NCWnd { ... };`.
2. Переопределите нужные виртуальные методы: `OnCreate` (сначала вызовите `NCWnd::OnCreate()`, загрузите текстуры
   через `FindTexture`, создайте заголовок и элементы), `OnDestroy` (уничтожьте дочерние окна, затем
   `NCWnd::OnDestroy()`), `WindowProc` (`NWM_COMMAND`), `OnPaint`.
3. Создавайте его на рабочем столе, когда нужно (`SetWindowRect(0, 0, W, H)`, `GConsole->GetDesktop()` как родитель в
   `CreateChild(window, NWS_VISIBLE)`, `HideWindow()`), и показывайте через `ShowWindow()` +
   `Desktop->BringChildToTop(Entry)`.
4. Образцы в коде: `NCSystemMenuWnd` (оформленный заголовок `NCFrameCtrl(1)`, кнопки), окна рецептов C2
   (`NCRecipeBookWnd`, `NCRecipeShopWnd`, прокручиваемые списки) и, в проекте C1, `src/features/craft/`.

## 6. Рецепт: новый цвет чата

Сервер не присылает цвет: он присылает **тип** сообщения (`Say2`), а цвет выбирает клиент в `NConsoleWnd::Say2`
(`src/nwindow/NConsoleWndEvents.cpp`). C2 окрашивает типы 1 (крик, `0xFFFF7200`), 2 (личное, `0xFFFF00FF`), 3 (группа,
`0xFF00FF00`), 4 (клан, `0xFF7D77FF`), 6/7 (петиция, `0xFF80FFFF`), 8 (торговля, `0xFFEAA5F5`), 9 (альянс,
`0xFF77FF99`) и 10 (объявление); всё остальное попадает в последний `else`, белым.

1. В последнем `else` метода `Say2`, в `#if NW_FEATURES`, спросите возможность (`nwf::ChatColors_Find(Type, &Color)`
   в C1: точка вызова уже там) и вызовите `GetDesktop()->Say(ObjectID, Name, Text, Color, 0)`.
2. Строка по-прежнему идёт в чат как обычная (вкладка All, фильтр Normal). Не используйте типы с побочными эффектами
   (TELL добавляет говорящего в список личных сообщений).
3. Смена цвета типа, который C2 **уже** окрашивает, — это изменение «верной» ветки: делайте это под `#if NW_FEATURES`
   внутри `Say2`, сохранив оригинальный цвет в ветке `#else`.

## 7. Рецепт: новая строка в подсказке предмета

Полную подсказку предмета рисуют процедуры из `include/nwindow/NItemToolTip.h`, которые подключают сетки предметов:
`SetTexts`/`SetStoreTexts` заполняют `ToolTipName` и `ToolTipText` (текст, который **задаёт размер** рамки), а
`DrawLines` рисует строки с шагом 13 пикселей (`DrawTypeLines`, затем `DrawTailLines`, который заканчивается
описанием).

1. Добавьте строку в обоих местах, в `#if NW_FEATURES`: в текст, задающий размер (чтобы рамка выросла), и в
   отрисовку (например, в `DrawTailLines` перед описанием, через `DrawLabel`/`DrawValue` и `Y += 13`).
2. Процедуры — шаблоны, которые каждая сетка инстанцирует с разными параметрами (инвентарь, кукла, обмен, склад,
   питомец, личные лавки): проверьте каждую копию, которую меняете.
3. Для подсказки другого окна — его `OnDrawToolTip`; одна точка даёт дополнительную высоту, другая рисует строку.

## 8. Цикл сборки и проверки

```bat
set NWD_CLIENT_SYSTEM=<client>\system_nwtest
tools\port\cycle.bat
python tools\cycle.py --game-source --standalone --no-install
```

1. Сборка без ошибок и без новых предупреждений.
2. Запустите оба цикла: они доказывают, что режим хуков и standalone не изменились («судьи» сравнивают их с
   оригиналом).
3. Проверка в игре: закройте игру, сохраните `NWindow.dll` **копии** папки `system` и скопируйте поверх него
   собранную DLL. Цикл не рисует интерфейс: положение, графика и перетаскивание видны только в игре.
4. Диагностика сбоя standalone: `NWindow_crash.log` рядом с `l2.exe` и
   `python tools\port\map_addr.py --log NWindow_crash.log`.

## 9. Правила безопасности

- Никогда не добавляйте поля или новые виртуальные слоты в оригинальный класс: его размер зарегистрирован в движке,
  а оригинальный код обращается к нему и к его vtable по смещениям. Переопределять слот, который класс уже
  наследует, можно. Состояние возможности — в глобальных переменных `.cpp` или в новом классе цели возможностей.
- В декомпилированном коде — только точка вызова, в `#if NW_FEATURES` с `// phase 6: ...`; оригинальная ветка
  остаётся как есть.
- Любая правка другого модуля во время работы сначала проверяет байты и при несовпадении отключает возможность со
  строкой в логе.
- Тот же протокол, что у оригинального клиента: сервер не меняется; соблюдайте ограничения его фильтра пакетов.
- Проверяйте на копии клиента, никогда в папке, из которой играете; ни один бинарный файл игры не попадает в
  репозиторий.
- Небольшие коммиты, по одной теме, с «зелёным» циклом.
