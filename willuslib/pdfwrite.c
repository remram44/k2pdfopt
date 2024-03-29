/*
** pdfwrite.c   Routines to help write a PDF file.
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "willus.h"

#ifdef HAVE_Z_LIB
#include <zlib.h>
#endif

#define MAXPDFPAGES 10000

#define ABOVEBASEMEAN 0.6
#define WIDTHMEAN     0.45
#define NEXTCHARMEAN  0.12

extern WILLUSCHARINFO pdffonts_helvetica[];

static int lastfont=-1;
static double lastfontsize=-1;

static void pdffile_start(PDFFILE *pdf,int pages_at_end);
static int pdffile_page_reference(PDFFILE *pdf,int pageno);
static void pdffile_unicode_map(PDFFILE *pdf,WILLUSCHARMAPLIST *cmaplist,int nf);
static void thumbnail_create(WILLUSBITMAP *thumb,WILLUSBITMAP *bmp);
static void pdffile_bmp_stream(PDFFILE *pdf,WILLUSBITMAP *bmp,int quality,int halfsize,int thumb);
static void bmp_flate_decode(WILLUSBITMAP *bmp,FILE *f,compress_handle handle,int halfsize);
static void pdffile_new_object(PDFFILE *pdf,int flags);
static void pdffile_add_object(PDFFILE *pdf,PDFOBJECT *object);
#ifdef HAVE_Z_LIB
static int pdf_numpages_1(void *ptr,int bufsize);
static int decodecheck(FILE *f,int np);
static int wpdf_getline(char *buf,int maxlen,FILE *f);
static int wpdf_getbufline(char *buf,int maxlen,char *opbuf,int *i0,int bufsize);
#endif
static void insert_length(FILE *f,long pos,int len);
static void ocrwords_to_pdf_stream(OCRWORDS *ocrwords,FILE *f,double dpi,
                                   double page_height_pts,int text_render_mode,
                                   WILLUSCHARMAPLIST *cmaplist,int use_spaces,int ocr_flags);
static double ocrwords_median_size(OCRWORDS *ocrwords,double dpi,WILLUSCHARMAPLIST *cmaplist);
static void ocrword_width_and_maxheight(OCRWORD *word,double *width,double *maxheight,
                                        WILLUSCHARMAPLIST *cmaplist,double *charpos);
static double size_round_off(double size,double median_size,double log_size_increment);
static void ocrwords_optimize_spaces(OCRWORD *sentence,OCRWORD *word,int n,double dpi,
                                     WILLUSCHARMAPLIST *cmaplist,int optimize);
static void ocrwords_sentence_construct(OCRWORD *sentence,OCRWORD *word,int n,int *nspaces);
static void sentence_check_alignment(OCRWORD *word,int n,int *nspaces,double *pos,
                                     WILLUSCHARMAPLIST *cmaplist);
static void ocrword_to_pdf_stream(OCRWORD *word,FILE *f,double dpi,
                                  double page_height_pts,double median_size_pts,
                                  WILLUSCHARMAPLIST *cmaplist,int ocr_flags);
static void willuscharmaplist_init(WILLUSCHARMAPLIST *list);
static void willuscharmaplist_free(WILLUSCHARMAPLIST *list);
static void willuscharmaplist_add_charmap(WILLUSCHARMAPLIST *list,int unichar);
static int  willuscharmaplist_maxcid(WILLUSCHARMAPLIST *list);
static int  willuscharmaplist_cid_index(WILLUSCHARMAPLIST *list,int unichar);
static void willuscharmaplist_populate(WILLUSCHARMAPLIST *cmaplist,OCRWORDS *ocrwords);
static void willuscharmaplist_populate_string(WILLUSCHARMAPLIST *cmaplist,char *s);


FILE *pdffile_init(PDFFILE *pdf,char *filename,int pages_at_end)

    {
    pdf->n=pdf->na=0;
    pdf->object=NULL;
    pdf->pae=0;
    pdf->imc=0;
    strncpy(pdf->filename,filename,511);
    pdf->filename[511]='\0';
    pdf->f = wfile_fopen_utf8(filename,"wb");
    if (pdf->f!=NULL)
        fclose(pdf->f);
    pdf->f = wfile_fopen_utf8(filename,"rb+");
    if (pdf->f!=NULL)
        pdffile_start(pdf,pages_at_end);
    return(pdf->f);
    }

void pdffile_close(PDFFILE *pdf)

    {
    if (pdf->f!=NULL)
        {
        fclose(pdf->f);
        pdf->f=NULL;
        }
    willus_mem_free((double **)&pdf->object,"pdffile_close");
    pdf->n=pdf->na=pdf->imc=0;
    }


static void pdffile_start(PDFFILE *pdf,int pages_at_end)

    {
    fprintf(pdf->f,"%%PDF-1.3 \n");
    pdffile_new_object(pdf,2);
    fprintf(pdf->f,"<<\n"
                   "/Pages ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[1]=ftell(pdf->f);
    if (pages_at_end)
        fprintf(pdf->f,"      ");
    else
        fprintf(pdf->f,"2");
    fprintf(pdf->f," 0 R\n"
                   "/Outlines ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[2]=ftell(pdf->f);
    fprintf(pdf->f,"       0 R\n"
                   "/Type /Catalog\n"
                   ">>\n"
                   "endobj\n");
    if (!pages_at_end)
        {
        int i;
        char cline[73];
        pdffile_new_object(pdf,4);
        fprintf(pdf->f,"<<\n"
                       "/Type /Pages\n"
                       "/Kids [");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        pdf->pae=ftell(pdf->f);
        cline[0]='%';
        cline[1]='%';
        for (i=2;i<71;i++)
            cline[i]=' ';
        cline[71]='\n';
        cline[72]='\0';
        for (i=0;i<120;i++)
            fprintf(pdf->f,"%s",cline);
        }
    else
        pdf->pae=0;
    }


int pdffile_page_count(PDFFILE *pdf)

    {
    int i,np;

    for (np=i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&1)
            np++;
    return(np);
    }


static int pdffile_page_reference(PDFFILE *pdf,int pageno)

    {
    int i,np;

    for (np=i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&1)
            {
            np++;
            if (np==pageno)
                return(i+1);
            }
    return(-1);
    }


/*
** Must be called after all pages are added
*/
void pdffile_add_outline(PDFFILE *pdf,WPDFOUTLINE *outline)

    {
    int i,n0,n,nl,np,rcount;

    if (outline==NULL)
        return;
    np=pdffile_page_count(pdf);
    wpdfoutline_fill_in_blank_dstpages(outline,np);
    n=wpdfoutline_num_anchors_recursive(outline);
    if (n==0)
         return;
    nl=wpdfoutline_num_anchors_on_level(outline,&rcount);
    /* Outline head */
    pdffile_new_object(pdf,4);
    fprintf(pdf->f,"<<\n"
                   "  /Count %d\n"
                   "  /First %d 0 R\n"
                   "  /Last %d 0 R\n"
                   "  /Type /Outlines\n"
                   ">>\n"
                   "endobj\n\n",
                   nl,pdf->n+1,pdf->n+1+rcount*2);
    n0=pdf->n+1;
    for (i=0;i<n;i++)
        {
        WPDFOUTLINE *local,*next;

        pdffile_new_object(pdf,8);
        local=wpdfoutline_by_index(outline,i);
        fprintf(pdf->f,"<<\n"
                       "  /A %d 0 R\n",pdf->n+1);
        if (local->down!=NULL)
            {
            int nl2,rc2;

            nl2=wpdfoutline_num_anchors_on_level(local->down,&rc2);
            fprintf(pdf->f,"  /Count %d\n"
                           "  /First %d 0 R\n"
                           "  /Last %d 0 R\n",
                           nl2,
                           pdf->n+2,
                           pdf->n+2+rc2*2);
            }
        if (local->next!=NULL)
            {
            int rc2;
            rc2=wpdfoutline_num_anchors_recursive(local->down);
            fprintf(pdf->f,"  /Next %d 0 R\n",pdf->n+2+rc2*2);
            }
        next=wpdfoutline_previous(outline,local);
        if (next!=NULL)
            fprintf(pdf->f,"  /Prev %d 0 R\n",n0+wpdfoutline_index(outline,next)*2);
        next=wpdfoutline_parent(outline,local);
        if (next!=NULL)
            fprintf(pdf->f,"  /Parent %d 0 R\n",n0+wpdfoutline_index(outline,next)*2);
        else
            fprintf(pdf->f,"  /Parent %d 0 R\n",n0-1);
        fprintf(pdf->f,"  /Title ");
        pdf_utf8_out(pdf->f,local->title);
        fprintf(pdf->f,"\n"
                       ">>\n"
                       "endobj\n\n");
        pdffile_new_object(pdf,16);
        fprintf(pdf->f,"<<\n"
                       "  /D [ %d 0 R /Fit ]\n"
                       "  /S /GoTo\n"
                       ">>\n"
                       "endobj\n\n",pdffile_page_reference(pdf,local->dstpage+1));
        }
    }


void pdffile_add_bitmap(PDFFILE *pdf,WILLUSBITMAP *bmp,double dpi,int quality,int halfsize)

    {
    pdffile_add_bitmap_with_ocrwords(pdf,bmp,dpi,quality,halfsize,NULL,1);
    }


/*
** Use quality=-1 for PNG
**
** If quality < 0, the deflate (PNG-style) method is used.
**
** halfsize==0 for 8-bits per color plane
**         ==1 for 4-bits per color plane
**         ==2 for 2-bits per color plane
**         ==3 for 1-bit  per color plane
**
** ocr_render_flags
**     Bit 1 (1):  1=Show source bitmap
**     Bit 2 (2):  1=Show OCR text
**     Bit 3 (4):  1=Box around text
**     Bit 4 (8):  1=Use spaces, 1 space per word
**     Bit 5 (16): 1=Use spaces, optimize number of spaces per word
**     Bit 6 (32): 1=Do not round off font sizes
**
*/
void pdffile_add_bitmap_with_ocrwords(PDFFILE *pdf,WILLUSBITMAP *bmp,double dpi,
                                      int quality,int halfsize,OCRWORDS *ocrwords,
                                      int ocr_render_flags)

    {
    double pw,ph;
    int ptr1,ptr2,ptrlen,showbitmap,nf;
    WILLUSCHARMAPLIST *cmaplist,_cmaplist;

/*
{
int i;
printf("ADDING BITMAP.\n");
printf("    Words=%d\n",ocrwords->n);
printf("    dpi=%g\n",dpi);
printf("    bmp=%dx%d\n",bmp->width,bmp->height);
for (i=0;i<ocrwords->n;i++)
printf("    OCR %s: (c=%d,r=%d) (%dx%d)\n",
ocrwords->word[i].text,
ocrwords->word[i].c,
ocrwords->word[i].r,
ocrwords->word[i].w,
ocrwords->word[i].h);
}
*/
    lastfont=-1;
    lastfontsize=-1;
    /* Fix: 24 Nov 2016 */
    showbitmap = (ocr_render_flags&5);
    /* If only showing boxes, clear the bitmap */
    if (showbitmap && !(ocr_render_flags&1))
        bmp_fill(bmp,255,255,255);

    pw=bmp->width*72./dpi;
    ph=bmp->height*72./dpi;

    /* New page object */
    pdffile_new_object(pdf,3);
    pdf->imc++;
    fprintf(pdf->f,"<<\n"
                   "/Type /Page\n"
                   "/Parent ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[1]=ftell(pdf->f);
    fprintf(pdf->f,"%s 0 R\n"
                   "/Resources\n    <<\n",
                   pdf->pae>0 ? "2" : "      ");
    if (ocrwords!=NULL)
        {
        int maxid,ifont;
        cmaplist=&_cmaplist;
        willuscharmaplist_init(cmaplist);
        /*
        ** Map all unicode chars to fonts
        */
        willuscharmaplist_populate(cmaplist,ocrwords);
        maxid=willuscharmaplist_maxcid(cmaplist);
        nf=(maxid>>8)&0xfff;
// nf++;
        /*
        ** Declare the fonts (all Helvetica, but w/different unicode mappings)
        */
        fprintf(pdf->f,"    /Font << /F1 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>");
        for (ifont=1;ifont<=nf;ifont++)
            fprintf(pdf->f,"\n             /F%d << /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding /ToUnicode %d 0 R >>",ifont+1,pdf->n+ifont);
        fprintf(pdf->f," >>\n");
        }
    else
        nf=0;
    if (showbitmap)
        fprintf(pdf->f,"    /XObject << /Im%d %d 0 R >>\n"
                   "    /ProcSet [ /PDF /Text /ImageC ]\n",
                   pdf->imc,pdf->n+nf+2);
    fprintf(pdf->f,"    >>\n"
                   "/MediaBox [0 0 %.1f %.1f]\n"
                   "/CropBox [0 0 %.1f %.1f]\n"
                   "/Contents %d 0 R\n",
                   pw,ph,pw,ph,
                   pdf->n+nf+1); /* Contents stream */
    if (showbitmap)
        fprintf(pdf->f,"/Thumb %d 0 R\n",pdf->n+nf+3);
    fprintf(pdf->f,">>\n"
                   "endobj\n");

    /*
    ** Write the unicode mappings for each font to the PDF file
    */
    if (ocrwords!=NULL)
        {
        int i;
        for (i=0;i<nf;i++)
            pdffile_unicode_map(pdf,cmaplist,i+1);
        }

    /* Execution stream:  draw bitmap and OCR words */
    pdffile_new_object(pdf,0);
    fprintf(pdf->f,"<< /Length ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptrlen=ftell(pdf->f);
    fprintf(pdf->f,"         >>\n"
                   "stream\n");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr1=ftell(pdf->f);
    if (showbitmap)
        fprintf(pdf->f,"q\n%.1f 0 0 %.1f 0 0 cm\n/Im%d Do\nQ\n",pw,ph,pdf->imc);
    if (ocrwords!=NULL)
        {
        int use_spaces;

        if (ocr_render_flags&16)
            use_spaces=2;
        else if (ocr_render_flags&8)
            use_spaces=1;
        else
            use_spaces=0;
        ocrwords_to_pdf_stream(ocrwords,pdf->f,dpi,ph,(ocr_render_flags&2)?0:3,cmaplist,use_spaces,
                               ocr_render_flags);
        /* 2-1-14: Fix memory leak */
        willuscharmaplist_free(cmaplist);
        }
    if (ocr_render_flags&4)
        ocrwords_box(ocrwords,bmp);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr2=ftell(pdf->f);
    fprintf(pdf->f,"endstream\n"
                   "endobj\n");
    insert_length(pdf->f,ptrlen,ptr2-ptr1);
    if (showbitmap)
        {
        /* Stream the bitmap */
        pdffile_bmp_stream(pdf,bmp,quality,halfsize,0);
        /* Stream the thumbnail */
        pdffile_bmp_stream(pdf,bmp,quality,halfsize,1);
        }
    }

/*
** Example fonts string:
**     /Font << /F1 << /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>
**              /F2 << /Type /Font /Sybtype /Type1 /BaseFont /Helvetica-Bold >> >>
**
*/
void pdffile_add_page_with_stream(PDFFILE *pdf,char *fonts,char *streamtext)

    {
    int ptr1,ptr2,ptrlen;

    lastfont=-1;
    lastfontsize=-1;

    /* New page object */
    pdffile_new_object(pdf,3);
    fprintf(pdf->f,"<<\n"
                   "/Type /Page\n"
                   "/MediaBox [0 0 612 792]\n"
                   "/Rotate 0\n"
                   "/Parent ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    pdf->object[pdf->n-1].ptr[1]=ftell(pdf->f);
    fprintf(pdf->f,"%s 0 R\n"
                   "/Resources\n    <<\n",
                   pdf->pae>0 ? "2" : "      ");
    fprintf(pdf->f,"    /Procset [ /PDF /Text ]\n");
    if (fonts!=NULL && fonts[0]!='\0')
        fprintf(pdf->f,"    %s\n",fonts);
    fprintf(pdf->f," >>\n");
    fprintf(pdf->f,"/Contents %d 0 R\n",
                   pdf->n+1); /* Contents stream */
    fprintf(pdf->f,">>\n"
                   "endobj\n");

    /* Execution stream:  Write the text stream */
    pdffile_new_object(pdf,0);
    fprintf(pdf->f,"<< /Length ");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptrlen=ftell(pdf->f);
    fprintf(pdf->f,"         >>\n"
                   "stream\n");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr1=ftell(pdf->f);
    fprintf(pdf->f,"%s\n",streamtext);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr2=ftell(pdf->f);
    fprintf(pdf->f,"endstream\n"
                   "endobj\n");
    insert_length(pdf->f,ptrlen,ptr2-ptr1);
    }


static void pdffile_unicode_map(PDFFILE *pdf,WILLUSCHARMAPLIST *cmaplist,int nf)

    {
    int i,c;
    int *uni;
    static char *funcname="pdffile_unicode_map";

// if(nf>1)nf--;
    willus_mem_alloc_warn((void **)&uni,sizeof(int)*256,funcname,10);
    for (i=0;i<256;i++)
        uni[i]=-1;
    pdffile_new_object(pdf,0);
    for (i=c=0;i<cmaplist->n;i++)
        {
        if (((cmaplist->cmap[i].cid>>8)&0xfff)==nf)
            {
            uni[cmaplist->cmap[i].cid&0xff]=cmaplist->cmap[i].unicode;
            c++;
            }
        }
    if (c>0)
        {
        int ptr1,ptr2,ptrlen;

        fprintf(pdf->f,"<< /Length ");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptrlen=ftell(pdf->f);
        fprintf(pdf->f,"         >>\n"
                       "stream\n");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptr1=ftell(pdf->f);
        fprintf(pdf->f,"/CIDInit /ProcSet findresource begin\n"
                       "12 dict begin\n"
                       "begincmap\n"
                       "/CIDSystemInfo\n"
                       "<< /Registry (UC%03d)\n"
                       "/Ordering (T42UV)\n"
                       "/Supplement 0\n"
                       ">> def\n"
                       "/CMapName /UC%03d def\n"
                       "/CMapType 2 def\n"
                       "1 begincodespacerange\n"
                       "<00> <FF>\n"
                       "endcodespacerange\n"
                       "%d beginbfchar\n",nf,nf,c);
        for (i=0;i<256;i++)
            if (uni[i]>=0)
                fprintf(pdf->f,"<%02x> <%04x>\n",i,uni[i]);
        fprintf(pdf->f,"endbfchar\n"
                       "endcmap\n"
                       "CMapName currentdict /CMap defineresource pop\n"
                       "end\n"
                       "end\n"
                       "endstream\n");
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptr2=ftell(pdf->f);
        fprintf(pdf->f,"endstream\n"
                       "endobj\n");
        insert_length(pdf->f,ptrlen,ptr2-ptr1);
        }
    else
        fprintf(pdf->f,"endobj\n");
    willus_mem_free((double **)&uni,funcname);
    }


static void thumbnail_create(WILLUSBITMAP *thumb,WILLUSBITMAP *bmp)

    {
    if (bmp->width > bmp->height)
        {
        thumb->width = bmp->width<106 ? bmp->width : 106;
        thumb->height = (int)(((double)bmp->height/bmp->width)*thumb->width+.5);
        if (thumb->height<1)
            thumb->height=1;
        }
    else
        {
        thumb->height = bmp->height<106 ? bmp->height : 106;
        thumb->width = (int)(((double)bmp->width/bmp->height)*thumb->height+.5);
        if (thumb->width<1)
            thumb->width=1;
        }
    bmp_resample(thumb,bmp,0.,0.,(double)bmp->width,(double)bmp->height,
                 thumb->width,thumb->height);
    if (bmp->bpp==8)
        bmp_convert_to_greyscale(thumb);
    }


static void pdffile_bmp_stream(PDFFILE *pdf,WILLUSBITMAP *src,int quality,int halfsize,int thumb)

    {
    int ptrlen,ptr1,ptr2,bpc;
    WILLUSBITMAP *bmp,_bmp;

    if (thumb)
        {
        bmp=&_bmp;
        bmp_init(bmp);
        thumbnail_create(bmp,src);
        }
    else
        bmp=src;
    if (quality<0 && halfsize>0 && halfsize<4)
        bpc=8>>halfsize;
    else
        bpc=8;
    /* The bitmap */
    pdffile_new_object(pdf,0);
    fprintf(pdf->f,"<<\n");
    if (!thumb)
        fprintf(pdf->f,"/Type /XObject\n"
                       "/Subtype /Image\n");
#ifdef HAVE_JPEG_LIB
    if (quality>0)
        fprintf(pdf->f,"/Filter %s/DCTDecode%s\n",thumb?"[ ":"",thumb?" ]":"");
#endif
#if (defined(HAVE_JPEG_LIB) && defined(HAVE_Z_LIB))
    else
#endif
#ifdef HAVE_Z_LIB
        fprintf(pdf->f,"/Filter %s/FlateDecode%s\n",thumb?"[ ":"",thumb?" ]":"");
#endif
    fprintf(pdf->f,"/Width %d\n"
                   "/Height %d\n"
                   "/ColorSpace /Device%s\n"
                   "/BitsPerComponent %d\n"
                   "/Length ",
                   bmp->width,bmp->height,
                   bmp->bpp==8?"Gray":"RGB",
                   bpc);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptrlen=(int)ftell(pdf->f);
    fprintf(pdf->f,"         \n"
                   ">>\n"
                   "stream\n");
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr1=(int)ftell(pdf->f);
#ifdef HAVE_JPEG_LIB
    if (quality>0)
        {
        bmp_write_jpeg_stream(bmp,pdf->f,quality,NULL);
        fprintf(pdf->f,"\n");
        }
    else
#endif
        {
        compress_handle h;
        h=compress_start(pdf->f,7); /* compression level = 7 */
        bmp_flate_decode(bmp,pdf->f,h,halfsize);
        compress_done(pdf->f,&h);
        fprintf(pdf->f,"\n");
        }
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr2=(int)ftell(pdf->f)-1;
    fprintf(pdf->f,"endstream\nendobj\n");
    insert_length(pdf->f,ptrlen,ptr2-ptr1);
    if (thumb)
        bmp_free(bmp);
    }



/*
** halfsize==0 for 8-bits per color plane
**         ==1 for 4-bits per color plane
**         ==2 for 2-bits per color plane
**         ==3 for 1-bit  per color plane
**
** To do:  Check for errors when writing
*/
static void bmp_flate_decode(WILLUSBITMAP *bmp,FILE *f,compress_handle handle,int halfsize)

    {
    int row;
    static char *funcname="bmp_flate_decode";

    if (halfsize==1)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+1)/2;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=2)
                data[i]=(p[0] & 0xf0) | (p[1] >> 4);
            if (nb&1)
                data[i]=p[0]&0xf0;
            else
                data[i]=(p[0]&0xf0) | (p[1] >> 4);
            compress_write(f,handle,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else if (halfsize==2)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+3)/4;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i,j,k;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=4)
                data[i]=(p[0] & 0xc0) | ((p[1] >> 2)&0x30) | ((p[2]>>4)&0xc) | (p[3]>>6);
            data[i]=0;
            j=(nb&3);
            if (j==0)
                j=4;
            for (k=0;k<j;k++)
                data[i]|=((p[k]&0xc0)>>(k*2));
            compress_write(f,handle,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else if (halfsize==3)
        {
        int w2,nb;
        unsigned char *data;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        w2=(nb+7)/8;
        willus_mem_alloc_warn((void **)&data,w2,funcname,10);
        for (row=0;row<bmp->height;row++)
            {
            int i,j,k;
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            for (i=0;i<w2-1;i++,p+=8)
                data[i]=(p[0] & 0x80) | ((p[1]&0x80) >> 1)
                                      | ((p[2]&0x80) >> 2)
                                      | ((p[3]&0x80) >> 3)
                                      | ((p[4]&0x80) >> 4)
                                      | ((p[5]&0x80) >> 5)
                                      | ((p[6]&0x80) >> 6)
                                      | ((p[7]&0x80) >> 7);
            data[i]=0;
            j=(nb&7);
            if (j==0)
                j=8;
            for (k=0;k<j;k++)
                data[i]|=((p[k]&0x80)>>k);
            compress_write(f,handle,data,w2);
            }
        willus_mem_free((double **)&data,funcname);
        }
    else
        {
        int nb;
        nb=bmp->bpp==8 ? bmp->width : bmp->width*3;
        for (row=0;row<bmp->height;row++)
            {
            unsigned char *p;
            p=bmp_rowptr_from_top(bmp,row);
            compress_write(f,handle,p,nb);
            }
        }
    }


void pdffile_finish(PDFFILE *pdf,char *title,char *author,char *producer,char *cdate)

    {
    int icat,i,pagecount;
    time_t now;
    struct tm today;
    size_t ptr;
    char nbuf[12];
    char buf[128];
    char mdate[128];
    char basename[256];

    time(&now);
    today=(*localtime(&now));

    /* Insert outline reference if available */
    for (i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&4)
            break;
    if (i<pdf->n)
        {
        fflush(pdf->f);
        fseek(pdf->f,pdf->object[0].ptr[2],0);
        sprintf(nbuf,"%6d",i+1);
        fwrite(nbuf,1,6,pdf->f);
        }
    else
        {
        fflush(pdf->f);
        fseek(pdf->f,pdf->object[0].ptr[2]-10,0);
        strcpy(nbuf,"%% ");
        fwrite(nbuf,1,3,pdf->f);
        }
        
    ptr=0; /* Avoid compiler warning */
    fseek(pdf->f,0L,2);
    if (pdf->pae==0)
        {
        pdffile_new_object(pdf,0);
        icat=pdf->n;
        fprintf(pdf->f,"<<\n"
                   "/Type /Pages\n"
                   "/Kids [");
        }
    else
        {
        fflush(pdf->f);
        fseek(pdf->f,0L,1);
        ptr=ftell(pdf->f);
        icat=pdf->n;
        fseek(pdf->f,pdf->pae,0);
        }
    for (pagecount=i=0;i<pdf->n;i++)
        if (pdf->object[i].flags&1)
            {
            pagecount++;
            if (pagecount>MAXPDFPAGES && pdf->pae>0)
                {
                printf("WILLUS lib %s:  PDF page counts > %d not supported!\n",
                       willuslibversion(),MAXPDFPAGES);
                exit(10);
                }
            fprintf(pdf->f," %d 0 R",i+1);
            }
    fprintf(pdf->f," ]\n"
                   "/Count %d\n"
                   ">>\n"
                   "endobj\n",pagecount);
    if (pdf->pae > 0)
        {
        fseek(pdf->f,ptr,0);
        }

    pdffile_new_object(pdf,0);
    if (producer==NULL)
        sprintf(buf,"WILLUS lib %s",willuslibversion());
    else
        buf[0]='\0';
    for (i=0;buf[i]!='\0';i++)
        if (buf[i]=='(' || buf[i]==')')
            buf[i]=' ';
    sprintf(mdate,"D:%04d%02d%02d%02d%02d%02d%s",
                   today.tm_year+1900,today.tm_mon+1,today.tm_mday,
                   today.tm_hour,today.tm_min,today.tm_sec,
                   wsys_utc_string());
    fprintf(pdf->f,"<<\n");
    if (author!=NULL && author[0]!='\0')
        fprintf(pdf->f,"/Author (%s)\n",author);
    if (title==NULL || title[0]=='\0')
        wfile_basespec(basename,pdf->filename);
    fprintf(pdf->f,"/Title (%s)\n"
                   "/CreationDate (%s)\n"
                   "/ModDate (%s)\n"
                   "/Producer (%s)\n"
                   ">>\n"
                   "endobj\n",
                   title!=NULL && title[0]!='\0' ? title : basename,
                   cdate!=NULL && cdate[0]!='\0' ? cdate : mdate,
                   mdate,
                   producer==NULL ? buf : producer);
    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    ptr=ftell(pdf->f);
    /* Kindles require the space after the 'f' and 'n' in the lines below. */
    fprintf(pdf->f,"xref\n"
                   "0 %d\n"
                   "0000000000 65535 f \n",pdf->n+1);
    for (i=0;i<pdf->n;i++)
        fprintf(pdf->f,"%010d 00000 n \n",(int)pdf->object[i].ptr[0]);
    fprintf(pdf->f,"trailer\n"
                   "<<\n"
                   "/Size %d\n"
                   "/Info %d 0 R\n"
                   "/Root 1 0 R\n"
                   ">>\n"
                   "startxref\n"
                   "%d\n"
                   "%%%%EOF\n",pdf->n+1,pdf->n,(int)ptr);
    /*
    ** Go back and put in catalog block references
    */
    if (pdf->pae==0)
        {
        sprintf(nbuf,"%6d",icat);
        for (i=0;i<pdf->n;i++)
            if (pdf->object[i].flags&2)
                {
                fseek(pdf->f,pdf->object[i].ptr[1],0);
                fwrite(nbuf,1,6,pdf->f);
                }
        }
    fclose(pdf->f);
    pdf->f=wfile_fopen_utf8(pdf->filename,"ab");
    }


static void pdffile_new_object(PDFFILE *pdf,int flags)

    {
    PDFOBJECT obj;

    fflush(pdf->f);
    fseek(pdf->f,0L,1);
    obj.ptr[0]=obj.ptr[1]=ftell(pdf->f);
    obj.flags=flags;
    pdffile_add_object(pdf,&obj);
    fprintf(pdf->f,"%d 0 obj\n",pdf->n);
    }


static void pdffile_add_object(PDFFILE *pdf,PDFOBJECT *object)

    {
    static char *funcname="pdffile_add_object";

    if (pdf->n>=pdf->na)
        {
        int newsize;
        newsize = pdf->na < 512 ? 1024 : pdf->na*2;
        if (pdf->na==0)
            willus_mem_alloc_warn((void **)&pdf->object,newsize*sizeof(PDFOBJECT),funcname,10);
        else
            willus_mem_realloc_robust_warn((void **)&pdf->object,newsize*sizeof(PDFOBJECT),
                                        pdf->na*sizeof(PDFOBJECT),funcname,10);
        pdf->na=newsize;
        }
    pdf->object[pdf->n++]=(*object);
    }


#ifdef HAVE_Z_LIB
int pdf_numpages(char *filename)

    {
    FILE *f;
    int np;

    f=wfile_fopen_utf8(filename,"rb");
    if (f==NULL)
        return(-1);
    np=pdf_numpages_1((void *)f,0);
    fclose(f);
    return(np);
    }


static int pdf_numpages_1(void *ptr,int bufsize)

    {
    char buf[256];
    FILE *f;
    char *opbuf;
    int i,i0,status,np,gls;
    static char *kwords[]={"/Type","/Pages","/Kids","/Count",
                           "/Filter","/FlateDecode","/Length",
                           "/ObjStm","stream",""};

    f=NULL; /* Avoid compiler warning */
    opbuf=NULL; /* Avoid compiler warning */
    if (bufsize==0)
        f=(FILE *)ptr;
    else
        opbuf=(char *)ptr;
    status=0;
    i0=0;
    np=-1;
    while (1)
        {
        if (bufsize==0)
            gls=wpdf_getline(buf,254,f);
        else
            gls=wpdf_getbufline(buf,254,opbuf,&i0,bufsize);
        for (i=0;kwords[i][0]!='\0';i++)
            {
            int ip;

            ip=in_string(buf,kwords[i]);
            if (ip>=0)
                {
                status |= (1<<i);
                if (i==3 || i==6)
                    np=atoi(&buf[ip+strlen(kwords[i])]);
/*
printf("    '%s' %x np=%d\n",kwords[i],status,np);
*/
                if (status==15 && np>0)
                    break;
                if (bufsize==0 && (status&0x1f1)==0x1f1 && np>0)
                    {
                    np=decodecheck(f,np);
                    if (np>0)
                        {
                        status=15;
                        break;
                        }
                    }
                }
            }
        if (status==15 && np>0)
            break;
        if (in_string(buf,"endobj")>=0)
            {
            status=0;
            np=-1;
            }
        if (!gls)
            break;
        }
    if (np>0)
        return(np);
    return(-2);
    }


static int decodecheck(FILE *f,int np)

    {
    char *inbuf,*outbuf;
    z_stream zstrm;
    int i0,status,obsize,extra;
    static char *funcname="decodecheck";

// printf("@decodecheck(np=%d)\n",np);    
    extra=4;
    willus_mem_alloc_warn((void **)&inbuf,np+extra,funcname,10);
    obsize=np*10;
    if (obsize<1024)
        obsize=1024;
    willus_mem_alloc_warn((void **)&outbuf,obsize,funcname,10);
    fread(inbuf,1,np+extra,f);
    i0=0;
    if (inbuf[i0]=='\n' || inbuf[i0]=='\r')
        i0++;
    memset(&zstrm,0,sizeof(zstrm));
    zstrm.avail_in=np+extra-i0;
    zstrm.avail_out=obsize;
    zstrm.next_in=(Bytef*)&inbuf[i0];
    zstrm.next_out=(Bytef*)outbuf;
    status=inflateInit(&zstrm);
    if (status!=Z_OK)
        {
        willus_mem_free((double **)&outbuf,funcname);
        willus_mem_free((double **)&inbuf,funcname);
        return(0);
        }
    status=inflate(&zstrm,Z_FINISH);
/*
printf("    Total output bytes = %d, status = %d\n",(int)zstrm.total_out,status);
printf("    ");
fwrite(outbuf,1,zstrm.total_out>2048 ? 2048:zstrm.total_out,stdout);
*/
    if (zstrm.total_out>0)
        np=pdf_numpages_1(outbuf,(int)zstrm.total_out);
    else
        np=0;
    willus_mem_free((double **)&outbuf,funcname);
    willus_mem_free((double **)&inbuf,funcname);
    return(np);
    }


static int wpdf_getline(char *buf,int maxlen,FILE *f)

    {
    int i,c;

    i=0;
    while ((c=fgetc(f))!=EOF)
        {
        if (c=='\n' || c=='\r')
            break;
        buf[i++]=c;
        if (i>=maxlen)
            break;
        }
    buf[i]='\0';
    return(c!=EOF);
    }


static int wpdf_getbufline(char *buf,int maxlen,char *opbuf,int *i0,int bufsize)

    {
    int i,c;

    i=0;
    while ((*i0) < bufsize)
        {
        c=opbuf[(*i0)];
        (*i0)=(*i0)+1;
        if (c=='\n' || c=='\r')
            break;
        buf[i++]=c;
        if (i>=maxlen)
            break;
        }
    buf[i]='\0';
    return((*i0)<bufsize);
    }
#endif /* HAVE_Z_LIB */


static void insert_length(FILE *f,long pos,int len)

    {
    long ptr;
    int i;
    char nbuf[64];

    fflush(f);
    fseek(f,0L,1);
    ptr=ftell(f);
    fseek(f,pos,0);
    sprintf(nbuf,"%d",len);
    for (i=0;i<8 && nbuf[i]!='\0';i++)
        fputc(nbuf[i],f);
    fseek(f,ptr,0);
    }


void ocrwords_box(OCRWORDS *ocrwords,WILLUSBITMAP *bmp)

    {
    int i,bpp;

    if (ocrwords==NULL)
        return;
    bpp=bmp->bpp==24 ? 3 : 1;
    for (i=0;i<ocrwords->n;i++)
        {
        int j;
        unsigned char *p;
        OCRWORD *word;
        word=&ocrwords->word[i];
        p=bmp_rowptr_from_top(bmp,word->r)+word->c*bpp;
        for (j=0;j<word->w;j++,p+=bpp)
            {
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        p=bmp_rowptr_from_top(bmp,word->r-word->maxheight)+word->c*bpp;
        for (j=0;j<word->w;j++,p+=bpp)
            {
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        for (j=0;j<word->maxheight;j++)
            {
            p=bmp_rowptr_from_top(bmp,word->r-j)+word->c*bpp;
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            p=bmp_rowptr_from_top(bmp,word->r-j)+(word->c+word->w-1)*bpp;
            (*p)=0;
            if (bpp==3)
                {
                p[1]=0;
                p[2]=255;
                }
            }
        }
    }


static void ocrwords_to_pdf_stream(OCRWORDS *ocrwords,FILE *f,double dpi,
                                   double page_height_pts,int text_render_mode,
                                   WILLUSCHARMAPLIST *cmaplist,int use_spaces,int ocr_flags)

    {
    int i;
    double median_size;

    fprintf(f,"BT\n%d Tr\n",text_render_mode);
    median_size=ocrwords_median_size(ocrwords,dpi,cmaplist);
    if (use_spaces)
        {
        int i1;

        for (i1=i=0;i<ocrwords->n;i++)
            {
            if (i==ocrwords->n-1 || ocrwords->word[i+1].r!=ocrwords->word[i].r)
                {
                OCRWORD word;
                ocrword_init(&word);
                ocrwords_optimize_spaces(&word,&ocrwords->word[i1],i-i1+1,dpi,cmaplist,
                                         use_spaces==2 ? 1 : 0);
                ocrword_to_pdf_stream(&word,f,dpi,page_height_pts,median_size,cmaplist,ocr_flags);
                ocrword_free(&word);
                i1=i+1;
                }
            }
        }
    else
        for (i=0;i<ocrwords->n;i++)
            ocrword_to_pdf_stream(&ocrwords->word[i],f,dpi,page_height_pts,median_size,cmaplist,
                                  ocr_flags);
    fprintf(f,"ET\n");
    }


static double ocrwords_median_size(OCRWORDS *ocrwords,double dpi,WILLUSCHARMAPLIST *cmaplist)

    {
    static char *funcname="ocrwords_to_histogram";
    static double *fontsize_hist;
    double msize;
    int i;

    if (ocrwords->n<=0)
        return(1.);
    willus_mem_alloc_warn((void **)&fontsize_hist,sizeof(double)*ocrwords->n,funcname,10);
    for (i=0;i<ocrwords->n;i++)
        {
        double w,h;
        ocrword_width_and_maxheight(&ocrwords->word[i],&w,&h,cmaplist,NULL);
        fontsize_hist[i] = (72.*ocrwords->word[i].maxheight/dpi) / h;
        }
    sortd(fontsize_hist,ocrwords->n);
    msize=fontsize_hist[ocrwords->n/2];
    if (msize < 0.5)
        msize = 0.5;
    willus_mem_free(&fontsize_hist,funcname);
    return(msize);
    }


/*
** If charpos[] is not NULL, it gets the position of the right side of each character.
** Must be dimensioned from [0..n-1] where n is the number of chars in the word.
**
** Returned values are in points for a 1-point-sized font (points per point).
**
*/
static void ocrword_width_and_maxheight(OCRWORD *word,double *width,double *maxheight,
                                        WILLUSCHARMAPLIST *cmaplist,double *charpos)

    {
    int i,n;
    int *d;
    static char *funcname="ocrword_width_and_maxheight";

    n=strlen(word->text)+2;
    willus_mem_alloc_warn((void **)&d,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(d,word->text,n-1);
    (*width)=0.;
    /*
    ** 7-10-2020--return a standard height for all chars for better
    **            selection consistency.
    */
    /* (*maxheight)=0.; */
    (*maxheight)=0.65;
    for (i=0;i<n;i++)
        {
        int c,cid,index;

        if (d[i]<256)
            cid=d[i];
        else
            {
            index=willuscharmaplist_cid_index(cmaplist,d[i]);
            if (index<0 || index>=cmaplist->n || cmaplist->cmap[index].unicode!=d[i])
                cid=32;
            else
                cid=cmaplist->cmap[index].cid&0xff;
            }
        c=cid-32;
        if (c<0 || c>=224)
            c=0;
        /* 3-15-2020: Replace -1's in dataset with means */
        /* Hoping to prevent bad text selection of certain unicode chars */
        {
        double w,nc,ab;

        w=pdffonts_helvetica[c].width;
        nc=pdffonts_helvetica[c].nextchar;
        ab=pdffonts_helvetica[c].abovebase;
        if (w<=0.)
            w=WIDTHMEAN;
        if (nc<=0.)
            nc=NEXTCHARMEAN+WIDTHMEAN;
        if (ab<=0.)
            ab=ABOVEBASEMEAN;
        if (word->text[i+1]=='\0')
            (*width) += w;
        else
            (*width) += nc;
        if (charpos!=NULL)
            charpos[i]=(*width);
        /* 7-10-2020 -- see above */
        /*
        if (ab > (*maxheight))
            (*maxheight)=ab;
        */
        }
        }
    /* Limit checks -- 7-21-2013 */
    if ((*width) < .01)
        (*width) = .01;
    if ((*maxheight) < .01)
        (*maxheight) = .01;
    willus_mem_free((double **)&d,funcname);
    }


static double size_round_off(double size,double median_size,double log_size_increment)

    {
    double rat,lograt;

    if (size < .5)
        size = .5;
    rat=size / median_size;
    /* limit to prevent overflow / infinity -- 7-21-2013  */
    if (rat < 1e-3)
        rat = 1e-3;
    if (rat > 1e5)
        rat = 1e5;
    lograt = floor(log10(rat)/log_size_increment+.5);
    return(median_size*pow(10.,lograt*log_size_increment));
    }


/*
** In "sentence", put all of the words in "words" separated by an optimum
** number of spaces between each word so that the words best line up with
** where their bitmaps are on the page.
*/
static void ocrwords_optimize_spaces(OCRWORD *sentence,OCRWORD *word,int n,double dpi,
                                     WILLUSCHARMAPLIST *cmaplist,int optimize)
                              

    {
    int i,pixwidth;
    int *nspaces;
    double *pos; /* Left position of each word rel to sentence length */
    static char *funcname="ocrwords_optimize_spaces";

    pixwidth=word[n-1].c+word[n-1].w-word[0].c;
/*
printf("Text row: ");
for (i=0;i<n;i++)
printf(" %s (%5.3f)",word[i].text,(double)word[i].w/pixwidth);
printf("\n");
printf("pixwidth=%d\n",pixwidth);
*/
    willus_mem_alloc_warn((void **)&pos,sizeof(double)*2*n,funcname,10);
    willus_mem_alloc_warn((void **)&nspaces,sizeof(int)*n,funcname,10);
    for (i=0;i<n-1;i++)
        nspaces[i]=1;
    nspaces[n-1]=0;
    if (optimize)
        {
        /* First find number of spaces on end that makes all pos[] values negative */
        for (i=0;i<n;i++)
            nspaces[i]=1;
        for (i=0;i<2000;i++)
            {
            int j;

            nspaces[n-1]=i;
            sentence_check_alignment(word,n,nspaces,pos,cmaplist);
/*
{
int k;
printf("  i=%3d ",i);
for (k=0;k<n;k++)
printf(" %5.3f",pos[k*2]-(k==0?0.:pos[k*2-1]));
printf("\n");
}
*/
            /* Are any words still too long? */
            for (j=0;j<n;j++)
                if (pos[j*2]-(j==0 ? 0. : pos[j*2-1]) > (double)word[j].w/pixwidth)
                    break;
            if (j>=n)
                break;
            }
        for (i=0;i<n-1;i++)
            {
            int j,ns1;
            double err;
            double wordpos;

            ns1=nspaces[n-1];
            wordpos=(double)(word[i+1].c-word[0].c)/pixwidth;
            err=0.;
            for (j=1;j<ns1;j++)
                {
                nspaces[i]=j;
                nspaces[n-1]=ns1+1-j;
                sentence_check_alignment(word,n,nspaces,pos,cmaplist);
                if (pos[2*i+1] > wordpos)
                    {
                    if (j>1 && pos[2*i+1]-wordpos > err)
                        {
                        nspaces[i]=j-1;
                        nspaces[n-1]=ns1+1-(j-1);
                        sentence_check_alignment(word,n,nspaces,pos,cmaplist);
                        }
                    break;
                    }
                err=wordpos-pos[2*i+1];
                }
            }
/*
{
int k;
printf("    Pass %d: ",i);
for (k=0;k<n-1;k++)
printf(" %d (%5.3f, %5.3f)",nspaces[k],pos[2*k],pos[2*k+1]);
printf("\n");
}
*/
        }
    ocrwords_sentence_construct(sentence,word,n,nspaces);
    willus_mem_free((double **)&nspaces,funcname);
    willus_mem_free(&pos,funcname);
    }


/*
** Construct single OCRWORD "sentence" from array of OCRWORDs and nspaces[] array
*/
static void ocrwords_sentence_construct(OCRWORD *sentence,OCRWORD *word,int n,int *nspaces)

    {
    int i,len;
    static char *funcname="ocrwords_sentence_construct";

    for (len=i=0;i<n;i++)
        len += strlen(word[i].text)+nspaces[i];
    ocrword_free(sentence);
    willus_mem_alloc_warn((void **)&sentence->text,len+1,funcname,10);
    sentence->text[0]='\0';
    sentence->r=word[0].r;
    sentence->c=word[0].c;
    sentence->w=word[n-1].c+word[n-1].w-word[0].c;
    sentence->maxheight=0;
    sentence->rot=word[0].rot;
    for (i=0;i<n;i++)
        {
        int j;

        strcat(sentence->text,word[i].text);
        if (word[i].maxheight > sentence->maxheight)
            sentence->maxheight = word[i].maxheight;
        len=strlen(sentence->text);
        for (j=0;j<nspaces[i];j++)
            sentence->text[len+j]=' ';
        sentence->text[len+j]='\0';
        }
    }


/*
** Inputs:  word[0..n-1] = words in sentence / row of text.
**          nspaces[0..n-1] = number of spaces after word[0..n-1]
**
** Output:  pos[0..2*n-1] = positional error of right side of word[i/2] or space[i/2].
**              pos[0] = right side of first word
**              pos[1] = right side of space after first word (left side of next word)
**              pos[2] = right side of 2nd word
**              pos[3] = right side of space after second word
**              ...
**          positive means the OCR layer word starts to the right of the bitmap word.
**
*/
static void sentence_check_alignment(OCRWORD *word,int n,int *nspaces,double *pos,
                                     WILLUSCHARMAPLIST *cmaplist)

    {
    int i,index,len;
    double *charpos;
    static char *funcname="sentence_check_alignment";
    double ptwidth,ptheight;
    OCRWORD _sentence,*sentence;

    sentence=&_sentence;
    ocrword_init(sentence);
    ocrwords_sentence_construct(sentence,word,n,nspaces);
    len=strlen(sentence->text);
    willus_mem_alloc_warn((void **)&charpos,sizeof(double)*(len+2),funcname,10);
    ocrword_width_and_maxheight(sentence,&ptwidth,&ptheight,cmaplist,charpos);
/*
{
int wordlen;
printf("S:");
wordlen=utf8_to_unicode(NULL,sentence->text,len+2);
for (i=0;i<wordlen;i++)
printf(" %.3f",charpos[i]/ptwidth);
printf("\n");
}
*/
    ocrword_free(sentence);
    for (i=index=0;i<n;i++)
        {
        int wordlen;
        double xpt;

        wordlen=utf8_to_unicode(NULL,word[i].text,len+2);
        xpt=charpos[index+wordlen-1];
        pos[2*i]=xpt/ptwidth;
        if (i<n-1)
            {
            xpt=charpos[index+wordlen+nspaces[i]-1];
            pos[2*i+1]=xpt/ptwidth;
            }
        index += wordlen+nspaces[i];
        }
    willus_mem_free((double **)&charpos,funcname);
    }


static void ocrword_to_pdf_stream(OCRWORD *word,FILE *f,double dpi,
                                  double page_height_pts,double median_size_pts,
                                  WILLUSCHARMAPLIST *cmaplist,int ocr_flags)

    {
    int cc,i,n,wordw;
    double fontsize_width,fontsize_height,ybase,x0,y0;
    double width_per_point,height_per_point,arat;
    char rotbuf[48];
    int *d;
    static char *funcname="ocrword_to_pdf_stream";

/*
printf("word->text='%s'\n",word->text);
printf("    wxh = %dx%d, dpi=%g\n",word->w,word->h,dpi);
*/
    n=strlen(word->text)+2;
    willus_mem_alloc_warn((void **)&d,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(d,word->text,n-1);
    ocrword_width_and_maxheight(word,&width_per_point,&height_per_point,cmaplist,NULL);
/*
printf("    word->maxheight=%g, height_per_point=%g\n",word->maxheight,height_per_point);
*/
    if (word->w/10. < word->lcheight)
        wordw = 0.9*word->w;
    else
        wordw = word->w-word->lcheight;
    fontsize_width = 72.*wordw/dpi / width_per_point;
    fontsize_height = (72.*word->maxheight/dpi) / height_per_point;
    if (!(ocr_flags & 0x20))
        fontsize_height = size_round_off(fontsize_height,median_size_pts,.25);
/*
printf("    fontsize_height=%g\n",fontsize_height);
*/
    /*
    ** Keeping the height small generally improves the ability to graphically
    ** select the text in PDF readers, so multiply by 0.5.
    */
    fontsize_height = 0.5*fontsize_height;
    arat = fontsize_width / fontsize_height;
    ybase = page_height_pts - 72.*word->r/dpi;
    if (word->rot==0)
        sprintf(rotbuf,"%.4f 0 0 1",arat);
    else if (word->rot==90)
        sprintf(rotbuf,"0 %.4f -1 0",arat);
    else
        {
        double theta,sinth,costh;

        theta=word->rot*PI/180.;
        sinth=sin(theta);
        costh=cos(theta);
        sprintf(rotbuf,"%.3f %.3f %.3f %.3f",costh*arat,sinth*arat,-sinth,costh);
        }
    cc=0;
    x0=72.*word->c/dpi;
    y0=ybase;
    /*
    ** Go through word letter by letter and select correct font for each
    ** letter so that unicode copy / paste works.
    */
    for (i=0;i<n;i++)
        {
        int cid,index,fn;

        if (d[i]<256)
            {
            fn=1;
            cid=d[i];
            }
        else
            {
            index=willuscharmaplist_cid_index(cmaplist,d[i]);
            if (index<0 || index>=cmaplist->n || cmaplist->cmap[index].unicode!=d[i])
                {
                cid=32;
                fn=1;
                }
            else
                {
                cid=cmaplist->cmap[index].cid&0xff;
                fn=1+((cmaplist->cmap[index].cid>>8)&0xfff);
                }
            }
        if (cid<32 || cid>255)
            cid=32;
        if (fn!=lastfont || fabs(fontsize_height-lastfontsize)>.01)
            {
            if (cc>0)
                {
                fprintf(f,"> Tj\n");
                cc=0;
                }
            fprintf(f,"/F%d %.2f Tf\n",fn,fontsize_height);
            lastfontsize=fontsize_height;
            lastfont=fn;
            }
        if (i==0)
            fprintf(f,"%s %.2f %.2f Tm\n",rotbuf,x0,y0);
        fprintf(f,"%s%02X",cc==0?"<":"",cid);
        cc++;
        x0 += fontsize_height*arat*pdffonts_helvetica[cid-32].nextchar;
        }
    /* 2-1-14: Memory leak fixed */
    willus_mem_free((double **)&d,funcname);
    if (cc>0)
        fprintf(f,"> Tj\n");
    }


static void willuscharmaplist_init(WILLUSCHARMAPLIST *list)

    {
    list->n=list->na=0;
    list->cmap=NULL;
    }


static void willuscharmaplist_free(WILLUSCHARMAPLIST *list)

    {
    willus_mem_free((double **)&list->cmap,"willuscharmaplist_free");
    list->n=list->na=0;
    }


static void willuscharmaplist_add_charmap(WILLUSCHARMAPLIST *list,int unichar)

    {
    static char *funcname="willuscharmaplist_add_charmap";
    int i,cid;

    i=willuscharmaplist_cid_index(list,unichar);
    if (i>=0 && i<list->n && list->cmap[i].unicode==unichar)
        return;
    if (list->n >= list->na)
        {
        int newsize;
        newsize = list->na < 512 ? 1024 : list->na*2;
        willus_mem_realloc_robust_warn((void **)&list->cmap,sizeof(WILLUSCHARMAP)*newsize,
                                   sizeof(WILLUSCHARMAP)*list->na,funcname,10);
        list->na=newsize;
        }
    if (i<list->n)
        memmove(&list->cmap[i+1],&list->cmap[i],(list->n-i)*sizeof(WILLUSCHARMAP));
    cid=willuscharmaplist_maxcid(list)+1;
    if (cid<=0x120)
        cid=0x121;
    /* Issue with Adobe copy/paste--only do one dummy char in first font then skip */
    /* to next font. */
    if (list->n==1)
        cid=0x221;
    while (1)
        {
        if ((cid&0xff)<0x21)
            cid=(cid&0xfff00)|0x21;
/*
if ((cid&0xff)<0x41)
cid=(cid&0xfff00)|0x41;
if ((cid&0xff)>0x57)
cid=((cid+0x100)&0xfff00)|0x41;
*/
        /* Choose a letter that isn't too thin or too far above/below the baseline */
        if (pdffonts_helvetica[(cid&0xff)-32].abovebase < .47
             || pdffonts_helvetica[(cid&0xff)-32].abovebase > 1.0
             || pdffonts_helvetica[(cid&0xff)-32].belowbase < -.001
             || pdffonts_helvetica[(cid&0xff)-32].belowbase > 0.2
             || pdffonts_helvetica[(cid&0xff)-32].width < 0.4)
            {
            cid++;
            continue;
            }
        break;
        }
    list->cmap[i].cid = cid;
    list->cmap[i].unicode=unichar;
    list->n++;
    }


static int willuscharmaplist_maxcid(WILLUSCHARMAPLIST *list)

    {
    int i,max;

    for (i=max=0;i<list->n;i++)
        if (list->cmap[i].cid > max)
            max=list->cmap[i].cid;
    return(max);
    }


/*
** Must be sorted by unicode value!
*/
static int willuscharmaplist_cid_index(WILLUSCHARMAPLIST *list,int unichar)

    {
    int i1,i2;

    if (list->n<=0)
        return(0);
    if (unichar <= list->cmap[0].unicode)
        return(0);
    if (unichar > list->cmap[list->n-1].unicode)
        return(list->n);
    if (unichar == list->cmap[list->n-1].unicode)
        return(list->n-1);
    i1=0;
    i2=list->n-1;
    while (i2-i1>1)
        {
        int i;
        i=(i1+i2)/2;
        if (unichar==list->cmap[i].unicode)
            return(i);
        if (unichar>list->cmap[i].unicode)
            i1=i;
        else
            i2=i;
        }
    return(i2);
    }


/*
** Map all unicode characters on the page to character ID's (cids).
** Use multiple fonts (multiple copies of Helvetica with different
** unicode mappings) if necessary.
*/
static void willuscharmaplist_populate(WILLUSCHARMAPLIST *cmaplist,OCRWORDS *ocrwords)

    {
    int i;

    /* Add one dummy char--issue with Adobe */
    willuscharmaplist_add_charmap(cmaplist,0xffff);
    for (i=0;i<ocrwords->n;i++)
        willuscharmaplist_populate_string(cmaplist,ocrwords->word[i].text);
    }


static void willuscharmaplist_populate_string(WILLUSCHARMAPLIST *cmaplist,char *s)

    {
    int *d;
    int i,n;
    static char *funcname="willuscharmaplist_populate_string";

    n=strlen(s)+2;
    willus_mem_alloc_warn((void **)&d,sizeof(int)*n,funcname,10);
    n=utf8_to_unicode(d,s,n-1);
    for (i=0;i<n;i++)
        if (d[i]>=256)
            willuscharmaplist_add_charmap(cmaplist,d[i]);
    willus_mem_free((double **)&d,funcname);
    }
