/*

  Calculate or Check MD5 Signature of File or Command Line Argument

			    by John Walker
		       http://www.fourmilab.ch/

		This program is in the public domain.
*/

#include "stdafx.h"

#ifndef MD5_H
#define MD5_H

/*  The following tests optimise behaviour on little-endian
    machines, where there is no need to reverse the byte order
    of 32 bit words in the MD5 computation.  By default,
    HIGHFIRST is defined, which indicates we're running on a
    big-endian (most significant byte first) machine, on which
    the byteReverse function in md5.c must be invoked. However,
    byteReverse is coded in such a way that it is an identity
    function when run on a little-endian machine, so calling it
    on such a platform causes no harm apart from wasting time. 
    If the platform is known to be little-endian, we speed
    things up by undefining HIGHFIRST, which defines
    byteReverse as a null macro.  Doing things in this manner
    insures we work on new platforms regardless of their byte
    order.  */

#define HIGHFIRST

#ifdef __i386__
#undef HIGHFIRST
#endif

/*  On machines where "long" is 64 bits, we need to declare
    uint32 as something guaranteed to be 32 bits.  */

#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif

struct MD5Context {
        uint32 buf[4];
        uint32 bits[2];
        unsigned char in[64];
};

void MD5Init();
void MD5Update();
void MD5Final();
//extern void MD5Transform();
void MD5Transform(uint32 buf[4],uint32 in[16]);
/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct MD5Context MD5_CTX;

/*  Define CHECK_HARDWARE_PROPERTIES to have main,c verify
    byte order and uint32 settings.  */
#define CHECK_HARDWARE_PROPERTIES

#endif /* !MD5_H */



#define VERSION     "2.2 (2008-01-14)"




#ifndef HIGHFIRST
#define byteReverse(buf, len)	/* Nothing */
#else
/*
 * Note: this code is harmless on little-endian machines.
 */
static void byteReverse(    unsigned char *buf,unsigned longs)

{
    uint32 t;
    do {
	t = (uint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
	    ((unsigned) buf[1] << 8 | buf[0]);
	*(uint32 *) buf = t;
	buf += 4;
    } while (--longs);
}
#endif

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void MD5Init(    struct MD5Context *ctx)

{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void MD5Update(    struct MD5Context *ctx, unsigned char *buf, unsigned len)

{
    uint32 t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((uint32) len << 3)) < t)
	ctx->bits[1]++; 	/* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
	unsigned char *p = (unsigned char *) ctx->in + t;

	t = 64 - t;
	if (len < t) {
	    memcpy(p, buf, len);
	    return;
	}
	memcpy(p, buf, t);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32 *) ctx->in);
	buf += t;
	len -= t;
    }
    /* Process data in 64-byte chunks */

    while (len >= 64) {
	memcpy(ctx->in, buf, 64);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32 *) ctx->in);
	buf += 64;
	len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
static void MD5Final(    unsigned char digest[16], struct MD5Context *ctx)

{
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset(p, 0, count);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32 *) ctx->in);

	/* Now fill the next block with 56 bytes */
	memset(ctx->in, 0, 56);
    } else {
	/* Pad block to 56 bytes */
	memset(p, 0, count - 8);
    }
    byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((uint32 *) ctx->in)[14] = ctx->bits[0];
    ((uint32 *) ctx->in)[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (uint32 *) ctx->in);
    byteReverse((unsigned char *) ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset(ctx, 0, sizeof(ctx));        /* In case it's sensitive */
}


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void MD5Transform(uint32 buf[4],uint32 in[16])

{
    register uint32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}













#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif



#define FALSE	0
#define TRUE	1

#define EOS     '\0'

/*  Main program  */

static int md5main(  int argc, char *argv[])

{
    int i, j, opt, cdata = FALSE, docheck = FALSE, showfile = TRUE, f = 0;
    unsigned int bp;
    char *cp, *clabel, *ifname, *hexfmt = "%02X";
    FILE *in = stdin, *out = stdout;
    unsigned char buffer[16384], signature[16], csig[16];
    struct MD5Context md5c;
    
    /*	Build parameter quality control.  Verify machine
    	properties were properly set in md5.h and refuse
	to run if they're not correct.  */
	
#ifdef CHECK_HARDWARE_PROPERTIES
    /*	Verify unit32 is, in fact, a 32 bit data type.  */
    if (sizeof(uint32) != 4) {
    	fprintf(stderr, "** Configuration error.  Setting for uint32 in file md5.h\n");
	fprintf(stderr, "   is incorrect.  This must be a 32 bit data type, but it\n");
	fprintf(stderr, "   is configured as a %d bit data type.\n", sizeof(uint32) * 8);
	return 2;
    }
    
    /*	If HIGHFIRST is not defined, verify that this machine is,
    	in fact, a little-endian architecture.  */
	
#ifndef HIGHFIRST
    {	uint32 t = 0x12345678;
    
    	if (*((char *) &t) != 0x78) {
    	    fprintf(stderr, "** Configuration error.  Setting for HIGHFIRST in file md5.h\n");
	    fprintf(stderr, "   is incorrect.  This symbol has not been defined, yet this\n");
	    fprintf(stderr, "   machine is a big-endian (most significant byte first in\n");
	    fprintf(stderr, "   memory) architecture.  Please modify md5.h so HIGHFIRST is\n");
	    fprintf(stderr, "   defined when building for this machine.\n");
	    return 2;
	}
    }
#endif
#endif
    
    /*	Process command line options.  */

    for (i = 1; i < argc; i++) {
	cp = argv[i];
        if (*cp == '-') {
	    if (strlen(cp) == 1) {
	    	i++;
	    	break;	    	      /* -  --  Mark end of options; balance are files */
	    }
	    opt = *(++cp);
	    if (islower(opt)) {
		opt = toupper(opt);
	    }

	    switch (opt) {

                case 'C':             /* -Csignature  --  Check signature, set return code */
		    docheck = TRUE;
		    if (strlen(cp + 1) != 32) {
			docheck = FALSE;
		    }
		    memset(csig, 0, 16);
		    clabel = cp + 1;
		    for (j = 0; j < 16; j++) {
			if (isxdigit((int) clabel[0]) && isxdigit((int) clabel[1]) &&
                            sscanf((cp + 1 + (j * 2)), hexfmt, &bp) == 1) {
			    csig[j] = (unsigned char) bp;
			} else {
			    docheck = FALSE;
			    break;
			}
			clabel += 2;
		    }
		    if (!docheck) {
                        fprintf(stderr, "Error in signature specification.  Must be 32 hex digits.\n");
			return 2;
		    }
		    break;

                case 'D':             /* -Dtext  --  Compute signature of given text */
		    MD5Init(&md5c);
		    MD5Update(&md5c, (unsigned char *) (cp + 1), strlen(cp + 1));
		    cdata = TRUE;
		    f++;	      /* Mark no infile argument needed */
		    break;
		    
		case 'L':   	      /* -L  --  Use lower case letters as hex digits */
		    hexfmt = "%02x";
		    break;
		    
		case 'N':   	      /* -N  --  Don't show file name after sum */
		    showfile = FALSE;
		    break;
		    
		case 'O':   	      /* -Ofname  --  Write output to fname (- = stdout) */
		    cp++;
                    if (strcmp(cp, "-") != 0) {
		    	if (out != stdout) {
			    fprintf(stderr, "Redundant output file specification.\n");
			    return 2;
    	    	    	}
                        if ((out = fopen(cp, "w")) == NULL) {
                            fprintf(stderr, "Cannot open output file %s\n", cp);
			    return 2;
			}
		    }
		    break;

                case '?':             /* -U, -? -H  --  Print how to call information. */
                case 'H':
                case 'U':
    printf("\nMD5  --  Calculate MD5 signature of file.  Call");
    printf("\n         with md5 [ options ] [file ...]");
    printf("\n");
    printf("\n         Options:");
    printf("\n              -csig   Check against sig, set exit status 0 = OK");
    printf("\n              -dtext  Compute signature of text argument");
    printf("\n              -l      Use lower case letters for hexadecimal digits");
    printf("\n              -n      Do not show file name after sum");
    printf("\n              -ofname Write output to fname (- = stdout)");
    printf("\n              -u      Print this message");
    printf("\n              -v      Print version information");
    printf("\n");
    printf("\nby John Walker  --  http://www.fourmilab.ch/");
    printf("\nVersion %s\n", VERSION);
    printf("\nThis program is in the public domain.\n");
    printf("\n");
#ifdef CHECK_HARDWARE_PROPERTIES
#ifdef HIGHFIRST
    {	uint32 t = 0x12345678;
    
    	if (*((char *) &t) == 0x78) {
    	fprintf(stderr, "** Note.  md5 is not optimally configured for use on this\n");
	    fprintf(stderr, "   machine.  This is a little-endian (least significant byte\n");
	    fprintf(stderr, "   first in memory) architecture, yet md5 has been built with the\n");
	    fprintf(stderr, "   symbol HIGHFIRST defined in md5.h, which includes code which\n");
	    fprintf(stderr, "   supports both big- and little-endian machines.  Modifying\n");
	    fprintf(stderr, "   md5.h to undefine HIGHFIRST for this platform will make md5\n");
	    fprintf(stderr, "   run faster on it.\n");
	}
    }
#endif
#endif
		    return 0;
		    
		case 'V':   	      /* -V  --  Print version number */
		    printf("%s\n", VERSION);
		    return 0;
	    }
	} else {
	    break;
	}
    }
    
    if (cdata && (i < argc)) {
    	fprintf(stderr, "Cannot specify both -d option and input file.\n");
	return 2;
    }
    
    if ((i >= argc) && (f == 0)) {
    	f++;
    }
    
    for (; (f > 0) || (i < argc); i++) {
    	if ((!cdata) && (f > 0)) {
	    ifname = "-";
	} else {
    	    ifname = argv[i];
	}
	f = 0;

	if (!cdata) {
	    int opened = FALSE;
	    
	    /* If the data weren't supplied on the command line with
	       the "-d" option, read it now from the input file. */
	
	    if (strcmp(ifname, "-") != 0) {
		if ((in = fopen(ifname, "rb")) == NULL) {
	    	    fprintf(stderr, "Cannot open input file %s\n", ifname);
		    return 2;
		}
		opened = TRUE;
	    } else {
		in = stdin;
	    }
#ifdef _WIN32

	    /** Warning!  On systems which distinguish text mode and
		binary I/O (MS-DOS, Macintosh, etc.) the modes in the open
        	statement for "in" should have forced the input file into
        	binary mode.  But what if we're reading from standard
		input?  Well, then we need to do a system-specific tweak
        	to make sure it's in binary mode.  While we're at it,
        	let's set the mode to binary regardless of however fopen
		set it.

		The following code, conditional on _WIN32, sets binary
		mode using the method prescribed by Microsoft Visual C 7.0
        	("Monkey C"); this may require modification if you're
		using a different compiler or release of Monkey C.	If
        	you're porting this code to a different system which
        	distinguishes text and binary files, you'll need to add
		the equivalent call for that system. */

	    _setmode(_fileno(in), _O_BINARY);
#endif
    
    	    MD5Init(&md5c);
	    while ((j = (int) fread(buffer, 1, sizeof buffer, in)) > 0) {
		MD5Update(&md5c, buffer, (unsigned) j);
	    }
	    
	    if (opened) {
	    	fclose(in);
	    }
	}
	MD5Final(signature, &md5c);

	if (docheck) {
	    docheck = 0;
	    for (j = 0; j < sizeof signature; j++) {
		if (signature[j] != csig[j]) {
		    docheck = 1;
		    break;
		}
	    }
	    if (i < (argc - 1)) {
	    	fprintf(stderr, "Only one file may be tested with the -c option.\n");
		return 2;
	    }
	} else {




	    for (j = 0; j < sizeof signature; j++) {
        	fprintf(out, hexfmt, signature[j]);
	    }
	    if ((!cdata) && showfile) {
		fprintf(out, "  %s", (in == stdin) ? "-" : ifname);
	    }
            fprintf(out, "\n");
	}
    }
	CStringA temp;
//	temp.Format("%S",(char *)out->_base);
	char strtemp[33];
	strncpy(strtemp,out->_base,32);
	strtemp[32]=0;
	::MessageBoxA(NULL,strtemp,"°¥",0);
    return docheck;
	
}
