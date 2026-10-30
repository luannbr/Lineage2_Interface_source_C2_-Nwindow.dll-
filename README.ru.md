# NWindow.dll Lineage II C2 — исходный код, восстановленный на C++

[Português](README.md) · [English](README.en.md) · **Русский**

Пофункциональное восстановление `NWindow.dll` клиента Lineage II **Chronicle 2 (Age of Splendor)**: нативной DLL
интерфейса (окна, элементы управления, HTML диалогов NPC, IME, чат, консоль, MSN). Цель — оригинальная сборка от
03.03.2005 (TimeDateStamp `0x4227239B`, Visual C++ .NET 2002 `13.00.9466`, md5 `0111f657…`, 225 нативных классов,
227 экспортов, 7 117 функций). Это форк [проекта C1](https://github.com/luannbr/Lineage2_Interface_source_C1_-Nwindow.dll-):
те же инструменты и тот же метод, исходный код перенесён на образ C2 (`tools/port/`). Один и тот же код собирается
двумя способами:

| Цель CMake | Результат | Что это |
|---|---|---|
| `NWindowDecomp` | `build/bin/Release/NWindowDecomp.dll` | **режим хуков**: работает рядом с оригинальным `NWindow.dll` и заменяет каждую восстановленную функцию переходом `jmp` на нашу |
| `NWindowStandalone` | `build/standalone/Release/NWindow.dll` | **standalone**: весь `NWindow.dll` только из нашего кода; именно его «судьи» сравнивают с оригиналом |

Слоя новых возможностей («фаза 6» проекта C1, `NWindowL2`) в этом репозитории нет: как его добавить, описано в
[docs/MODIFY_INTERFACE.ru.md](docs/MODIFY_INTERFACE.ru.md).

## Как это работает

**Режим хуков.** Установщик (`tools/install_test.py`) добавляет в оригинальный `NWindow.dll` импорт
`NWindowDecomp.dll`. Она загружается раньше, чем выполняется любой код оригинала, и ставит `jmp` на вход каждой уже
восстановленной функции (макрос `NW_HOOK_*` рядом с каждым определением, `src/re/NwHook.h`). Остальное — по-прежнему
код оригинала. Любую функцию можно отключить в `NWindowDecomp.ini` (`[Hooks] Disable=`), чтобы локализовать ошибку.

**Standalone (фаза 5).** При `NW_HOOK_MODE=0` макросы хуков пусты, и код сам собирает `NWindow.dll`: 225 `UClass`
(`IMPLEMENT_CLASS`, `src/standalone/Classes.gen.cpp`), глобальные переменные со значениями оригинала (`NwGlobals.cpp`),
227 экспортов с ординалами оригинала (`NWindow.def`) и 7 цветных курсоров. Движок (`Core.dll`/`Engine.dll`)
загружает нашу DLL так же, как загружал оригинал.

Два сознательных отличия от оригинала (подробности — в [docs/notas/C2_Fase5.md](docs/notas/C2_Fase5.md), на
португальском):
- сторонняя библиотека nProtect GameGuard, статически вшитая в оригинал (85 функций), **не входит в этот код**: в
  standalone три точки входа, которые вызывает игра, — документированные пустые функции
  (`include/nwindow/NGameGuard.h`), как в пропатченном клиенте, взятом за образец;
- NKeyCrypt (`npkcrypt.dll`, в оригинале импортируется по ординалам) загружается во время работы (`LoadLibrary` +
  ординал, `src/nwindow/NKeyCrypt.cpp`): standalone работает и с этой DLL, и без неё.

Standalone пишет рядом с `l2.exe` два диагностических файла: `NWindow_crash.log` (только при сбое процесса: регистры
и стек в виде модуль+RVA; имена даёт `python tools/port/map_addr.py <rva>`) и `NWindow_trace.log` (события клана,
`NW_TRACE`).

## Состояние

- Режим хуков: готово **3 451 из 4 652** функций, подлежащих восстановлению (**92,08 %** байтов кода); верификатор
  принимает их все (расхождение — только с объяснением в `db/verify_notes.csv`), `regcheck` чист, ни один хук не
  доходит до тела `int3`. Без хука, по причине: 416 по соглашению (*scalar deleting destructors*,
  `InternalConstructor`) и 686, которые компилятор сам генерирует в standalone (статическая инициализация,
  `DllMain`). Без хука и без причины: 99, из них 85 — библиотека nProtect и три её вызывающие функции (в standalone
  не входят) и 14 — крошечные тела, склеенные линкером. Живой счёт: [docs/PROGRESSO.md](docs/PROGRESSO.md)
  (генерирует `tools/build_db.py`).
- Standalone: линкуется, проходит `verify_standalone.py` (PE, импорты, ресурсы, 227 экспортов, 225 классов,
  227 vtable), smoke-тест (вывод совпадает с оригиналом построчно), `gen_globals.py --check` (82 глобальные
  переменные совпадают) и использовался в игре с сервером C2 (клан и личные лавки проверены в реальной сессии:
  [docs/notas/C2_Pacotes.md](docs/notas/C2_Pacotes.md)).
- Ещё не сделано: `difftest` (его случаи вызывают оригинал по адресам C1, и `tools/difftest.py` отказывается работать
  с образом C2) и `db/verified.csv` (ни одна функция не отмечена как проверенная в игре по отдельности).
- Технические заметки (`docs/`) — на португальском; комментарии в коде — на английском.

## Требования

- Windows, **Visual Studio 2022** с C++ для x86 (MSVC v143), **CMake ≥ 3.20**;
- **Python 3** с `pefile` и `capstone` (`pip install pefile capstone`): нужны сборке standalone (извлечение курсоров)
  и всем инструментам; `unicorn` — для дифференциальных тестов в `tools/port/emu/`;
- **ваш собственный клиент Lineage II C2**. В репозитории нет ни одного файла игры: скопируйте в `orig/` файлы из
  [orig/README.md](orig/README.md) (у `NWindow.dll` должен быть указанный там md5);
- для «судей» (`verify_hooks.py`, `regcheck.py`, `verify_standalone.py`, `build_db.py`, `tools/port/*`): экспорт Ghidra
  в `analysis/ghidra/`, который создаёт `bash tools/ghidra/run_full.sh` (Ghidra 11.3.2 и JDK 21; пути в
  `NWD_GHIDRA_HOME` и `GHIDRA_JAVA_HOME`).

## Сборка

В *Developer Command Prompt* x86 для VS 2022 (или после `vcvars32.bat`), в корне репозитория:

```bat
cmake -S . -B build -A Win32 -DNWD_GAME_SOURCES=ON
cmake --build build --config Release
cmake --build build --config Release --target NWindowStandalone
```

`-DNWD_GAME_SOURCES=ON` компилирует игровой код (`src/nwindow`). Без него (значение по умолчанию в `CMakeLists.txt`)
`NWindowDecomp` содержит только среду режима хуков и ничего не заменяет: это проверка цепочки установки. Первый
`--build` собирает DLL режима хуков и тестовые инструменты (`nwd_smoke`, `nwd_difftest`) и не требует `orig/`;
`NWindowStandalone` требует `orig/NWindow.dll`. Строка `'pwsh.exe' не является ...` в конце каждого проекта идёт от
окружения Visual Studio и не делает сборку неудачной.

## Установка и проверка

Используйте **копию** папки `system` клиента, а не ту, из которой вы играете
(`xcopy /E /I /Y "<client>\system" "<client>\system_nwtest"`).

**Режим хуков:**

```bat
python tools\install_test.py --system "<client>\system_nwtest"
python tools\install_test.py --system "<client>\system_nwtest" --restore
```

Первая команда записывает `NWindow.dll`, пересобранный из `orig/NWindow.dll` с дополнительным импортом,
`NWindowDecomp.dll` и `NWindowDecomp.ini` (бывший там `NWindow.dll` сохраняется как `NWindow.dll.pre-decomp-bak`);
вторая возвращает то, что было. `NWindowDecomp.log` в этой папке перечисляет все установленные хуки. В `.ini`:
`[Hooks] Enabled=0` оставляет весь код оригинала; `Disable=` принимает адрес (`1000cc40`), метод
(`NCGDevice::DrawText`) или класс (`NCGDevice::*`); `[Debug] Marker=1` рисует метки, которые рисует только наш код
(жёлтая линия сверху, `#` в пароле), чтобы убедиться, что хуки активны.

**Standalone:** закройте игру, сохраните `NWindow.dll` тестовой папки и скопируйте поверх него
`build\standalone\Release\NWindow.dll`. Только этот файл. Чтобы вернуться, положите оригинал обратно.

**Полный цикл** (останавливается на первом неудачном шаге):

```bat
set NWD_CLIENT_SYSTEM=<client>\system_nwtest
python tools\cycle.py --game-source --no-install
python tools\cycle.py --game-source --standalone --no-install
tools\port\cycle.bat
```

`cycle.py` конфигурирует и собирает `build/`, устанавливает в минимальную копию `build/smoke_system` (копирует из
тестовой папки `Core.dll`, `Engine.dll`, их зависимости и `npkcrypt.dll`/`npkpdb.dll`), загружает вне игры,
проверяет, запускает `regcheck` и обновляет базу; `--standalone` добавляет шаги standalone; без `--no-install`
устанавливает ещё и в тестовую папку (игра из неё должна быть закрыта). `tools\port\cycle.bat` — цикл, который должно
пройти каждое изменение: `cycle.py --game-source --no-install`, затем `verify_data.py` и `check_breakers.py` (он сам
вызывает `vcvars32.bat` из VS 2022 Community).

**Пути машины.** Ни один инструмент не содержит путей машины автора: папка вне репозитория берётся из параметра
командной строки или из одной из этих переменных (параметр важнее; инструмент, которому она нужна и которой нет,
останавливается и сообщает, какая):

| Переменная | Указывает на | Используется |
|---|---|---|
| `NWD_CLIENT_SYSTEM` | папку `system` тестового клиента (копию: установка пишет в неё) | `--system` по умолчанию в `install_test.py`, `cycle.py`, `smoke_standalone.py`, `difftest.py`; `verify_hooks.py --log`; `ue2_layouts.py` (`Engine.u`, `Core.u`) |
| `NWD_GHIDRA_HOME` | установку Ghidra (с `support/analyzeHeadless.bat`; `GHIDRA` по-прежнему имеет приоритет) | `tools/ghidra/*.sh`, `tools/c2/ghidra_census.sh` |
| `NWD_C1_ROOT` | копию [проекта C1](https://github.com/luannbr/Lineage2_Interface_source_C1_-Nwindow.dll-) с его `orig/` и `analysis/ghidra/` | `tools/port/*` (перенос C1 → C2), `tools/c2/compare_nwindow.py`, `tools/ue2_console_slots.py`, `ue2_gamedata_regions.py`, `ue2_ret_diff.py`, `ue2_vtable_diff.py` |
| `NWD_C2_SERVER` | папку развёртывания сервера (страницы в `gameserver/data/html-en`) | `difftest.py`, набор страниц `server` (без неё пропускается) |

## «Судьи»

| Судья | Команда | Что доказывает |
|---|---|---|
| smoke | `build\smoke\Release\nwd_smoke.exe build\smoke_system` | пропатченный `NWindow.dll` загружается вне игры, и каждый хук отвечает `ok` (`NWindowDecomp.log`) |
| verify | `python tools/verify_hooks.py --log build/smoke_system/NWindowDecomp.log` | каждая наша функция против оригинальной на уровне ассемблера: байты, снимаемые `ret`, слоты vtable и вызываемые импорты; расхождение — только с объяснением в `db/verify_notes.csv` |
| regcheck | `python tools/regcheck.py --game-src` | ни один вызывающий код оригинала не полагается на регистр, который наша функция может изменить (оригинал собран с `/GL`) |
| verify_data | `python tools/port/verify_data.py` | данные, которых касается функция (константы, смещения полей, строки), совпадают с оригиналом; просмотренные расхождения — в `db/verify_data_notes.csv` |
| breakers | `python tools/port/check_breakers.py` | ни один хук не доходит до функции с телом `int3` (`NW_UNPORTED`) |
| emu | `python tools/port/emu/t_<имя>.py ...` (например, `t_nkeycrypt.py 1500 1`) | оригинальная и наша функция выполняются рядом в unicorn на случайных входах и пишут одно и то же |
| standalone: объявления | `python tools/verify_standalone.py --checks source` | наши `NW_DECLARE_CLASS` / `NW_CHECK_VTABLE` против таблицы классов оригинала |
| standalone: verify | `python tools/verify_standalone.py --dll build/standalone/Release/NWindow.dll` | PE, импорты, ресурсы, экспорты, классы и vtable совпадают с оригиналом, без запуска |
| standalone: smoke | `python tools/smoke_standalone.py --dll build/standalone/Release/NWindow.dll` | загружается так, как загружает движок: классы зарегистрированы, `ConsoleWnd`/`L2Font` созданы, вывод совпадает с оригиналом |
| db | `python tools/build_db.py --game-src` | обновляет `db/functions.csv` и `docs/PROGRESSO.md` |

## Структура

```
orig/          бинарные файлы вашего клиента (вне git; md5 в orig/README.md)
analysis/      факты, извлечённые из оригинала (классы, EH, vtable; vtable Core/Engine; раскладки классов Engine); analysis/ghidra/ = экспорт Ghidra (генерируется, вне git)
db/            база функций (functions.csv генерируется) + имена, заметки и проверки, внесённые вручную; карты переноса C1 → C2
include/ue2/   объявления Core.dll/Engine.dll C2, сверенные с экспортами
include/nwindow/, src/nwindow/    декомпилированный код, один класс на файл
src/re/        среда режима хуков (установка хуков, лог, ini, совместимость CRT)
src/standalone/                   IMPLEMENT_CLASS / IMPLEMENT_PACKAGE, глобальные переменные, NWindow.def, пустой GameGuard, диагностические логи
tests/         эталонные выводы (golden, пока от C1) и списки standalone
third_party/ue_import/            .def экспортов Core/Engine (при сборке превращаются в .lib)
tools/         анализ, перенос C1 → C2 (tools/port), установка, судьи, цикл
docs/          план, соглашения, прогресс и технические заметки (на португальском)
```

## Руководства

- [docs/PLANO.md](docs/PLANO.md) — план и метод (унаследованы от C1); [docs/PLANO_C2.md](docs/PLANO_C2.md) — что
  относится к C2 (константы образа, инструменты); [docs/CONVENCOES.md](docs/CONVENCOES.md) — правила кода и ловушки
  (на португальском)
- [docs/notas/C2_Porte.md](docs/notas/C2_Porte.md) — перенос C1 → C2; [docs/notas/C2_Fase5.md](docs/notas/C2_Fase5.md) — standalone
- [docs/MODIFY_INTERFACE.ru.md](docs/MODIFY_INTERFACE.ru.md) — как изменять интерфейс (панель ярлыков, кнопка, окно, цвет чата, строка подсказки)
  · [Português](docs/MODIFICAR_INTERFACE.md) · [English](docs/MODIFY_INTERFACE.md)
- [docs/LLM.md](docs/LLM.md) — руководство для ИИ-агентов, работающих с этим кодом (на английском)

## Правовая оговорка

Независимый проект обратной разработки ради совместимости и сохранения. Он не связан с NCSoft, не одобрен и не
поддерживается ею. *Lineage II* — зарегистрированная торговая марка NCSoft Corporation. **Ни одного оригинального
файла игры не включено** (ни DLL, ни исполняемых файлов, ни пакетов данных): каждый пользователь использует свой
собственный клиент.

## Лицензия

[GNU General Public License v3.0](LICENSE).
