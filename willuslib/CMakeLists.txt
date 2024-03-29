project(k2pdfopt C)
cmake_minimum_required(VERSION 2.6)
#
# Last update 31 May 2020 willus.com
#

include_directories(..)

set(WILLUSLIB_SRC
    ansi.c array.c bmp.c bmpdjvu.c bmpmupdf.c dtcompress.c filelist.c
    fontdata.c fontrender.c gslpolyfit.c linux.c math.c mem.c ocr.c
    ocrgocr.c ocrtess.c ocrwords.c pdffonts.c pdfwrite.c point2d.c
    render.c strbuf.c string.c token.c wfile.c wgs.c wgui.c
    willusversion.c win.c winbmp.c wincomdlg.c wininet.c winmbox.c
    winshell.c winshellwapi.c wleptonica.c wmupdf.c wmupdfinfo.c wpdf.c
    wpdfoutline.c wpdfutil.c wsys.c wzfile.c
)
# ocr.c  
# win.c  winbmp.c winmbox.c wincomdlg.c wgui.c winshell.c

if(DJVU_FOUND)
  set(WILLUSLIB_SRC ${WILLUSLIB_SRC} bmpdjvu.c)
endif(DJVU_FOUND)

if(MUPDF_FOUND)
  set(WILLUSLIB_SRC ${WILLUSLIB_SRC} bmpmupdf.c wmupdf.c)
endif(MUPDF_FOUND)

message("-- Willuslib sources: ${WILLUSLIB_SRC}")

add_library(willuslib ${WILLUSLIB_SRC})

# HAVE_GOCR_LIB: ocrjocr.c
# HAVE_TESSERACT_LIB: ocrtess.c 

