#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>

#include "iconv.h"

void print_hex(char *str)
{
	printf("%.2x", (unsigned char) *str);
	str++;
	while (*str) {
		printf(" %.2x", (unsigned char) *str);
		str++;
	}
}

void convert_string(iconv_t cd, char *str)
{
	char outbuf[256];
	char *outptr = outbuf;
	const char *inptr = str;
	size_t outsize = sizeof(outbuf);
	size_t insize = strlen(str);
	int ret, err = 0;
	memset(outbuf, 0, sizeof(outbuf));
	print_hex(str);
	printf(" -> ");
	ret = iconv(cd, &inptr, &insize, &outptr, &outsize);
	if (ret < 0)
		err= errno;
	print_hex(outbuf);
	if (err)
		printf(" (%s)", strerror(err));
	printf("\n");
}

void convert_args(char *from, char *to, int argc, char **argv)
{
	iconv_t cd;
	int i;

	printf("\n>>> %s: %s -> %s\n", basename(argv[0]), from, to);
	cd = iconv_open(to, from);
	if (cd < 0) {
		printf("iconv_open(\"%s\", \"%s\"): %s\n", to, from, strerror(errno));
		return;
	}

	for (i = 1; i < argc; i++)
		convert_string(cd, argv[i]);

	iconv_close(cd);
}

int main(int argc, char **argv)
{
	char *codesets[] = { "ASCII", "ISO-8859-1", "iso8859-1", "UTF-8", "utf8", "invalid", NULL };
	char **from, **to;

	for (from = codesets; *from; from++) 
		for (to = codesets; *to; to++)
			convert_args(*from, *to, argc, argv);

	return 0;
}
