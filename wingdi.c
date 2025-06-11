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


static void define_consts_polyfill_mode(JanetTable *env)
{
#define __def(const_name)                                    \
    janet_def(env, #const_name, jw32_wrap_int(const_name),   \
              "Constant for PolyFill modes.")

    __def(ALTERNATE);
    __def(WINDING);
    __def(POLYFILL_LAST);

#undef __def
}


static void define_consts_region(JanetTable *env)
{
#define __def(const_name)                                    \
    janet_def(env, #const_name, jw32_wrap_int(const_name),   \
              "Constant for region types.")

    /* XXX: This name is too generic, skip it to avoid confusion.
     *      Use RGN_ERROR instead.
     */
//    __def(ERROR);
    __def(NULLREGION);
    __def(SIMPLEREGION);
    __def(COMPLEXREGION);
    __def(RGN_ERROR);

#undef __def
}


static void define_consts_rgn(JanetTable *env)
{
#define __def(const_name)                                    \
    janet_def(env, #const_name, jw32_wrap_int(const_name),   \
              "Constant for CombineRgn modes.")

    __def(RGN_AND);
    __def(RGN_OR);
    __def(RGN_XOR);
    __def(RGN_DIFF);
    __def(RGN_COPY);
    __def(RGN_MIN);
    __def(RGN_MAX);

    /* See define_consts_region() for RGN_ERROR */

#undef __def
}


static void define_consts_rdh(JanetTable *env)
{
    janet_def(env, "RDH_RECTANGLES", jw32_wrap_dword(RDH_RECTANGLES),
              "Constant for region data header type.");
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


static Janet cfun_CreateRectRgn(int32_t argc, Janet *argv)
{
    int x1, y1, x2, y2;

    HRGN hRet;

    janet_fixarity(argc, 4);

    x1 = jw32_get_int(argv, 0);
    y1 = jw32_get_int(argv, 1);
    x2 = jw32_get_int(argv, 2);
    y2 = jw32_get_int(argv, 3);

    hRet = CreateRectRgn(x1, y1, x2, y2);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_CreateRectRgnIndirect(int32_t argc, Janet *argv)
{
    RECT rect;

    HRGN hRet;

    janet_fixarity(argc, 1);

    rect = jw32_get_rect(argv, 0);

    hRet = CreateRectRgnIndirect(&rect);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_CreateRoundRectRgn(int32_t argc, Janet *argv)
{
    int x1, y1, x2, y2, w, h;

    HRGN hRet;

    janet_fixarity(argc, 6);

    x1 = jw32_get_int(argv, 0);
    y1 = jw32_get_int(argv, 1);
    x2 = jw32_get_int(argv, 2);
    y2 = jw32_get_int(argv, 3);
    w  = jw32_get_int(argv, 4);
    h  = jw32_get_int(argv, 5);

    hRet = CreateRoundRectRgn(x1, y1, x2, y2, w, h);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_CreateEllipticRgn(int32_t argc, Janet *argv)
{
    int x1, y1, x2, y2;

    HRGN hRet;

    janet_fixarity(argc, 4);

    x1 = jw32_get_int(argv, 0);
    y1 = jw32_get_int(argv, 1);
    x2 = jw32_get_int(argv, 2);
    y2 = jw32_get_int(argv, 3);

    hRet = CreateEllipticRgn(x1, y1, x2, y2);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_CreateEllipticRgnIndirect(int32_t argc, Janet *argv)
{
    RECT rect;

    HRGN hRet;

    janet_fixarity(argc, 1);

    rect = jw32_get_rect(argv, 0);

    hRet = CreateEllipticRgnIndirect(&rect);
    return jw32_wrap_handle(hRet);
}


static Janet cfun_CreatePolygonRgn(int32_t argc, Janet *argv)
{
    JanetView points;
    int iMode;

    HRGN hRet;

    POINT *pptl = NULL;

    janet_fixarity(argc, 2);

    points = janet_getindexed(argv, 0);
    iMode  = jw32_get_int(argv, 1);

    pptl = janet_malloc(sizeof(*pptl) * points.len);
    if (!pptl) {
        janet_panic("failed to allocate memory for points");
    }

    for (int32_t i = 0; i < points.len; i++) {
        Janet item = points.items[i];
        JanetView view;

        if (!janet_indexed_view(item, &view.items, &view.len)) {
            janet_free(pptl);
            janet_panicf("invalid point #%d: %v", i, item);
        }
        if (2 != view.len) {
            janet_free(pptl);
            janet_panicf("bad point #%d: expected tuple or array of length 2, got %d", i, view.len);
        }
        if (!janet_checkint(view.items[0]) || !janet_checkint(view.items[1])) {
            janet_free(pptl);
            janet_panicf("bad coordinates for point #%d: expected 32 bit signed integers, got %v and %v",
                         i, view.items[0], view.items[1]);
        }

        pptl[i].x = jw32_unwrap_long(view.items[0]);
        pptl[i].y = jw32_unwrap_long(view.items[1]);
    }

    hRet = CreatePolygonRgn(pptl, points.len, iMode);

    janet_free(pptl);

    return jw32_wrap_handle(hRet);
}


static Janet cfun_CreatePolyPolygonRgn(int32_t argc, Janet *argv)
{
    JanetView polygons;
    int iMode;

    HRGN hRet;

    POINT *pptl = NULL;
    int *pc = NULL;
    int cPoly = 0;
    int cPoint = 0;
    JanetView *p_views = NULL;

    janet_fixarity(argc, 2);

    polygons = janet_getindexed(argv, 0);
    iMode  = jw32_get_int(argv, 1);

    p_views = janet_malloc(sizeof(*p_views) * polygons.len);
    if (!p_views) {
        janet_panic("failed to allocate memory for polygon views");
    }

#define __cleanup()                             \
    do {                                        \
        if (pptl)    { janet_free(pptl); }      \
        if (pc)      { janet_free(pc); }        \
        if (p_views) { janet_free(p_views); }   \
    } while (0)

    for (int32_t i = 0; i < polygons.len; i++) {
        Janet poly = polygons.items[i];

        if (!janet_indexed_view(poly, &(p_views[i].items), &(p_views[i].len))) {
            __cleanup();
            janet_panicf("bad polygon #%d: expected a tuple or array, got %v", i, poly);
        }

        cPoly++;
        cPoint += p_views[i].len;
    }

    pptl = janet_malloc(sizeof(*pptl) * cPoint);
    if (!pptl) {
        __cleanup();
        janet_panic("failed to allocate memory for points");
    }

    pc = janet_malloc(sizeof(*pc) * cPoly);
    if (!pc) {
        __cleanup();
        janet_panic("failed to allocate memory for point counts");
    }

    POINT *cur_pt = pptl;

    for (int32_t i = 0; i < polygons.len; i++) {
        JanetView *pv = &(p_views[i]);

        for (int32_t j = 0; j < pv->len; j++) {
            Janet item = pv->items[j];
            JanetView view;

            if (!janet_indexed_view(item, &view.items, &view.len)) {
                __cleanup();
                janet_panicf("invalid point #%d,%d: %v", i, j, item);
            }
            if (2 != view.len) {
                __cleanup();
                janet_panicf("bad point #%d,%d: expected tuple or array of length 2, got %d",
                             i, j, view.len);
            }
            if (!janet_checkint(view.items[0]) || !janet_checkint(view.items[1])) {
                __cleanup();
                janet_panicf("bad coordinates for point #%d,%d: expected 32 bit signed integers, got %v and %v",
                             i, j, view.items[0], view.items[1]);
            }

            cur_pt->x = jw32_unwrap_long(view.items[0]);
            cur_pt->y = jw32_unwrap_long(view.items[1]);
            cur_pt++;
        }

        pc[i] = pv->len;
    }

    hRet = CreatePolyPolygonRgn(pptl, pc, cPoly, iMode);

    __cleanup();

#undef __cleanup

    return jw32_wrap_handle(hRet);
}


static Janet cfun_CombineRgn(int32_t argc, Janet *argv)
{
    HRGN hrgnDst, hrgnSrc1, hrgnSrc2;
    int iMode;

    int iRet;

    janet_fixarity(argc, 4);

    hrgnDst = jw32_get_handle(argv, 0);
    hrgnSrc1 = jw32_get_handle(argv, 1);
    hrgnSrc2 = jw32_get_handle(argv, 2);
    iMode = jw32_get_int(argv, 3);

    iRet = CombineRgn(hrgnDst, hrgnSrc1, hrgnSrc2, iMode);
    return jw32_wrap_int(iRet);
}


static Janet cfun_EqualRgn(int32_t argc, Janet *argv)
{
    HRGN hrgn1, hrgn2;

    BOOL bRet;

    janet_fixarity(argc, 2);

    hrgn1 = jw32_get_handle(argv, 0);
    hrgn2 = jw32_get_handle(argv, 1);

    bRet = EqualRgn(hrgn1, hrgn2);
    return jw32_wrap_bool(bRet);
}


static int RGNDATA_get(void *p, Janet key, Janet *out)
{
    LPRGNDATA lpRgnData = (LPRGNDATA)p;

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type) do {                 \
        if (!janet_cstrcmp(kw, #member)) {              \
            *out = jw32_wrap_##type(lpRgnData->member);       \
            return 1;                                   \
        }                                               \
    } while (0)

    __get_member(rdh.dwSize, dword);
    __get_member(rdh.iType, dword);
    __get_member(rdh.nCount, dword);
    __get_member(rdh.nRgnSize, dword);
    if (!janet_cstrcmp(kw, "rdh.rcBound")) {
        *out = janet_wrap_struct(jw32_rect_to_struct(&(lpRgnData->rdh.rcBound)));
        return 1;
    }
    if (!janet_cstrcmp(kw, "Buffer")) {
        DWORD nCount = lpRgnData->rdh.nCount;
        JanetArray *buf_arr = janet_array(nCount);
        RECT *aRect = (RECT *)&(lpRgnData->Buffer);
        for (DWORD i = 0; i < nCount; i++) {
            janet_array_push(buf_arr, janet_wrap_struct(jw32_rect_to_struct(&aRect[i])));
        }
        *out = janet_wrap_array(buf_arr);
        return 1;
    }

#undef __get_member

    return 0;
}

static const JanetAbstractType jw32_at_RGNDATA = {
    .name = MOD_NAME "/RGNDATA",
    .gc = NULL,
    .gcmark = NULL,
    .get = RGNDATA_get,
    JANET_ATEND_GET
};


static Janet cfun_GetRegionData(int32_t argc, Janet *argv)
{
    HRGN hrgn;

    LPRGNDATA lpRgnData = NULL;

    DWORD nCount = 0;
    DWORD nRetCount = 0;

    janet_fixarity(argc, 1);

    hrgn = jw32_get_handle(argv, 0);

    nCount = GetRegionData(hrgn, 0, NULL);
    if (!nCount) {
        /* hrgn may be invalid, don't bother retrying */
        return janet_wrap_nil();
    }

    lpRgnData = janet_abstract(&jw32_at_RGNDATA, nCount);
    nRetCount = GetRegionData(hrgn, nCount, lpRgnData);
    if (!nRetCount) {
        /* lpRgnData will be freed by GC */
        return janet_wrap_nil();
    } else {
        return janet_wrap_abstract(lpRgnData);
    }
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
    {
        "CreateRectRgn",
        cfun_CreateRectRgn,
        "(" MOD_NAME "/CreateRectRgn x1 y1 x2 y2)\n\n"
        "Creates a rectangular region.",
    },
    {
        "CreateRectRgnIndirect",
        cfun_CreateRectRgnIndirect,
        "(" MOD_NAME "/CreateRectRgnIndirect lprect)\n\n"
        "Creates a rectangular region.",
    },
    {
        "CreateRoundRectRgn",
        cfun_CreateRoundRectRgn,
        "(" MOD_NAME "/CreateRoundRectRgn x1 y1 x2 y2 w h)\n\n"
        "Creates a rectangular region with rounded corners.",
    },
    {
        "CreateEllipticRgn",
        cfun_CreateEllipticRgn,
        "(" MOD_NAME "/CreateEllipticRgn x1 y1 x2 y2)\n\n"
        "Creates an elliptical region.",
    },
    {
        "CreateEllipticRgnIndirect",
        cfun_CreateEllipticRgnIndirect,
        "(" MOD_NAME "/CreateEllipticRgnIndirect lprect)\n\n"
        "Creates an elliptical region.",
    },
    {
        "CreatePolygonRgn",
        cfun_CreatePolygonRgn,
        "(" MOD_NAME "/CreatePolygonRgn points iMode)\n\n"
        "Creates a polygonal region.",
    },
    {
        "CreatePolyPolygonRgn",
        cfun_CreatePolyPolygonRgn,
        "(" MOD_NAME "/CreatePolyPolygonRgn polygons iMode)\n\n"
        "Creates a region consisting of a series of polygons.",
    },
    {
        "CombineRgn",
        cfun_CombineRgn,
        "(" MOD_NAME "/CombineRgn hrgnDst hrgnSrc1 hrgnSrc2 iMode)\n\n"
        "Combines two regions.",
    },
    {
        "EqualRgn",
        cfun_EqualRgn,
        "(" MOD_NAME "/EqualRgn hrgn1 hrgn2)\n\n"
        "Checks if two regions are equal in size and shape.",
    },
    {
        "GetRegionData",
        cfun_GetRegionData,
        "(" MOD_NAME "/GetRegionData hrgn)\n\n"
        "Retrieves data describing a region.",
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
    define_consts_polyfill_mode(env);
    define_consts_region(env);
    define_consts_rgn(env);
    define_consts_rdh(env);

    janet_register_abstract_type(&jw32_at_RGNDATA);

    janet_cfuns(env, MOD_NAME, cfuns);
}
