
#include <errno.h>
#include <ctype.h>

#define __need_size_t
#include <stddef.h>
#include <string.h>

#include "iconv.h"


static int decode_ascii(int *scalar, const unsigned char *inbuf, int count);
static int decode_iso_8859_1(int *scalar, const unsigned char *inbuf,
	int count);
static int decode_utf_8(int *scalar, const unsigned char *inbuf, int count);

static int encode_ascii(int scalar, unsigned char *outbuf);
static int encode_iso_8859_1(int scalar, unsigned char *outbuf);
static int encode_utf_8(int scalar, unsigned char *outbuf);

enum {
	ASCII = 0,
	UTF_8,
	ISO_8859_1
};

struct converter {
	int code;
	const char *name;
	int (*decode)(int *scalar, const unsigned char *inbuf, int count);
	int (*encode)(int scalar, unsigned char *outbuf);
};

struct converter supported_codesets[] = {
	{
		.code = ASCII,
		.name = "ASCII",
		.decode = &decode_ascii,
		.encode = &encode_ascii,
	}, {
		.code = UTF_8,
		.name = "UTF-8" "\0" "UTF8" "\0",
		.decode = &decode_utf_8,
		.encode = &encode_utf_8,
	}, {
		.code = ISO_8859_1,
		.name = "ISO-8859-1" "\0" "ISO8859-1" "\0",
		.decode = &decode_iso_8859_1,
		.encode = &encode_iso_8859_1
	}, {
		.code = -1,
	}
};

/* ASCII */
static int decode_ascii(int *scalar, const unsigned char *inbuf,
	int count)
{
	if (*inbuf >= 0x80) {
		errno = EILSEQ;
		return -1;
	}
	*scalar = *inbuf;
	return 1;
}

static int encode_ascii(int scalar, unsigned char *outbuf)
{
	if (scalar >= 0x80)
		return 0;
	*outbuf = scalar;
	return 1;
}

/* ISO-8859-1 */
static int decode_iso_8859_1(int *scalar, const unsigned char *inbuf,
	int count)
{
	*scalar = *inbuf;
	return 1;
}

static int encode_iso_8859_1(int scalar, unsigned char *outbuf)
{
	if (scalar > 0xff)
		return 0;
	*outbuf = scalar;
	return 1;
}


/* UTF-8 */
/* scalar value			utf-8
 * 00000000 00000000 0zzzzzzz	0zzzzzzz
 * 00000000 00000yyy yyzzzzzz 	110yyyyy 10zzzzzz
 * 00000000 xxxxyyyy yyzzzzzz 	1110xxxx 10yyyyyy 10zzzzzz
 * 000wwwxx xxxxyyyy yyzzzzzz 	11110www 10xxxxxx 10yyyyyy 10zzzzzz
 *
 * This implementation only supports the 2 first variants
 */
static int decode_utf_8(int *scalar, const unsigned char *inbuf, int count)
{
	int i = inbuf[0];

	if (i < 0x80) {
		*scalar = i;
		return 1;
	}

	if (count < 2) {
		errno = EINVAL;
		return -1;
	}

	i = (i << 8) | inbuf[1];
	if ((i & 0xe0c0) != 0xc080) {
		errno = EILSEQ;
		return -1;
	}

	*scalar = ((i & 0x1700) >> 2) | (i & 0x3f);
	return 2;
}

static int encode_utf_8(int scalar, unsigned char *outbuf)
{
	if (scalar < 0x80) {
		*outbuf = scalar;
		return 1;
	}

	if (scalar > 0x7ff)
		return 0;

	outbuf[0] = (scalar >> 6) | 0xc0;
	outbuf[1] = (scalar & 0x3f) | 0x80;
	return 2;
}

static void toupperstr(char *p)
{
	while (p && *p) {
		if (*p == '_')
		       *p = '-';
		*p = toupper(*p);
		p++;
	}
}

static int is_codeset(const char *str, const char *list)
{
	while (*list) {
		if (strcmp(str, list) == 0)
			return 1;
		list += strlen(list) + 1;
	}
	return 0;
}

static int find_converter(const char *str)
{
	int i;
	char buf[16];
	strncpy(buf, str, sizeof(buf));
	buf[15] = '\0';
	toupperstr(buf);
	for (i = 0; supported_codesets[i].code != -1; i++) {
		if (is_codeset(buf, supported_codesets[i].name))
			return i;
	}
	return -1;
}

iconv_t iconv_open(const char *tocode, const char *fromcode)
{
	int src, dest;
	iconv_t cd = (iconv_t) -1;

	if (tocode == NULL || fromcode == NULL) {
		errno = EINVAL;
		goto return_error;
	}

	src = find_converter(fromcode);
	dest = find_converter(tocode);
	if (src == -1 || dest == -1) {
		errno = EINVAL;
		goto return_error;
	}

	cd = (iconv_t) (dest << 8) | src;

return_error:
	return cd;
}

int iconv_close(iconv_t cd)
{
	return 0;
}

size_t iconv(iconv_t cd, const char **inbuf, size_t *insize,
		char **outbuf, size_t *outsize)
{
	struct converter *in = &supported_codesets[cd & 0xff];
	struct converter *out = &supported_codesets[cd >> 8];
	size_t nonidentical = 0;
	if (!inbuf || !*inbuf || !outbuf || !*outbuf)
		return 0;
	while (*insize) {
		int scalar, infwd, outfwd;
		infwd = in->decode(&scalar, (unsigned char *)*inbuf, *insize);
		if (infwd < 0)
			goto ret_error;

		outfwd = out->encode(scalar, (unsigned char *)*outbuf);
		if (outfwd > *outsize) {
			errno = E2BIG;
			goto ret_error;
		}

		if (**inbuf != scalar || outfwd == 0)
			nonidentical++;

		*inbuf += infwd;
		*insize -= infwd;
		*outbuf += outfwd;
		*outsize -= outfwd;
	}
	if (*outsize)
		*outbuf = '\0';
	return nonidentical;

ret_error:
	return -1;
}
