/* Copied from https://beej.us/guide/bgnet/html/ */
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
** packi16() -- store a 16-bit int into a char buffer (like htons())
*/
void packi16(unsigned char *buf, unsigned int i) {
    *buf++ = i >> 8;
    *buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/
void packi32(unsigned char *buf, unsigned long int i) {
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf++ = i;
}

/*
** packi64() -- store a 64-bit int into a char buffer (like htonl())
*/
void packi64(unsigned char *buf, unsigned long long int i) {
    *buf++ = i >> 56;
    *buf++ = i >> 48;
    *buf++ = i >> 40;
    *buf++ = i >> 32;
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf++ = i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/
int unpacki16(unsigned char *buf) {
    unsigned int i2 = ((unsigned int)buf[0] << 8) | buf[1];
    int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffu) {
        i = i2;
    } else {
        i = -1 - (unsigned int)(0xffffu - i2);
    }

    return i;
}

/*
** unpacku16() -- unpack a 16-bit unsigned from a char buffer (like ntohs())
*/
unsigned int unpacku16(unsigned char *buf) {
    return ((unsigned int)buf[0] << 8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/
long int unpacki32(unsigned char *buf) {
    unsigned long int i2 = ((unsigned long int)buf[0] << 24) |
                           ((unsigned long int)buf[1] << 16) |
                           ((unsigned long int)buf[2] << 8) | buf[3];
    long int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffu) {
        i = i2;
    } else {
        i = -1 - (long int)(0xffffffffu - i2);
    }

    return i;
}

/*
** unpacku32() -- unpack a 32-bit unsigned from a char buffer (like ntohl())
*/
unsigned long int unpacku32(unsigned char *buf) {
    return ((unsigned long int)buf[0] << 24) |
           ((unsigned long int)buf[1] << 16) |
           ((unsigned long int)buf[2] << 8) | buf[3];
}

/*
** unpacki64() -- unpack a 64-bit int from a char buffer (like ntohl())
*/
long long int unpacki64(unsigned char *buf) {
    unsigned long long int i2 = ((unsigned long long int)buf[0] << 56) |
                                ((unsigned long long int)buf[1] << 48) |
                                ((unsigned long long int)buf[2] << 40) |
                                ((unsigned long long int)buf[3] << 32) |
                                ((unsigned long long int)buf[4] << 24) |
                                ((unsigned long long int)buf[5] << 16) |
                                ((unsigned long long int)buf[6] << 8) | buf[7];
    long long int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffffffffffu) {
        i = i2;
    } else {
        i = -1 - (long long int)(0xffffffffffffffffu - i2);
    }

    return i;
}

/*
** unpacku64() -- unpack a 64-bit unsigned from a char buffer (like ntohl())
*/
unsigned long long int unpacku64(unsigned char *buf) {
    return ((unsigned long long int)buf[0] << 56) |
           ((unsigned long long int)buf[1] << 48) |
           ((unsigned long long int)buf[2] << 40) |
           ((unsigned long long int)buf[3] << 32) |
           ((unsigned long long int)buf[4] << 24) |
           ((unsigned long long int)buf[5] << 16) |
           ((unsigned long long int)buf[6] << 8) | buf[7];
}

/*
** pack() -- store data dictated by the format string in the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (16-bit unsigned length is automatically prepended to strings)
*/

unsigned int pack(unsigned char *buf, char *format, ...) {
    va_list ap;

    signed char c;  // 8-bit
    unsigned char C;

    int h;  // 16-bit
    unsigned int H;

    long int l;  // 32-bit
    unsigned long int L;

    long long int q;  // 64-bit
    unsigned long long int Q;

    char *s;  // strings
    unsigned int len;

    unsigned int size = 0;

    va_start(ap, format);

    for (; *format != '\0'; format++) {
        switch (*format) {
            case 'c':  // 8-bit
                size += 1;
                c = (signed char)va_arg(ap, int);  // promoted
                *buf++ = c;
                break;

            case 'C':  // 8-bit unsigned
                size += 1;
                C = (unsigned char)va_arg(ap, unsigned int);  // promoted
                *buf++ = C;
                break;

            case 'h':  // 16-bit
                size += 2;
                h = va_arg(ap, int);
                packi16(buf, h);
                buf += 2;
                break;

            case 'H':  // 16-bit unsigned
                size += 2;
                H = va_arg(ap, unsigned int);
                packi16(buf, H);
                buf += 2;
                break;

            case 'l':  // 32-bit
                size += 4;
                l = va_arg(ap, long int);
                packi32(buf, l);
                buf += 4;
                break;

            case 'L':  // 32-bit unsigned
                size += 4;
                L = va_arg(ap, unsigned long int);
                packi32(buf, L);
                buf += 4;
                break;

            case 'q':  // 64-bit
                size += 8;
                q = va_arg(ap, long long int);
                packi64(buf, q);
                buf += 8;
                break;

            case 'Q':  // 64-bit unsigned
                size += 8;
                Q = va_arg(ap, unsigned long long int);
                packi64(buf, Q);
                buf += 8;
                break;

            case 's':  // string
                s = va_arg(ap, char *);
                len = strlen(s);
                size += len + 2;
                packi16(buf, len);
                buf += 2;
                memcpy(buf, s, len);
                buf += len;
                break;
        }
    }

    va_end(ap);

    return size;
}

/*
** unpack() -- unpack data dictated by the format string into the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (string is extracted based on its stored length, but 's' can be
**  prepended with a max length)
*/
void unpack(unsigned char *buf, char *format, ...) {
    va_list ap;

    signed char *c;  // 8-bit
    unsigned char *C;

    int *h;  // 16-bit
    unsigned int *H;

    long int *l;  // 32-bit
    unsigned long int *L;

    long long int *q;  // 64-bit
    unsigned long long int *Q;

    char *s;
    unsigned int len, maxstrlen = 0, count;

    va_start(ap, format);

    for (; *format != '\0'; format++) {
        switch (*format) {
            case 'c':  // 8-bit
                c = va_arg(ap, signed char *);
                if (*buf <= 0x7f) {
                    *c = *buf;
                }  // re-sign
                else {
                    *c = -1 - (unsigned char)(0xffu - *buf);
                }
                buf++;
                break;

            case 'C':  // 8-bit unsigned
                C = va_arg(ap, unsigned char *);
                *C = *buf++;
                break;

            case 'h':  // 16-bit
                h = va_arg(ap, int *);
                *h = unpacki16(buf);
                buf += 2;
                break;

            case 'H':  // 16-bit unsigned
                H = va_arg(ap, unsigned int *);
                *H = unpacku16(buf);
                buf += 2;
                break;

            case 'l':  // 32-bit
                l = va_arg(ap, long int *);
                *l = unpacki32(buf);
                buf += 4;
                break;

            case 'L':  // 32-bit unsigned
                L = va_arg(ap, unsigned long int *);
                *L = unpacku32(buf);
                buf += 4;
                break;

            case 'q':  // 64-bit
                q = va_arg(ap, long long int *);
                *q = unpacki64(buf);
                buf += 8;
                break;

            case 'Q':  // 64-bit unsigned
                Q = va_arg(ap, unsigned long long int *);
                *Q = unpacku64(buf);
                buf += 8;
                break;

            case 's':  // string
                s = va_arg(ap, char *);
                len = unpacku16(buf);
                buf += 2;
                if (maxstrlen > 0 && len > maxstrlen)
                    count = maxstrlen - 1;
                else
                    count = len;
                memcpy(s, buf, count);
                s[count] = '\0';
                buf += len;
                break;

            default:
                if (isdigit(*format)) {  // track max str len
                    maxstrlen = maxstrlen * 10 + (*format - '0');
                }
        }

        if (!isdigit(*format)) maxstrlen = 0;
    }

    va_end(ap);
}
