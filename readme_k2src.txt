K2pdfopt build help.
http://willus.com
Original: 7 September 2012
Last updated: 3 November 2012

This "read me" file describes the source code distribution for k2pdfopt.

K2pdfopt Source Files
---------------------
1. k2pdfopt.c (main module)

2. willus.com general-purpose C library (27 C files + 1 header file) in
   willuslib subfolder.
   (New in k2pdfopt v1.60: wmupdf.c module)
   Compile all C files in this subfolder and build them into libwillus.a.


Third Party Libraries
---------------------
There are also a number of required 3rd-party open-source C/C++ libraries:

    REQUIRED
    --------
    1.  Z-lib 1.2.6 (zlib.net) -- SEE NOTE 1.
    2.  libpng 1.5.9 (www.libpng.org)
    3.  Turbo JPEG lib 1.2.0 (sourceforge.net/projects/libjpeg-turbo/)

    TO INCLUDE MuPDF LIBRARY (search for HAVE_MUPDF in k2pdfopt.c)
    --------------------------------------------------------------
    4.  JBIG2Dec 0.11 (jbig2dec.sourceforge.net)
    5.  OpenJPEG 1.5.0 (www.openjpeg.org)
    6.  FreeType 2.4.10 (freetype.sourceforge.net/index2.html)
    7.  Mupdf 1.1 (mupdf.com)

    TO INCLUDE DjVuLibre LIBRARY (search for HAVE_DJVU in k2pdfopt.c)
    -----------------------------------------------------------------
    8.  DJVULibre 3.5.25 (C++) (djvu.sourceforge.net)

    FOR OCR VERSIONS OF K2PDFOPT (search for HAVE_OCR in k2pdfopt.c)
    ----------------------------------------------------------------
    9.  GOCR 0.49 (sourceforge.net/jocr/)
    10. Leptonica 1.69 (leptonica.com)
    11. Tesseract 3.02.02 (C++) (code.google.com/tesseract-ocr/) -- SEE NOTE 2.

    If you don't include MuPDF, DjVuLibre, or OCR, then k2pdfopt will
    look for an installation of Ghostscript.


Notes
-----
1. For zlib, I did a custom mod that is used by my pdfwrite.c code in my
   library to write a deflated stream.  You'll need to compile the Z-lib
   library with the mods that I put into the zlib_mod folder (gzwrite.c and
   gzlib.c).  Search on "WILLUS MOD" in those files to see where I modified them.

2. Tesseract requires my small C API for it which is in the tesseract_mod
   folder.  I also did a very minor mod to tessdatamanager.cpp.  It also
   needs you to download one of the data packages for it from the Tesseract
   web site and to point the TESSDATA_PREFIX environment variable to the
   root tesseract folder (e.g. TESSDATA_PREFIX=c:\tesseract-ocr\).

3. For a lot of the 3rd-party libraries, I combined their headers into one
   header that gets included by some of the willus library source files.
   I can't always remember which ones I did this for, so if you can't find
   any of the included files (if they didn't come standard w/the 3rd party
   library), look in my include_mod subfolder.

4. I apologize for the excessive use of global variables in the main program
   code (and for other less-than-rigorous approaches).  Globals are mostly used
   for storing command-line options.  This code started as a quick-and-dirty
   program to do a job for me and at this point it would be a considerable
   effort to remove all the globals.


Build Steps on Windows
----------------------
My compile steps with gcc (MinGW) are as follows (assuming all the libraries are built
to libxxx.a files in d:\3rdparty_lib and headers are in d:\3rdparty_include):

    64-bit
    ------
    1. echo k2pdfopt ICON k2pdfopt.ico > k2pdfopt.rc

    2. windres --target=pe-x86-64 -o resfile.o k2pdfopt.rc

    3. gcc -Ofast -m64 -Wall -c -Id:\3rdparty_include k2pdfopt.c

    4. g++ -Ofast -m64 -Wall -o k2pdfopt.exe k2pdfopt.o resfile.o -static-libgcc -static-libstdc++ d:\mingw\x64\lib\crt_noglob.o -Ld:\3rdparty_lib -lwillus -lgocr -ltesseract -lleptonica -ldjvu -lmupdf -lfreetype -ljbig2 -ljpeglib -lopenjpeg -lpng -lzlib -lgdi32

    32-bit
    ------
    1. echo k2pdfopt ICON k2pdfopt.ico > k2pdfopt.rc

    2. windres --target=pe-i386 -o resfile.o k2pdfopt.rc

    3. gcc -Ofast -m32 -Wall -c -Id:\3rdparty_include k2pdfopt.c

    4. g++ -Ofast -m32 -Wall -o k2pdfopt.exe k2pdfopt.o resfile.o -static-libgcc -static-libstdc++ d:\mingw\i386\lib\crt_noglob.o -Ld:\3rdparty_lib -lwillus -lgocr -ltesseract -lleptonica -ldjvu -lmupdf -lfreetype -ljbig2 -ljpeglib -lopenjpeg -lpng -lzlib -lgdi32 -lwsock32


Build Steps on Linux and OS/X (64-bit)
--------------------------------------
1. gcc -Wall -O3 -ffast-math -m64 -o k2pdfopt.o -c k2pdfopt.c

2. g++ -O3 -ffast-math -m64 -o k2pdfopt k2pdfopt.o -static-libgcc -lwillus -lgocr -ltesseract -lleptonica -ldjvu -lmupdf -lfreetype -ljbig2 -ljpeglib -lopenjpeg -lpng -lzlib -lpthread
