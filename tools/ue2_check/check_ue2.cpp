// check_ue2.cpp - compile-only translation unit for include/ue2/UE2Core.h and UE2Engine.h (C2).
//
// tools/ue2_check/run_check.py compiles it with the project's flags (NWD_CXX_FLAGS / NWD_DEFS of CMakeLists.txt,
// NW_HOOK_MODE=1 and 0) and requires: no diagnostic at all, which includes every static_assert of the headers
// (NW_ASSERT_OFFSET / NW_ASSERT_SIZE of the engine classes). The other two checks of the same script work on units it
// generates from the headers (tools/ue2_check/gen_checks.py): the link test of every ENGINE_API / CORE_API name
// against the import libraries of third_party/ue_import/*.def, and the slot and signature test of every declared
// virtual against the C2 DLLs.
//
// The extra asserts below are the facts of the C2 binaries the headers rest on, written once more here from the
// DLL analysis (tools/ue2_vtable_diff.py, tools/ue2_layouts.py), so that a header edit that drifts away from them
// fails here even if its own NW_ASSERT was edited along.
#include "ue2/UE2Core.h"
#include "ue2/UE2Engine.h"

// UObject keeps its C1 size; the C2 vtable has 25 slots (ClearL2Game at 22); class sizes from the IMPLEMENT_CLASS
// initializers of the C2 Engine.dll (tools/ue2_engine_classes.py)
static_assert(sizeof(UObject) == 0x28, "UObject is 0x28 bytes in C2");
static_assert(sizeof(AActor) == 0x394, "AActor is 0x394 bytes in C2 (C1 0x5DC)");
static_assert(sizeof(APawn) == 0x14F4, "APawn is 0x14F4 bytes in C2");
static_assert(sizeof(AController) == 0x534, "AController is 0x534 bytes in C2");
static_assert(sizeof(UCanvas) == 0x13C, "UCanvas is 0x13C bytes in C2");
static_assert(sizeof(UEngine) == 0x174, "UEngine is 0x174 bytes in C2");
static_assert(sizeof(UGameEngine) == 0x330, "UGameEngine is 0x330 bytes in C2");
static_assert(sizeof(FParser) == 0x418, "FParser::operator= copies 0x106 dwords in both builds");

// the by-value sizes that changed (tools/ue2_ret_diff.py: the `ret N` of the by-value exports); the headers still
// hold the C1 layouts of these, they are listed here so the day somebody redoes them this file says what they must be
//   User 0x1F8 (header: 0x208), PetInfo 0xAC (header: 0xB4), FL2GameData 0x605FC (header: 0x3AF90),
//   UL2ConsoleWnd 0x3518 (header: 0x36B8)
