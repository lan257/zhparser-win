/* iconv_compat.c
 *
 * Minimal iconv compatibility layer for Windows using Win32 WideChar APIs.
 *
 * This is not a full iconv implementation. It provides a thin wrapper:
 *   - iconv_open: accepts a "to-encoding//IGNORE" style or simple names like "UTF-8"
 *   - iconv: handles common UTF-8 <-> UCS-2/UTF-16 conversions used by scws
 *   - iconv_close
 *
 * If scws requires exotic encodings you may need to provide a full iconv library.
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* A very small iconv_t type */
typedef struct {
    char tocode[64];
    char fromcode[64];
} *iconv_t;

iconv_t iconv_open(const char *tocode_raw, const char *fromcode_raw) {
    iconv_t cd = (iconv_t)malloc(sizeof(*cd));
    if (!cd) return (iconv_t)-1;
    /* store lower-case simple names */
    strncpy(cd->tocode, tocode_raw ? tocode_raw : "", sizeof(cd->tocode)-1);
    strncpy(cd->fromcode, fromcode_raw ? fromcode_raw : "", sizeof(cd->fromcode)-1);
    return cd;
}

size_t iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft) {
    if (!cd || !inbuf || !*inbuf) return (size_t)-1;
    /* We handle UTF-8 <-> UTF-16LE conversions which cover common cases.
       If both encodings contain 'UTF-8' or both contain 'UCS-2' treat as no-op. */
    const char *from = cd->fromcode;
    const char *to = cd->tocode;

    /* Simple case: if encodings equal, copy */
    if (strstr(from, "UTF-8") && strstr(to, "UTF-8")) {
        size_t need = *inbytesleft;
        if (need > *outbytesleft) {
            errno = E2BIG;
            return (size_t)-1;
        }
        memcpy(*outbuf, *inbuf, need);
        *outbuf += need;
        *inbuf += need;
        *outbytesleft -= need;
        *inbytesleft = 0;
        return 0;
    }

    /* Convert from UTF-8 to UTF-16LE */
    if (strstr(from, "UTF-8") && (strstr(to, "UCS-2") || strstr(to, "UTF-16") || strstr(to, "UTF-16LE"))) {
        int wideChars = MultiByteToWideChar(CP_UTF8, 0, *inbuf, (int)*inbytesleft, NULL, 0);
        int bytesNeeded = wideChars * sizeof(wchar_t);
        if ((size_t)bytesNeeded > *outbytesleft) {
            errno = E2BIG;
            return (size_t)-1;
        }
        MultiByteToWideChar(CP_UTF8, 0, *inbuf, (int)*inbytesleft, (wchar_t*)(*outbuf), wideChars);
        *outbuf += bytesNeeded;
        *inbuf += *inbytesleft;
        *outbytesleft -= bytesNeeded;
        *inbytesleft = 0;
        return 0;
    }

    /* Convert from UTF-16LE/UCS-2 to UTF-8 */
    if ((strstr(from, "UCS-2") || strstr(from, "UTF-16") || strstr(from, "UTF-16LE")) && strstr(to, "UTF-8")) {
        int wideChars = (int)(*inbytesleft / sizeof(wchar_t));
        int bytesNeeded = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)(*inbuf), wideChars, NULL, 0, NULL, NULL);
        if (bytesNeeded > (int)*outbytesleft) {
            errno = E2BIG;
            return (size_t)-1;
        }
        WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)(*inbuf), wideChars, *outbuf, bytesNeeded, NULL, NULL);
        *outbuf += bytesNeeded;
        *inbuf += *inbytesleft;
        *outbytesleft -= bytesNeeded;
        *inbytesleft = 0;
        return 0;
    }

    /* Unsupported conversion */
    errno = EINVAL;
    return (size_t)-1;
}

int iconv_close(iconv_t cd) {
    if (!cd) return -1;
    free(cd);
    return 0;
}

#endif /* _WIN32 */