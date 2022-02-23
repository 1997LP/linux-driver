#include <stdarg.h>
#include "types.h"
#include "printf.h"
#include "ctype.h"
#include "common.h"
#include "hardware_test_dw_apb_uart.h"
#include <stddef.h>

char * strcpy(char * dest,const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

char * strncpy(char * dest,const char *src,size_t count)
{
	char *tmp = dest;

	while (count-- && (*dest++ = *src++) != '\0')
		/* nothing */;

	return tmp;
}

int strcmp(const char * cs,const char * ct)
{
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

int memcmp(const void * cs,const void * ct,size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}


void * memset(void * s,int c,size_t count)
{
	unsigned long *sl = (unsigned long *) s;
	char *s8;

	s8 = (char *)sl;
	while (count--)
		*s8++ = c;

	return s;
}

void * memcpy(void *dest, const void *src, size_t count)
{
	unsigned long *dl = (unsigned long *)dest, *sl = (unsigned long *)src;
	char *d8, *s8;

	if (src == dest)
		return dest;

	/* while all data is aligned (common case), copy a word at a time */
	if ( (((ulong)dest | (ulong)src) & (sizeof(*dl) - 1)) == 0) {
		while (count >= sizeof(*dl)) {
			*dl++ = *sl++;
			count -= sizeof(*dl);
		}
	}
	/* copy the reset one byte at a time */
	d8 = (char *)dl;
	s8 = (char *)sl;
	while (count--)
		*d8++ = *s8++;

	return dest;
}



size_t strlen(const char * s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

static const char *_parse_integer_fixup_radix(const char *s, unsigned int *base)
{
	if (*base == 0) {
		if (s[0] == '0') {
			if (tolower(s[1]) == 'x' && isxdigit(s[2]))
				*base = 16;
			else
				*base = 8;
		} else {
			int i = 0;
			char var;

			*base = 10;

			do {
				var = tolower(s[i++]);
				if (var >= 'a' && var <= 'f') {
					*base = 16;
					break;
				}

				if (!(var >= '0' && var <= '9'))
					break;
			} while (var);
		}
	}

	if (*base == 16 && s[0] == '0' && tolower(s[1]) == 'x')
		s += 2;
	return s;
}

unsigned long simple_strtoul(const char *cp, char **endp,
				unsigned int base)
{
	unsigned long result = 0;
	unsigned long value;

	cp = _parse_integer_fixup_radix(cp, &base);

	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}

	if (endp)
		*endp = (char *)cp;

	return result;
}

int strict_strtoul(const char *cp, unsigned int base, unsigned long *res)
{
	char *tail;
	unsigned long val;
	size_t len;

	*res = 0;
	len = strlen(cp);
	if (len == 0)
		return -1;

	val = simple_strtoul(cp, &tail, base);
	if (tail == cp)
		return -1;

	if ((*tail == '\0') ||
		((len == (size_t)(tail - cp) + 1) && (*tail == '\n'))) {
		*res = val;
		return 0;
	}

	return -1;
}

long simple_strtol(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoul(cp + 1, endp, base);

	return simple_strtoul(cp, endp, base);
}

unsigned long ustrtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = simple_strtoul(cp, endp, base);
	switch (tolower(**endp)) {
	case 'g':
		result *= 1024;
		/* fall through */
	case 'm':
		result *= 1024;
		/* fall through */
	case 'k':
		result *= 1024;
		(*endp)++;
		if (**endp == 'i')
			(*endp)++;
		if (**endp == 'B')
			(*endp)++;
	}
	return result;
}

unsigned long long ustrtoull(const char *cp, char **endp, unsigned int base)
{
	unsigned long long result = simple_strtoull(cp, endp, base);
	switch (tolower(**endp)) {
	case 'g':
		result *= 1024;
		/* fall through */
	case 'm':
		result *= 1024;
		/* fall through */
	case 'k':
		result *= 1024;
		(*endp)++;
		if (**endp == 'i')
			(*endp)++;
		if (**endp == 'B')
			(*endp)++;
	}
	return result;
}

unsigned long long simple_strtoull(const char *cp, char **endp,
					unsigned int base)
{
	unsigned long long result = 0, value;

	cp = _parse_integer_fixup_radix(cp, &base);

	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0'
		: (islower(*cp) ? toupper(*cp) : *cp) - 'A' + 10) < base) {
		result = result * base + value;
		cp++;
	}

	if (endp)
		*endp = (char *) cp;

	return result;
}

long trailing_strtoln(const char *str, const char *end)
{
	const char *p;

	if (!end)
		end = str + strlen(str);
	if (isdigit(end[-1])) {
		for (p = end - 1; p > str; p--) {
			if (!isdigit(*p))
				return simple_strtoul(p + 1, NULL, 10);
		}
	}

	return -1;
}

long trailing_strtol(const char *str)
{
	return trailing_strtoln(str, NULL);
}

void str_to_upper(const char *in, char *out, size_t len)
{
	for (; len > 0 && *in; len--)
		*out++ = toupper(*in++);
	if (len)
		*out = '\0';
}

size_t strnlen(const char * s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

void hex2str(unsigned char *pHex, unsigned char *pAscii, int nLen)
{
    unsigned char Nibble[2];
    int i,j;
	if(pHex && pAscii && (nLen > 0)){
	    for (i = 0; i < nLen; i++){
	        Nibble[0] = (pHex[nLen - 1 - i] & 0xF0) >> 4;
	        Nibble[1] = pHex[nLen - 1 - i] & 0x0F;
	        for (j = 0; j < 2; j++){
	            if (Nibble[j] < 10){            
	                Nibble[j] += 0x30;
	            }
	            else{
	                if (Nibble[j] < 16)
	                    Nibble[j] = Nibble[j] - 10 + 'A';
	            }
	            *pAscii++ = Nibble[j];
	        }
	    }
	}
	*pAscii = 0;
}


void wr_addr32(unsigned long addr, unsigned int data)
{
	*((volatile u32 *)addr) = data;
}

unsigned int rd_addr32(unsigned long addr)
{
	return (*(volatile u32 *)addr);
}

void wr_addr8(unsigned long addr, unsigned char data)
{
	*((volatile u8 *)addr) = data;
}

unsigned char rd_addr8(unsigned long addr)
{
	return (*(volatile u8 *)addr);
}

void wr_addr16(unsigned    long addr ,short data)
{
    *(short *)addr = data;
}

short rd_addr16(unsigned long addr)
{
    return (*(short *)addr);
}

void wr_addr64(unsigned long addr, long long data)
{
	*(long long *)addr = data;
}

long long rd_addr64(unsigned long addr)
{
	return (*(long long *)addr);
}

void pass(void)
{
    *(volatile unsigned int *)0x20000 = 0xaaaaaaaa;
}

void fail(void)
{
    *(volatile unsigned int *)0x20000 = 0xbbbbbbbb;
}

void secondary_boot_func(void)
{

}


