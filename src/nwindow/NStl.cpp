/*=============================================================================
    NStl.cpp - the std::wstring and std::vector instances of the stock build (VC 7.1 STL)
    without the STL. Stock: 0x10027bb0 .. 0x10027cac, 0x100286d0 .. 0x100286fb,
    0x10028820 .. 0x10028924, 0x1002a000 .. 0x1002ad5e, 0x1002c720 .. 0x1002c742,
    0x1002c920 .. 0x1002ca72, 0x1002cbf0 .. 0x1002cf9c, 0x1002cfa0 .. 0x1002cff5,
    0x1002d030 .. 0x1002d52e, 0x1002e030 .. 0x1002e1ac, 0x1002e7b0 .. 0x1002f169,
    0x1002f470 .. 0x1002f4d1, 0x1002f4e0 .. 0x1002f54d, 0x1002f5b0 .. 0x1002f63b.
    char_traits::copy is memcpy, char_traits::move memmove, as in the stock code. The vectors'
    length error is thrown like std::string's (NStdLib).
=============================================================================*/
#include "nwindow/NStl.h"
#include "nwindow/NHtmlLayout.h"
#include "nwindow/NStdLib.h"

#include <string.h>

/*-----------------------------------------------------------------------------
    std::wstring.
-----------------------------------------------------------------------------*/

// 0x100316e0 - the hint is not used (the stock code tail-jumps into GMalloc->Malloc)
TCHAR* NStlCharAllocator::Allocate(UINT Count, const void* Hint)
{
    return (TCHAR*)appMalloc(Count * sizeof(TCHAR), TEXT("new"));
}
NW_HOOK_METHOD(0x100316e0, NStlCharAllocator, Allocate)

// <xstring>'s _Xran / _Xlen (NStdLib; hook mode: straight into the stock library code)
#if NW_HOOK_MODE
NW_NAKED void NStlWString::Xran() const NW_STOCK(0x100e7261)
NW_NAKED void NStlWString::Xlen() const NW_STOCK(0x100e72b9)
#else
void NStlWString::Xran() const { NStdStringXran(); }
void NStlWString::Xlen() const { NStdStringXlen(); }
#endif

// 0x100323a0 (out-of-line copy of the destructor, for the stock unwind code)
NW_HOOK_DTOR(0x100323a0, NStlWString)

// char_traits<wchar_t>::find / compare, inlined in find
static FORCEINLINE const TCHAR* TraitsFind(const TCHAR* Str, UINT Count, TCHAR Ch)
{
    for (; Count > 0; --Count, ++Str)
        if (*Str == Ch)
            return Str;
    return NULL;
}
static FORCEINLINE INT TraitsCompare(const TCHAR* A, const TCHAR* B, UINT Count)
{
    for (; Count > 0; --Count, ++A, ++B)
        if (*A != *B)
            return *A < *B ? -1 : 1;
    return 0;
}

// 0x10031600 - an empty string matches at Off (if inside)
UINT NStlWString::Find(const TCHAR* Str, UINT Off, UINT Count) const
{
    if (Count == 0 && Off <= Size)
        return Off;
    UINT Nm;
    if (Off < Size && Count <= (Nm = Size - Off))
    {
        const TCHAR* U;
        const TCHAR* V;
        for (Nm -= Count - 1, V = CStr() + Off; (U = TraitsFind(V, Nm, *Str)) != NULL;
             Nm -= U - V + 1, V = U + 1)
            if (TraitsCompare(U, Str, Count) == 0)
                return U - CStr();
    }
    return NPos;
}
NW_HOOK_METHOD(0x10031600, NStlWString, Find)

// 0x10032460 - rounded up to a multiple of 8 characters less one; no exponential growth
void NStlWString::Copy(UINT NewSize, UINT OldLen)
{
    UINT NewRes = NewSize | 7;
    if (NewRes > 0x7FFFFFFE)
        NewRes = NewSize;
    TCHAR* NewPtr = (TCHAR*)appMalloc(NewRes * sizeof(TCHAR) + sizeof(TCHAR), TEXT("new"));
    if (OldLen > 0)
        memcpy(NewPtr, CStr(), OldLen * sizeof(TCHAR));
    if (Res >= 8)
        appFree(Ptr);
    Buf[0] = 0;
    Ptr = NewPtr;
    Res = NewRes;
    Size = OldLen;
    CStr()[OldLen] = 0;
}
NW_HOOK_METHOD(0x10032460, NStlWString, Copy)

// 0x10033ee0 - trimming discards the old text; true while NewSize > 0
bool NStlWString::Grow(UINT NewSize, bool bTrim)
{
    if (NewSize > 0x7FFFFFFE)
        Xlen();
    if (Res < NewSize)
        Copy(NewSize, bTrim ? 0 : Size);
    else if (bTrim && NewSize < 8)
    {
        if (Res >= 8)
            appFree(Ptr);
        Res = 7;
        Size = 0;
        Buf[0] = 0;
    }
    else if (NewSize == 0)
        Eos(0);
    return 0 < NewSize;
}
NW_HOOK_METHOD(0x10033ee0, NStlWString, Grow)

// 0x10033fb0 - [Off, Off + N0) replaced with Right [ROff, ROff + Count); Right may be this string
NStlWString& NStlWString::Replace(UINT Off, UINT N0, const NStlWString& Right, UINT ROff,
                                  UINT Count)
{
    if (Size < Off || Right.Size < ROff)
        Xran();
    if (Size - Off < N0)
        N0 = Size - Off;
    UINT Num = Right.Size - ROff;
    if (Num < Count)
        Count = Num;
    if (NPos - Count <= Size - N0)
        Xlen();

    UINT Nm = Size - N0 - Off;             // the tail kept after the hole
    UINT NewSize = Size + Count - N0;
    if (Size < NewSize)
        Grow(NewSize);

    if (this != &Right)
    {
        memmove(CStr() + Off + Count, CStr() + Off + N0, Nm * sizeof(TCHAR));
        memcpy(CStr() + Off, Right.CStr() + ROff, Count * sizeof(TCHAR));
    }
    else if (Count <= N0)
    {
        // the hole does not grow: copy the substring in, then the tail up
        memmove(CStr() + Off, CStr() + ROff, Count * sizeof(TCHAR));
        memmove(CStr() + Off + Count, CStr() + Off + N0, Nm * sizeof(TCHAR));
    }
    else if (ROff <= Off)
    {
        // it grows, the substring starts before it
        memmove(CStr() + Off + Count, CStr() + Off + N0, Nm * sizeof(TCHAR));
        memmove(CStr() + Off, CStr() + ROff, Count * sizeof(TCHAR));
    }
    else if (Off + N0 <= ROff)
    {
        // it grows, the substring starts after it
        memmove(CStr() + Off + Count, CStr() + Off + N0, Nm * sizeof(TCHAR));
        memmove(CStr() + Off, CStr() + (ROff + Count - N0), Count * sizeof(TCHAR));
    }
    else
    {
        // it grows, the substring starts inside it
        memmove(CStr() + Off, CStr() + ROff, N0 * sizeof(TCHAR));
        memmove(CStr() + Off + Count, CStr() + Off + N0, Nm * sizeof(TCHAR));
        memmove(CStr() + Off + N0, CStr() + ROff + Count, (Count - N0) * sizeof(TCHAR));
    }

    if (Size < NewSize || Grow(NewSize))
        Eos(NewSize);
    return *this;
}
NW_HOOK_METHOD_SIG(0x10033fb0, NStlWString, Replace,
                   NStlWString& (NStlWString::*)(UINT, UINT, const NStlWString&, UINT, UINT))

// 0x10034940
NStlWString& NStlWString::Erase(UINT Off, UINT Count)
{
    if (Size < Off)
        Xran();
    if (Size - Off < Count)
        Count = Size - Off;
    if (0 < Count)
    {
        memmove(CStr() + Off, CStr() + Off + Count, (Size - Off - Count) * sizeof(TCHAR));
        UINT NewSize = Size - Count;
        if (Grow(NewSize))
            Eos(NewSize);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x10034940, NStlWString, Erase, NStlWString& (NStlWString::*)(UINT, UINT))

// 0x10034a10 - a null iterator counts as the start (_Pdif)
TCHAR** NStlWString::Erase(TCHAR** Result, TCHAR* First, TCHAR* Last)
{
    UINT Count = First == NULL ? 0 : First - CStr();
    Erase(Count, Last == NULL ? 0 : Last - First);
    *Result = CStr() + Count;
    return Result;
}
NW_HOOK_METHOD_SIG(0x10034a10, NStlWString, Erase,
                   TCHAR** (NStlWString::*)(TCHAR**, TCHAR*, TCHAR*))

// 0x10034a80
NStlWString& NStlWString::Replace(UINT Off, UINT N0, const TCHAR* Str, UINT Count)
{
    if (Inside(Str))
        return Replace(Off, N0, *this, Str - CStr(), Count);
    if (Size < Off)
        Xran();
    if (Size - Off < N0)
        N0 = Size - Off;
    if (NPos - Count <= Size - N0)
        Xlen();
    UINT Nm = Size - N0 - Off;
    if (Count < N0)
        memmove(CStr() + Off + Count, CStr() + Off + N0, Nm * sizeof(TCHAR));
    UINT Num;
    if ((0 < Count || 0 < N0) && Grow(Num = Size + Count - N0))
    {
        if (N0 < Count)
            memmove(CStr() + Off + Count, CStr() + Off + N0, Nm * sizeof(TCHAR));
        memcpy(CStr() + Off, Str, Count * sizeof(TCHAR));
        Eos(Num);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x10034a80, NStlWString, Replace,
                   NStlWString& (NStlWString::*)(UINT, UINT, const TCHAR*, UINT))

// 0x10034c70
NStlWString& NStlWString::Assign(const NStlWString& Right, UINT ROff, UINT Count)
{
    if (Right.Size < ROff)
        Xran();
    UINT Num = Right.Size - ROff;
    if (Count < Num)
        Num = Count;
    if (this == &Right)
    {
        Erase(ROff + Num, NPos);
        Erase(0, ROff);
    }
    else if (Grow(Num, true))
    {
        memcpy(CStr(), Right.CStr() + ROff, Num * sizeof(TCHAR));
        Eos(Num);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x10034c70, NStlWString, Assign,
                   NStlWString& (NStlWString::*)(const NStlWString&, UINT, UINT))

// 0x10034d30
NStlWString& NStlWString::Assign(const TCHAR* Str, UINT Count)
{
    if (Inside(Str))
        return Assign(*this, Str - CStr(), Count);
    if (Grow(Count, true))
    {
        memcpy(CStr(), Str, Count * sizeof(TCHAR));
        Eos(Count);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x10034d30, NStlWString, Assign,
                   NStlWString& (NStlWString::*)(const TCHAR*, UINT))

// 0x10034f90
NStlWString& NStlWString::Assign(const TCHAR* Str)
{
    return Assign(Str, wcslen(Str));
}
NW_HOOK_METHOD_SIG(0x10034f90, NStlWString, Assign, NStlWString& (NStlWString::*)(const TCHAR*))

// 0x10036ea0
NStlWString& NStlWString::Append(const NStlWString& Right, UINT ROff, UINT Count)
{
    if (Right.Size < ROff)
        Xran();
    UINT Num = Right.Size - ROff;
    if (Num < Count)
        Count = Num;
    if (NPos - Size <= Count)
        Xlen();
    if (0 < Count && Grow(Num = Size + Count))
    {
        memcpy(CStr() + Size, Right.CStr() + ROff, Count * sizeof(TCHAR));
        Eos(Num);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x10036ea0, NStlWString, Append,
                   NStlWString& (NStlWString::*)(const NStlWString&, UINT, UINT))

// 0x10036fa0
NStlWString& NStlWString::Append(const TCHAR* Str, UINT Count)
{
    if (Inside(Str))
        return Append(*this, Str - CStr(), Count);
    if (NPos - Size <= Count)
        Xlen();
    UINT Num;
    if (0 < Count && Grow(Num = Size + Count))
    {
        memcpy(CStr() + Size, Str, Count * sizeof(TCHAR));
        Eos(Num);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x10036fa0, NStlWString, Append,
                   NStlWString& (NStlWString::*)(const TCHAR*, UINT))

// 0x100370c0
NStlWString& NStlWString::Insert(UINT Off, const NStlWString& Right, UINT ROff, UINT Count)
{
    if (Size < Off || Right.Size < ROff)
        Xran();
    UINT Num = Right.Size - ROff;
    if (Num < Count)
        Count = Num;
    if (NPos - Size <= Count)
        Xlen();
    if (0 < Count && Grow(Num = Size + Count))
    {
        memmove(CStr() + Off + Count, CStr() + Off, (Size - Off) * sizeof(TCHAR));
        if (this == &Right)
            memmove(CStr() + Off, CStr() + (Off < ROff ? ROff + Count : ROff),
                    Count * sizeof(TCHAR));
        else
            memcpy(CStr() + Off, Right.CStr() + ROff, Count * sizeof(TCHAR));
        Eos(Num);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x100370c0, NStlWString, Insert,
                   NStlWString& (NStlWString::*)(UINT, const NStlWString&, UINT, UINT))

// 0x100372b0
NStlWString& NStlWString::Insert(UINT Off, const TCHAR* Str, UINT Count)
{
    if (Inside(Str))
        return Insert(Off, *this, Str - CStr(), Count);
    if (Size < Off)
        Xran();
    if (NPos - Size <= Count)
        Xlen();
    UINT Num;
    if (0 < Count && Grow(Num = Size + Count))
    {
        memmove(CStr() + Off + Count, CStr() + Off, (Size - Off) * sizeof(TCHAR));
        memcpy(CStr() + Off, Str, Count * sizeof(TCHAR));
        Eos(Num);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x100372b0, NStlWString, Insert,
                   NStlWString& (NStlWString::*)(UINT, const TCHAR*, UINT))

// 0x100375e0 - insert(pos, ptr): the length of ptr is taken (the HTML word wrapper's tool calls it)
NStlWString& NStlWString::Insert(UINT Off, const TCHAR* Str)
{
    return Insert(Off, Str, wcslen(Str));
}
NW_HOOK_METHOD_SIG(0x100375e0, NStlWString, Insert, NStlWString& (NStlWString::*)(UINT, const TCHAR*))

// 0x10037430
NStlWString& NStlWString::Insert(UINT Off, UINT Count, TCHAR Ch)
{
    if (Size < Off)
        Xran();
    if (NPos - Size <= Count)
        Xlen();
    UINT Num;
    if (0 < Count && Grow(Num = Size + Count))
    {
        memmove(CStr() + Off + Count, CStr() + Off, (Size - Off) * sizeof(TCHAR));
        TCHAR* P = CStr() + Off;
        for (UINT i = 0; i < Count; i++)
            P[i] = Ch;
        Eos(Num);
    }
    return *this;
}
NW_HOOK_METHOD_SIG(0x10037430, NStlWString, Insert,
                   NStlWString& (NStlWString::*)(UINT, UINT, TCHAR))

// 0x10037860 - returned in Result (constructed here)
NStlWString* NStlWString::Substr(NStlWString* Result, UINT Off, UINT Count) const
{
    ::new (static_cast<void*>(Result)) NStlWString;
    Result->Assign(*this, Off, Count);
    return Result;
}
NW_HOOK_METHOD(0x10037860, NStlWString, Substr)

/*-----------------------------------------------------------------------------
    std::vector.
-----------------------------------------------------------------------------*/

// 0x10038500 - the three vectors' (one body)
void NStlVectorBase::Xlen() const
{
    NStdThrowLengthError("vector<T> too long");
}
NW_HOOK_METHOD(0x10038500, NStlVectorBase, Xlen)

// the algorithms, per element type
NW_HOOK_FUNC(0x10036cf0, NStlFill<_WORD>)
NW_HOOK_FUNC(0x10036d10, NStlFill<NHtmlTag*>)
NW_HOOK_FUNC(0x100378a0, NStlFill<NStlWString>)
NW_HOOK_FUNC(0x10036d30, NStlUninitCopy<_WORD>)
NW_HOOK_FUNC(0x10036d60, NStlUninitCopy<NHtmlTag*>)
NW_HOOK_FUNC(0x10037910, NStlUninitCopy<NStlWString>)
NW_HOOK_FUNC(0x10036df0, NStlUninitFillN<_WORD>)
NW_HOOK_FUNC(0x100385c0, NStlUninitFillN<NStlWString>)
NW_HOOK_FUNC(0x10036d90, NStlCopyBackward<_WORD>)
NW_HOOK_FUNC(0x10036dc0, NStlCopyBackward<NHtmlTag*>)
NW_HOOK_FUNC(0x10038580, NStlCopyBackward<NStlWString>)
NW_HOOK_FUNC(0x100378d0, NStlCopyBackwardOpt<NStlWString>)
NW_HOOK_FUNC(0x10037610, NStlDestroyRange<NStlWString>)
NW_HOOK_METHOD(0x100375b0, NStlWStringAllocator, Destroy)
NW_HOOK_METHOD(0x10037250, NStlWordVector, Ucopy)
NW_HOOK_METHOD(0x10037550, NStlWordVector, Ufill)
NW_HOOK_METHOD(0x10038c60, NStlWordVector, InsertN)
NW_HOOK_METHOD(0x10039920, NStlWordVector, PushBack)

// vector<NHtmlTag*> (the parser's tag list)
NW_HOOK_METHOD(0x10037280, NHtmlTagList, Ucopy)
NW_HOOK_METHOD(0x10037580, NHtmlTagList, Ufill)
NW_HOOK_METHOD(0x10038f10, NHtmlTagList, InsertN)

// vector<NStlWString> (NHtmlTag::AttrValues)
NW_HOOK_DTOR(0x10039200, NStlWStringVector)
NW_HOOK_METHOD(0x10036af0, NStlWStringVector, Size)
NW_HOOK_METHOD(0x100384e0, NStlWStringVector, Destroy)
NW_HOOK_METHOD(0x100391d0, NStlWStringVector, Ucopy)
NW_HOOK_METHOD(0x10039250, NStlWStringVector, Ufill)
NW_HOOK_METHOD(0x10039280, NStlWStringVector, InsertN)
NW_HOOK_METHOD(0x10039990, NStlWStringVector, Insert)
NW_HOOK_METHOD(0x10039a30, NStlWStringVector, PushBack)
