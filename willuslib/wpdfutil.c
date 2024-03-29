/*
** wpdfutil.c  Misc PDF support routines that are mostly divorced from other
**             large code modules or libraries.
**
** Part of willus.com general purpose C code library.
**
** Copyright (C) 2023  http://willus.com
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Affero General Public License as
** published by the Free Software Foundation, either version 3 of the
** License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
*/
#include <stdio.h>
#include <math.h>
#include "willus.h"

static int legal_pdf_encoded_byte(int c);


void pdf_utf8_out(FILE *out,char *utf8_text)


    {
    STRBUF _s,*s;

    s=&_s;
    strbuf_init(s);
    strbuf_cat_pdf_utf8(s,utf8_text);
    strbuf_to_file(s,out);
    strbuf_free(s);
    }


void strbuf_cat_pdf_utf8(STRBUF *s,char *utf8)

    {
    static char *funcname="strbuf_cat_pdf_utf8";
    int i,len,docenc;
    int *d;
    unsigned char *de;

    willus_mem_alloc_warn((void **)&de,strlen(utf8)+2,funcname,10);
    docenc=wpdf_docenc_from_utf8((char *)de,utf8,strlen(utf8)+1);
    if (docenc)
        {
        char buf[32];
        strbuf_cat_ex2(s,"(",0);
        for (i=0;de[i]!='\0';i++)
            {
            if (de[i]>=32 && de[i]<=127)
                {
                buf[0]=de[i];
                buf[1]='\0';
                }
            else
                sprintf(buf,"\\%03o",de[i]);
            strbuf_cat_ex2(s,buf,0);
            }
        strbuf_cat_ex2(s,")",0);
        }
    willus_mem_free((double **)&de,funcname);
    if (docenc)
        return;
    /* If UTF-8 text can't be PDF document-encoded, convert to Unicode-16 */
    len=strlen(utf8)+2;
    willus_mem_alloc_warn((void **)&d,sizeof(int)*len,funcname,10);
    len=utf8_to_unicode(d,utf8,len-1);
    strbuf_cat_ex2(s,"<FEFF",0);
    for (i=0;i<len;i++)
        {
        char buf[32];
        sprintf(buf,"%04X",d[i]);
        strbuf_cat_ex2(s,buf,0);
        }
    strbuf_cat_ex2(s,">",0);
    willus_mem_free((double **)&d,funcname);
    }


int wpdf_docenc_from_utf8(char *dst,char *src_utf8,int maxlen)

    {
    static char *funcname="wpdf_docenc_from_utf8";
	int i,j,n,legal;
    int *utf16;

    legal=1;
    n=strlen(src_utf8)+2;
    willus_mem_alloc_warn((void **)&utf16,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(utf16,src_utf8,n-1);
	for (i=j=0;i<n;i++)
        {
        int c;
        if ((c=legal_pdf_encoded_byte(utf16[i]))!=0)
            {
            if (dst!=NULL && j<maxlen)
                dst[j++] = c;
            }
        else
            legal=0;
        }
    if (dst!=NULL)
        dst[j]='\0';
    willus_mem_free((double **)&utf16,funcname);
    return(legal);
    }


static int legal_pdf_encoded_byte(int c)

    {
    int k;
    const unsigned short wpdfdoc_encoding_table[256] =
        {
        /* 0x0 to 0x17 except \t, \n and \r are really undefined */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x02d8, 0x02c7, 0x02c6, 0x02d9, 0x02dd, 0x02db, 0x02da, 0x02dc,
        0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
        0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
        0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
        0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
        0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
        0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
        0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
        0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
        0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
        0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
        0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
        0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x0000,
        0x2022, 0x2020, 0x2021, 0x2026, 0x2014, 0x2013, 0x0192, 0x2044,
        0x2039, 0x203a, 0x2212, 0x2030, 0x201e, 0x201c, 0x201d, 0x2018,
        0x2019, 0x201a, 0x2122, 0xfb01, 0xfb02, 0x0141, 0x0152, 0x0160,
        0x0178, 0x017d, 0x0131, 0x0142, 0x0153, 0x0161, 0x017e, 0x0000,
        0x20ac, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
        0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x0000, 0x00ae, 0x00af,
        0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
        0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
        0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
        0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
        0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
        0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
        0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
        0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
        0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
        0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
        };

    for (k=1;k<256;k++)
        if (c==wpdfdoc_encoding_table[k])
            return(k);
    return(0);
    }
