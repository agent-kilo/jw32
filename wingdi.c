#include "jw32.h"
#include "debug.h"

#define MOD_NAME "wingdi"


static void define_consts_stock_object(JanetTable *env)
{
#define __def(const_name)                                    \
    janet_def(env, #const_name, jw32_wrap_int(const_name),   \
              "Constants stock GDI objects.")

    __def(WHITE_BRUSH);
    __def(LTGRAY_BRUSH);
    __def(GRAY_BRUSH);
    __def(DKGRAY_BRUSH);
    __def(BLACK_BRUSH);
    __def(NULL_BRUSH);
    __def(HOLLOW_BRUSH);
    __def(WHITE_PEN);
    __def(BLACK_PEN);
    __def(NULL_PEN);
    __def(OEM_FIXED_FONT);
    __def(ANSI_FIXED_FONT);
    __def(ANSI_VAR_FONT);
    __def(SYSTEM_FONT);
    __def(DEVICE_DEFAULT_FONT);
    __def(DEFAULT_PALETTE);
    __def(SYSTEM_FIXED_FONT);
#if(WINVER >= 0x0400)
    __def(DEFAULT_GUI_FONT);
#endif /* WINVER >= 0x0400 */
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)
    __def(DC_BRUSH);
    __def(DC_PEN);
#endif
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)
    __def(STOCK_LAST);
#elif (WINVER >= 0x0400)
    __def(STOCK_LAST);
#else
    __def(STOCK_LAST);
#endif

#undef __def
}


static void define_consts_clr(JanetTable *env)
{
    janet_def(env, "CLR_INVALID", jw32_wrap_dword(CLR_INVALID),
              "Constant for invalid color.");
}


static void define_consts_bk_mode(JanetTable *env)
{
#define __def(const_name)                                    \
    janet_def(env, #const_name, jw32_wrap_int(const_name),   \
              "Constant for background modes.")

    __def(OPAQUE);
    __def(TRANSPARENT);
    __def(BKMODE_LAST);

#undef __def
}


static void define_consts_stretch_blt_mode(JanetTable *env)
{
#define __def(const_name)                                    \
    janet_def(env, #const_name, jw32_wrap_int(const_name),   \
              "Constant for StretchBlt modes.")

    __def(BLACKONWHITE);
    __def(WHITEONBLACK);
    __def(COLORONCOLOR);
    __def(HALFTONE);
    __def(MAXSTRETCHBLTMODE);
#if(WINVER >= 0x0400)
    __def(STRETCH_ANDSCANS);
    __def(STRETCH_ORSCANS);
    __def(STRETCH_DELETESCANS);
    __def(STRETCH_HALFTONE);
#endif /* WINVER >= 0x0400 */

#undef __def
}


static void define_consts_raster_operation_code(JanetTable *env)
{
#define __def(const_name)                                    \
    janet_def(env, #const_name, jw32_wrap_int(const_name),   \
              "Constant for raster-operation codes.")

    /* Binary raster ops */
    __def(R2_BLACK);
    __def(R2_NOTMERGEPEN);
    __def(R2_MASKNOTPEN);
    __def(R2_NOTCOPYPEN);
    __def(R2_MASKPENNOT);
    __def(R2_NOT);
    __def(R2_XORPEN);
    __def(R2_NOTMASKPEN);
    __def(R2_MASKPEN);
    __def(R2_NOTXORPEN);
    __def(R2_NOP);
    __def(R2_MERGENOTPEN);
    __def(R2_COPYPEN);
    __def(R2_MERGEPENNOT);
    __def(R2_MERGEPEN);
    __def(R2_WHITE);
    __def(R2_LAST);

    /* Ternary raster operations */
    __def(SRCCOPY);
    __def(SRCPAINT);
    __def(SRCAND);
    __def(SRCINVERT);
    __def(SRCERASE);
    __def(NOTSRCCOPY);
    __def(NOTSRCERASE);
    __def(MERGECOPY);
    __def(MERGEPAINT);
    __def(PATCOPY);
    __def(PATPAINT);
    __def(PATINVERT);
    __def(DSTINVERT);
    __def(BLACKNESS);
    __def(WHITENESS);
#if(WINVER >= 0x0500)
    __def(NOMIRRORBITMAP);
    __def(CAPTUREBLT);
#endif /* WINVER >= 0x0500 */

#undef __def
}


static Janet cfun_CreateCompatibleDC(int32_t argc, Janet *argv)
{
    HDC hdc;

    HDC hRet;

    janet_fixarity(argc, 1);

    hdc = jw32_get_handle(argv, 0);
    hRet = CreateCompatibleDC(hdc);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_DeleteDC(int32_t argc, Janet *argv)
{
    HDC hdc;

    BOOL bRet;

    janet_fixarity(argc, 1);

    hdc = jw32_get_handle(argv, 0);
    bRet = DeleteDC(hdc);
    return jw32_wrap_bool(bRet);
}


static Janet cfun_CreateCompatibleBitmap(int32_t argc, Janet *argv)
{
    HDC hdc;
    int cx, cy;

    HBITMAP hRet;

    janet_fixarity(argc, 3);

    hdc = jw32_get_handle(argv, 0);
    cx = jw32_get_int(argv, 1);
    cy = jw32_get_int(argv, 2);

    hRet = CreateCompatibleBitmap(hdc, cx, cy);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_SelectObject(int32_t argc, Janet *argv)
{
    HDC hdc;
    HGDIOBJ h;

    HGDIOBJ hRet;

    janet_fixarity(argc, 2);

    hdc = jw32_get_handle(argv, 0);
    h = jw32_get_handle(argv, 1);
    hRet = SelectObject(hdc, h);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_DeleteObject(int32_t argc, Janet *argv)
{
    HGDIOBJ ho;

    BOOL bRet;

    janet_fixarity(argc, 1);

    ho = jw32_get_handle(argv, 0);
    bRet = DeleteObject(ho);
    return jw32_wrap_bool(bRet);
}


static Janet cfun_Rectangle(int32_t argc, Janet *argv)
{
    HDC hdc;
    int left, top, right, bottom;

    BOOL bRet;

    janet_fixarity(argc, 5);

    hdc = jw32_get_handle(argv, 0);
    left = jw32_get_int(argv, 1);
    top = jw32_get_int(argv, 2);
    right = jw32_get_int(argv, 3);
    bottom = jw32_get_int(argv, 4);

    bRet = Rectangle(hdc, left, top, right, bottom);
    return jw32_wrap_bool(bRet);
}


static Janet cfun_GetStockObject(int32_t argc, Janet *argv)
{
    int i;

    HGDIOBJ hRet;

    janet_fixarity(argc, 1);

    i = jw32_get_int(argv, 0);

    hRet = GetStockObject(i);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_CreateFont(int32_t argc, Janet *argv)
{
    int cHeight,
        cWidth,
        cEscapement,
        cOrientation,
        cWeight;
    DWORD bItalic,
        bUnderline,
        bStrikeOut,
        iCharSet,
        iOutPrecision,
        iClipPrecision,
        iQuality,
        iPitchAndFamily;
    LPCSTR pszFaceName;

    HFONT hRet;

    janet_fixarity(argc, 14);

    cHeight = jw32_get_int(argv, 0);
    cWidth = jw32_get_int(argv, 1);
    cEscapement = jw32_get_int(argv, 2);
    cOrientation = jw32_get_int(argv, 3);
    cWeight = jw32_get_int(argv, 4);

    bItalic = jw32_get_dword(argv, 5);
    bUnderline = jw32_get_dword(argv, 6);
    bStrikeOut = jw32_get_dword(argv, 7);
    iCharSet = jw32_get_dword(argv, 8);
    iOutPrecision = jw32_get_dword(argv, 9);
    iClipPrecision = jw32_get_dword(argv, 10);
    iQuality = jw32_get_dword(argv, 11);
    iPitchAndFamily = jw32_get_dword(argv, 12);

    pszFaceName = jw32_get_lpcstr(argv, 13);

    hRet = CreateFont(cHeight,
                      cWidth,
                      cEscapement,
                      cOrientation,
                      cWeight,
                      bItalic,
                      bUnderline,
                      bStrikeOut,
                      iCharSet,
                      iOutPrecision,
                      iClipPrecision,
                      iQuality,
                      iPitchAndFamily,
                      pszFaceName);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_SetDCBrushColor(int32_t argc, Janet *argv)
{
    HDC hdc;
    COLORREF color;

    COLORREF cRet;

    janet_fixarity(argc, 2);

    hdc = jw32_get_handle(argv, 0);
    color = jw32_get_dword(argv, 1);

    cRet = SetDCBrushColor(hdc, color);
    return jw32_wrap_dword(cRet);
}


static Janet cfun_GetDCBrushColor(int32_t argc, Janet *argv)
{
    HDC hdc;

    COLORREF cRet;

    janet_fixarity(argc, 1);

    hdc = jw32_get_handle(argv, 0);

    cRet = GetDCBrushColor(hdc);
    return jw32_wrap_dword(cRet);
}


static Janet cfun_SetDCPenColor(int32_t argc, Janet *argv)
{
    HDC hdc;
    COLORREF color;

    COLORREF cRet;

    janet_fixarity(argc, 2);

    hdc = jw32_get_handle(argv, 0);
    color = jw32_get_dword(argv, 1);

    cRet = SetDCPenColor(hdc, color);
    return jw32_wrap_dword(cRet);
}


static Janet cfun_GetDCPenColor(int32_t argc, Janet *argv)
{
    HDC hdc;

    COLORREF cRet;

    janet_fixarity(argc, 1);

    hdc = jw32_get_handle(argv, 0);

    cRet = GetDCPenColor(hdc);
    return jw32_wrap_dword(cRet);
}


static Janet cfun_SetTextColor(int32_t argc, Janet *argv)
{
    HDC hdc;
    COLORREF color;

    COLORREF cRet;

    janet_fixarity(argc, 2);

    hdc = jw32_get_handle(argv, 0);
    color = jw32_get_dword(argv, 1);

    cRet = SetTextColor(hdc, color);
    return jw32_wrap_dword(cRet);
}


static Janet cfun_GetTextColor(int32_t argc, Janet *argv)
{
    HDC hdc;

    COLORREF cRet;

    janet_fixarity(argc, 1);

    hdc = jw32_get_handle(argv, 0);

    cRet = GetTextColor(hdc);
    return jw32_wrap_dword(cRet);
}


static Janet cfun_SetBkColor(int32_t argc, Janet *argv)
{
    HDC hdc;
    COLORREF color;

    COLORREF cRet;

    janet_fixarity(argc, 2);

    hdc = jw32_get_handle(argv, 0);
    color = jw32_get_dword(argv, 1);

    cRet = SetBkColor(hdc, color);
    return jw32_wrap_dword(cRet);
}


static Janet cfun_GetBkColor(int32_t argc, Janet *argv)
{
    HDC hdc;

    COLORREF cRet;

    janet_fixarity(argc, 1);

    hdc = jw32_get_handle(argv, 0);

    cRet = GetBkColor(hdc);
    return jw32_wrap_dword(cRet);
}


static Janet cfun_SetBkMode(int32_t argc, Janet *argv)
{
    HDC hdc;
    int mode;

    int iRet;

    janet_fixarity(argc, 2);

    hdc = jw32_get_handle(argv, 0);
    mode = jw32_get_int(argv, 1);

    iRet = SetBkMode(hdc, mode);
    return jw32_wrap_int(iRet);
}


static Janet cfun_GetBkMode(int32_t argc, Janet *argv)
{
    HDC hdc;

    int iRet;

    janet_fixarity(argc, 1);

    hdc = jw32_get_handle(argv, 0);

    iRet = GetBkMode(hdc);
    return jw32_wrap_int(iRet);
}


static Janet cfun_CreateSolidBrush(int32_t argc, Janet *argv)
{
    COLORREF color;

    HBRUSH hRet;

    janet_fixarity(argc, 1);

    color = jw32_get_dword(argv, 0);
    hRet = CreateSolidBrush(color);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_BitBlt(int32_t argc, Janet *argv)
{
    HDC hdc;
    int x, y;
    int cx, cy;
    HDC hdcSrc;
    int x1, y1;
    DWORD rop;

    BOOL bRet;

    janet_fixarity(argc, 9);

    hdc = jw32_get_handle(argv, 0);
    x = jw32_get_int(argv, 1);
    y = jw32_get_int(argv, 2);
    cx = jw32_get_int(argv, 3);
    cy = jw32_get_int(argv, 4);
    hdcSrc = jw32_get_handle(argv, 5);
    x1 = jw32_get_int(argv, 6);
    y1 = jw32_get_int(argv, 7);
    rop = jw32_get_dword(argv, 8);

    bRet = BitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
    return jw32_wrap_bool(bRet);
}


static Janet cfun_StretchBlt(int32_t argc, Janet *argv)
{
    HDC hdcDest;
    int xDest, yDest;
    int wDest, hDest;
    HDC hdcSrc;
    int xSrc, ySrc;
    int wSrc, hSrc;
    DWORD rop;

    BOOL bRet;

    janet_fixarity(argc, 11);

    hdcDest = jw32_get_handle(argv, 0);
    xDest = jw32_get_int(argv, 1);
    yDest = jw32_get_int(argv, 2);
    wDest = jw32_get_int(argv, 3);
    hDest = jw32_get_int(argv, 4);

    hdcSrc = jw32_get_handle(argv, 5);
    xSrc = jw32_get_int(argv, 6);
    ySrc = jw32_get_int(argv, 7);
    wSrc = jw32_get_int(argv, 8);
    hSrc = jw32_get_int(argv, 9);

    rop = jw32_get_dword(argv, 10);

    bRet = StretchBlt(hdcDest, xDest, yDest, wDest, hDest,
                      hdcSrc,  xSrc,  ySrc,  wSrc,  hSrc,
                      rop);
    return jw32_wrap_bool(bRet);
}


static Janet cfun_SetStretchBltMode(int32_t argc, Janet *argv)
{
    HDC hdc;
    int mode;

    int iRet;

    janet_fixarity(argc, 2);

    hdc = jw32_get_handle(argv, 0);
    mode = jw32_get_int(argv, 1);

    iRet = SetStretchBltMode(hdc, mode);
    return jw32_wrap_int(iRet);
}


static Janet cfun_GetStretchBltMode(int32_t argc, Janet *argv)
{
    HDC hdc;

    int iRet;

    janet_fixarity(argc, 1);

    hdc = jw32_get_handle(argv, 0);

    iRet = GetStretchBltMode(hdc);
    return jw32_wrap_int(iRet);
}


static const JanetReg cfuns[] = {
    {
        "CreateCompatibleDC",
        cfun_CreateCompatibleDC,
        "(" MOD_NAME "/CreateCompatibleDC hdc)\n\n"
        "Creates a memory device context compatible with the specified one.",
    },
    {
        "DeleteDC",
        cfun_DeleteDC,
        "(" MOD_NAME "/DeleteDC hdc)\n\n"
        "Deletes a device context.",
    },
    {
        "CreateCompatibleBitmap",
        cfun_CreateCompatibleBitmap,
        "(" MOD_NAME "/CreateCompatibleBitmap hdc cx cy)\n\n"
        "Creates a bitmap compatible with the device that is associated with the specified device context.",
    },
    {
        "SelectObject",
        cfun_SelectObject,
        "(" MOD_NAME "/Selection hdc h)\n\n"
        "Selects an GDI object into the specified device context.",
    },
    {
        "DeleteObject",
        cfun_DeleteObject,
        "(" MOD_NAME "/DeleteObject ho)\n\n"
        "Deletes a GDI object.",
    },
    {
        "Rectangle",
        cfun_Rectangle,
        "(" MOD_NAME "/Rectangle hdc left top right bottom)\n\n"
        "Draws a rectangle.",
    },
    {
        "GetStockObject",
        cfun_GetStockObject,
        "(" MOD_NAME "/GetStockObject i)\n\n"
        "Retrieves a handle to one of the stock GDI objects.",
    },
    {
        "CreateFont",
        cfun_CreateFont,
        "(" MOD_NAME "/CreateFont cHeight cWidth cEscapement cOrientation cWeight bItalic bUnderline bStrikeOut iCharSet iOutPrecision iClipPrecision iQuality iPitchAndFamily pszFaceName)\n\n"
        "Creates a logical font with the specified characteristics.",
    },
    {
        "SetDCBrushColor",
        cfun_SetDCBrushColor,
        "(" MOD_NAME "/SetDCBrushColor hdc color)\n\n"
        "Sets the current DC brush color.",
    },
    {
        "GetDCBrushColor",
        cfun_GetDCBrushColor,
        "(" MOD_NAME "/GetDCBrushColor hdc)\n\n"
        "Retrieves the current DC brush color.",
    },
    {
        "SetDCPenColor",
        cfun_SetDCPenColor,
        "(" MOD_NAME "/SetDCPenColor hdc color)\n\n"
        "Sets the current DC pen color.",
    },
    {
        "GetDCPenColor",
        cfun_GetDCPenColor,
        "(" MOD_NAME "/GetDCPenColor hdc)\n\n"
        "Retrieves the current DC pen color.",
    },
    {
        "SetTextColor",
        cfun_SetTextColor,
        "(" MOD_NAME "/SetTextColor hdc color)\n\n"
        "Sets the DC text color.",
    },
    {
        "GetTextColor",
        cfun_GetTextColor,
        "(" MOD_NAME "/GetTextColor hdc)\n\n"
        "Retrieves the DC text color.",
    },
    {
        "SetBkColor",
        cfun_SetBkColor,
        "(" MOD_NAME "/SetBkColor hdc color)\n\n"
        "Sets the DC background color.",
    },
    {
        "GetBkColor",
        cfun_GetBkColor,
        "(" MOD_NAME "/GetBkColor hdc)\n\n"
        "Retrieves the DC background color.",
    },
    {
        "SetBkMode",
        cfun_SetBkMode,
        "(" MOD_NAME "/SetBkMode hdc mode)\n\n"
        "Sets the DC background mode.",
    },
    {
        "GetBkMode",
        cfun_GetBkMode,
        "(" MOD_NAME "/GetBkMode hdc)\n\n"
        "Retrieves the DC background mode.",
    },
    {
        "CreateSolidBrush",
        cfun_CreateSolidBrush,
        "(" MOD_NAME "/CreateSolidBrush color)\n\n"
        "Creates a solid color brush.",
    },
    {
        "BitBlt",
        cfun_BitBlt,
        "(" MOD_NAME "/BitBlt hdc x y cx cy hdcSrc x1 y1 rop)\n\n"
        "Performs a bit-block transfer.",
    },
    {
        "StretchBlt",
        cfun_StretchBlt,
        "(" MOD_NAME "/StretchBlt hdcDest xDest yDest wDest hDest hdcSrc xSrc ySrc wSrc hSrc rop)\n\n"
        "Stretches or compresses a bitmap from a source rectangle, and fill the resulting bitmap into a destination rectangle.",
    },
    {
        "SetStretchBltMode",
        cfun_SetStretchBltMode,
        "(" MOD_NAME "/SetStretchBltMode hdc mode)\n\n"
        "Sets the DC bitmap stretching mode.",
    },
    {
        "GetStretchBltMode",
        cfun_GetStretchBltMode,
        "(" MOD_NAME "/GetStretchBltMode hdc)\n\n"
        "Retrieves the DC bitmap stretching mode.",
    },

    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_stock_object(env);
    define_consts_clr(env);
    define_consts_bk_mode(env);
    define_consts_stretch_blt_mode(env);
    define_consts_raster_operation_code(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
