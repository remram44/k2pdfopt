K2pdfopt build help.
http://willus.com, 7 September 2012

Source code distribution for k2pdfopt:

1. k2pdfopt.c (main module)

2. willus.com general-purpose C library (26 C files + 1 header file) in
   willuslib subfolder.
   Compile all C files in this subfolder and build them into libwillus.a.

There are also a number of required 3rd-party open-source C/C++ libraries:

1.  Z-lib 1.2.6 (zlib.net) -- SEE NOTE 1.
2.  libpng 1.5.9 (www.libpng.org)
3.  Turbo JPEG lib 1.2.0 (sourceforge.net/projects/libjpeg-turbo/)
4.  JasPer 1.900.1 (www.ece.uvic.ca/~frodo/jasper/) (JPEG 2000 library)
5.  GSL 1.15 (gnu.org/software/gsl/) -- SEE NOTE 2.
6.  JBIG2Dec 0.11 (jbig2dec.sourceforge.net)
7.  OpenJPEG 1.5.0 (www.openjpeg.org)
8.  FreeType 2.4.9 (freetype.sourceforge.net/index2.html)
9.  Mupdf 1.0 (mupdf.com)
10. DJVULibre 3.5.25 (C++) (djvu.sourceforge.net)

FOR OCR VERSIONS OF K2PDFOPT (search for HAVE_OCR in k2pdfopt.c)
----------------------------------------------------------------
11. GOCR 0.49 (sourceforge.net/jocr/)
12. Leptonica 1.68 (leptonica.com)
13. Tesseract 3.01 (C++) (code.google.com/tesseract-ocr/) -- SEE NOTE 3.

Notes
-----
1. For zlib, I did a custom mod that is used by my pdfwrite.c code in my
   library to write a deflated stream.  You'll need to compile the Z-lib
   library with the mods that I put into the zlib_mod folder (gzwrite.c and
   gzlib.c).  Search on "WILLUS MOD" in those files to see where I modified them.

2. I think you can get away without the GSL library if you just comment
   out the code in gslpolyfit.c (make the one function empty).  I have
   not verified this yet.

3. Tesseract requires my small C API for it which is in the tesseract_mod
   folder.  I also did a very minor mod to tessdatamanager.cpp.  It also
   needs you to download one of the data packages for it from the Tesseract
   web site and to point the TESSDATA_PREFIX environment variable to the
   root tesseract folder (e.g. TESSDATA_PREFIX=c:\tesseract-ocr\).

4. For a lot of the 3rd-party libraries, I combined their headers into one
   header that gets included by some of the willus library source files.
   I can't always remember which ones I did this for, so if you can't find
   any of the included files (if they didn't come standard w/the 3rd party
   library), look in my include_mod subfolder.

5. I apologize for the excessive use of global variables in the main program
   code (and for other less-than-rigorous approaches).  This code started as
   a quick-and-dirty program to do a job for me and I haven't taken the time
   to completely re-do it in a more rigorous and stylistically correct way.


Build steps on Windows (similar on other gcc platforms)
-------------------------------------------------------
My compile steps with gcc (MinGW) are as follows (assuming all the libraries are built
to libxxx.a files in d:\3rdparty_lib and headers are in d:\3rdparty_include):

1. gcc -Id:\3rdparty_include -Ofast -m32 -Wall -c k2pdfopt.c

2. g++ -Ofast -m32 -Wall k2pdfopt.o -o k2pdfopt.exe -static-libgcc -static-libstdc++ d:\mingw\i386\lib\crt_noglob.o -Ld:\3rdparty_lib -ldjvu -lfreetype -lgsl -ljasper -ljbig2 -ljpeglib -lleptonica -lmupdf -lopenjpeg -lpng -lwillus -lzlib -ldjvu -lfreetype -lgsl -ljasper -ljbig2 -ljpeglib -lleptonica -lmupdf -lopenjpeg -lpng -lwillus -lzlib -ldjvu -lfreetype -lgsl -ljasper -ljbig2 -ljpeglib -lleptonica -lmupdf -lopenjpeg -lpng -lwillus -lzlib -lpthread -lwininet -lwsock32 -lole32 -lwinmm -lwinspool -lopengl32 -ladvapi32 -lgdi32 -loleaut32 -luuid -lpsapi -lkernel32 -luser32

(You probably don't have to specify each library three times, but I do it just as an overkill to avoid any issues with interdependent libraries.)


Build steps on Linux (64-bit)
-----------------------------
1. gcc -Wall -O3 -ffast-math -m64 -o k2pdfopt.o -c k2pdfopt.c

2. g++ -O3 -ffast-math -m64 -o k2pdfopt k2pdfopt.o -lwillus -ltesseract -lleptonica -lgocr -ldjvu -lmupdf -lfreetype -lopenjpeg -ljbig2 -ljasper -ljpeglib -lpng -lzlib -lgsl -lwillus -lm -static-libgcc
