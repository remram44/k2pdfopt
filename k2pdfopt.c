/*
** k2pdfopt.c   K2pdfopt optimizes PDF/DJVU files for mobile e-readers
**              (e.g. the Kindle) and smartphones. It works well on
**              multi-column PDF/DJVU files. K2pdfopt is freeware.
**
** Copyright (C) 2012  http://willus.com
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
** FUTURE MODIFICATIONS
**
**           v1.60?  implement by cropping/scaling/translating of
**                   native PDF pages.
**                   (Check for right-to-left fonts??)
**                   - Chapter markers?
**                   - Multithread option?
**           Wish list  
**                   - Little blurbs in the margins--better way to handle?
**                   - bryce.pdf (James Bryce Holy Roman Empire)--hanging
**                     indents like on p. 43 with the date--auto-detect?
**                   - Options for type of Tesseract OCR (cube works best)?
**                   - Installer program for Tesseract data?
**                   - Better algorithm for finding whitethresh?
**                     (contrast adjust)
**                   - Autostraighten each column (e.g. when one page of
**                     a book is tilted).
**                   - Way to set overwrite_minsize_mb in menu?
**                   - Option to change suffix or specify output name?
**                   - Figure out how to get bounding boxes for text
**                     primitives from MuPDF so that cropping could be
**                     done to these bounding boxes (Markom).
**           v1.51:
**                   - Copy/Paste-able unicode-16 strings in PDF so that
**                     foreign characters are supported.
**                     (how to handle in Helvetica font...)
**
** VERSION HISTORY
**
** v1.50     9-7-12
**           MAJOR NEW FEATURE: OCR
**           - For PDFs in English, added optical character recognition
**             using two different open source libraries:  Tesseract v3.01
**             (http://code.google.com/p/tesseract-ocr/) and GOCR v0.49
**             (http://jocr.sourceforge.net).  The OCR'd text
**             is embedded into the document as invisible ASCII text
**             stored in the same location as the bitmapped words,
**             exactly like some document scanning software works
**             (e.g. Canon's Canoscan software).  This allows the resultant
**             PDF document to be searched for text, assuming the OCR
**             is successful (won't always be the case, but should
**             work reasonably well if the source text is clear enough).
**           - Tesseract works far better than GOCR, but requires that
**             you download the "trained data" file for your language
**             from http://code.google.com/p/tesseract-ocr/downloads/list
**             and point the environment var TESSDATA_PREFIX to the root
**             folder for your trained data.  If the tesseract trained
**             data files are not found, k2pdfopt falls back to GOCR.
**             E.g. if your data is in c:\tesseract-ocr\tessdata\...
**             then set TESSDATA_PREFIX= c:\tessseract-ocr\
**           - I wrestled with this, but the default for OCR (for now)
**             is for it to be turned off.  You must explicitly turn it
**             on with the -ocr command-line option or "oc" at the
**             interactive menu.  I did this because it does significantly
**             slow down processing (about 20 words/second on a fast PC
**             using Tesseract).
**           - -wc option sets the OCR word color (visibility)
**             (e.g. -wc 0 for invisible OCR text, the default).
**           - -ocrhmax option sets max height of OCR'd word in inches.
**
**           OTHER NEW FEATURES:
**           - Added -evl option (menu item "e") to erase vertical lines.
**             This allows the option, for example, to get rid of
**             column divider lines which often prevent k2pdfopt from
**             properly separating columns and/or wrapping text.
**           - Detects and eliminates hyphens when wrapping text.  Turn
**             this off with -hy- ("w" interactive menu option).
**           - New option -wrap+ option will unwrap/re-flow narrow columns
**             of text to your wider device screen (typically desired
**             on a Kindle DX, for example).  Best if combined with -fc-.
**           - There is now a max column gap threshold option so that
**             columns are not detected if the gap between them is too large.
**             (-gtcmax command-line option).  Default = 1.5.
**           - New option -o controls when files get overwritten.  E.g.
**             -o 10 tells k2pdfopt not to overwrite any existing files
**             larger than 10 MB without prompting (the default).
**           - New option -f2p ("fit to page") can be used to fit tall
**             figures or the "red-boxed" regions (when using -sm) onto
**             single pages.
**
**           BUG FIXES / MISC
**           - Fixed description of -whitethresh in usage (had wrong default).
**           - Interactive menu more obvious about what files are specifed
**             and allows wildcards for file specification.
**           - Fixed bug in word_gaps_add() where the gap array was getting
**             erroneously filled with zeros, leading to some cases where
**             words got put together with no gap between them during text
**             wrapping/re-flow.
**           - The -sm option (show marked source) now works correctly when
**             used with the -c (color) option.
**           - Removed the separate usage note about Ghostscript and put it
**             under the -gs option usage.
**           - Fixed bug where zero height bitmap was sometimes passed to
**             bmp_src_to_dst().  Also rounded off (rather than floor()-ing)
**             scaling height used in the bitmap passed to bmp_src_to_dst().
**           - Due to some minor bitmap rendering improvements, this version
**             seems to be generally a little faster (~2-4%) compared to
**             v1.41 (under same conditions--no OCR).
**               
** v1.41     6-11-2012
**           IMPROVEMENTS
**           - Compiled w/MuPDF v1.0.
**           - Tweaked the auto-straightening algorithm--hopefully more
**             accurate and robust.  Now straighten even if only tilted
**             by 0.1 degree or more.
**           - Improved auto-contrast adjust algorithm and added option
**             to force a contrast setting by suppying a negative
**             value for -cmax.  Does a better job on scans of older
**             documents with significantly yellowed or browned pages.
**           - Options -? and -ui- when specified together now correctly
**             echo the entire usage without pausing so that you can
**             redirect to a file (as claimed in the usage for -?).
**
**           BUG FIXES
**           - Fixed bug where the column finding algorithm became far
**             too slow on certain types of pages (to the point where
**             k2pdfopt appeared to have crashed).
**           - Fixed bug where k2pdfopt wasn't working correctly when
**             the -c option was specified (color output).
**           - Fixed bug where if max columns was set to 3 in the
**             interactive menu, it didn't get upgraded to 4.
**           - Fixed memory leak in bmpregion_add() (temp bitmap
**             wasn't getting freed).
**           - Fixed memory leak in bmp_src_to_dst() (temp 8-bit
**             bitmap not getting freed).
**           - Fixed memory leak in bmpregion_one_row_wrap_and_add()
**             (breakinfo_free).
**           - Check for zero regions in breakinfo_compute_row_gaps()
**             and breakinfo_compute_col_gaps().
**           - Autostraighten no longer inadvertently turned on
**             when debugging.
**
** v1.40     4-3-2012
**           - This is probably my most substantial update so far.
**             I did a re-write of many parts of the code and
**             consequently have spent many hours doing regression
**             testing.
**           - Major new features:
**             * Does true word wrap (brings words up from the
**               next line if necessary).
**             * Preserves indentation, justification, and vertical
**               spacing more faithfully.  Overall, particularly for
**               cases with text wrapping, I think the output looks
**               much better.
**             * Ignores defects in scanned documents.
**             * Compiled with all of the very latest third party
**               libraries, including mupdf 0.9.
**             * v1.40 is about 4% faster than v1.35 on average
**               (PC x64 version).
**           - New justification command-line option is:
**                 -j [-1|0|1|2][+/-]
**             Using -1 tells k2pdfopt to use the document's own
**             justification.  A + after will attempt to fully
**             justify the text.  A - will force no full justification.
**             Nothing after the number will attempt to determine
**             whether or not to use full justification based on
**             if the source document is fully justified.
**           - The default defect size to ignore in scanned documents
**             is a specified user size (default is 1 point).  The
**             command-line option is -de (user menu option "de").
**           - Command line options -vls, -vb, and -vs control
**             vertical spacing, breaks, and gaps.  They are all
**             under the interactive user menu under "v".
**           - Line spacing is controlled by -vls.
**             Example:  -vls -1.2 (the default) will preserve
**             the default document line spacing up to 1.2 x
**             single-spaced.  If line spacing exceeds 1.2 x in the
**             source document, the lines are spaced at 1.2 x.
**             The negative value (-1.2) tells k2pdfopt to use it
**             as a limit rather than forcing the spacing to be 
**             exactly 1.2 x.  A positive value, on the other hand,
**             forces the spacing.  E.g. -vls 2.0 will force line
**             spacing to be double-spaced.
**           - Regions are broken up vertically using the new -vb
**             option.  It defaults to 2 which breaks up regions
**             separated by gap 2 X larger than the median line gap.
**             For behavior more like v1.35, or to not break up the
**             document into vertical regions, use -vb -1.  Vertical
**             breaks between regions are shown with green lines when
**             using -sm.
**           - The new -vs option sets the maximum gap between regions
**             in the source document before they are truncated.
**             Default is -vs 0.25 (inches).
**           - Added menu option for -cg under "co".
**           - Reduced default min column gap from 0.125 to 0.1 inches.
**           - The -ws (word spacing threshold) value is now specified
**             as a fraction of the lowercase letter height (e.g. a
**             small 'o').  The new default is 0.375.
**
** v1.35     2-15-2012
**           - Changed how the columns in a PDF file are interpreted
**             when the column divider moves around some.  The column
**             divider is now allowed to move around on the page
**             but still have the columns be considered contiguous.
**             This is controlled by the -comax option.  Use
**             -comax -1 to revert to v1.34 and before.  The
**             default is -comax 0.2.  See example at:
**             http://willus.com/k2pdfopt/help/column_divider.shtml
**           - Added nice debugging tool with the -sm command-line
**             option ("sm" on interactive menu) which shows marked
**             source pages so you can clearly see how k2pdfopt
**             is interpreting your PDF file and what affect the
**             options are having.
**           - The last line in a paragraph, if shorter than the
**             other lines significantly, will be split differently
**             and not fully justified.
**           - Modified the column search function to better find
**             optimal gaps.
**           - The height of a multi-column region is calculated
**             more correctly now (does not include blank space,
**             and both columns must exceed the minimum height
**             requirement).
**           - Text immediately after a large rectangular block
**             (typically a figure) will not be wrapped, since it
**             is often the axis labels for the figure.
**           - Fixed array-out-of-bounds bug in 
**             bmpregion_wrap_and_add().
**           - colcount and rowcount allocated only once per page.
**
** v1.34a    12-30-2011
**           - Some build corrections after the first release of
**             v1.34 which had issues in Linux and Windows.
**           - Fixed interpretation of -jpg flag when it's the last
**             command-line option specified.
**
** v1.34     12-30-2011
**           - I've collected enough bug reports and new feature
**             requests that I decided to do an update.
**           - Added -cgr and -crgh options to give more control
**             over how k2pdfopt selects multi-column regions.
**           - Don't switch to Ghostscript on DJVU docs.
**           - Continues processing files even if has an error on
**             one page.
**           - Fixed bug in orientation detection (minimum returned
**             value is now 0.01 so as not to kill the average).
**           - Added document scale factor (-ds or "ds" in menu)
**             which allows users to correct PDF docs that are the
**             wrong size (e.g. if your PDF reader says your
**             document is 17 x 22 inches when it should be
**             8.5 x 11, use -ds 0.5).
**           - Fixed bug in break_point() where bp1 and bp2 did not
**             get initialized correctly.
**
** v1.33     11-11-2011
**           - Added autodetection of the orientation of the PDF
**             file.  This is somewhat experimental and comes with
**             several caveats, but I have made it the default
**             because I think it works pretty well.
**             Caveat #1:  It assumes the PDF/DJVU file is mostly
**             lines of text and looks for regularly spaced lines
**             of text to determine the orientation.
**             Caveat #2:  If it determines that the page is
**             sideways, it rotates it 90 degrees clockwise, so it
**             may end up upside down.
**           - The autodetection is set with the -rt command-line
**             option (or the "rt" menu option):
**             1. Set it to a number to rotate your PDF/DJVU file
**                that many degrees counter-clockwise.
**             2. Set it to "auto" and k2pdfopt will examine up
**                to 10 pages of the file to determine the
**                orientation it will use.
**             3. Set it to "aep" to auto-detect the rotation of
**                every page.  If you have different pages that
**                are rotated differently from each other within
**                one file, you can use this option to try to
**                auto-rotate each page.
**             4. To revert to v1.32 and turn off the orientation
**                detection, just put -rt 0 on the command line.
**           - Added option to attempt full justification when
**             breaking lines of text.  This is experimental and
**             will only work well if the output dpi is chosen so
**             that rows break approximately evenly.  To turn on,
**             use the "j" option in the interactive menu or the
**             -j command-line option with a + after the selection,
**             e.g.
**                 -j 0+  (left/full justification)
**                 -j 1+  (center/full justification)
**                 -j 2+  (right/full justification)
**
** v1.32     10-25-2011
**           - Make sure locale is set so that decimal marker is
**             a period for numbers.  This was causing problems
**             in locales where the decimal marker is a comma,
**             resulting in unreadable PDF output files.  This
**             was introduced by having to compile for the DJVU
**             library in v1.31.
**           - Slightly modified compile of DJVU lib (re: locale).
**           - Remove "cd" option from interactive menu (it was
**             obsoleted in v1.27).
**           - Warn user if source bitmap is excessively large.
**           - Print more info in header (compiler, O/S, chip).
**
** v1.31     10-17-2011
**           - Now able to read DJVU (.djvu) files using ddjvuapi
**             from djvulibre v3.5.24.  All output is still PDF.
**           - Now offer generic i386 versions for Win and Linux
**             which are more compatible w/older CPUs, and fixed
**             issue with MuPDF so it doesn't crash on older CPUs
**             when compiled w/my version of MinGW gcc.
**
** v1.30     10-4-2011
**           - Just after I posted v1.29, I found a bug I'd introduced
**             in v1.27 where k2pdfopt didn't quit when you typed 'q'.
**             I fixed that.
**           - Made user menu a little smarter--allows different
**             entries depending on whether a source file has already
**             been specified.
**
** v1.29     10-4-2011
**           - Input file dpi now defaults to twice the output dpi.
**             (See -idpi option.)
**           - Added option to break input pages at the end of each
**             output page.  ("Break pages" in menu or -bp option.)
**           - Set dpi minimums to 50 for input and 20 for output.
**
** v1.28     10-1-2011
**           - Fixed bug that was causing vertical stripes to show
**             up on Mac and Linux version output.
**           - OSX 64-bit version now available.
**
** v1.27     9-25-2011
**           - Changed default max columns to two.  There were
**             too many cases of false detection of sub-columns.
**             Use -col 4 to detect up to 4 columns (or select
**             the "co" option in the user menu).
**           - The environment variable K2PDFOPT now can be
**             use to supply default command-line options.  It
**             replaces all previous environment variables,
**             which are now ignored.  The options on the
**             command line override the options in K2PDFOPT.
**           - Added -rt ("rt" in menu) option to rotate the source
**             pages by 90 (or 180 or 270) degrees if desired.
**           - Default startup is now to show the user menu rather
**             than command line usage.  Type '?' for command line
**             usage or use the -? command line option to see usage.
**           - Added three new "expert-mode" options for controlling
**             detection of gaps between columns, rows, and words:
**             -gtc, -gtr, -gtw.  The -gtc option replaces
**             the -cd option from v1.26.  These can all be set
**             with the "gt" menu option.  Use the "u" option for
**             more info (to see usage).
**           - In conjunction with the new "expert-mode" options,
**             I adjusted how gaps between columns, rows, and words
**             are detected and adjusted the defaults to hopefully
**             be more robust.
**           - You can now enter all four margin settings (left,
**             top, right, bottom) from the user input menu for
**             "m" and "om".
**           - Added -x option to get k2pdfopt to exit without asking
**             you to press <Enter> first.
**
** v1.26     9-18-2011
**           - Added column detection threshold input (-cd).  Set
**             higher to make it easier to detect multiple columns.
**           - Adjusted the default column detection to make column
**             detection a bit easier on scanned docs with
**             imperfections.
**
** v1.25     9-16-2011
**           - Smarter detection of number of TTY rows.
**
** v1.24     9-12-2011
**           - Input on user menu fixed not to truncate file names
**             longer than 32 chars for Mac and Linux.
**              
** v1.23     9-11-2011
**           - Added right-to-left (-r) option for scanning pages.
**
** v1.22     9-10-2011
**           - First version compiled under Mac OS X.
**           - Made some changes to run on OS X.  Kludgey, but works.
**             You have to double-click the icon and then drag a file
**             to the display window and press <Enter>.  I've made
**             linux work similarly.
**           - Since Mac and Linux shells default to black on white,
**             I've made the the text colors more friendly to that
**             scheme for linux and Mac.  Use -a- to turn off text
**             coloring altogether, or set the env variable
**             K2PDFOPT_NO_TEXT_COLORING.
**           - Re-vamped the print out of the cmd-line options some.
**
** v1.21     9-7-2011
**           - Moved some bmp functions to standard library.
**           - JPEG images always done at 8 bpc (no dithering).
**           - Fixed dithering of 1-bit-per-colorplane images.
**
** v1.20     9-2-2011
**           - Added dithering for bpc < 8.  Use -d- to turn off.
**           - Adjusted gamma correction algorithm slightly (so that
**             pure white stays pure white).
**
** v1.19     9-2-2011
**           - Added gamma adjust.  Setting to a value lower than 1.0
**             will darken the font some and appear to thicken it up.
**             Default is 0.5. Thanks to PaperCrop for the idea.
**           - Interactive menu now uses letters for the options.
**             This should keep the option choices the same even if
**             I add new ones, and now the user can enter a page range
**             as the final entry.
**
** v1.18     8-30-2011
**           - break_point() function now uses same white threshold
**             as all other functions.
**           - Added "-wt" option to manually specify "white threshold"
**             value above which all pixels are considered white.
**           - Tweaked the contrast adjustment algorithm and changed
**             the max to 2.0 (was much higher).
**           - Added "-cmax" option to limit contrast adjustment.
**
** v1.17     8-29-2011
**           - Min region width now 1.0 inches.  Bug fixed when
**             output dpi set too large--it is now reduced so that
**             the output display has at least 1-inch of display.
**
** v1.16     8-29-2011
**           - Now queries user for options when run (just press
**             <Enter> to go ahead with the conversion).
**             Use -ui- to disable this (it is automatically disabled
**             when run from the command line in Windows).
**           - Fixed bug in MuPDF calling sequence that results in
**             more robust reading of PDF files. (Fixes the parsing
**             of the second two-column example on my web page.)
**           - Fixed bug in MuPDF library that prevented it from
**             correctly parsing encrypted sections in PDF files.
**             (This bug is not in the 0.8.165 tarball but it
**              was in the version that I got via "git".)
**             This only affected a small number of PDF files.
**           - New landscape mode (not the default) is enabled
**             with the -ls option.  This turns the output sideways
**             on the kindle, resulting in a more magnified display
**             for typical 2-column files.  Thanks to Taesoo Kwon
**             for this idea.
**           - Default PDF output is now much smaller--about half
**             the original size.  This is because the bitmaps are
**             saved with 4 bits per colorplane (same as the Kindle).
**             You can set this to 1, 2, 4, or 8 with the -bpc option.
**             Thanks to Taesoo Kwon and PaperCrop for this idea.
**           - Default -m value is now 0.25 inches (was 0.03 inches).
**             This ignores anything within 0.25 inches of the edge
**             of the source page.
**           - Now uses precise Kindle 2 (and 3?) display resolution
**             by default.  Thanks to the PaperCrop forum for pointing
**             out that Shift-ALT-G saves screenshot on Kindle.
**             The kindle is a weird beast, though--after lots of
**             testing, I figured out that I have to do the
**             following to get it to display the bitmaps with
**             a 1:1 mapping to the Kindle's 560 x 735 resolution:
**                 (a) Make the actual bitmap in the PDF file 
**                     563 x 739 and don't use the excess pixels.
**                     I.e. pad the output bitmap with 3 extra
**                     columns and 4 extra rows.
**                 (b) Put black dots in the corners at the 560x735
**                     locations, otherwise the kindle will scale
**                     the bitmap to fit its screen.
**             This is accomplished with the new -pr (pad right), -pb
**             (pad bottom), and -mc (mark corners) options.  The
**             defaults are -pr 3 -pb 4 -mc. 
**           - New -as option will attempt to automatically straighten
**             source pages.  This is not on by default since it slows
**             down the conversion and is somewhat experimental, but I've
**             found it to be pretty reliable and it is good to use on
**             scanned PDFs that are a bit tilted since the pages need
**             to be straight to accurately detect cropping regions.
**           - Reads 8-bit grayscale directly from PDF now for faster
**             processing (unless -c is specified for full color).
**           - Individual bitmaps created only in debug mode.
**             k2_src_dir and k2_dst_dir folders no longer needed.
**
** v1.15     8-3-2011
**           - Substantial code re-write, mostly to clean things up
**             internally.  Hopefully won't introduce too many bugs!
**           - Can handle up to 4 columns now (see -col option).
**           - Added -c for full color output.
**           - If column width is close too destination screen width,
**             the column is fit to the device.  Controlled with -fc
**             option.
**           - Optimized much of code for 8-bit grayscale bitmaps--
**             up to 50% faster than v1.14.
**           - Added -wrap- option to disable text wrapping.
**           - Can convert specific pages now--see -p option.
**           - Added margin ignoring options:  -m, -ml, -mr, -mt, -mb.
**           - Added options for margins on the destination device:
**                 -om, -oml, -omr, -omt, -omb.
**           - Min column gap now 0.125 inches and min column height
**             now 1.5 inches.  Options -cg and -ch added to control
**             this.
**           - Min word spacing now 0.25.  See -ws option.
**
** v1.14     7-26-2011
**           - Smarter line wrapping and text sizing based on custom options.
**             (e.g. should work better for any size destination screen
**              --not just 6-inch.)
**           - Bug fix.  -w option fixed.
**           - First page text doesn't butt right up against top of page.
**
** v1.13     7-25-2011
**           - Added more command-line options:  justification, encoding
**             type, source and destination dpi, destination width
**             and height, and source margin width to ignore.
**             Use -ui to turn on user input query.
**           - Now applies a sharpening algorithm to the output images
**             (can be turned off w/command-line option).
**
** v1.12     7-20-2011
**           - Fixed a bug in the PDF output that was ignored by some readers
**             (including the kindle itself), but not by Adobe's reader.
**             PDF files should be readable by all software now.
**
** v1.11     7-5-2011
**           - Doesn't put "Press <ENTER> to exit." if launched as a
**             command in a console window (in Windows).  No change to
**             Linux version.
**
** v1.10     7-2-2011
**           - Integrated with mupdf 0.8.165 so that Ghostscript is
**             no longer required!  Ghostscript can still be used/
**             will be tried if mupdf fails to decypher the pdf file.
**           - PDF page number count now much more reliable.
**
** v1.07     7-1-2011
**           - Fixed bugs in the pdf writing that were making the
**             pdf files incompatible with the kindle.
**           - Compiled w/gcc 4.5.2.
**           - Added smarter determination of # of PDF pages in source,
**             though it doesn't always work on newer PDF formats.
**             This can cause an issue with the win32 version because
**             calling Ghostscript on a page number beyond what is in
**             the PDF file seems to sometimes result in an exception.
**
** v1.06     6-23-2011
**           - k2pdfopt now first tries to find Ghostscript using the registry
**             (Windows only).  If not found, searches path and common folders.
**           - Compiled w/turbo jpeg lib 1.1.1, libpng 1.5.2, and zlib 1.2.5.
**           - Correctly sources single bitmap files.
**
** v1.05     6-22-2011
**           Fixed bug in routine that looks for Ghostscript.
**           Also, Win64 version now looks for gsdll64.dll/gswin64c.exe
**           before gsdll32.dll/gswin32c.exe.
**
** v1.04     6-6-2011
**           No longer requires imagemagick's convert program.
**
** v1.03     3-29-2010
**           Made some minor mods for Linux compatibility.
**
** v1.02     3-28-2010
**           Changed rules for two-column detection to hopefully avoid
**           false detection.  At least 0.1 inches must separate columns.
**
** v1.01     3-22-2010
**           Fixed some bugs with file names having spaces in them.
**           Added program icon.  Cleaned up screen output some.
**
** v1.00     3-20-2010
**           First released version.  Auto adjusts contrast, clears
**           edges.
**
*/

/*
** WILLUSDEBUGX flags:
** 1 = Generic
** 2 = breakinfo row analysis
** 4 = word wrapping
** 8 = word wrapping II
** 16 = hyphens
** 32 = OCR
**
*/
// #define WILLUSDEBUGX 32
// #define WILLUSDEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math2.h>
#include <constant.h>
#include <willus.h>

/* Undefine HAVE_GOCR and HAVE_TESSERACT to turn off OCR completely */
#define HAVE_GOCR
#define HAVE_TESSERACT

#if (defined(HAVE_GOCR) || defined(HAVE_TESSERACT))
#if (!defined(HAVE_OCR))
#define HAVE_OCR
#endif
#else
#if (defined(HAVE_OCR))
#undef HAVE_OCR
#endif
#endif


#define VERSION "v1.50"
#define GRAYLEVEL(r,g,b) ((int)(((r)*0.3+(g)*0.59+(b)*0.11)*1.002))
#if (defined(WIN32) || defined(WIN64))
#define TTEXT_BOLD    ANSI_WHITE
#define TTEXT_NORMAL  ANSI_NORMAL
#define TTEXT_BOLD2   ANSI_YELLOW
#define TTEXT_INPUT   ANSI_GREEN
#define TTEXT_WARN    ANSI_RED
#define TTEXT_HEADER  ANSI_CYAN
#define TTEXT_MAGENTA ANSI_MAGENTA
#else
#define TTEXT_BOLD    "\x1b[0m\x1b[34m"
#define TTEXT_NORMAL  "\x1b[0m"
#define TTEXT_BOLD2   "\x1b[0m\x1b[33m"
#define TTEXT_INPUT   "\x1b[0m\x1b[32m"
#define TTEXT_WARN    "\x1b[0m\x1b[31m"
#define TTEXT_HEADER  "\x1b[0m\x1b[36m"
#define TTEXT_MAGENTA "\x1b[0m\x1b[35m"
#endif

#define SRC_TYPE_PDF     1
#define SRC_TYPE_DJVU    2
#define SRC_TYPE_OTHER   3

/* DATA STRUCTURES */

typedef struct
    {
    int page;        /* Source page */
    double rot_deg;  /* Source rotation (happens first) */
    double x0,y0;    /* x0,y0, in points, of lower left point on rectangle */
    double w,h;      /* width and height of rectangle in points */
    double scale;    /* Scale rectangle by this factor on destination page */
    double x1,y1;    /* (x,y) position of lower left point on destination page, in points */
    } PDFBOX;

typedef struct
    {
    PDFBOX *box;
    int n;
    int na;
    } PDFBOXES;

typedef struct
    {
    int pageno;          /* Source page number */
    double page_rot_deg; /* Source page rotation */
    PDFBOXES boxes;
    } PAGEINFO;

typedef struct
    {
    int ch;    /* Hyphen starting point -- < 0 for no hyphen */
    int c2;    /* End of end region if hyphen is erased */
    int r1;    /* Top of hyphen */
    int r2;    /* Bottom of hyphen */
    } HYPHENINFO;

typedef struct
    {
    int c1,c2;   /* Left and right columns */
    int r1,r2;   /* Top and bottom of region in pixels */
    int rowbase; /* Baseline of row */
    int gap;     /* Gap to next region in pixels */
    int rowheight; /* text + gap */
    int capheight;
    int h5050;
    int lcheight;
    HYPHENINFO hyphen;
    } TEXTROW;

typedef struct
    {
    TEXTROW *textrow;
    int rhmean_pixels;  /* Mean row height (text) */
    int centered;       /* Is this set of rows centered? */
    int n,na;
    } BREAKINFO;

typedef struct
    {
    int r1,r2;     /* row position from top of bmp, inclusive */
    int c1,c2;     /* column positions, inclusive */
    int rowbase;   /* Baseline of text row */
    int capheight; /* capital letter height */
    int h5050;
    int lcheight;  /* lower-case letter height */
    int bgcolor;   /* 0 - 255 */
    HYPHENINFO hyphen;
    WILLUSBITMAP *bmp;
    WILLUSBITMAP *bmp8;
    WILLUSBITMAP *marked;
    } BMPREGION;

typedef struct
    {
    WILLUSBITMAP bmp;
    int rows;
    int published_pages;
    int bgcolor;
    int fit_to_page;
    int wordcount;
    char debugfolder[256];
    } MASTERINFO;

static int verbose=0;
static int debug=0;

static char *usageintro=
"usage:  k2pdfopt [opts] <input pdf/djvu | folder>\n\n"
"(Or just drag a PDF or DJVU (.djvu) file to this icon.)\n\n"
"Attempts to optimize PDF (or DJVU) files (especially two-column ones) for\n"
"display on the Kindle (or other mobile readers/smartphones) by looking for\n"
"rectangular regions in the file and re-paginating them without margins and\n"
"excess white space.  Works on any PDF or DJVU (.djvu) file, but assumes it\n"
"has a mostly-white background.  Native PDF files (not scanned) work best.\n\n"
"If given a folder, k2pdfopt first looks for bitmaps in the folder and if\n"
"any are found, converts those bitmaps to a PDF as if they were pages of a\n"
"PDF file.  If there are no bitmaps in the folder and if PDF files are in\n"
"the folder, then each PDF file will be converted in sequence.\n\n"
"Output files are always .pdf and have _k2opt added to the source name.\n\n";

static char *usageenv=
"K2PDFOPT environment variable\n"
"-----------------------------\n"
"You can supply command-line options via the environment variable K2PDFOPT,\n"
"for example,\n\n"
"     set K2PDFOPT=-ui- -x -j 0 -m 0.25\n\n"
"Command line options from the command line take precedence over the ones in\n"
"the environment variable K2PDFOPT.\n\n";

static char *k2pdfopt_options=
"-?[-]             Show [don't show] usage only (no file processing).\n"
"                  Combine with -ui- to get something you can redirect\n"
"                  to a file.\n"
"-a[-]             Turn on [off] text coloring (use of ANSI color codes) on\n"
"                  the screen output.  Default is on.\n"
/*
"-arlim <ar>       Set aspect ratio limit to avoid wrapping.\n"
*/
"-as [<maxdeg>]    Attempt to automatically straighten tilted source pages.\n"
"                  Will rotate up to +/-<maxdegrees> degrees if a value is\n"
"                  specified, otherwise defaults to 4 degrees max.  Use -1 to\n"
"                  turn off. Default is off (-as -1).\n"
"-bp[-]            Break [do not break] output pages at end of each input page.\n"
"                  Default is -bp-.\n\n"
"-bpc <nn>         Set the bits per color plane on the output device to <nn>.\n"
"                  The value of <nn> can be 1, 2, 4, or 8.  The default is 4\n"
"                  to match the kindle's display capability.\n"
"-c[-]             Output in color [grayscale].  Default is grayscale.\n"
/*
"-cd <threshold>   Set column detection threshold.  Default = 0.01.  Range\n"
"                  is 0 to 100.  Higher makes it easier to detect columns.\n"
"                  If PDF is scanned and speckled, might set to .02 or .03.\n"
*/
"-col <maxcol>     Set max number of columns.  <maxcol> can be 1, 2, or 4.\n"
"                  Default is -col 4.  -col 1 disables column searching.\n"
"-cg <inches>      Minimum column gap width in inches for detecting multiple\n"
"                  columns.  Default = 0.1 inches.  Setting this too large\n"
"                  will give very poor results for multicolumn files.\n"
"-cgr <range>      Set column-gap range, 0 - 1.  This is the horizontal range\n"
"                  over which k2pdfopt will search for a column gap, as a\n"
"                  fraction of the page width.  E.g. -cgr 0.5 will search\n"
"                  from 0.25 to 0.75 of the page width for a column gap.\n"
"                  Set this to a small value, e.g. 0.05, to only search for\n"
"                  column breaks in the middle of the page.  Default = 0.33.\n"
"-ch <inches>      Minimum column height in inches for detecting multiple\n"
"                  columns.  Default = 1.5 inches.\n"
"-cmax <max>       Set max contrast increase on source pages.  1.0 keeps\n"
"                  contrast from being adjusted.  Use a negative value to\n"
"                  specify a fixed contrast adjustment.  Def = 2.0.\n"
"-comax <range>    Stands for Column Offset Maximum.  The <range> given is as a\n"
"                  fraction of the total horizontal 2-column span, as with -cgr,\n"
"                  and it specifies how much the column divider can move around\n"
"                  and still have the columns considered contiguous.  Set to -1\n"
"                  to revert back to how columns were treated in k2pdfopt v1.34\n"
"                  and before.  Default = 0.2.\n"
"-crgh <inches>    Set the min height of the blank area that separates regions\n"
"                  with different numbers of columns.  Default = 1/72 inch.\n"
"-d[-]             Turn on [off] dithering for bpc values < 8.  See -bpc.\n"
"                  Default is on.\n"
"-de <size>        Defect size in points.  For scanned documents, marks\n"
"                  or defects smaller than this size are ignored when bounding\n"
"                  rectangular regions.  The period at the end of a sentence is\n"
"                  typically over 1 point in size.  The default is 1.0.\n"
"-ds <factor>      Override the document size with a scale factor.  E.g. if\n"
"                  your PDF reader says the PDF file is 17 x 22 inches and\n"
"                  it should actually be 8.5 x 11 inches, use -ds 0.5.  Default\n"
"                  is 1.0.\n"
/* "-debug [<n>]      Set debug mode to <n> (def = 1).\n" */
"-evl <n>          Detects and erases vertical lines in the source document\n"
"                  which may be keeping k2pdfopt from correctly separating\n"
"                  columns or wrapping text, e.g. column dividers.  If <n> is\n"
"                  zero, this is turned off (the default).  If <n> is 1, only\n"
"                  free-standing vertical lines are removed.  If <n> is 2,\n"
"                  vertical lines are erased even if they are the sides of\n"
"                  an enclosed rectangle or figure, for example.\n"
"-f2p <val>        Fit-to-page option.  The quantity <val> controls fitting\n"
"                  tall contiguous objects (like figures or photographs) to\n"
"                  the device screen.  Normally these are fit to the width\n"
"                  of the device.  If <val>=10, for example, they are allowed\n"
"                  to be 10%% narrower than the screen in order to fit\n"
"                  vertically.  Use -1 to fit the object no matter what.\n"
"                  Use -2 as a special case--all \"red-boxed\" regions (see\n"
"                  -sm option) are placed one per page. Default is -f2p 0.\n"
"-fc[-]            For multiple column documents, fit [don't fit] columns to\n"
"                  the width of the reader screen regardless of -odpi.\n"
"                  Default is to fit the columns to the reader.\n"
"-g <gamma>        Set gamma value of output bitmaps. A value less than 1.0\n"
"                  makes the page darker and may make the font more readable.\n"
"                  Default is 0.5.\n"
"-gtc <inches>     Threshold for detecting column gaps (expert mode).\n"
"                  See -gtr.  Default = .005.\n"
"-gtcmax <inches>  Max allowed gap between columns (expert mode).  See -gtc,\n"
"                  -gtr.  Default = 1.5.  Use -1 for no limit.\n"
/*
"-gtm <inches>     Threshold for trimming excess margins (xpert mode).\n"
"                  See -gtr.  Default = .005.\n"
*/
"-gtr <inches>     Threshold for detecting gaps between rows (expert mode).\n"
"                  This sets the maximum total black pixels, in inches, on\n"
"                  average, that can be in each row of pixels before the gap is\n"
"                  no longer considered a gap.  A higher value makes it easier\n"
"                  to detect gaps between rows of text.  Too high of a value\n"
"                  may inadvertently split figures and other graphics.\n"
"                  Default = 0.006.\n"
"-gtw <inches>     Threshold for detecting word gaps (expert mode).\n"
"                  See -gtr.  Default = .0015.\n"
"-h <height>       Set height of output in pixels (def=735).  See -hq.\n"
"-hq               Higher quality (convert source to higher res bitmaps).\n"
"                  Equivalent to -idpi 400 -odpi 333 -w 1120 -h 1470.\n"
"-hy[-]            Turn on [off] hyphen detection/elimination when wrapping\n"
"                  text.  Default is on.\n"
"-gs[-]            Force use of Ghostscript instead of MuPDF to read PDFs.\n"
"                  K2pdfopt has built-in PDF translation (via the MuPDF\n"
"                  library) but will try to use Ghostscript if Ghostscript\n"
"                  is available and the internal (MuPDF) translation fails\n"
"                  (virtually never happens).  You can force Ghostscript to\n"
"                  be used with this -gs option.  Use -gs- to turn off.\n"
"                  Download ghostscript at http://www.ghostscript.com.\n"
"-idpi <dpi>       Set pixels per inch for input file.  Use a negative value\n"
"                  as a multiplier on the output dpi (e.g. -2 will set the\n"
"                  input file dpi to twice the output file dpi (see -odpi).\n"
"                  Default is -2.0.\n"
"-j [-1|0|1|2][+/-] Set output text justification.  0 = left, 1 = center,\n"
"                  2 = right.  Add a + to attempt full justification or a -\n"
"                  to explicitly turn it off.  The default is -1, which tells\n"
"                  k2pdfopt to try and maintain the justification of the\n"
"                  document as it is.  See also -wrap.\n"
"-jpg [<quality>]  Use JPEG compression in PDF file with quality level\n"
"                  <quality> (def=90).  A lower quality value will make your\n"
"                  file smaller.  See also -png.\n"
"-ls[-]            Set output to be in landscape [portrait] mode.  The\n"
"                  default is portrait.\n"
"-m[b|l|r|t] <in>  Ignore <in> inches around the [bottom|left|right|top]\n"
"                  margin[s] of the source file.  Default = 0.25 inches.\n"
"                  E.g. -m 0.5 (set all margins to 0.5 inches)\n"
"                       -mb 0.75 (set bottom margin to 0.75 inches)\n"
"-mc[-]            Mark [don't mark] corners of the output bitmaps with a\n"
"                  small dot to prevent the reading device from re-scaling.\n"
"                  Default = mark.\n"
"-o[-] [<mb>]      Set the minimum file size (in MB) where overwriting the\n"
"                  file will not be done without prompting.  Set to -1 (or\n"
"                  just -o with no value) to overwrite all files with no\n"
"                  prompting.  Set to 0 (or just -o-) to prompt for any\n"
"                  overwritten file.  Def = -o 10 (any existing file\n"
"                  over 10 MB will not be overwritten without prompting).\n"
"-om[b|l|r|t] <in> Set [bottom|left|right|top] margin[s] on output device in\n"
"                  inches.  Default = 0.02 inches.\n"
"                  E.g. -om 0.25 (set all margins on device to 0.25 inches)\n"
"                       -omb 0.4 (set bottom margin on device to 0.4 inches)\n"
#ifdef HAVE_OCR
"-ocr[-] [g|t]     Attempt [don't attempt] to use optical character\n"
"                  recognition (OCR) in order to embed searchable text into\n"
"                  the output PDF document.  If followed by t or g, specifies\n"
"                  the ocr engine to use (tesseract or gocr).  Default if not\n"
"                  specified is tesseract.  See also -wc and -ocrhmax.\n"
"-ocrhmax <in>     Set max height for an OCR'd word in inches.  Any graphic\n"
"                  exceeding this height will not be processed with the OCR\n"
"                  engine.  Default = 1.5.  See -ocr.\n"
#endif
"-odpi <dpi>       Set pixels per inch of output screen (def=167). See also\n"
"                  -fc and -hq.\n"
"-p <pagelist>     Specify pages to convert.  <pagelist> must not have any\n"
"                  spaces.  E.g. -p 1-3,5,9,10- would do pages 1 through 3,\n"
"                  page 5, page 9, and pages 10 through the end.\n"
"-p[b|l|r|t] <nn>  Pad [bottom|left|right|top] side of destination bitmap with\n"
"                  <nn> rows.  Defaults = 4 (bottom), 0 (left), 3 (right), and\n"
"                  0 (top).  Example:  -pb 10.  This is typically only used on\n"
"                  certain devices to get the page to come out just right.  For\n"
"                  setting margins on the output device, use -om.\n"
/*
"-pi[-]            Preserve [don't preserve] indentation when wrapping text,\n"
"                  e.g. if the first line of each paragraph is indented, keep\n"
"                  it that way.  The default is to ignore indentation.  Also,\n"
"                  this is only used with left justification turned on (-j 0).\n"
*/
"-png              (Default) Use PNG compression in PDF file.  See also -jpeg.\n"
"-r[-]             Right-to-left [left-to-right] page scans.  Default is\n"
"                  left to right.\n"
"-rt <deg>|auto|aep  Rotate source page counter clockwise by <deg> degrees.\n"
"                  Can be 90, 180, 270.  Or use \"-rt auto\" to examine up to\n"
"                  10 pages of each file to determine the orientation used\n"
"                  on the entire file (this is the default).  Or use \"-rt aep\"\n"
"                  to auto-detect the rotation of every page.  If you have\n"
"                  different pages that are rotated differently from each other\n"
"                  within one file, you can use this option to try to auto-\n"
"                  rotate each source page.\n"
/*
"-rwmin <min>      Set min row width before the row can be considered for\n"
"                  glueing to other rows (inches).\n"
*/
"-s[-]             Sharpen [don't sharpen] images.  Default is to sharpen.\n"
"-sm[-]            Show [don't show] marked source.  This is a debugging tool\n"
"                  where k2pdfopt will mark the source file with the regions it\n"
"                  finds on them and the order in which it processes them and\n"
"                  save it as <srcfile>_marked.pdf.  Default is not to show\n"
"                  marked source.\n"
"-ui[-]            User input query turned on [off].  Default = on for linux or\n"
"                  if not run from command line in Windows.\n"
"-v                Verbose output.\n"
"-vb <thresh>      Set gap-size vertical-break threshold between regions that\n"
"                  cause them to be treated as separate regions.  E.g. -vb 2\n"
"                  will break regions anywhere the vertical gap between them\n"
"                  exceeds 2 times the median gap between lines.  Use -vb -1\n"
"                  to disallow region breaking.  Default is -vb 1.75.\n"
/*
"-vm <mult>        Vertical spacing multiplier.  Reduces gaps and line spacings\n"
"                  in the document using the multiplier <mult>.\n"
"                  E.g. for -vm 0.9, text lines would be spaced at 90% of their\n"
"                  original spacing.  This is applied before -vs.\n"
"                  Default value is 1.0.  See also -vs.\n"
*/
"-vls <spacing>    Set vertical line spacing as a fraction of the text size.\n"
"                  This can be used to override the line spacing in a document.\n"
"                  If 1, then single spacing is used.  2 = double spacing.\n"
"                  If negative, then the absolute value acts as the limiting\n"
"                  case.  E.g., if you set -vls -1.5, then any the line\n"
"                  spacing of the original document is preserved unless it\n"
"                  exceeds 1.5 (times single spacing).  Default = -1.2.\n"
"                  See also -vs.\n"
"-vs <maxgap>      Preserve up to <maxgap> inches of vertical spacing between\n"
"                  regions in the document.  Default value is 0.25.  See also\n"
"                  -vls.\n"
"-w <width>        Set width of output in pixels (def=560). See -hq.\n"
#ifdef HAVE_OCR
"-wc <color>       Set OCR word color flags.  The <color> value is a sum of\n"
"                  powers of 2:  1=make OCR text visible; 2=make bitmapped text\n"
"                  invisible, 4=put a box around the OCR'd words.  So use 3 to\n"
"                  see only the OCR text.  Or 5 to see everything.  Default=0.\n"
"                  See -ocr.\n"
#endif
"-wrap[-|+]        Enable [disable] text wrapping.  Default = enabled.  If\n"
"                  -wrap+, regions of text with lines shorter than the mobile\n"
"                  device screen are re-flowed to fit the screen width.  If\n"
"                  you use -wrap+, you may want to also specify -fc- so that\n"
"                  narrow columns of text are not magnified to fit your device.\n"
"                  See also -ws, -j, -pi, -fc.\n"
/*
"-whmax <height>   Max height allowed for wrapping a row (inches).\n"
*/
"-ws <spacing>     Set minimum word spacing for line breaking as a fraction of\n"
"                  the height of a lowercase 'o'.  Use a larger value to make it\n"
"                  harder to break lines.  Def = 0.375.  See also -wrap.\n"
"-wt <whitethresh> Any pixels whiter than <whitethresh> (0-255) are made pure\n"
"                  white.  Setting this lower can help k2pdfopt better process\n"
"                  some poorly-quality scanned pages.  The default is -1, which\n"
"                  tells k2pdfopt to pick the optimum value.  See also -cmax.\n"
"-x[-]             Exit [don't exit--wait for <Enter>] after completion.\n";

#define DEFAULT_WIDTH 560
#define DEFAULT_HEIGHT 735
#define MIN_REGION_WIDTH_INCHES 1.0
#define SRCROT_AUTO   -999.
#define SRCROT_AUTOEP -998.

static PDFFILE _gpdf,*gpdf;
static PDFFILE _mpdf,*mpdf;
static char uifile[512];
static double cdthresh=.01;
/*
** Blank Area Threshold Widths--average black pixel width, in inches, that
** prevents a region from being determined as "blank" or clear.
*/
static int src_rot=SRCROT_AUTO;
static double gtc_in=.005; // detecting gap between columns
static double gtcmax_in=1.5; // max gap between columns
static double gtr_in=.006; // detecting gap between rows
static double gtw_in=.0015; // detecting gap between words
// static double gtm_in=.005; // detecting margins for trimming
static int show_usage=0;
static int tty_rows=25;
static int src_left_to_right=1;
static int src_whitethresh=-1;
#ifdef HAVE_OCR
static int dst_ocr=0;
static int dst_ocr_wordcolor=0;
static double ocr_max_height_inches=1.5;
#ifdef HAVE_TESSERACT
static int ocrtess_status=0;
#endif
static OCRWORDS *dst_ocrwords=NULL;
static OCRWORDS _dst_ocrwords;
#endif
static int dst_dpi=167;
static int dst_dither=1;
static int dst_break_pages=0;
static int render_dpi=167;
static int fit_columns=1;
static double user_src_dpi=-2.0;
static double document_scale_factor=1.0;
static int src_dpi=300;
static int usegs=0;
static int query_user=1;
static int query_user_explicit=0;
static int jpeg_quality=-1;
static int dst_width=DEFAULT_WIDTH;
static int dst_height=DEFAULT_HEIGHT;
static int dst_userwidth=DEFAULT_WIDTH;
static int dst_userheight=DEFAULT_HEIGHT;
static int dst_justify=-1; // 0 = left, 1 = center
static int dst_fulljustify=-1; // 0 = no, 1 = yes
static int dst_sharpen=1;
static int dst_color=0;
static int dst_bpc=4;
static int dst_landscape=0;
static int src_autostraighten=0;
static double dst_mar=0.02;
static double dst_martop=-1.0;
static double dst_marbot=-1.0;
static double dst_marleft=-1.0;
static double dst_marright=-1.0;
static int pad_left=0;
static int pad_right=3;
static int pad_bottom=4;
static int pad_top=0;
static int mark_corners=1;
static double min_column_gap_inches=0.1;
static double min_column_height_inches=1.5;
static double mar_top=-1.0;
static double mar_bot=-1.0;
static double mar_left=-1.0;
static double mar_right=-1.0;
static double max_region_width_inches = 3.6;
static int max_columns=2;
static double column_gap_range=0.33;
static double column_offset_max=0.2;
static double column_row_gap_height_in=1./72.;
static int text_wrap=1;
static double word_spacing=0.375;
static double display_width_inches = 3.6;
static char pagelist[1024];
static int column_fitted=0;
static double lm_org,bm_org,tm_org,rm_org,dpi_org;
static double contrast_max = 2.0;
static double dst_gamma=0.5;
static int exit_on_complete=-1;
static int show_marked_source=0;
static char k2pdfopt_os[64];
static char k2pdfopt_chip[64];
static char k2pdfopt_compiler[64];
static int use_crop_boxes=0;
static int preserve_indentation=1;
static double defect_size_pts=1.0;
static double max_vertical_gap_inches=0.25;
static double vertical_multiplier=1.0;
static double vertical_line_spacing=-1.2;
static double vertical_break_threshold=1.75;
static int erase_vertical_lines=0;
static int k2_hyphen_detect=1;
static double overwrite_minsize_mb=10;
static int dst_fit_to_page=0;
/*
** Undocumented cmd-line args
*/
static double no_wrap_ar_limit=0.2; /* -arlim */
static double no_wrap_height_limit_inches=0.55; /* -whmax */
static double little_piece_threshold_inches=0.5; /* -rwmin */
/*
** Keeping track of vertical gaps
*/
static double last_scale_factor_internal = -1.0;
static int line_spacing_internal; /* If > 0, try to maintain regular line spacing.  If < 0,   */
                         /* indicates desired vert. gap before next region is added. */
static int last_rowbase_internal; /* Pixels between last text row baseline and current end */
                         /* of destination bitmap. */
static int beginning_gap_internal=-1;
static int last_h5050_internal=-1;
static int just_flushed_internal=0;
static int gap_override_internal; /* If > 0, apply this gap in wrapbmp_flush() and then reset. */

static void k2pdfopt_sys_close(void);
static void k2pdfopt_sys_init(void);
#ifdef HAVE_OCR
static void k2pdfopt_ocr_init(void);
#endif
static void k2_enter_to_exit(void);
static int parse_cmd_args(int argc,char *argv[],int setvals,int procfiles,char *firstfile);
static void k2pdfopt_header(void);
static int k2pdfopt_usage(void);
static int prcmdopts(char *s,int nl);
static int cmdoplines(char *s);
static char *pr1cmdopt(char *s);
static void prlines(char *s,int nlines);
static int user_input(int filecount,char *firstfile);
static int user_float(char *message,double defval,double *dstval,int nmax,
                      double min,double max,char *extra_message);
static int user_integer(char *message,int defval,int *dstval,int min,int max);
static int user_string(char *message,char *selection[],char *def);
static int user_any_string(char *message,char *dstval,int maxlen,char *defname);
static void strcpy_no_spaces(char *d,char *s);
static int valid_page_range(char *s);
static void k2pdfopt_proc_wildarg(char *arg);
static void k2pdfopt_proc_arg(char *arg);
static void adjust_params_init(void);
static void set_region_widths(void);
static double k2pdfopt_proc_one(char *filename,double rot_deg);
static int overwrite_fail(char *outname);
static void publish_marked_page(WILLUSBITMAP *src);
static void mark_source_page(BMPREGION *region,int caller_id,int mark_flags);
static int bmp_get_one_document_page(WILLUSBITMAP *src,int src_type,char *filename,
                                     int pageno,double dpi,int bpp,FILE *out);
static int  wait_enter(void);
static void fit_column_to_screen(double column_width_inches);
static void restore_output_dpi(void);
static void adjust_contrast(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,int *white);
static int bmpregion_row_black_count(BMPREGION *region,int r0);
static void bmpregion_row_histogram(BMPREGION *region);
static int bmpregion_find_multicolumn_divider(BMPREGION *region,int *row_black_count,
                                             BMPREGION *pageregion,int *npr,
                                             int *colcount,int *rowcount);
static int bmpregion_column_height_and_gap_test(BMPREGION *column,BMPREGION *region,
                                        int r1,int r2,int cmid,
                                        int *colcount,int *rowcount);
static int bmpregion_is_clear(BMPREGION *region,int *row_is_clear,double gt_in);
static void bmpregion_multicolumn_add(BMPREGION *region,MASTERINFO *masterinfo,int level,
                                      PAGEINFO *pageinfo,int colgap0_pixels);
static void bmpregion_vertically_break(BMPREGION *region,MASTERINFO *masterinfo,
                          int allow_text_wrapping,double force_scale,
                          int *colcount,int *rowcount,PAGEINFO *pageinfo,
                          int colgap_pixels,int ncols);
static void bmpregion_add(BMPREGION *region,BREAKINFO *breakinfo,
                          MASTERINFO *masterinfo,int allow_text_wrapping,
                          int trim_flags,int allow_vertical_breaks,
                          double force_scale,int justify_flags,int caller_id,
                          int *colcount,int *rowcount,PAGEINFO *pageinfo,
                          int mark_flags,int rowbase_delta);
static void dst_add_gap_src_pixels(char *caller,MASTERINFO *masterinfo,int pixels);
static void dst_add_gap(MASTERINFO *masterinfo,double inches);
static void bmp_src_to_dst(MASTERINFO *masterinfo,WILLUSBITMAP *src,int justification_flags,
                           int whitethresh,int nocr);
static void bmp_fully_justify(WILLUSBITMAP *jbmp,WILLUSBITMAP *src,int nocr,int whitethresh,
                              int just);
#ifdef HAVE_OCR
static void ocrwords_fill_in(OCRWORDS *words,WILLUSBITMAP *src,int whitethresh);
#endif
static void bmpregion_trim_margins(BMPREGION *region,int *colcount0,int *rowcount0,int flags);
static void bmpregion_hyphen_detect(BMPREGION *region);
#if (WILLUSDEBUGX & 6)
static void breakinfo_echo(BREAKINFO *bi);
#endif
#if (defined(WILLUSDEBUGX) || defined(WILLUSDEBUG))
static void bmpregion_write(BMPREGION *region,char *filename);
#endif
static int height2_calc(int *rc,int n);
static void trim_to(int *count,int *i1,int i2,double gaplen);
static void bmpregion_analyze_justification_and_line_spacing(BMPREGION *region,
                                   BREAKINFO *breakinfo,MASTERINFO *masterinfo,
                                   int *colcount,int *rowcount,PAGEINFO *pageinfo,
                                   int allow_text_wrapping,double force_scale);
static int bmpregion_is_centered(BMPREGION *region,BREAKINFO *breakinfo,int i1,int i2,
                                 int *textheight);
static double median_val(double *x,int n);
static void bmpregion_find_vertical_breaks(BMPREGION *region,BREAKINFO *breakinfo,
                                           int *colcount,int *rowcount,double apsize_in);
static void textrow_assign_bmpregion(TEXTROW *textrow,BMPREGION *region);
static void breakinfo_compute_row_gaps(BREAKINFO *breakinfo,int r2);
static void breakinfo_compute_col_gaps(BREAKINFO *breakinfo,int c2);
static void breakinfo_remove_small_col_gaps(BREAKINFO *breakinfo,int lcheight,double mingap);
static void breakinfo_remove_small_rows(BREAKINFO *breakinfo,double fracrh,double fracgap,
                                        BMPREGION *region,int *colcount,int *rowcount);
static void breakinfo_alloc(int index,BREAKINFO *breakinfo,int nrows);
static void breakinfo_free(int index,BREAKINFO *breakinfo);
static void breakinfo_sort_by_gap(BREAKINFO *breakinfo);
static void breakinfo_sort_by_row_position(BREAKINFO *breakinfo);
static void bmpregion_one_row_find_breaks(BMPREGION *region,BREAKINFO *breakinfo,
                                          int *colcount,int *rowcount,int add_to_dbase);
static void wrapbmp_init(void);
static int wrapbmp_ends_in_hyphen(void);
static void wrapbmp_set_color(int is_color);
static void wrapbmp_free(void);
static void wrapbmp_set_maxgap(int value);
static int wrapbmp_width(void);
static int wrapbmp_remaining(void);
static void wrapbmp_add(BMPREGION *region,int gap,int line_spacing,int rbase,int gio,
                                          int justification_flags);
static void wrapbmp_flush(MASTERINFO *masterinfo,int allow_full_justify,PAGEINFO *pageinfo,
                          int use_bgi);
static void wrapbmp_hyphen_erase(void);
static void bmpregion_one_row_wrap_and_add(BMPREGION *region,BREAKINFO *breakinfo,
                                           int index,int i0,int i1,
                                           MASTERINFO *masterinfo,int justflags,
                                           int *colcount,int *rowcount,
                                           PAGEINFO *pageinfo,int rheight,int mean_row_gap,
                                           int rowbase,int marking_flags,int pi);
static void publish_master(MASTERINFO *masterinfo,PAGEINFO *pageinfo,int flushall);
static int break_point(MASTERINFO *masterinfo,int maxsize);
static void white_margins(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey);
static void get_white_margins(BMPREGION *region);
static int pagelist_page_by_index(char *pagelist,int index,int maxpages);
static int pagelist_count(char *pagelist,int maxpages);
static int pagelist_next_pages(char *pagelist,int maxpages,int *index,int *n1,int *n2);
/* Bitmap orientation detection functions */
static double bitmap_orientation(WILLUSBITMAP *bmp);
static double bmp_inflections_vertical(WILLUSBITMAP *srcgrey,int ndivisions,int delta,int *wthresh);
static double bmp_inflections_horizontal(WILLUSBITMAP *srcgrey,int ndivisions,int delta,int *wthresh);
static int inflection_count(double *x,int n,int delta,int *wthresh);
static void pdfboxes_init(PDFBOXES *boxes);
static void pdfboxes_free(PDFBOXES *boxes);
/*
static void pdfboxes_add_box(PDFBOXES *boxes,PDFBOX *box);
static void pdfboxes_delete(PDFBOXES *boxes,int n);
*/
static void word_gaps_add(BREAKINFO *breakinfo,int lcheight,double *median_gap);
static void bmp_detect_vertical_lines(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,double dpi,double minwidth_in,
                                      double maxwidth_in,double minheight_in,double anglemax_deg,
                                      int white_thresh);
static int vert_line_erase(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,WILLUSBITMAP *tmp,
                           int row0,int col0,double tanthx,double minheight_in,
                           double minwidth_in,double maxwidth_in,int white_thresh);
static void willus_dmem_alloc_warn(int index,void **ptr,int size,char *funcname,
                                int exitcode);
static void willus_dmem_free(int index,double **ptr,char *funcname);


int main(int argc,char *argv[])

    {
    int i,j,filecount;
    char firstfile[256];

    k2pdfopt_sys_init();
    wrapbmp_init();
    gpdf=&_gpdf;
    mpdf=&_mpdf;
#if (defined(WIN32) || defined(WIN64))
    tty_rows=25;
#else
    tty_rows=24;
#endif
    if (ansi_rows_cols(stdout,&i,&j))
        tty_rows=i;
    exit_on_complete=-1;
    query_user=-1;
    query_user_explicit=0;
    /* Only set ansi and user interface */
    filecount=parse_cmd_args(argc,argv,2,0,firstfile);
    if (show_usage)
        {
        k2pdfopt_header();
        if (query_user==0 
#if (defined(WIN32) || defined(WIN64))
              || !win_has_own_window()
#endif
                          )
            {
            printf("%s",usageintro);
            printf("%s",usageenv);
            printf("Command Line Options\n"
                   "--------------------\n"
                   "%s\n",k2pdfopt_options);
            }
        else
            {
            if (!k2pdfopt_usage())
                {
                wrapbmp_free();
                k2pdfopt_sys_close();
                return(0);
                }
            }
        if (query_user!=0)
            k2_enter_to_exit();
        wrapbmp_free();
        k2pdfopt_sys_close();
        return(0);
        }
    if (query_user<0)
#if (defined(WIN32) || defined(WIN64))
        {
        if (win_has_own_window())
            query_user=1;
        else
            query_user=(filecount==0);
        }
#else
        query_user=1;
#endif
    uifile[0]='\0';
#if (!defined(WIN32) && !defined(WIN64))
    if (query_user)
        {
        for (i=0;i<tty_rows-16;i++)
            aprintf("\n");
        }
#endif
    k2pdfopt_header();
    show_marked_source=0;
    dst_dither=1;
    dst_break_pages=0;
    dst_gamma=0.5;
    column_fitted=0;
    dst_color=0;
    wrapbmp_set_color(dst_color);
    jpeg_quality=-1;
    verbose=0;
    usegs=0;
    dst_width=dst_userwidth=DEFAULT_WIDTH;
    dst_height=dst_userheight=DEFAULT_HEIGHT;
    src_autostraighten=0;
    cdthresh=.01;
    contrast_max=2.0;
    user_src_dpi=-2.0;
    document_scale_factor=1.0;
    dst_dpi=167;
#ifdef HAVE_OCR
    dst_ocr=0;
    dst_ocr_wordcolor=0;
#endif
    render_dpi=167;
    dst_sharpen=1;
    dst_justify=-1;
    dst_fulljustify=-1;
    fit_columns=1;
    src_rot=SRCROT_AUTO;
    mar_top=-1.0;
    mar_bot=-1.0;
    mar_left=-1.0;
    mar_right=-1.0;
    pad_left=0;
    pad_right=3;
    pad_bottom=4;
    pad_top=0;
    mark_corners=1;
    dst_mar=0.02;
    dst_martop=-1.0;
    dst_marbot=-1.0;
    dst_marleft=-1.0;
    dst_marright=-1.0;
    dst_bpc=4;
    min_column_gap_inches=0.1;
    min_column_height_inches=1.5;
    max_columns=2;
    column_gap_range=0.33;
    column_offset_max=0.2;
    column_row_gap_height_in=1./72.;
    src_left_to_right=1;
    preserve_indentation=1;
    max_vertical_gap_inches=0.25;
    vertical_multiplier=1.0;
    vertical_line_spacing=-1.2;
    defect_size_pts=0.75;
    use_crop_boxes=0;
    text_wrap=1;
    no_wrap_ar_limit=0.2;
    no_wrap_height_limit_inches=0.55;
    little_piece_threshold_inches=0.5;
    gtc_in=.005; // detecting gap between columns
    gtcmax_in=1.5; // max gap between columns
    gtr_in=.006; // detecting gap between rows
    gtw_in=.0015; // detecting gap between words
    // gtm_in=.005; // detecting margins for trimming
    /* Note that word_spacing = 0.35 is a better default for pooh.pdf, but    */
    /* for most of the IEEE journal stuff, 0.25 or even 0.15 - 0.2 is better. */
    word_spacing=0.375;
    vertical_break_threshold=1.75;
    erase_vertical_lines=0;
    k2_hyphen_detect=1;
    dst_fit_to_page=0;
    overwrite_minsize_mb=10;
    line_spacing_internal=0;
    last_scale_factor_internal=-1.0;
    last_rowbase_internal=0;
    gap_override_internal=-1;
    beginning_gap_internal=-1;
    last_h5050_internal=-1;
    just_flushed_internal=0;
    pagelist[0]='\0';

    /*
    ** Get all command-line settings
    */
    parse_cmd_args(argc,argv,1,0,NULL);
    /*
    ** Get user input
    */
    if (user_input(filecount,firstfile)==-1)
        {
        wrapbmp_free();
        k2pdfopt_sys_close();
        return(0);
        }

    /*
    ** -f2p -2 affects other options.
    */
    if (dst_fit_to_page==-2)
        {
        vertical_break_threshold=-1.;
        text_wrap=0;
        }
    /*
    ** With all parameters set, adjust output DPI so viewable region
    ** width >= MIN_REGION_WIDTH_INCHES
    */
    adjust_params_init();
    /*
    ** Set source DPI
    */
    if (dst_dpi < 20.)
        dst_dpi = 20.;
    src_dpi=user_src_dpi < 0. ? (int)(fabs(user_src_dpi)*dst_dpi+.5) : (int)(user_src_dpi+.5);
    if (src_dpi < 50.)
        src_dpi = 50.;
    /*
    ** Process files
    */
    if (uifile[0]!='\0')
        k2pdfopt_proc_wildarg(uifile);
    parse_cmd_args(argc,argv,0,1,NULL);
    /*
    ** All done.
    */
    k2_enter_to_exit();
    wrapbmp_free();
    k2pdfopt_sys_close();
    return(0);
    }


#ifdef HAVE_OCR
static int k2_ocr_inited=0;
#endif

static void k2pdfopt_sys_close(void)

    {
    sys_set_decimal_period(0);
#ifdef HAVE_OCR
    if (dst_ocr && k2_ocr_inited)
        {
#ifdef HAVE_TESSERACT
        if (dst_ocr=='t')
            ocrtess_end();
#endif
        ocrwords_free(dst_ocrwords);
        }
#endif
    }


static void k2pdfopt_sys_init(void)

    {
    system_version(NULL,k2pdfopt_os,k2pdfopt_chip,k2pdfopt_compiler);
    sys_set_decimal_period(1);
    }


#ifdef HAVE_OCR
static void k2pdfopt_ocr_init(void)

    {
    if (!dst_ocr || k2_ocr_inited)
        return;
    k2_ocr_inited=1;
    dst_ocrwords=&_dst_ocrwords;
    ocrwords_init(dst_ocrwords);
#if (!defined(HAVE_TESSERACT) && defined(HAVE_GOCR))
    if (dst_ocr=='t')
        {
        aprintf(TTEXT_WARN "\a** Tesseract not compiled into this version.  Using GOCR. **"
                TTEXT_NORMAL "\n\n");
        dst_ocr='g';
        }
#endif
#if (defined(HAVE_TESSERACT) && !defined(HAVE_GOCR))
    if (dst_ocr=='g')
        {
        aprintf(TTEXT_WARN "\a** GOCR not compiled into this version.  Using Tesseract. **"
                TTEXT_NORMAL "\n\n");
        dst_ocr='t';
        }
#endif
#ifdef HAVE_TESSERACT
#ifdef HAVE_GOCR
    if (dst_ocr=='t')
        {
#endif
        aprintf(TTEXT_BOLD);
        ocrtess_status=ocrtess_init(NULL,NULL,3,stdout);
        aprintf(TTEXT_NORMAL);
        if (ocrtess_status)
            aprintf(TTEXT_WARN "Could not find Tesseract data" TTEXT_NORMAL " (env var = TESSDATA_PREFIX).\nUsing GOCR v0.49.\n\n");
        else
            aprintf("\n");
#ifdef HAVE_GOCR
        }
    else
#endif
#endif
#ifdef HAVE_GOCR
        aprintf(TTEXT_BOLD "GOCR v0.49 OCR Engine" TTEXT_NORMAL "\n\n");
#endif
    }
#endif


static void k2_enter_to_exit(void)

    {
    static char *mesg=TTEXT_BOLD2 "Press <ENTER> to exit." TTEXT_NORMAL;

    if (exit_on_complete==1)
        return;
    if (exit_on_complete==0)
        {
        char buf[16];
        aprintf("%s",mesg);
        fgets(buf,15,stdin);
        return;
        }
    sys_enter_to_exit(mesg);
    }

/*
** Return file count
** setvals==1 to set all values based on options
**        ==2 to set only ansi, user interface, exit on complete
**        ==0 to not set any values
** procfiles == 1 to process files
**           == 0 to count files only
*/
static int parse_cmd_args(int argc,char *argv[],int setvals,int procfiles,char *firstfile)

    {
    CMDLINEINPUT _cl,*cl;
    int filecount,readnext;

    cl=&_cl;
    filecount=0;
    cmdlineinput_init(cl,argc,argv,getenv("K2PDFOPT"));
    readnext=1;
    while (1)
        {
        if (readnext && cmdlineinput_next(cl)==NULL)
            break;
        readnext=1;
        if (!stricmp(cl->cmdarg,"-?") || !stricmp(cl->cmdarg,"-?-"))
            {
            if (setvals==2)
                show_usage = cl->cmdarg[2]=='-' ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-a") || !stricmp(cl->cmdarg,"-a-"))
            {
            if (setvals==2)
                ansi_set(cl->cmdarg[2]=='-' ? 0 : 1);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-x") || !stricmp(cl->cmdarg,"-x-"))
            {
            if (setvals==2)
                exit_on_complete=(cl->cmdarg[2]=='-' ? 0 : 1);
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-ui",3))
            {
            if (setvals==2)
                {
                if (cl->cmdarg[3]!='-')
                   query_user_explicit=1;
                query_user=(cl->cmdarg[3]!='-') ? 1 : 0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-evl"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                erase_vertical_lines=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vls"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                vertical_line_spacing=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vm"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                vertical_multiplier=fabs(atof(cl->cmdarg));
                if (vertical_multiplier < 0.1)
                    vertical_multiplier = 0.1;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vs"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                max_vertical_gap_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-de"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                defect_size_pts=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pi") || !stricmp(cl->cmdarg,"-pi-"))
            {
            if (setvals==1)
                preserve_indentation=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-wrap",5))
            {
            if (setvals==1)
                text_wrap=(cl->cmdarg[5]=='-') ? 0 : (cl->cmdarg[5]=='+' ? 2 : 1);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gs") || !stricmp(cl->cmdarg,"-gs-"))
            {
            if (setvals==1)
                usegs=(cl->cmdarg[3]=='-' ? 0 : 1);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-r") || !stricmp(cl->cmdarg,"-r-"))
            {
            if (setvals==1)
                src_left_to_right=(cl->cmdarg[2]=='-') ? 1 : 0;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-hy",3))
            {
            if (setvals==1)
                k2_hyphen_detect=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-ls",3))
            {
            if (setvals==1)
                dst_landscape=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-o") || !stricmp(cl->cmdarg,"-o-"))
            {
            int always_prompt;
            char *ptr;
            always_prompt = (cl->cmdarg[2]=='-');
            if (((ptr=cmdlineinput_next(cl))==NULL) || !is_a_number(cl->cmdarg))
                {
                readnext=0;
                if (setvals==1)
                    overwrite_minsize_mb= always_prompt ? 0 : -1;
                if (ptr==NULL)
                    break;
                continue;
                }
            if (setvals==1)
                overwrite_minsize_mb=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-f2p"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_fit_to_page=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-vb"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                vertical_break_threshold=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-sm") || !stricmp(cl->cmdarg,"-sm-"))
            {
            if (setvals==1)
                show_marked_source=(cl->cmdarg[3]=='-' ? 0 : 1);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-bp") || !stricmp(cl->cmdarg,"-bp-"))
            {
            if (setvals==1)
                dst_break_pages=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-fc",3))
            {
            if (setvals==1)
                fit_columns=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-d") || !stricmp(cl->cmdarg,"-d-"))
            {
            if (setvals==1)
                dst_dither=(cl->cmdarg[2]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-c") || !stricmp(cl->cmdarg,"-c-"))
            {
            if (setvals==1)
                {
                dst_color=(cl->cmdarg[2]=='-') ? 0 : 1;
                wrapbmp_set_color(dst_color);
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-v",2))
            {
            if (setvals==1)
                verbose=(cl->cmdarg[2]=='-') ? 0 : 1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-png",4))
            {
            if (setvals==1)
                jpeg_quality=(cl->cmdarg[4]=='-') ? 90 : -1;
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-mc",3))
            {
            if (setvals==1)
                mark_corners=(cl->cmdarg[3]=='-') ? 0 : 1;
            continue;
            }
#ifdef HAVE_OCR
        if (!stricmp(cl->cmdarg,"-wc"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_ocr_wordcolor=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocrhmax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                ocr_max_height_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ocr") || !stricmp(cl->cmdarg,"-ocr-"))
            {
            if (cl->cmdarg[4]=='-')
                {
                if (setvals==1)
                    dst_ocr=0;
                continue;
                }
            if (cmdlineinput_next(cl)==NULL || !stricmp(cl->cmdarg,"t"))
                { 
                if (setvals==1)
                    dst_ocr='t';
                continue;
                }
            if (!stricmp(cl->cmdarg,"g") || !stricmp(cl->cmdarg,"j"))
                {
                if (setvals==1)
                    dst_ocr='g';
                continue;
                }
            if (setvals==1)
#ifdef HAVE_TESSERACT
                dst_ocr='t';
#else
                dst_ocr='g';
#endif
            readnext=0;
            continue;
            }
#endif
        if (!stricmp(cl->cmdarg,"-s") || !stricmp(cl->cmdarg,"-s-"))
            {
            if (setvals==1)
                dst_sharpen=(cl->cmdarg[2]=='-') ? 0 : 1;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-as"))
            {
            if (setvals==1)
                src_autostraighten=4.;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (is_a_number(cl->cmdarg))
                {
                if (setvals==1)
                    src_autostraighten=atof(cl->cmdarg);
                }
            else
                readnext=0;
            if (src_autostraighten > 45.)
                src_autostraighten = 45.;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-rt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                if (!stricmp(cl->cmdarg,"auto"))
                    src_rot=SRCROT_AUTO;
                else if (!stricmp(cl->cmdarg,"aep"))
                    src_rot=SRCROT_AUTOEP;
                else
                    src_rot=atoi(cl->cmdarg);
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-crgh"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                column_row_gap_height_in=atof(cl->cmdarg);
                if (column_row_gap_height_in < 0.001)
                    column_row_gap_height_in = 0.001;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-cgr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                column_gap_range=atof(cl->cmdarg);
                if (column_gap_range < 0.)
                    column_gap_range = 0.;
                if (column_gap_range > 1.0)
                    column_gap_range = 1.0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-comax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                column_offset_max=atof(cl->cmdarg);
                if (column_offset_max > 1.0)
                    column_offset_max = 1.0;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-col"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                max_columns=atoi(cl->cmdarg);
                if (max_columns<1)
                    max_columns=1;
                if (max_columns>2)
                    max_columns=4;
                }
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-jpg",4) || !strnicmp(cl->cmdarg,"-jpeg",5))
            {
            int ic;
            ic = (tolower(cl->cmdarg[3])=='g') ? 4 : 5;
            if (cl->cmdarg[ic]=='-')
                {
                if (setvals==1)
                    jpeg_quality=-1;
                }
            else
                {
                if (cmdlineinput_next(cl)==NULL)
                    {
                    if (setvals==1)
                        jpeg_quality=90;
                    }
                else if (is_an_integer(cl->cmdarg))
                    {
                    if (setvals==1)
                        jpeg_quality=atoi(cl->cmdarg);
                    }
                else
                    {
                    readnext=0;
                    if (setvals==1)
                        jpeg_quality=90;
                    }
                }
            if (jpeg_quality>100)
                jpeg_quality=100;
            continue;
            }
        if (!stricmp(cl->cmdarg,"-col"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                max_columns=atoi(cl->cmdarg);
                if (max_columns<1)
                    max_columns=1;
                if (max_columns>2)
                    max_columns=4;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-p"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                strncpy(pagelist,cl->cmdarg,1023);
                pagelist[1023]='\0';
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-bpc"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                dst_bpc=atoi(cl->cmdarg);
                if (dst_bpc>=6)
                    dst_bpc=8;
                else if (dst_bpc>=3)
                    dst_bpc=4;
                else if (dst_bpc<1)
                    dst_bpc=1;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-g"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                dst_gamma=atof(cl->cmdarg);
                if (dst_gamma<.01)
                    dst_gamma=.01;
                if (dst_gamma>100.)
                    dst_gamma=100.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-cg"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                min_column_gap_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                gtr_in=atof(cl->cmdarg);
                if (gtr_in<0.)
                    gtr_in=0.;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtcmax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                gtcmax_in=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-gtc"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                gtc_in=atof(cl->cmdarg);
                if (gtc_in<0.)
                    gtc_in=0.;
                }
            continue;
            }
        /*
        if (!stricmp(cl->cmdarg,"-gtm"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                gtm_in=atof(cl->cmdarg);
                if (gtm_in<0.)
                    gtm_in=0.;
                }
            continue;
            }
        */
        if (!stricmp(cl->cmdarg,"-gtw"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                gtw_in=atof(cl->cmdarg);
                if (gtw_in<0.)
                    gtw_in=0.;
                }
            continue;
            }
/*
        if (i<argc-1 && !stricmp(cl->cmdarg,"-cd"))
            {
            if (setvals==1)
                {
                cdthresh=atof(argv[++i]);
                if (cdthresh<0.)
                    cdthresh=0.;
                else if (cdthresh>100.)
                    cdthresh=100.;
                }
            else
                i++;
            continue;
            }
*/
        if (!stricmp(cl->cmdarg,"-cmax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                contrast_max=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ch"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                min_column_height_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ds"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1 && atof(cl->cmdarg)>0.)
                document_scale_factor=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-idpi"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1 && atof(cl->cmdarg)!=0.)
                user_src_dpi=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-odpi"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_dpi=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-j"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                dst_justify=atoi(cl->cmdarg);
                if (in_string(cl->cmdarg,"+")>=0)
                    dst_fulljustify=1;
                else if (in_string(&cl->cmdarg[1],"-")>=0)
                    dst_fulljustify=0;
                else
                    dst_fulljustify=-1;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-h"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_userheight=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ws"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                word_spacing=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-wt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                {
                src_whitethresh=atoi(cl->cmdarg);
                if (src_whitethresh>255)
                    src_whitethresh=255;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-w"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_userwidth=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-omb"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_marbot=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-omt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_martop=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-omr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_marright=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-oml"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_marleft=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-om"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                dst_mar=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-mb"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                mar_bot=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-mt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                mar_top=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-mr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                mar_right=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-ml"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                mar_left=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pb"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                pad_bottom=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pt"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                pad_top=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pr"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                pad_right=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-pl"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                pad_left=atoi(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-m"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                mar_top=mar_bot=mar_left=mar_right=atof(cl->cmdarg);
            continue;
            }
        if (!strnicmp(cl->cmdarg,"-hq",3))
            {
            if (setvals==1)
                continue;
            if (cl->cmdarg[3]=='-')
                {
                dst_dpi=167;
                user_src_dpi = -2.0;
                dst_userwidth=DEFAULT_WIDTH;
                dst_userheight=DEFAULT_HEIGHT;
                }
            else
                {
                dst_dpi=333;
                user_src_dpi = -2.0;
                dst_userwidth=DEFAULT_WIDTH*2;
                dst_userheight=DEFAULT_HEIGHT*2;
                }
            continue;
            }
        if (!stricmp(cl->cmdarg,"-debug"))
            {
            if (setvals==1)
                debug=1;
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (is_an_integer(cl->cmdarg))
                {
                if (setvals==1)
                    debug=atoi(cl->cmdarg);
                }
            else
                readnext=0;
            continue;
            }
        /*
        ** UNDOCUMENTED COMMAND-LINE ARGS
        */
        if (!stricmp(cl->cmdarg,"-whmax"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                no_wrap_height_limit_inches=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-arlim"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                no_wrap_ar_limit=atof(cl->cmdarg);
            continue;
            }
        if (!stricmp(cl->cmdarg,"-rwmin"))
            {
            if (cmdlineinput_next(cl)==NULL)
                break;
            if (setvals==1)
                little_piece_threshold_inches=atof(cl->cmdarg);
            continue;
            }
        filecount++;
        if (filecount==1 && firstfile!=NULL)
            {
            strncpy(firstfile,cl->cmdarg,255);
            firstfile[255]='\0';
            }
        if (procfiles)
            k2pdfopt_proc_wildarg(cl->cmdarg);
        }
    return(filecount);
    }


static void k2pdfopt_header(void)

    {
    char date[32];

    strcpy(date,__DATE__);
    aprintf(TTEXT_HEADER "k2pdfopt %s" TTEXT_NORMAL "%s (c) %s, GPLv3, http://willus.com\n"
            "    Compiled %s with %s for %s on %s.\n\n",
           VERSION,
#ifdef HAVE_OCR
           " (w/OCR)",
#else
           "",
#endif
           &date[strlen(date)-4],
           __DATE__,k2pdfopt_compiler,k2pdfopt_os,k2pdfopt_chip);
    }


static int k2pdfopt_usage(void)

    {
    int nl;

    nl=tty_rows;
    if (nl < 20)
        nl=20;
    prlines(usageintro,nl-4);
    if (wait_enter()<0)
        return(0);
    prlines(usageenv,nl-1);
    if (wait_enter()<0)
        return(0);
    if (!prcmdopts(k2pdfopt_options,nl))
        return(0);
    return(1);
    }


static int prcmdopts(char *s,int nl)

    {
    int i,ll;
   
    for (i=0;1;i++)
        { 
        if (i==0)
            aprintf(TTEXT_BOLD "Command Line Options\n"
                               "--------------------\n" TTEXT_NORMAL);
        else
            aprintf(TTEXT_BOLD "Command Line Options (cont'd)\n"
                               "-----------------------------\n" TTEXT_NORMAL);
        ll=nl-2;
        while (1)
            {
            int nlo;
            nlo=cmdoplines(s);
            if (s[0]=='\0' || ll-nlo-2<0)
                break;
            s=pr1cmdopt(s);
            ll-=nlo;
            }
        while (ll>1)
            {
            aprintf("\n");
            ll--;
            }
        if (!i)
            aprintf("\n");
        if (wait_enter()<0)
            return(0);
        if (s[0]=='\0')
            break;
        }
    return(1);
    }


static int cmdoplines(char *s)

    {
    int i,j;

    for (j=0,i=1;1;i++)
        {
        for (;s[j]!='\n' && s[j]!='\0';j++);
        if (s[j]=='\0')
            return(i);
        j++;
        if (s[j]!=' ')
            return(i);
        }
    }


static char *pr1cmdopt(char *s)

    {
    int j,k,k0;
    char buf[128];

    for (j=0;1;)
        {
        for (k=0;k<18 && s[j]!=' ' && s[j]!='\n' && s[j]!='\0';j++)
            buf[k++]=s[j];
        buf[k]='\0';
        aprintf(TTEXT_BOLD "%s" TTEXT_NORMAL,buf);
        if (k<17 && s[j]==' ' && s[j+1]!=' ')
            {
            for (k0=0;k<18 && s[j]!='\n' && s[j]!='\0';j++,k++)
                buf[k0++]=s[j];
            buf[k0]='\0';
            aprintf(TTEXT_MAGENTA "%s" TTEXT_NORMAL,buf);
            }
        if (s[j]!='\0' && s[j]!='\n')
            {
            for (k=0;s[j]!='\n' && s[j]!='\0';j++)
                buf[k++]=s[j];
            buf[k]='\0';
            aprintf("%s\n",buf);
            }
        if (s[j]=='\0')
            return(&s[j]);
        j++;
        if (s[j]!=' ')
            return(&s[j]);
        }
    }
    

static void prlines(char *s,int nlines)

    {
    int i,ns;

    for (i=ns=0;s[i]!='\0';i++)
        if (s[i]=='\n')
            ns++;
    aprintf("%s",s);
    for (i=ns;i<nlines;i++)
        aprintf("\n");
    }


static int user_input(int filecount,char *firstfile)

    {
    int status;
    static char *ansyesno[]={"yes","no",""};
    static char *jpegpng[]={"png","jpeg",""};
    static char *ansjust[]={"left","center","right",""};
    double defmar;
    char specfile[512];
    static char *options[] =
        {
        "a. Autostraighten (-as)",
        "b. Bitmap type (-jpg,-png,-bpc)",
        "bp. Break pages (-bp,-f2p)",
        "c. Color output (-c)",
        "co. Column detection (-col,-ch,...)",
        "cm. Contrast max (-cmax)",
        "d. Display resolution (-h,-w)",
        "de. Defect size (-de)",
        "ds. Document scale factor (-ds)",
        "e. Erase vertical lines (-evl)",
        "f. Fit to single column (-fc)",
        "g. Gamma value (-g)",
        "gs. Ghostscript interpreter (-gs)",
        "gt. Gap thresholds (-gt...)",
        "i. Input file dpi (-idpi)",
        "j. Justification (-j)",
        "l. Landscape mode (-ls)",
        "m. Margin to ignore (-m)",
        "mc. Mark corners (-mc)",
        "o. Output device dpi (-odpi)",
#ifdef HAVE_OCR
        "oc. OCR (-ocr,-wc)",
#endif
        "om. Output margins (-om)",
        "p. Page range (-p)",
        "pd. Padding on output (-pl,...)",
        "r. Right-to-left page scans (-r)",
        "rt. Rotate source page (-sr)",
        "s. Sharpening (-s)",
        "sm. Show marked source (-sm)",
        "u. (or ?) Usage (command line opts)",
        "v. Vertical spacing (-vb,-vs)",
        "w. Wrap text option (-wrap)",
        "ws. Word spacing threshold (-ws)",
        "wt. White threshold (-wt)",
        "x. Exit on completion (-x)",
        ""};

    if (filecount>0)
        strcpy(specfile,firstfile);
    if (!query_user)
        return(0);
    while (1)
        {
        int i,no,newmenu;
        char buf[512];
        for (i=0;options[i][0]!='\0';i++);
        no=i;
        for (i=0;i<(no+1)/2;i++)
            {
            char opt[8];
            int j,k;
            for (j=0;options[i][j]!='.';j++)
                opt[j]=options[i][j];
            opt[j]='\0';
            aprintf(TTEXT_BOLD "%2s" TTEXT_NORMAL "%-34s",opt,&options[i][j]);
            k=i+(no+1)/2;
            if (k < no)
                {
                for (j=0;options[k][j]!='.';j++)
                    opt[j]=options[k][j];
                opt[j]='\0';
                aprintf(TTEXT_BOLD "%2s" TTEXT_NORMAL "%s",opt,&options[k][j]);
                }
            aprintf("\n");
            }
        newmenu=0;
        while (1)
            {
            int goodspec;

            if (filecount>0)
                {
                if (filecount==1)
                    aprintf("\nSource file: " TTEXT_MAGENTA "%s" TTEXT_NORMAL "\n",specfile);
                else 
                    aprintf("\nSource file: (multiple files specified)\n");
                aprintf(TTEXT_BOLD2 "Enter option above" TTEXT_NORMAL
                 " or " TTEXT_BOLD2 "?" TTEXT_NORMAL " for help"
                 " or " TTEXT_BOLD2 "page range" TTEXT_NORMAL " (e.g. 2,4,8-10) to convert\n"
                 "or " TTEXT_BOLD2 "q" TTEXT_NORMAL " to quit or just "
                 TTEXT_BOLD2 "<Enter>" TTEXT_NORMAL " to convert all pages: "
                 TTEXT_INPUT,no);
                }
            else
                aprintf("\n(No source file specified.)\n" 
                        TTEXT_BOLD2 "Enter option above" TTEXT_NORMAL
                 " or " TTEXT_BOLD2 "?" TTEXT_NORMAL " for help"
                 " or " TTEXT_BOLD2 "q" TTEXT_NORMAL " to quit\n"
                 "or type in a file name to convert: "
                 TTEXT_INPUT,no);
            fgets(buf,511,stdin);
            aprintf(TTEXT_NORMAL "\n");
            clean_line(buf);
            if (buf[0]=='?' || !stricmp(buf,"help"))
                strcpy(buf,"u");
            if (buf[0]=='\0')
                return(0);
            if (tolower(buf[0])=='q')
                return(-1);
            for (i=0;options[i][0]!='\0';i++)
                {
                if (options[i][1]=='.' && buf[1]=='\0' && tolower(buf[0])==tolower(options[i][0]))
                    break;
                if (options[i][2]=='.' && buf[2]=='\0' && !strnicmp(buf,options[i],2))
                    break;
                }
            if (options[i][0]!='\0')
                break;
            if (filecount>0 && valid_page_range(buf))
                {
                strcpy_no_spaces(pagelist,buf);
                return(0);
                }
            strncpy(uifile,buf,511);
            uifile[511]='\0';
#if (!defined(WIN32) && !defined(WIN64))
            /* On Mac, backslashes are inserted before each space, */
            /* so try getting rid of them if the file isn't found. */
            if (wfile_status(uifile)==0)
				{
				int i,j;
				for (i=j=0;uifile[i]!='\0';i++)
					{
					if (uifile[i]=='\\')
						i++;
					uifile[j]=uifile[i];
					j++;
					}
				uifile[j]='\0';
				}
#endif
            {
            FILELIST *fl,_fl;
            fl=&_fl;
            filelist_init(fl);
            filelist_fill_from_disk_1(fl,uifile,0,0);
            goodspec = (fl->n>0);
            filelist_free(fl);
            if (filecount==0 && goodspec)
                {
                strcpy(specfile,uifile);
                filecount=1;
                newmenu=1;
                break;
                }
            }
            if (filecount>0)
                aprintf(TTEXT_WARN "\a** Invalid entry. (File%s already specified.) **" TTEXT_NORMAL "\n\n",filecount>1?"s":"");
            else
                aprintf(TTEXT_WARN "\a** No files found matching %s. **" TTEXT_NORMAL "\n\n",uifile);
            }
        if (newmenu)
            continue;
        if (!stricmp(buf,"a"))
            {
            status=user_string("Auto-straighten the pages",ansyesno,src_autostraighten?"y":"n");
            if (status<0)
                return(status);
            src_autostraighten=(status==0) ? 4.0 : -1.0;
            }
        else if (!stricmp(buf,"b"))
            {
            status=user_string("Bitmap encoding (png=lossless)",jpegpng,"png");
            if (status<0)
                return(status);
            if (status==0)
                {
                jpeg_quality=-1;
                status=user_integer("Bits per color plane (1, 2, 4, or 8)",4,&dst_bpc,1,8);
                if (status<0)
                    return(status);
                if (dst_bpc>=6)
                    dst_bpc=8;
                else if (dst_bpc>=3)
                    dst_bpc=4;
                if (dst_bpc<8)
                    {
                    status=user_string("Apply dithering",ansyesno,dst_dither?"y":"n");
                    if (status<0)
                        return(status);
                    dst_dither=(status==0) ? 1 : 0;
                    }
                }
            else
                {
                status=user_integer("JPEG quality (1-99, lower=smaller size file)",
                                     90,&jpeg_quality,1,99);
                if (status<0)
                    return(status);
                }
            }
        else if (!stricmp(buf,"bp"))
            {
            status=user_string("Break output pages at end of each input page",ansyesno,dst_break_pages?"y":"n");
            if (status<0)
                return(status);
            dst_break_pages=!status;
            status=user_integer("Fit-to-page value",dst_fit_to_page,&dst_fit_to_page,
                                 -2,999);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"c"))
            {
            status=user_string("Full color output",ansyesno,dst_color?"y":"n");
            if (status<0)
                return(status);
            dst_color=!status;
            wrapbmp_set_color(dst_color);
            }
        else if (!stricmp(buf,"e"))
            {
            printf("\n0. Don't erase vertical lines.\n"
                     "1. Detect and erase only free-standing vertical lines.\n"
                     "2. Detect and erase all vertical lines.\n\n");
            status=user_integer("Enter option above (0, 1, or 2)",
                                 erase_vertical_lines,&erase_vertical_lines,0,2);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"co"))
            {
            status=user_integer("Max number of columns (1, 2, or 4)",4,&max_columns,1,4);
            if (status<0)
                return(status);
            if (max_columns==3)
                max_columns=4;
            if (max_columns>1)
                {
                status=user_float("Min gap between columns (inches)",min_column_gap_inches,
                            &min_column_gap_inches,1,0.0,20.,NULL);
                if (status<0)
                    return(status);
                status=user_float("Min column height (inches)",min_column_height_inches,
                            &min_column_height_inches,1,0.05,20.,NULL);
                if (status<0)
                    return(status);
                status=user_float("Column gap range (0 - 1)",column_gap_range,
                            &column_gap_range,1,0.0,1.,NULL);
                if (status<0)
                    return(status);
                status=user_float("Column row gap height (inches)",column_row_gap_height_in,
                            &column_row_gap_height_in,1,0.001,5.0,NULL);
                if (status<0)
                    return(status);
                status=user_float("Column offset maximum (0 to 1 or -1 to disable)",column_offset_max,
                            &column_offset_max,1,-1.5,1.,NULL);
                if (status<0)
                    return(status);
                }
            }
        else if (!stricmp(buf,"cm"))
            {
            status=user_float("Max contrast adjust (1.0=no adjust)",contrast_max,&contrast_max,1,
                             -200.,200.,NULL);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"ds"))
            {
            status=user_float("Document scale factor (1.0=no change)",document_scale_factor,&document_scale_factor,1,0.01,100.,NULL);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"de"))
            {
            status=user_float("Defect size in points",defect_size_pts,&defect_size_pts,1,0.0,100.,NULL);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"f"))
            {
            status=user_string("Fit single column to reader",ansyesno,fit_columns?"y":"n");
            if (status<0)
                return(status);
            fit_columns=!status;
            }
        else if (!stricmp(buf,"d"))
            {
            status=user_integer("Destination pixel width",DEFAULT_WIDTH,&dst_userwidth,10,6000);
            if (status<0)
                return(status);
            status=user_integer("Destination pixel height",DEFAULT_HEIGHT,&dst_userheight,10,8000);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"g"))
            {
            status=user_float("Gamma value (1.0=no adjustment)",dst_gamma,&dst_gamma,1,
                             0.01,100.,NULL);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"gs"))
            {
            status=user_string("Use Ghostscript interpreter",ansyesno,usegs?"y":"n");
            if (status<0)
                return(status);
            usegs=!status;
            }
        else if (!stricmp(buf,"gt"))
            {
            status=user_float("Gap threshold for columns (inches)",gtc_in,&gtc_in,1,0.,20.,NULL);
            if (status<0)
                return(status);
            status=user_float("Max gap between columns (inches)",gtcmax_in,&gtcmax_in,1,0.,99.,NULL);
            if (status<0)
                return(status);
            /*
            status=user_float("Gap threshold for margins (inches)",gtm_in,&gtm_in,1,0.,20.,NULL);
            if (status<0)
                return(status);
            */
            status=user_float("Gap threshold for rows (inches)",gtr_in,&gtr_in,1,0.,20.,NULL);
            if (status<0)
                return(status);
            status=user_float("Gap threshold for words (inches)",gtw_in,&gtw_in,1,0.,20.,NULL);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"i"))
            {
            status=user_float("Input/Source pixels per inch",user_src_dpi,&user_src_dpi,1,-10.,1200.,NULL);
            if (status<0)
                return(status);
            while (user_src_dpi > -.25 &&  user_src_dpi < 50.)
                {
                aprintf(TTEXT_WARN "\n\a** Invalid response.  Dpi must be <= -.25 or >= 50. **" TTEXT_NORMAL "\n\n");
                status=user_float("Input/Source pixels per inch",user_src_dpi,&user_src_dpi,1,-10.,1200.,NULL);
                if (status<0)
                    return(status);
                }
            }
        else if (!stricmp(buf,"j"))
            {
            status=user_string("Use default document justification",ansyesno,dst_justify<0?"y":"n");
            if (status<0)
                return(status);
            if (status==0)
                dst_justify=-1;
            else
                {
                status=user_string("Justification",ansjust,"center");
                if (status<0)
                    return(status);
                dst_justify=status;
                }
            status=user_string("Use default full justification (same as document)",ansyesno,dst_fulljustify<0?"y":"n");
            if (status<0)
                return(status);
            if (status==0)
                dst_fulljustify=-1;
            else
                {
                status=user_string("Attempt full justification",ansyesno,dst_fulljustify?"y":"n");
                if (status<0)
                    return(status);
                dst_fulljustify=!status;
                }
            }
        else if (!stricmp(buf,"l"))
            {
            status=user_string("Landscape mode",ansyesno,dst_landscape?"y":"n");
            if (status<0)
                return(status);
            dst_landscape=!status;
            }
        else if (!stricmp(buf,"m"))
            {
            double v[4];
            int i,na;

            defmar=-1.0;
            if (defmar<0. && mar_left>=0.)
                defmar=mar_left;
            if (defmar<0. && mar_top>=0.)
                defmar=mar_top;
            if (defmar<0. && mar_right>=0.)
                defmar=mar_right;
            if (defmar<0. && mar_bot>=0.)
                defmar=mar_bot;
            if (defmar<0.)
                defmar=0.25;
            na=user_float("Inches of source border to ignore",defmar,v,4,0.,10.,
                          "Enter one value or left,top,right,bottom values comma-separated.");
            if (na<0)
                return(na);
            i=0;
            mar_left=v[i];
            if (i<na-1)
                i++;
            mar_top=v[i];
            if (i<na-1)
                i++;
            mar_right=v[i];
            if (i<na-1)
                i++;
            mar_bot=v[i];
            }
        else if (!stricmp(buf,"mc"))
            {
            status=user_string("Mark corners of bitmap with a dot",ansyesno,mark_corners?"y":"n");
            if (status<0)
                return(status);
            mark_corners=!status;
            }
        else if (!stricmp(buf,"om"))
            {
            double v[4];
            int i,na;
            na=user_float("Output device margin",dst_mar,v,4,0.,10.,
                          "Enter one value or left,top,right,bottom values comma-separated.");
            if (na<0)
                return(na);
            i=0;
            dst_marleft=v[i];
            if (i<na-1)
                i++;
            dst_martop=v[i];
            if (i<na-1)
                i++;
            dst_marright=v[i];
            if (i<na-1)
                i++;
            dst_marbot=v[i];
            }
        else if (!stricmp(buf,"o"))
            {
            status=user_integer("Output/Destination pixels per inch",167,&dst_dpi,20,1200);
            if (status<0)
                return(status);
            }
#ifdef HAVE_OCR
        else if (!stricmp(buf,"oc"))
            {
            static char *ocropts[]={"Tesseract","Gocr","None",""};

            status=user_string("OCR choice",ocropts,dst_ocr=='t'?"t":(dst_ocr=='g')?"g":"n");
            if (status<0)
                return(status);
            dst_ocr=tolower(ocropts[status][0]);
            if (dst_ocr=='n')
                dst_ocr=0;
            if (dst_ocr)
                {
                status=user_float("Max OCR word height (in)",ocr_max_height_inches,
                                  &ocr_max_height_inches,1,0.,999.,"");
                if (status<0)
                    return(status);
                status=user_integer("OCR word color (def=0 for invisible)",dst_ocr_wordcolor,
                            &dst_ocr_wordcolor,0,3);
                if (status<0)
                    return(status);
                }
            }
#endif
        else if (!stricmp(buf,"p"))
            {
            status=user_any_string("Pages to convert (e.g. 1-5,6,9-)",pagelist,1023,"all");
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"pd"))
            {
            int defpad=0;
            status=user_integer("Output bitmap padding",defpad,&defpad,0,6000);
            if (status>=0.)
                pad_left=pad_right=pad_bottom=pad_top=defpad;
            else
                return(status);
            }
        else if (!stricmp(buf,"r"))
            {
            status=user_string("Scan right to left",ansyesno,src_left_to_right?"n":"y");
            if (status<0)
                return(status);
            src_left_to_right=status;
            }
        else if (!stricmp(buf,"rt"))
            {
            status=user_string("Auto-detect entire doc rotation",ansyesno,
                               fabs(src_rot-SRCROT_AUTO)<.5?"y":"n");
            if (status<0)
                return(status);
            if (!status)
                src_rot=SRCROT_AUTO;
            else
                {
                status=user_string("Auto-detect rotation of each page",ansyesno,
                                   fabs(src_rot-SRCROT_AUTOEP)<.5?"y":"n");
                if (status<0)
                    return(status);
                if (!status)
                    src_rot=SRCROT_AUTOEP;
                else
                    {
                    double defval;
                    defval = (src_rot < -900.) ? 0. : src_rot;
                    status=user_integer("Source rotation (degrees c.c.)",defval,&src_rot,-360,360);
                    if (status<0)
                        return(status);
                    }
                }
            }
        else if (!stricmp(buf,"s"))
            {
            status=user_string("Sharpen the output images",ansyesno,dst_sharpen?"y":"n");
            if (status<0)
                return(status);
            dst_sharpen=!status;
            }
        else if (!stricmp(buf,"u"))
            {
            int i;
            k2pdfopt_header();
            if (!k2pdfopt_usage())
                return(-1);
            if (wait_enter()<0)
                return(-1);
            for (i=0;i<tty_rows-16;i++)
                aprintf("\n");
            }
        else if (!stricmp(buf,"v"))
            {
            status=user_float("Vertical break threshold (-1 = don't allow)",
                   vertical_break_threshold,&vertical_break_threshold,1,-1.,100.,NULL);
            if (status<0)
                return(status);
            /*
            status=user_float("Vertical Multiplier",vertical_multiplier,
                               &vertical_multiplier,1,0.1,10.,NULL);
            */
            status=user_float("Vertical line spacing",vertical_line_spacing,
                               &vertical_line_spacing,1,-10.,10.,NULL);
            if (status<0)
                return(status);
            status=user_float("Max Vertical Gap (inches)",max_vertical_gap_inches,
                               &max_vertical_gap_inches,1,0.0,100.,NULL);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"wt"))
            {
            status=user_integer("White threshold (-1=autocalc)",src_whitethresh,&src_whitethresh,1,255);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"ws"))
            {
            status=user_float("Word spacing threshold (as fraction of lowercase 'o' height)",word_spacing,&word_spacing,
                                1,0.01,10.,NULL);
            if (status<0)
                return(status);
            }
        else if (!stricmp(buf,"w"))
            {
            status=user_string("Wrap text",ansyesno,text_wrap?"y":"n");
            if (status<0)
                return(status);
            text_wrap=!status ? 1 : 0;
            if (text_wrap)
                {
                int reflow_short=0;
                status=user_string("Re-flow short lines",ansyesno,reflow_short?"y":"n");
                if (status<0)
                    return(status);
                if (!status)
                    text_wrap=2;
                status=user_string("Preserve indentation",ansyesno,preserve_indentation?"y":"n");
                if (status<0)
                    return(status);
                preserve_indentation=!status;
                status=user_string("Detect/eliminate hyphens",ansyesno,k2_hyphen_detect?"y":"n");
                if (status<0)
                    return(status);
                k2_hyphen_detect=!status;
                }
            }
        else if (!stricmp(buf,"sm"))
            {
            status=user_string("Show marked source",ansyesno,show_marked_source==1?"y":"n");
            if (status<0)
                return(status);
            show_marked_source=!status;
            }
        else if (!stricmp(buf,"x"))
            {
            status=user_string("Exit on completion",ansyesno,exit_on_complete==1?"y":"n");
            if (status<0)
                return(status);
            exit_on_complete=!status;
            }
        aprintf("\n");
        }
    }


static int user_float(char *message,double defval,double *dstval,int nmax,
                      double min,double max,char *extramessage)

    {
    char buf[256];
    int i,na;
    double v[8];

    if (nmax>8)
        nmax=8;
    while (1)
        {
        if (extramessage!=NULL && extramessage[0]!='\0')
            aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL "\n",extramessage);
        aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL " [%g]: " TTEXT_INPUT,
                message,defval);
        fgets(buf,255,stdin);
        aprintf(TTEXT_NORMAL "\n");
        clean_line(buf);
        if (buf[0]=='\0')
            {
            dstval[0]=defval;
            return(1);
            }
        if (tolower(buf[0])=='q')
            return(-1);
        na=string_read_doubles(buf,v,nmax);
        if (na<=0)
            {
            aprintf(TTEXT_WARN "\aThe response '%s' is not valid.\n\n" TTEXT_NORMAL,buf);
            continue;
            }
        for (i=0;i<na;i++)
            {
            if (atof(buf)<min || atof(buf)>max)
                {
                aprintf(TTEXT_WARN "\aThe response must be between %g and %g.\n\n" TTEXT_NORMAL,min,max);
                break;
                }
            }
        if (i<na)
            continue;
        for (i=0;i<na;i++)
            dstval[i]=v[i];
        return(na);
        }
    }


static int user_integer(char *message,int defval,int *dstval,int min,int max)

    {
    char buf[256];

    while (1)
        {
        aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL " [%d]: " TTEXT_INPUT,
                message,defval);
        fgets(buf,255,stdin);
        aprintf(TTEXT_NORMAL "\n");
        clean_line(buf);
        if (buf[0]=='\0')
            {
            (*dstval)=defval;
            return(0);
            }
        if (tolower(buf[0])=='q')
            return(-1);
        if (!is_an_integer(buf))
            {
            aprintf(TTEXT_WARN "\aThe response '%s' is not valid.\n\n" TTEXT_NORMAL,buf);
            continue;
            }
        if (atoi(buf)<min || atoi(buf)>max)
            {
            aprintf(TTEXT_WARN "\aThe response must be between %d and %d.\n\n" TTEXT_NORMAL,min,max);
            continue;
            }
        (*dstval)=atoi(buf);
        return(0);
        }
    }


static int user_any_string(char *message,char *dstval,int maxlen,char *defname)

    {
    char buf[1024];

    if (maxlen>1023)
        maxlen=1023;
    while (1)
        {
        aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL " [%s]: " TTEXT_INPUT,
                message,defname);
        fgets(buf,maxlen,stdin);
        aprintf(TTEXT_NORMAL "\n");
        clean_line(buf);
        if (buf[0]=='\0')
            {
            dstval[0]='\0';
            return(0);
            }
        if (tolower(buf[0])=='q')
            return(-1);
        strncpy(dstval,buf,maxlen-1);
        dstval[maxlen-1]='\0';
        return(0);
        }
    }


static int user_string(char *message,char *selection[],char *def)

    {
    char buf[256];
    int i;

    while (1)
        {
        aprintf(TTEXT_BOLD2 "%s" TTEXT_NORMAL " (",message);
        for (i=0;selection[i][0]!='\0';i++)
            aprintf("%s" TTEXT_BOLD "%c" TTEXT_NORMAL "%s",
                    i>0 ? ", " : "",selection[i][0],&selection[i][1]);
        aprintf(") [%c]: " TTEXT_INPUT,def[0]);
        fgets(buf,255,stdin);
        aprintf(TTEXT_NORMAL "\n");
        clean_line(buf);
        if (buf[0]=='\0')
            strcpy(buf,def);
        if (tolower(buf[0])=='q')
            return(-1);
        for (i=0;selection[i][0]!='\0';i++)
            if (tolower(buf[0])==tolower(selection[i][0]))
                return(i);
        aprintf(TTEXT_WARN "\aThe response '%s' is not valid.\n\n" TTEXT_NORMAL,
                buf);
        }
    }


static void strcpy_no_spaces(char *d,char *s)

    {
    int i,j;

    for (j=i=0;s[i]!='\0';i++)
        {
        if (s[i]==' ' || s[i]=='\t')
            continue;
        d[j++]=s[i];
        }
    d[j]='\0';
    }


static int valid_page_range(char *s)

    {
    int i;

    for (i=0;s[i]!='\0';i++)
        {
        if (s[i]==' ' || s[i]=='\t' || s[i]==',' || s[i]=='-' || (s[i]>='0' && s[i]<='9'))
            continue;
        else
            break;
        }
    return(s[i]=='\0');
    }


static void k2pdfopt_proc_wildarg(char *arg)

    {
    int i;

    if (wfile_status(arg)==0)
        {
        FILELIST *fl,_fl;

        fl=&_fl;
        filelist_init(fl);
        filelist_fill_from_disk_1(fl,arg,0,0);
        if (fl->n==0)
            {
            printf("File or folder %s could not be opened.\n",arg);
            return;
            }
        for (i=0;i<fl->n;i++)
            {
            char fullname[512];
            wfile_fullname(fullname,fl->dir,fl->entry[i].name);
            k2pdfopt_proc_arg(fullname);
            }
        }
    else
        k2pdfopt_proc_arg(arg);
    }


static void k2pdfopt_proc_arg(char *arg)

    {
    char filename[256];
    int i;
    double rot;

    strcpy(filename,arg);
    if (wfile_status(filename)==0)
        {
        printf("File or folder %s could not be opened.\n",filename);
        return;
        }
    if (wfile_status(filename)==2)
        {
        static char *iolist[]={"*.png","*.jpg",""};
        static char *eolist[]={""};
        static char *pdflist[]={"*.pdf",""};
        FILELIST *fl,_fl;

        fl=&_fl;
        filelist_init(fl);
        filelist_fill_from_disk(fl,filename,iolist,eolist,0,0);
        if (fl->n==0)
            {
            filelist_fill_from_disk(fl,filename,pdflist,eolist,0,0);
            if (fl->n>0)
                {
                for (i=0;i<fl->n;i++)
                    {
                    char fullname[512];

                    wfile_fullname(fullname,filename,fl->entry[i].name);
                    if (fabs(src_rot-SRCROT_AUTO)<.5 || fabs(src_rot-SRCROT_AUTOEP)<.5)
                        rot=k2pdfopt_proc_one(fullname,SRCROT_AUTO);
                    else
                        rot=src_rot;
                    k2pdfopt_proc_one(fullname,rot);
                    }
                }
            else
                printf("No files in folder %s.\n\n",filename);
            }
        filelist_free(fl);
        return;
        }
    if (fabs(src_rot-SRCROT_AUTO)<.5 || fabs(src_rot-SRCROT_AUTOEP)<.5)
        rot=k2pdfopt_proc_one(filename,SRCROT_AUTO);
    else
        rot=src_rot;
    k2pdfopt_proc_one(filename,rot);
    }

/*
** Ensure that max_region_width_inches will be > MIN_REGION_WIDTH_INCHES
**
** Should only be called once, after all params are set.
**
*/
static void adjust_params_init(void)

    {
    if (dst_landscape)
        {
        dst_width=dst_userheight;
        dst_height=dst_userwidth;
        }
    else
        {
        dst_width=dst_userwidth;
        dst_height=dst_userheight;
        }
    if (dst_mar<0.)
        dst_mar=0.02;
    if (dst_martop<0.)
        dst_martop=dst_mar;
    if (dst_marbot<0.)
        dst_marbot=dst_mar;
    if (dst_marleft<0.)
        dst_marleft=dst_mar;
    if (dst_marright<0.)
        dst_marright=dst_mar;
    if ((double)dst_width/dst_dpi - dst_marleft - dst_marright < MIN_REGION_WIDTH_INCHES)
        {
        int olddpi;
        olddpi = dst_dpi;
        dst_dpi = (int)((double)dst_width/(MIN_REGION_WIDTH_INCHES+dst_marleft+dst_marright));
        aprintf(TTEXT_BOLD2 "Output DPI of %d is too large.  Reduced to %d." TTEXT_NORMAL "\n\n",
                olddpi,dst_dpi);
        }
    }

static void set_region_widths(void)

    {
    max_region_width_inches=display_width_inches=(double)dst_width/dst_dpi;
    max_region_width_inches -= (dst_marleft+dst_marright);
    /* This is ensured by adjust_dst_dpi() as of v1.17 */
    /*
    if (max_region_width_inches < MIN_REGION_WIDTH_INCHES)
        max_region_width_inches = MIN_REGION_WIDTH_INCHES;
    */
    }


static double k2pdfopt_proc_one(char *filename,double rot_deg)

    {
    static MASTERINFO _masterinfo,*masterinfo;
    char dstfile[256];
    char markedfile[256];
    char cmd[512];
    char rotstr[128];
    PAGEINFO _pageinfo,*pageinfo;
    WILLUSBITMAP _src,*src;
    WILLUSBITMAP _srcgrey,*srcgrey;
    WILLUSBITMAP _marked,*marked;
    int i,status,white,pw,np,src_type,or_detect,orep_detect,second_time_through;
    int pagecount,pagestep,pages_done,is_gray;
    FILELIST *fl,_fl;
    int folder,dpi;
    double size,area_ratio,bormean;
    static char *mupdferr1=TTEXT_WARN "\a\n ** ERROR reading from " TTEXT_BOLD2 "%s" TTEXT_WARN ".  Will try Ghostscript!\n\n" TTEXT_NORMAL;
    static char *mupdferr=TTEXT_WARN "\a\n ** ERROR reading page %d from " TTEXT_BOLD2 "%s" TTEXT_WARN ".  Will try Ghostscript!\n\n" TTEXT_NORMAL;
    static char *djvuerr=TTEXT_WARN "\a\n ** ERROR reading page %d from " TTEXT_BOLD2 "%s" TTEXT_NORMAL ".\n\n";

    masterinfo=&_masterinfo;
#ifdef HAVE_OCR
    if (dst_ocr)
        k2pdfopt_ocr_init();
#endif
    if (use_crop_boxes)
        {
        pageinfo=&_pageinfo;
        pdfboxes_init(&pageinfo->boxes);
        }
    else
        pageinfo=NULL;
    or_detect=(fabs(rot_deg-SRCROT_AUTO)<.5);
    orep_detect=(fabs(src_rot-SRCROT_AUTOEP)<.5);
    if ((fabs(src_rot-SRCROT_AUTO)<.5 || orep_detect) && !or_detect)
        second_time_through=1;
    else
        second_time_through=0;
    white=src_whitethresh; /* Will be set by adjust_contrast() or set to src_whitethresh */
    if (or_detect && src_dpi>300)
        dpi=300;
    else
        dpi=src_dpi;
    set_region_widths();
    folder=(wfile_status(filename)==2);
    /*
    if (folder && !second_time_through)
        aprintf("Processing " TTEXT_INPUT "BITMAP FOLDER %s" TTEXT_NORMAL "...\n",
               filename);
    */
    /*
    else
        aprintf("Processing " TTEXT_BOLD2 "PDF FILE %s" TTEXT_NORMAL "...\n",
               filename);
    */
    if (debug)
        {
        strcpy(masterinfo->debugfolder,"k2_dst_dir");
        wfile_remove_dir(masterinfo->debugfolder,1);
        wfile_makedir(masterinfo->debugfolder);
        }
    else
        masterinfo->debugfolder[0]='\0';
    fl=&_fl;
    filelist_init(fl);
    if (folder)
        {
        char basename[256];
        static char *iolist[]={"*.png","*.jpg",""};
        static char *eolist[]={""};

        wfile_basespec(basename,filename);
        if (!second_time_through)
            aprintf("Searching folder " TTEXT_BOLD2 "%s" TTEXT_NORMAL " ... ",basename);
        fflush(stdout);
        filelist_fill_from_disk(fl,filename,iolist,eolist,0,0);
        if (fl->n<=0)
            {
            printf("No bitmaps %s!\n",folder?"found":"created");
            printf("Command:  %s.\n\n",cmd);
            k2_enter_to_exit();
            exit(10);
            }
        if (!second_time_through)
            printf("%d bitmaps %s.\n",(int)fl->n,folder?"found":"created");
        filelist_sort_by_name(fl);
        }
    src=&_src;
    srcgrey=&_srcgrey;
    marked=&_marked;
    bmp_init(&masterinfo->bmp);
    bmp_init(src);
    bmp_init(srcgrey);
    bmp_init(marked);
#ifdef HAVE_OCR
    if (dst_ocr)
        ocrwords_clear(dst_ocrwords);
#endif
    if (dst_color)
        masterinfo->bmp.bpp=24;
    else
        {
        int ii;
        masterinfo->bmp.bpp=8;
        for (ii=0;ii<256;ii++)
            masterinfo->bmp.red[ii]=masterinfo->bmp.blue[ii]=masterinfo->bmp.green[ii]=ii;
        }
    masterinfo->bmp.width=dst_width;
    area_ratio = 8.5*11.0*dst_dpi*dst_dpi / (dst_width*dst_height);
    masterinfo->bmp.height=dst_height*area_ratio*1.5;
    if (!or_detect)
        {
        bmp_alloc(&masterinfo->bmp);
        bmp_fill(&masterinfo->bmp,255,255,255);
        }
    masterinfo->rows=0;
    masterinfo->published_pages=0;
    masterinfo->wordcount=0;
    pw=0;
    if (!or_detect)
        {
        wfile_newext(dstfile,filename,"");
        strcat(dstfile,"_k2opt.pdf");
        if ((status=overwrite_fail(dstfile))!=0)
            {
            bmp_free(&masterinfo->bmp);
            if (folder)
                filelist_free(fl);
            if (status<0)
                exit(20);
            return(0.);
            }
        if (pdffile_init(gpdf,dstfile,1)==NULL)
            {
            printf("\n\aCannot open PDF file %s for output!\n\n",dstfile);
            exit(30);
            }
        if (show_marked_source)
            {
            wfile_newext(markedfile,dstfile,"");
            if (strlen(markedfile)>6 && !strcmp(&markedfile[strlen(markedfile)-6],"_k2opt"))
                markedfile[strlen(markedfile)-6]='\0';
            strcat(markedfile,"_marked.pdf");
            if (pdffile_init(mpdf,markedfile,1)==NULL)
                {
                printf("\n\aCannot open PDF file %s for marked output!\n\n",markedfile);
                exit(40);
                }
            }
        }
    bmp_set_pdf_dpi(dpi); /* Input DPI */
    if (!stricmp(wfile_ext(filename),"pdf"))
        src_type = SRC_TYPE_PDF;
    else if (!stricmp(wfile_ext(filename),"djvu"))
        src_type = SRC_TYPE_DJVU;
    else if (!stricmp(wfile_ext(filename),"djv"))
        src_type = SRC_TYPE_DJVU;
    else
        src_type = SRC_TYPE_OTHER;
    if (src_type==SRC_TYPE_PDF || src_type==SRC_TYPE_DJVU)
        {
        sys_set_decimal_period(1);
        np= (src_type==SRC_TYPE_PDF) ? bmpmupdf_numpages(filename)
                                     : bmpdjvu_numpages(filename);
        sys_set_decimal_period(1);
        if (np==-1 && !usegs && src_type==SRC_TYPE_PDF)
            {
            aprintf(mupdferr1,filename);
            usegs=1;
            }
        if (np<=0 && src_type==SRC_TYPE_PDF)
            np=pdf_numpages(filename);
        pagecount=pagelist_count(pagelist,np);
        }
    else
        {
        np=-1;
        pagecount=-1;
        }
    if (pagecount<0 || !or_detect)
        pagestep=1;
    else
        {
        pagestep=pagecount/10;
        if (pagestep<1)
            pagestep=1;
        }
    pages_done=0;
    if (np>0 && pagecount==0)
        {
        if (!second_time_through)
            aprintf("\a\n" TTEXT_WARN "No pages to convert (-p %s)!" TTEXT_NORMAL "\n\n",pagelist);
        if (use_crop_boxes)
            pdfboxes_free(&pageinfo->boxes);
        bmp_free(&masterinfo->bmp);
        if (folder)
            filelist_free(fl);
        return(0.);
        }
    if (!second_time_through)
        {
        aprintf("Reading ");
        if (pagecount>0)
           {
           if (pagecount<np)
               aprintf("%d out of %d page%s",pagecount,np,np>1?"s":"");
           else
               aprintf("%d page%s",np,np>1?"s":"");
           }
        else
           aprintf("pages");
        aprintf(" from " TTEXT_BOLD2 "%s" TTEXT_NORMAL " ...\n",filename);
        }
    if (or_detect)
        aprintf("\nDetecting document orientation ... ");
    bormean=1.0;
    for (i=0;1;i+=pagestep)
        {
        BMPREGION region;
        char bmpfile[256];
        int pageno;

        pageno=0;
        is_gray=0;
        if (folder)
            {
            if (i>=fl->n)
                break;
            wfile_fullname(bmpfile,fl->dir,fl->entry[i].name);
            status=bmp_read(src,bmpfile,stdout);
            }
        else
            { 
            /* If it's not a PDF/DJVU, only read it once. */
            if (i>0 && src_type!=SRC_TYPE_PDF && src_type!=SRC_TYPE_DJVU)
                break;
            if (pagecount>0 && i+1>pagecount)
                break;
            pageno = pagelist_page_by_index(pagelist,i,np);
            if ((src_type==SRC_TYPE_PDF || src_type==SRC_TYPE_DJVU) 
                     && !pagelist_page_by_index(pagelist,pageno,np))
                continue;
            status=-1;
            if (!usegs)
                {
                static int errcnt=0;

                sys_set_decimal_period(1);
                /* Check bitmap size */
                status= bmp_get_one_document_page(src,src_type,filename,
                                                  pageno,10.,8,stdout);
                sys_set_decimal_period(1);
                if (status<0)
                    {
                    if (errcnt==0)
                        aprintf(src_type==SRC_TYPE_PDF ? mupdferr : djvuerr,pageno,filename);
                    errcnt++;
                    /* Switch to PS for rest of pages. */
                    if (src_type==SRC_TYPE_PDF)
                        usegs=1;
                    }
                if (status>=0)
                    {
                    double npix;
                    npix = (double)(dpi/10.)*(dpi/10.)*src->width*src->height;
                    if (npix > 2.5e8)
                        {
                        static int pixwarn=0;
                        if (!pixwarn)
                            {
                            int ww,hh;
                            ww=(int)((double)(dpi/10.)*src->width+.5);
                            hh=(int)((double)(dpi/10.)*src->height+.5);
                            aprintf("\a\n" TTEXT_WARN "\n\a ** Source resolution is very high (%d x %d pixels)!\n"
                                    "    You may want to reduce the -odpi or -idpi setting!\n"
                                    "    k2pdfopt may crash when reading the source file..."
                                    TTEXT_NORMAL "\n\n",ww,hh);
                            pixwarn=1;
                            }
                        }
                    sys_set_decimal_period(1);
                    if (dst_color)
                        status= bmp_get_one_document_page(src,src_type,filename,
                                                          pageno,dpi,24,stdout);
                    else
                        {
                        status= bmp_get_one_document_page(src,src_type,filename,
                                                          pageno,dpi,8,stdout);
                        is_gray=1;
                        }
                    if (debug && or_detect)
                        printf("Checking orientation of page %d ... ",pageno);
                    sys_set_decimal_period(1);
                    if (status<0)
                        {
                        if (errcnt==0)
                            aprintf(src_type==SRC_TYPE_PDF ? mupdferr : djvuerr,pageno,filename);
                        errcnt++;
                        /* Switch to PS for rest of pages. */
                        if (src_type==SRC_TYPE_PDF)
                            usegs=1;
                        }
                    }
                }
            if (status<0 && src_type==SRC_TYPE_PDF)
                {
                if (willusgs_init(stdout) < 0)
                    {
                    k2_enter_to_exit();
                    exit(20);
                    }
                bmp_set_pdf_pageno(i+1);
                /*
                aprintf("Converting " TTEXT_BOLD2 "%s" TTEXT_NORMAL 
                    " page %2d to %d dpi bitmap ... ",filename,i,dpi);
                fflush(stdout);
                */
                sys_set_decimal_period(1);
                status=bmp_read(src,filename,NULL);
                sys_set_decimal_period(1);
                }
            }
        if (status<0)
            {
            /* Allow to continue */
            /*
            if (!folder)
                break;
            */
            if (!second_time_through)
                aprintf(TTEXT_WARN "\n\aCould not read file %s.\n" TTEXT_NORMAL,bmpfile);
            continue;
            }
        if (use_crop_boxes)
            {
            pageinfo->pageno = pageno;
            pageinfo->page_rot_deg=0.;
            }
        if (!or_detect && !orep_detect && rot_deg!=0)
            {
            bmp_rotate_right_angle(src,rot_deg);
            if (use_crop_boxes)
                pageinfo->page_rot_deg=rot_deg;
            }
        if (is_gray || bmp_is_grayscale(src))
            bmp_copy(srcgrey,src);
        else
            bmp_convert_to_greyscale_ex(srcgrey,src);
        if (!or_detect && (dst_color || show_marked_source))
            bmp_promote_to_24(src);
        adjust_contrast(src,srcgrey,&white);
        /*
        if (src_whitethresh>0)
            white=src_whitethresh;
        */
        rotstr[0]='\0';
        if (or_detect || orep_detect)
            {
            double bor,rotnow;

            bor=bitmap_orientation(srcgrey);
            if (debug)
                printf("orientation factor = %g\n",bor);
            if (or_detect)
                {
                pages_done++;
                bormean *= bor;
                continue;
                }
            rotnow=rot_deg;
            if (fabs(rot_deg-270)<.5)
                {
                if (bor>10.)
                    {
                    sprintf(rotstr,"(custom rotation) ");
                    rotnow=0.;
                    }
                }
            else if (fabs(rot_deg)<.5)
                {
                if (bor<.1)
                    {
                    sprintf(rotstr,"(custom rotation) ");
                    rotnow=270.;
                    }
                }
            if (rotnow!=0)
                {
                bmp_rotate_right_angle(srcgrey,rotnow);
                if (dst_color)
                    bmp_rotate_right_angle(src,rotnow);
                }
            }
        if (erase_vertical_lines>0)
            bmp_detect_vertical_lines(srcgrey,src,(double)src_dpi,0.005,0.25,
                            min_column_height_inches,src_autostraighten,white);
        if (src_autostraighten > 0.)
            {
            double rot;
            rot=bmp_autostraighten(src,srcgrey,white,src_autostraighten,0.1,debug,stdout);
            if (use_crop_boxes)
                pageinfo->page_rot_deg += rot;
            }
        white_margins(src,srcgrey);
        aprintf("\n" TTEXT_HEADER "SOURCE PAGE %d",pageno);
        if (pagecount>0)
            {
            if (pagelist[0]!='\0')
                aprintf(" (%d of %d)",pages_done+1,pagecount);
            else
                aprintf(" of %d",pagecount);
            }
        aprintf(TTEXT_NORMAL 
                " (%.1f x %.1f in) ... %s",(double)srcgrey->width/src_dpi,
                  (double)srcgrey->height/src_dpi,rotstr);
        fflush(stdout);
        region.r1 = 0;
        region.r2 = srcgrey->height-1;
        region.c1 = 0;
        region.c2 = srcgrey->width-1;
        region.bgcolor = white;
        region.bmp = src;
        region.bmp8 = srcgrey;
        if (show_marked_source)
            {
            if (dst_color)
                {
                bmp_copy(marked,src);
                region.marked=marked;
                }
            else
                region.marked=region.bmp;
            }
        masterinfo->bgcolor=white;
        masterinfo->fit_to_page = dst_fit_to_page;
        /* Check to see if master bitmap might need more room */
        bmpregion_multicolumn_add(&region,masterinfo,1,pageinfo,
                                 pages_done==0. ? 0. : (int)(0.25*src_dpi+.5));
        pages_done++;
        if (verbose)
            {
            printf("    master->rows=%d\n",masterinfo->rows);
            printf("Publishing...\n");
            }
        /* Reset the display order for this source page */
        if (show_marked_source)
            mark_source_page(NULL,0,0xf);
        if (dst_fit_to_page!=-2)
            publish_master(masterinfo,pageinfo,dst_break_pages);
        if (show_marked_source)
            publish_marked_page(dst_color ? marked : src);
        printf("%d new pages saved.\n",masterinfo->published_pages-pw);
        pw=masterinfo->published_pages;
        }
    bmp_free(marked);
    bmp_free(srcgrey);
    bmp_free(src);
    /* Determine orientation of document */
    if (or_detect)
        {
        if (pages_done>0)
            {
            double thresh;
            /*
            ** bormean = 1.0 means neutral
            ** bormean >> 1.0 means document is likely portrait (no rotation necessary)
            ** bormean << 1.0 means document is likely landscape (need to rotate it)
            */
            bormean = pow(bormean,1./pages_done);
            thresh=10.-(double)pages_done/2.;
            if (thresh<5.)
                thresh=5.;
            if (bormean < 1./thresh)
                {
                printf("Rotating clockwise.\n");
                if (use_crop_boxes)
                    pdfboxes_free(&pageinfo->boxes);
                bmp_free(&masterinfo->bmp);
                if (folder)
                    filelist_free(fl);
                return(270.);
                }
            }
        printf("No rotation necessary.\n");
        if (use_crop_boxes)
            pdfboxes_free(&pageinfo->boxes);
        bmp_free(&masterinfo->bmp);
        if (folder)
            filelist_free(fl);
        return(0.);
        }
    if (!dst_break_pages && dst_fit_to_page!=-2)
        publish_master(masterinfo,pageinfo,1);
    {
    char producer[64];
    sprintf(producer,"K2pdfopt %s",VERSION);
    pdffile_finish(gpdf,producer);
    pdffile_close(gpdf);
    if (show_marked_source)
        {
        pdffile_finish(mpdf,producer);
        pdffile_close(mpdf);
        }
    }
    if (debug || verbose)
        printf("Cleaning up ...\n\n");
    /*
    if (folder)
        aprintf("Processing on " TTEXT_INPUT "folder %s" TTEXT_NORMAL " complete.  Total %d pages.\n\n",filename,masterinfo->published_pages);
    else
        aprintf("Processing on " TTEXT_BOLD2 "file %s" TTEXT_NORMAL " complete.  Total %d pages.\n\n",filename,masterinfo->published_pages);
    */
    size=wfile_size(dstfile);
    aprintf("\n" TTEXT_BOLD "%d pages" TTEXT_NORMAL,masterinfo->published_pages);
    if (masterinfo->wordcount>0)
        aprintf(" (%d words)",masterinfo->wordcount);
    aprintf(" written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",
            dstfile,size/1024./1024.);
    if (show_marked_source)
        {
        size=wfile_size(markedfile);
        aprintf(TTEXT_BOLD "%d pages" TTEXT_NORMAL " written to " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB).\n\n",pages_done,markedfile,size/1024./1024.);
        }
    bmp_free(&masterinfo->bmp);
    if (use_crop_boxes)
        pdfboxes_free(&pageinfo->boxes);
    if (folder)
        filelist_free(fl);
    return(0.);
    }


static int overwrite_fail(char *outname)

    {
    double size_mb;
    char basepath[512];
    char buf[512];
    char newname[512];
    static int all=0;

    if (wfile_status(outname)==0)
        return(0);
    if (overwrite_minsize_mb < 0.)
        return(0);
    if (all)
        return(0);
    size_mb = wfile_size(outname)/1024./1024.;
    if (size_mb < overwrite_minsize_mb)
        return(0);
    wfile_basepath(basepath,outname);
    strcpy(newname,outname);
    printf("\n\a");
    while (1)
        {
        while (1)
            {
            aprintf("File " TTEXT_MAGENTA "%s" TTEXT_NORMAL " (%.1f MB) already exists!\n"
                      "   Overwrite it (y[es]/n[o]/a[ll]/q[uit])? " TTEXT_INPUT,
                    newname,size_mb);
            fgets(buf,16,stdin);
            aprintf(TTEXT_NORMAL);
            clean_line(buf);
            buf[0]=tolower(buf[0]);
            if (buf[0]!='y' && buf[0]!='n' && buf[0]!='a' && buf[0]!='q')
                {
                aprintf("\a\n  ** Must respond with 'y', 'n', 'a', or 'q' **\n\n");
                continue;
                }
            break;
            }
        if (buf[0]=='q')
            return(-1);
        if (buf[0]=='a' || buf[0]=='y')
            {
            if (buf[0]=='a')
                all=1;
            return(0);
            }
        aprintf("Enter a new output base name (.pdf will be appended, q=quit).\n"
                "New name: " TTEXT_INPUT);
        fgets(buf,255,stdin);
        aprintf(TTEXT_NORMAL);
        clean_line(buf);
        if (!stricmp(buf,"q"))
            return(-1);
        if (buf[0]=='/' || buf[0]=='\\' || buf[1]==':')
            strcpy(newname,buf);
        else
            wfile_fullname(newname,basepath,buf);
        if (!strcmp(wfile_ext(newname),""))
            strcat(newname,".pdf");
        if (wfile_status(newname)==0)
            break;
        }
    strcpy(outname,newname);
    return(0);
    }

/*
** src guaranteed to be 24-bit color
*/
static void publish_marked_page(WILLUSBITMAP *src)

    {
    int newdpi;
    WILLUSBITMAP *bmp,_bmp;

#if (WILLUSDEBUGX & 9)
static int count=1;
char filename[256];
sprintf(filename,"outsrc%02d.png",count++);
bmp_write(src,filename,stdout,100);
#endif
    bmp=&_bmp;
    bmp_init(bmp);
    newdpi = src_dpi / 2;
    bmp->width=src->width/2;
    bmp->height=src->height/2;
    bmp->bpp=24;
    bmp_alloc(bmp);
    bmp_resample(bmp,src,(double)0.,(double)0.,(double)src->width,(double)src->height,
                 bmp->width,bmp->height);
    pdffile_add_bitmap(mpdf,bmp,newdpi,-1,1);
    bmp_free(bmp);
    }


/*
** Mark the region
** mark_flags & 1 :  Mark top
** mark_flags & 2 :  Mark bottom
** mark_flags & 4 :  Mark left
** mark_flags & 8 :  Mark right
**
*/
static void mark_source_page(BMPREGION *region0,int caller_id,int mark_flags)

    {
    static int display_order=0;
    int i,n,nn,fontsize,r,g,b,shownum;
    char num[16];
    BMPREGION *region,_region;
    BMPREGION *clip,_clip;

    if (!show_marked_source)
        return;

    if (region0==NULL)
        {
        display_order=0;
        return;
        }

    region=&_region;
    (*region)=(*region0);

    /* Clip the region w/ignored margins */
    clip=&_clip;
    clip->bmp=region0->bmp;
    get_white_margins(clip);
    if (region->c1 < clip->c1)
        region->c1 = clip->c1;
    if (region->c2 > clip->c2)
        region->c2 = clip->c2;
    if (region->r1 < clip->r1)
        region->r1 = clip->r1;
    if (region->r2 > clip->r2)
        region->r2 = clip->r2;
    if (region->r2 <= region->r1 || region->c2 <= region->c1)
        return;

    /* printf("@mark_source_page(display_order=%d)\n",display_order); */
    if (caller_id==1)
        {
        display_order++;
        shownum=1;
        n=(int)(src_dpi/60.+0.5);
        if (n<5)
            n=5;
        r=255;
        g=b=0;
        }
    else if (caller_id==2)
        {
        shownum=0;
        n=2;
        r=0;
        g=0;
        b=255;
        }
    else if (caller_id==3)
        {
        shownum=0;
        n=(int)(src_dpi/80.+0.5);
        if (n<4)
            n=4;
        r=0;
        g=255;
        b=0;
        }
    else if (caller_id==4)
        {
        shownum=0;
        n=2;
        r=255;
        g=0;
        b=255;
        }
    else
        {
        shownum=0;
        n=2;
        r=140;
        g=140;
        b=140;
        }
    if (n<2)
        n=2;
    nn=(region->c2+1-region->c1)/2;
    if (n>nn)
        n=nn;
    nn=(region->r2+1-region->r1)/2;
    if (n>nn)
        n=nn;
    if (n<1)
        n=1;
    for (i=0;i<n;i++)
        {
        int j;
        unsigned char *p;
        if (mark_flags & 1)
            {
            p=bmp_rowptr_from_top(region->marked,region->r1+i)+region->c1*3;
            for (j=region->c1;j<=region->c2;j++,p+=3)
                {
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
            }
        if (mark_flags & 2)
            {
            p=bmp_rowptr_from_top(region->marked,region->r2-i)+region->c1*3;
            for (j=region->c1;j<=region->c2;j++,p+=3)
                {
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
            }
        if (mark_flags & 16) /* rowbase */
            {
            p=bmp_rowptr_from_top(region->marked,region->rowbase-i)+region->c1*3;
            for (j=region->c1;j<=region->c2;j++,p+=3)
                {
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
            }
        if (mark_flags & 4)
            for (j=region->r1;j<=region->r2;j++)
                {
                p=bmp_rowptr_from_top(region->marked,j)+(region->c1+i)*3;
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
        if (mark_flags & 8)
            for (j=region->r1;j<=region->r2;j++)
                {
                p=bmp_rowptr_from_top(region->marked,j)+(region->c2-i)*3;
                p[0]=r;
                p[1]=g;
                p[2]=b;
                }
        }
    if (!shownum)
        return;
    fontsize=region->c2-region->c1+1;
    if (fontsize > region->r2-region->r1+1)
        fontsize=region->r2-region->r1+1;
    fontsize /= 2;
    if (fontsize > src_dpi)
        fontsize = src_dpi;
    if (fontsize < 5)
        return;
    fontrender_set_typeface("helvetica-bold");
    fontrender_set_fgcolor(r,g,b);
    fontrender_set_bgcolor(255,255,255);
    fontrender_set_pixel_size(fontsize);
    fontrender_set_justification(4);
    fontrender_set_or(1);
    sprintf(num,"%d",display_order);
    fontrender_render(region->marked,(double)(region->c1+region->c2)/2.,
                      (double)(region->marked->height-((region->r1+region->r2)/2.)),num,0,NULL);    
    /* printf("    done mark_source_page.\n"); */
    }


static int bmp_get_one_document_page(WILLUSBITMAP *src,int src_type,char *filename,
                                     int pageno,double dpi,int bpp,FILE *out)

    {
    if (src_type==SRC_TYPE_PDF)
        return(bmpmupdf_pdffile_to_bmp(src,filename,pageno,dpi*document_scale_factor,bpp));
    return(bmpdjvu_djvufile_to_bmp(src,filename,pageno,dpi*document_scale_factor,bpp,out));
    }


static int wait_enter(void)

    {
    char buf[32];

    aprintf(TTEXT_BOLD2 "Press <ENTER> to continue (q to quit)." TTEXT_NORMAL);
    fflush(stdout);
    fgets(buf,16,stdin);
    if (tolower(buf[0])=='q')
        return(-1);
    return(0);
    }


/*
** Process full source page bitmap into rectangular regions and add
** to the destination bitmap.  Start by looking for columns.
**
** level = recursion level.  First call = 1, then 2, ...
**
*/
static void bmpregion_multicolumn_add(BMPREGION *region,MASTERINFO *masterinfo,int level,
                                      PAGEINFO *pageinfo,int colgap0_pixels)

    {
    static char *funcname="bmpregion_multicolumn_add";
    int *row_black_count;
    int r2,rh,r0,cgr,maxlevel;
    BMPREGION *srcregion,_srcregion;
    BMPREGION *newregion,_newregion;
    BMPREGION *pageregion;
    double minh;
    int ipr,npr,na;
    int *colcount,*rowcount;

    willus_dmem_alloc_warn(1,(void **)&colcount,sizeof(int)*(region->c2+1),funcname,10);
    willus_dmem_alloc_warn(2,(void **)&rowcount,sizeof(int)*(region->r2+1),funcname,10);
    maxlevel = max_columns/2;
    if (debug)
        printf("@bmpregion_multicolumn_add (%d,%d) - (%d,%d) lev=%d\n",
               region->c1,region->r1,region->c2,region->r2,level);
    newregion=&_newregion;
    (*newregion)=(*region);
    /* Establish colcount, rowcount arrays */
    bmpregion_trim_margins(newregion,colcount,rowcount,0xf);
    (*newregion)=(*region);
    srcregion=&_srcregion;
    (*srcregion)=(*region);
    /* How many page regions do we need? */
    minh = min_column_height_inches;
    if (minh < .01)
        minh = .1;
    na = (srcregion->r2-srcregion->r1+1)/src_dpi/minh;
    if (na<1)
        na=1;
    na += 16;
    /* Allocate page regions */
    willus_dmem_alloc_warn(3,(void **)&pageregion,sizeof(BMPREGION)*na,funcname,10);
#ifdef COMMENT
    mindr=src_dpi*.045; /* src->height/250; */
    if (mindr<1)
        mindr=1;
#endif
//    white=250;
//    for (i=0;i<src->width;i++)
//        colcount[i]=0;
    if (debug)
        bmpregion_row_histogram(region);

    /*
    ** Store information about which rows are mostly clear for future
    ** processing (saves processing time).
    */
    willus_dmem_alloc_warn(4,(void **)&row_black_count,region->bmp8->height*sizeof(int),funcname,10);
    for (cgr=0,r0=0;r0<region->bmp8->height;r0++)
        {
        row_black_count[r0]=bmpregion_row_black_count(region,r0);
        if (row_black_count[r0]==0)
            cgr++;
        /*
        int dr;
        dr=mindr;
        if (r0+dr>region->bmp8->height)
            dr=region->bmp8->height-r0;
        if ((row_is_clear[r0]=bmpregion_row_mostly_white(region,r0,dr))!=0)
            cgr++;
        */
// printf("row_is_clear[%d]=%d\n",r0,row_is_clear[r0]);
        }
    if (verbose)
        printf("%d clear rows.\n",cgr);

    if (max_columns==1)
        {
        pageregion[0]=(*srcregion);
        /* Set c1 negative to indicate full span */
        pageregion[0].c1 = -1-pageregion[0].c1;
        npr=1;
        }
    else
        /* Find all column dividers in source region and store sequentially in pageregion[] array */
        for (npr=0,rh=0;srcregion->r1<=srcregion->r2;srcregion->r1+=rh)
            {
            static char *ierr= TTEXT_WARN "\n\aInternal error--not enough allocated regions.\n"
                               "Please inform the developer at willus.com.\n\n" TTEXT_NORMAL;
            if (npr>=na-3)
                {
                aprintf("%s",ierr);
                break;
                }
            rh=bmpregion_find_multicolumn_divider(srcregion,row_black_count,pageregion,&npr,
                                                  colcount,rowcount);
            if (verbose)
                printf("rh=%d/%d\n",rh,region->r2-region->r1+1);
            }

    /* Process page regions by column */
    if (debug)
        printf("Page regions:  %d\n",npr);
    r2 = -1;
    for (ipr=0;ipr<npr;)
        {
        int r20,jpr,colnum,colgap_pixels;

        for (colnum=1;colnum<=2;colnum++)
            {
            if (debug)
                {
                printf("ipr = %d of %d...\n",ipr,npr);
                printf("COLUMN %d...\n",colnum);
                }
            r20 = r2;
            for (jpr=ipr;jpr<npr;jpr+=2)
                {
                /* If we get to a page region that spans the entire source, stop */
                if (pageregion[jpr].c1<0)
                    break;
                /* See if we should suspend this column and start displaying the next one */
                if (jpr>ipr)
                    {
                    double cpdiff,cdiv1,cdiv2,rowgap1_in,rowgap2_in;

                    if (column_offset_max < 0.)
                        break;
                    /* Did column divider move too much? */
                    cdiv1=(pageregion[jpr].c2+pageregion[jpr+1].c1)/2.;
                    cdiv2=(pageregion[jpr-2].c2+pageregion[jpr-1].c1)/2.;
                    cpdiff=fabs((double)(cdiv1-cdiv2) / (srcregion->c2-srcregion->c1+1));
                    if (cpdiff>column_offset_max)
                        break;
                    /* Is gap between this column region and next column region too big? */
                    rowgap1_in=(double)(pageregion[jpr].r1-pageregion[jpr-2].r2)/src_dpi;
                    rowgap2_in=(double)(pageregion[jpr+1].r1-pageregion[jpr-1].r2)/src_dpi;
                    if (rowgap1_in > 0.28 && rowgap2_in > 0.28)
                        break;
                    }
                (*newregion)=pageregion[src_left_to_right ? jpr+colnum-1 : jpr+(2-colnum)];
                /* Preserve vertical gap between this region and last region */
                if (r20>=0 && newregion->r1-r20>=0)
                    colgap_pixels = newregion->r1-r20;
                else
                    colgap_pixels = colgap0_pixels;
                if (level<maxlevel)
                    bmpregion_multicolumn_add(newregion,masterinfo,level+1,pageinfo,
                                              colgap_pixels);
                else
                    {
                    bmpregion_vertically_break(newregion,masterinfo,
                                  text_wrap,fit_columns?-2.0:-1.0,colcount,rowcount,pageinfo,
                                  colgap_pixels,2*level);
                    if (masterinfo->fit_to_page==-2)
                        publish_master(masterinfo,pageinfo,1);
                    }
                r20=newregion->r2;
                }
            if (r20>r2)
                r2=r20;
            if (jpr==ipr)
                break;
            }
        if (jpr<npr && pageregion[jpr].c1<0)
            {
            if (debug)
                printf("SINGLE COLUMN REGION...\n");
            (*newregion)=pageregion[jpr];
            newregion->c1 = -1-newregion->c1;
            /* dst_add_gap_src_pixels("Col level",masterinfo,newregion->r1-r2); */
            colgap_pixels = newregion->r1-r2;
            bmpregion_vertically_break(newregion,masterinfo,text_wrap,
                          (fit_columns && (level>1)) ? -2.0:-1.0,
                          colcount,rowcount,pageinfo,colgap_pixels,level);
            if (masterinfo->fit_to_page==-2)
                publish_master(masterinfo,pageinfo,1);
            r2=newregion->r2;
            jpr++;
            }
        ipr=jpr;
        }
    willus_dmem_free(4,(double **)&row_black_count,funcname);
    willus_dmem_free(3,(double **)&pageregion,funcname);
    willus_dmem_free(2,(double **)&rowcount,funcname);
    willus_dmem_free(1,(double **)&colcount,funcname);
    }


static void fit_column_to_screen(double column_width_inches)

    {
    double text_width_pixels,lm_pixels,rm_pixels,tm_pixels,bm_pixels;

    if (!column_fitted)
        {
        dpi_org=dst_dpi;
        lm_org=dst_marleft;
        rm_org=dst_marright;
        tm_org=dst_martop;
        bm_org=dst_marbot;
        }
    text_width_pixels = max_region_width_inches*dst_dpi;
    lm_pixels = dst_marleft*dst_dpi;
    rm_pixels = dst_marright*dst_dpi;
    tm_pixels = dst_martop*dst_dpi;
    bm_pixels = dst_marbot*dst_dpi;
    dst_dpi = text_width_pixels / column_width_inches;
    dst_marleft = lm_pixels / dst_dpi;
    dst_marright = rm_pixels / dst_dpi;
    dst_martop = tm_pixels / dst_dpi;
    dst_marbot = bm_pixels / dst_dpi;
    set_region_widths();
    column_fitted=1;
    }


static void restore_output_dpi(void)

    {
    if (column_fitted)
        {
        dst_dpi=dpi_org;
        dst_marleft=lm_org;
        dst_marright=rm_org;
        dst_martop=tm_org;
        dst_marbot=bm_org;
        set_region_widths();
        }
    column_fitted=0;
    }


static void adjust_contrast(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey,int *white)

    {
    int i,j,tries,wc,tc,hist[256];
    double contrast,rat0;
    WILLUSBITMAP *dst,_dst;

    if (debug && verbose)
        printf("\nAt adjust_contrast.\n");
    if ((*white) <= 0)
        (*white)=192;
    /* If contrast_max negative, use it as fixed contrast adjustment. */
    if (contrast_max < 0.)
        {
        bmp_contrast_adjust(srcgrey,srcgrey,-contrast_max);
        if (dst_color && fabs(contrast_max+1.0)>1e-4)
            bmp_contrast_adjust(src,src,-contrast_max);
        return;
        }
    dst=&_dst;
    bmp_init(dst);
    wc=0; /* Avoid compiler warning */
    tc=srcgrey->width*srcgrey->height;
    rat0=0.5; /* Avoid compiler warning */
    for (contrast=1.0,tries=0;contrast<contrast_max+.01;tries++)
        {
        if (fabs(contrast-1.0)>1e-4)
            bmp_contrast_adjust(dst,srcgrey,contrast);
        else
            bmp_copy(dst,srcgrey);
        /*Get bitmap histogram */
        for (i=0;i<256;i++)
            hist[i]=0;
        for (j=0;j<dst->height;j++)
            {
            unsigned char *p;
            p=bmp_rowptr_from_top(dst,j);
            for (i=0;i<dst->width;i++,p++)
                hist[p[0]]++;
            }
        if (tries==0)
            {
            int h1;
            for (h1=0,j=(*white);j<256;j++)
                h1+=hist[j];
            rat0=(double)h1/tc;
            if (debug && verbose)
                printf("    rat0 = rat[%d-255]=%.4f\n",(*white),rat0);
            }
        
        /* Find white ratio */
        /*
        for (wc=hist[254],j=253;j>=252;j--)
            if (hist[j]>wc1)
                wc1=hist[j];
        */
        for (wc=0,j=252;j<=255;j++)
            wc += hist[j];
        /*
        if ((double)wc/tc >= rat0*0.7 && (double)hist[255]/wc > 0.995)
            break;
        */
        if (debug && verbose)
            printf("    %2d. Contrast=%7.2f, rat[252-255]/rat0=%.4f\n",
                        tries+1,contrast,(double)wc/tc/rat0);
        if ((double)wc/tc >= rat0*0.94)
            break;
        contrast *= 1.05;
        }
    if (debug)
        printf("Contrast=%7.2f, rat[252-255]/rat0=%.4f\n",
                       contrast,(double)wc/tc/rat0);
/*
bmp_write(dst,"outc.png",stdout,100);
wfile_written_info("outc.png",stdout);
exit(10);
*/
    bmp_copy(srcgrey,dst);
    /* Maybe don't adjust the contrast for the color bitmap? */
    if (dst_color && fabs(contrast-1.0)>1e-4)
        bmp_contrast_adjust(src,src,contrast);
    bmp_free(dst);
    }


static int bmpregion_row_black_count(BMPREGION *region,int r0)

    {
    unsigned char *p;
    int i,nc,c;

    p=bmp_rowptr_from_top(region->bmp8,r0)+region->c1;
    nc=region->c2-region->c1+1;
    for (c=i=0;i<nc;i++,p++)
        if (p[0]<region->bgcolor)
            c++;
    return(c);
    }


/*
** Returns height of region found and divider position in (*divider_column).
** (*divider_column) is absolute position on source bitmap.
**
*/
static int bmpregion_find_multicolumn_divider(BMPREGION *region,int *row_black_count,
                                             BMPREGION *pageregion,int *npr,
                                             int *colcount,int *rowcount)

    {
    int itop,i,dm,middle,divider_column,min_height_pixels,mhp2,min_col_gap_pixels;
    BMPREGION _newregion,*newregion,column[2];
    BREAKINFO *breakinfo,_breakinfo;
    int *rowmin,*rowmax;
    static char *funcname="bmpregion_find_multicolumn_divider";

    if (debug)
        printf("@bmpregion_find_multicolumn_divider(%d,%d)-(%d,%d)\n",
                 region->c1,region->r1,region->c2,region->r2);
    breakinfo=&_breakinfo;
    breakinfo->textrow=NULL;
    breakinfo_alloc(101,breakinfo,region->r2-region->r1+1);
    bmpregion_find_vertical_breaks(region,breakinfo,colcount,rowcount,column_row_gap_height_in);
    newregion=&_newregion;
    (*newregion)=(*region);
    min_height_pixels=min_column_height_inches*src_dpi; /* src->height/15; */ 
    mhp2 = min_height_pixels-1;
    if (mhp2 < 0)
        mhp2=0;
    dm=1+(region->c2-region->c1+1)*column_gap_range/2.;
    middle=(region->c2-region->c1+1)/2;
    min_col_gap_pixels=(int)(min_column_gap_inches*src_dpi+.5);
    if (verbose)
        {
        printf("(dm=%d, width=%d, min_gap=%d)\n",dm,region->c2-region->c1+1,min_col_gap_pixels);
        printf("Checking regions (r1=%d, r2=%d, minrh=%d)..",region->r1,region->r2,min_height_pixels);
        fflush(stdout);
        }
    breakinfo_sort_by_row_position(breakinfo);
    willus_dmem_alloc_warn(5,(void **)&rowmin,(region->c2+10)*2*sizeof(int),funcname,10);
    rowmax=&rowmin[region->c2+10];
    for (i=0;i<region->c2+2;i++)
        {
        rowmin[i]=region->r2+2;
        rowmax[i]=-1;
        }

    /* Start with top-most and bottom-most regions, look for column dividers */
    for (itop=0;itop<breakinfo->n 
                      && breakinfo->textrow[itop].r1<region->r2+1-min_height_pixels;itop++)
        {
        int ibottom;

        for (ibottom=breakinfo->n-1;ibottom>=itop 
              && breakinfo->textrow[ibottom].r2-breakinfo->textrow[itop].r1 >= min_height_pixels;
              ibottom--)
            {
            /*
            ** Look for vertical shaft of clear space that clearly demarcates
            ** two columns
            */
            for (i=0;i<dm;i++)
                {
                int foundgap,ii,c1,c2,iiopt,status;

                newregion->c1=region->c1+middle-i;
                /* If we've effectively already checked this shaft, move on */
                if (itop >= rowmin[newregion->c1] && ibottom <= rowmax[newregion->c1])
                    continue;
                newregion->c2=newregion->c1+min_col_gap_pixels-1;
                newregion->r1=breakinfo->textrow[itop].r1;
                newregion->r2=breakinfo->textrow[ibottom].r2;
                foundgap=bmpregion_is_clear(newregion,row_black_count,gtc_in);
                if (!foundgap && i>0)
                    {
                    newregion->c1=region->c1+middle+i;
                    newregion->c2=newregion->c1+min_col_gap_pixels-1;
                    foundgap=bmpregion_is_clear(newregion,row_black_count,gtc_in);
                    }
                if (!foundgap)
                    continue;
                /* Found a gap, but look for a better gap nearby */
                c1=newregion->c1;
                c2=newregion->c2;
                for (iiopt=0,ii=-min_col_gap_pixels;ii<=min_col_gap_pixels;ii++)
                    {
                    int newgap;
                    newregion->c1=c1+ii;
                    newregion->c2=c2+ii;
                    newgap=bmpregion_is_clear(newregion,row_black_count,gtc_in);
                    if (newgap>0 && newgap<foundgap)
                        {
                        iiopt=ii;
                        foundgap=newgap;
                        if (newgap==1)
                            break;
                        }
                    }
                newregion->c1=c1+iiopt;
                /* If we've effectively already checked this shaft, move on */
                if (itop >= rowmin[newregion->c1] && ibottom <= rowmax[newregion->c1])
                    continue;
                newregion->c2=c2+iiopt;
                divider_column=newregion->c1+min_col_gap_pixels/2;
                status=bmpregion_column_height_and_gap_test(column,region,
                                       breakinfo->textrow[itop].r1,
                                       breakinfo->textrow[ibottom].r2,
                                       divider_column,
                                       colcount,rowcount);
                /* If fails column height or gap test, mark as bad */
                if (status)
                    {
                    if (itop < rowmin[newregion->c1])
                        rowmin[newregion->c1]=itop;
                    if (ibottom > rowmax[newregion->c1])
                        rowmax[newregion->c1]=ibottom;
                    }
                /* If right column too short, stop looking */
                if (status&2)
                    break;
                if (!status)
                    {
                    int colheight;

/* printf("    GOT COLUMN DIVIDER AT x=%d.\n",(*divider_column)); */
                    if (verbose)
                        {
                        printf("\n    GOOD REGION: col gap=(%d,%d) - (%d,%d)\n"
                             "                 r1=%d, r2=%d\n",
                            newregion->c1,newregion->r1,newregion->c2,newregion->r2,
                            breakinfo->textrow[itop].r1,breakinfo->textrow[ibottom].r2);
                        }
                    if (itop>0)
                        {
                        /* add 1-column region */
                        pageregion[(*npr)]=(*region);
                        pageregion[(*npr)].r2=breakinfo->textrow[itop-1].r2;
                        if (pageregion[(*npr)].r2 > pageregion[(*npr)].bmp8->height-1)
                            pageregion[(*npr)].r2 = pageregion[(*npr)].bmp8->height-1;
                        bmpregion_trim_margins(&pageregion[(*npr)],colcount,rowcount,0xf);
                        /* Special flag to indicate full-width region */
                        pageregion[(*npr)].c1 = -1-pageregion[(*npr)].c1;
                        (*npr)=(*npr)+1;
                        }
                    pageregion[(*npr)]=column[0];
                    (*npr)=(*npr)+1;
                    pageregion[(*npr)]=column[1];
                    (*npr)=(*npr)+1;
                    colheight = breakinfo->textrow[ibottom].r2-region->r1+1;
                    breakinfo_free(101,breakinfo);
/*
printf("Returning %d divider column = %d - %d\n",region->r2-region->r1+1,newregion->c1,newregion->c2);
*/
                    return(colheight);
                    }
                }
            }
        }
    if (verbose)
        printf("NO GOOD REGION FOUND.\n");
    pageregion[(*npr)]=(*region);
    bmpregion_trim_margins(&pageregion[(*npr)],colcount,rowcount,0xf);
    /* Special flag to indicate full-width region */
    pageregion[(*npr)].c1 = -1-pageregion[(*npr)].c1;
    (*npr)=(*npr)+1;
    /* (*divider_column)=region->c2+1; */
    willus_dmem_free(5,(double **)&rowmin,funcname);
    breakinfo_free(101,breakinfo);
/*
printf("Returning %d\n",region->r2-region->r1+1);
*/
    return(region->r2-region->r1+1);
    }


/*
** 1 = column 1 too short
** 2 = column 2 too short
** 3 = both too short
** 0 = both okay
** Both columns must pass height requirement.
**
** Also, if gap between columns > gtcmax_in, fails test. (8-31-12)
**
*/
static int bmpregion_column_height_and_gap_test(BMPREGION *column,BMPREGION *region,
                                        int r1,int r2,
                                        int cmid,int *colcount,int *rowcount)

    {
    int min_height_pixels,status;

    status=0;
    min_height_pixels=min_column_height_inches*src_dpi;
    column[0]=(*region);
    column[0].r1=r1;
    column[0].r2=r2;
    column[0].c2=cmid-1;
    bmpregion_trim_margins(&column[0],colcount,rowcount,0xf);
/*
printf("    COL1:  pix=%d (%d - %d)\n",newregion->r2-newregion->r1+1,newregion->r1,newregion->r2);
*/
    if (column[0].r2-column[0].r1+1 < min_height_pixels)
        status |= 1;
    column[1]=(*region);
    column[1].r1=r1;
    column[1].r2=r2;
    column[1].c1=cmid;
    column[1].c2=region->c2;
    bmpregion_trim_margins(&column[1],colcount,rowcount,0xf);
/*
printf("    COL2:  pix=%d (%d - %d)\n",newregion->r2-newregion->r1+1,newregion->r1,newregion->r2);
*/
    if (column[1].r2-column[1].r1+1 < min_height_pixels)
        status |= 2;
    /* Make sure gap between columns is not too large */
    if (gtcmax_in>=0. && column[1].c1-column[0].c2-1 > gtcmax_in*src_dpi)
        status |= 4;
    return(status);
    }


/*
** Return 0 if there are dark pixels in the region.  NZ otherwise.
*/
static int bmpregion_is_clear(BMPREGION *region,int *row_black_count,double gt_in)

    {
    int r,c,nc,pt;

    /*
    ** row_black_count[] doesn't necessarily match up to this particular region's columns.
    ** So if row_black_count[] == 0, the row is clear, otherwise it has to be counted.
    ** because the columns are a subset.
    */
    /* nr=region->r2-region->r1+1; */
    nc=region->c2-region->c1+1;
    pt=(int)(gt_in*src_dpi*nc+.5);
    if (pt<0)
        pt=0;
    for (c=0,r=region->r1;r<=region->r2;r++)
        {
        if (r<0 || r>=region->bmp8->height)
            continue;
        if (row_black_count[r]==0)
            continue;
        c+=bmpregion_row_black_count(region,r);
        if (c>pt)
            return(0);
        }
/*
printf("(%d,%d)-(%d,%d):  c=%d, pt=%d (gt_in=%g)\n",
region->c1,region->r1,region->c2,region->r2,c,pt,gt_in);
*/
    return(1+(int)10*c/pt);
    }


static void bmpregion_row_histogram(BMPREGION *region)

    {
    static char *funcname="bmpregion_row_histogram";
    WILLUSBITMAP *src;
    FILE *out;
    static int *rowcount;
    static int *hist;
    int i,j,nn;

    willus_dmem_alloc_warn(6,(void **)&rowcount,(region->r2-region->r1+1)*sizeof(int),funcname,10);
    willus_dmem_alloc_warn(7,(void **)&hist,(region->c2-region->c1+1)*sizeof(int),funcname,10);
    src=region->bmp8;
    for (j=region->r1;j<=region->r2;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(src,j)+region->c1;
        rowcount[j-region->r1]=0;
        for (i=region->c1;i<=region->c2;i++,p++)
            if (p[0]<region->bgcolor)
                rowcount[j-region->r1]++;
        }
    for (i=region->c1;i<=region->c2;i++)
        hist[i-region->c1]=0;
    for (i=region->r1;i<=region->r2;i++)
        hist[rowcount[i-region->r1]]++;
    for (i=region->c2-region->c1+1;i>=0;i--)
        if (hist[i]>0)
            break;
    nn=i;
    out=fopen("hist.ep","w");
        for (i=0;i<=nn;i++)
            fprintf(out,"%5d %5d\n",i,hist[i]);
    fclose(out);
    out=fopen("rowcount.ep","w");
        for (i=0;i<region->r2-region->r1+1;i++)
            fprintf(out,"%5d %5d\n",i,rowcount[i]);
    fclose(out);
    willus_dmem_free(7,(double **)&hist,funcname);
    willus_dmem_free(6,(double **)&rowcount,funcname);
    }


/*
** Input:  A generic rectangular region from the source file.  It will not
**         be checked for multiple columns, but the text may be wrapped
**         (controlled by allow_text_wrapping input).
**
** force_scale == -2 :  Use same scale for entire column--fit to device
**
** This function looks for vertical gaps in the region and breaks it at
** the widest ones (if there are significantly wider ones).
**
*/
static void bmpregion_vertically_break(BMPREGION *region,MASTERINFO *masterinfo,
                          int allow_text_wrapping,double force_scale,
                          int *colcount,int *rowcount,PAGEINFO *pageinfo,
                          int colgap_pixels,int ncols)

    {
    static int ncols_last=-1;
    int regcount,i,i1,biggap,revert,trim_flags,allow_vertical_breaks;
    int justification_flags,caller_id,marking_flags,rbdelta;
    // int trim_left_and_right;
    BMPREGION *bregion,_bregion;
    BREAKINFO *breakinfo,_breakinfo;
    double region_width_inches,region_height_inches;

#if (WILLUSDEBUGX & 1)
printf("\n\n@bmpregion_vertically_break.  colgap_pixels=%d\n\n",colgap_pixels);
#endif
    trim_flags=0xf;
    allow_vertical_breaks=1;
    justification_flags=0x8f; /* Don't know region justification status yet.  Use user settings. */
    rbdelta=-1;
    breakinfo=&_breakinfo;
    breakinfo->textrow=NULL;
    breakinfo_alloc(102,breakinfo,region->r2-region->r1+1);
    bmpregion_find_vertical_breaks(region,breakinfo,colcount,rowcount,-1.0);
    /* Should there be a check for breakinfo->n==0 here? */
    /* Don't think it breaks anything to let it go.  -- 6-11-12 */
#if (WILLUSDEBUGX & 2)
breakinfo_echo(breakinfo);
#endif
    breakinfo_remove_small_rows(breakinfo,0.25,0.5,region,colcount,rowcount);
#if (WILLUSDEBUGX & 2)
breakinfo_echo(breakinfo);
#endif
    breakinfo->centered=bmpregion_is_centered(region,breakinfo,0,breakinfo->n-1,NULL);
#if (WILLUSDEBUGX & 2)
breakinfo_echo(breakinfo);
#endif
/*
newregion=&_newregion;
for (i=0;i<breakinfo->n;i++)
{
(*newregion)=(*region);
newregion->r1=breakinfo->textrow[i].r1;
newregion->r2=breakinfo->textrow[i].r2;
bmpregion_add(newregion,breakinfo,masterinfo,allow_text_wrapping,force_scale,0,1,
              colcount,rowcount,pageinfo,0,0xf);
}
breakinfo_free(breakinfo);
return;
*/
/*
    if (!vertical_breaks)
        {
        caller_id=100;
        marking_flags=0;
        bmpregion_add(region,breakinfo,masterinfo,allow_text_wrapping,trim_flags,
                      allow_vertical_breaks,force_scale,justification_flags,
                      caller_id,colcount,rowcount,pageinfo,marking_flags,rbdelta);
        breakinfo_free(breakinfo);
        return;
        }
*/
    /* Red, numbered region */
    mark_source_page(region,1,0xf);
    bregion=&_bregion;
    if (debug)
        {
        if (!allow_text_wrapping)
            printf("@bmpregion_vertically_break (no break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,force_scale);
        else
            printf("@bmpregion_vertically_break (allow break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,force_scale);
        }
    /*
    ** Tag blank rows and columns
    */
    if (vertical_break_threshold<0. || breakinfo->n < 6)
        biggap = -1.;
    else
        {
        int gap_median;
/*
        int rowheight_median;

        breakinfo_sort_by_rowheight(breakinfo);
        rowheight_median = breakinfo->textrow[breakinfo->n/2].rowheight;
*/
#ifdef WILLUSDEBUG
for (i=0;i<breakinfo->n;i++)
printf("    gap[%d]=%d\n",i,breakinfo->textrow[i].gap);
#endif
        breakinfo_sort_by_gap(breakinfo);
        gap_median = breakinfo->textrow[breakinfo->n/2].gap;
#ifdef WILLUSDEBUG
printf("    median=%d\n",gap_median);
#endif
        biggap = gap_median*vertical_break_threshold;
        breakinfo_sort_by_row_position(breakinfo);
        }
#ifdef WILLUSDEBUG
printf("    biggap=%d\n",biggap);
#endif
    region_width_inches = (double)(region->c2-region->c1+1)/src_dpi;
    region_height_inches = (double)(region->r2-region->r1+1)/src_dpi;
    /*
    trim_left_and_right = 1;
    if (region_width_inches <= max_region_width_inches)
        trim_left_and_right = 0;
    */
/*
printf("force_scale=%g, rwi = %g, rwi/mrwi = %g, rhi = %g\n",
force_scale,
region_width_inches,
region_width_inches / max_region_width_inches,
region_height_inches);
*/
    if (force_scale < -1.5 && region_width_inches > MIN_REGION_WIDTH_INCHES
                         && region_width_inches/max_region_width_inches < 1.25
                         && region_height_inches > 0.5)
        {
        revert=1;
        force_scale = -1.0;
        fit_column_to_screen(region_width_inches);
        // trim_left_and_right = 0;
        allow_text_wrapping = 0;
        }
    else
        revert=0;
    /* Add the regions (broken vertically) */
    caller_id=1;
    /*
    if (trim_left_and_right)
        trim_flags=0xf;
    else
        trim_flags=0xc;
    */
    trim_flags=0xf;
    for (regcount=i1=i=0;i1<breakinfo->n;i++)
        {
        int i2;
 
        i2 = i<breakinfo->n ? i : breakinfo->n-1;
        if (i>=breakinfo->n || (biggap>0. && breakinfo->textrow[i2].gap>=biggap))
            {
            int j,c1,c2,nc,nowrap;
            double regwidth,ar1,rh1;

// printf("CALLER 1:  i1=%d, i2=%d (breakinfo->n=%d)\n",i1,i2,breakinfo->n);
            (*bregion)=(*region);
            bregion->r1=breakinfo->textrow[i1].r1;
            bregion->r2=breakinfo->textrow[i2].r2;
            c1=breakinfo->textrow[i1].c1;
            c2=breakinfo->textrow[i1].c2;
            nc=c2-c1+1;
            if (nc<=0)
                nc=1;
            rh1=(double)(breakinfo->textrow[i1].r2-breakinfo->textrow[i1].r1+1)/src_dpi;
            ar1=(double)(breakinfo->textrow[i1].r2-breakinfo->textrow[i1].r1+1)/nc;
            for (j=i1+1;j<=i2;j++)
                {
                if (c1>breakinfo->textrow[j].c1)
                    c1=breakinfo->textrow[j].c1;
                if (c2<breakinfo->textrow[j].c2)
                    c2=breakinfo->textrow[j].c2;
                }
            regwidth=(double)(c2-c1+1)/src_dpi;
            marking_flags=(i1==0?0:1)|(i2==breakinfo->n-1?0:2);
            /* Green */
            mark_source_page(bregion,3,marking_flags);
            nowrap = ((regwidth <= max_region_width_inches  && allow_text_wrapping<2)
                    || (ar1 > no_wrap_ar_limit && rh1 > no_wrap_height_limit_inches));
            /*
            ** If between regions, or if the next region isn't going to be
            ** wrapped, or if the next region starts a different number of
            ** columns than before, then "flush and gap."
            */
            if (regcount>0 || just_flushed_internal || nowrap 
                           || (ncols_last>0 && ncols_last != ncols))
                {
                int gap;
#ifdef WILLUSDEBUG
printf("wrapflush1\n");
#endif
                if (!just_flushed_internal)
                    wrapbmp_flush(masterinfo,0,pageinfo,0);
                gap = regcount==0 ? colgap_pixels : breakinfo->textrow[i1-1].gap;
                if (regcount==0 && beginning_gap_internal>0)
                    {
                    if (last_h5050_internal > 0)
                        {
                        if (fabs(1.-(double)breakinfo->textrow[i1].h5050/last_h5050_internal)>.1)
                            dst_add_gap_src_pixels("Col/Page break",masterinfo,colgap_pixels);
                        last_h5050_internal=-1;
                        }
                    gap=beginning_gap_internal;
                    beginning_gap_internal = -1;
                    }
                dst_add_gap_src_pixels("Vert break",masterinfo,gap);
                }
            else
                {
                if (regcount==0 && beginning_gap_internal < 0)
                    beginning_gap_internal = colgap_pixels;
                }
            bmpregion_add(bregion,breakinfo,masterinfo,allow_text_wrapping,trim_flags,
                          allow_vertical_breaks,force_scale,justification_flags,caller_id,
                          colcount,rowcount,pageinfo,marking_flags,rbdelta);
            regcount++;
            i1=i2+1;
            }
        }
    ncols_last=ncols;
    if (revert)
        restore_output_dpi();
    breakinfo_free(102,breakinfo);
    }


/*
**
** MAIN BITMAP REGION ADDING FUNCTION
**
** NOTE:  This function calls itself recursively!
**
** Input:  A generic rectangular region from the source file.  It will not
**         be checked for multiple columns, but the text may be wrapped
**         (controlled by allow_text_wrapping input).
**
** First, excess margins are trimmed off of the region.
**
** Then, if the resulting trimmed region is wider than the max desirable width
** and allow_text_wrapping is non-zero, then the double_wide_add() function is called.
** Otherwise the region is scaled to fit and added to the master set of pages.
**
** justification_flags
**     Bits 6-7:  0 = document is not fully justified
**                1 = document is fully justified
**                2 = don't know document justification yet
**     Bits 4-5:  0 = Use user settings
**                1 = fully justify
**                2 = do not fully justify
**     Bits 2-3:  0 = document is left justified
**                1 = document is centered
**                2 = document is right justified
**                3 = don't know document justification yet
**     Bits 0-1:  0 = left justify document
**                1 = center document
**                2 = right justify document
**                3 = Use user settings
**
** force_scale = -2.0 : Fit column width to display width
** force_scale = -1.0 : Use output dpi unless the region doesn't fit.
**                      In that case, scale it down until it fits.
** force_scale > 0.0  : Scale region by force_scale.
**
** mark_flags & 1 :  Mark top
** mark_flags & 2 :  Mark bottom
** mark_flags & 4 :  Mark left
** mark_flags & 8 :  Mark right
**
** trim_flags & 0x80 :  Do NOT re-trim no matter what.
**
*/
static void bmpregion_add(BMPREGION *region,BREAKINFO *breakinfo,MASTERINFO *masterinfo,
                          int allow_text_wrapping,int trim_flags,
                          int allow_vertical_breaks,double force_scale,
                          int justification_flags,int caller_id,
                          int *colcount,int *rowcount,PAGEINFO *pageinfo,
                          int mark_flags,int rowbase_delta)

    {
    int w,i,nc,nr,h,bpp,nocr;
    double sr,region_width_inches;
    WILLUSBITMAP *bmp,_bmp;
    BMPREGION *newregion,_newregion;

    newregion=&_newregion;
    (*newregion)=(*region);
#if (WILLUSDEBUGX & 1)
printf("@bmpregion_add (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
printf("    trimflags = %X\n",trim_flags);
#endif
    if (debug)
        {
        if (!allow_text_wrapping)
            printf("@bmpregion_add (no break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,force_scale);
        else
            printf("@bmpregion_add (allow break) (%d,%d) - (%d,%d) (scale=%g)\n",
                region->c1,region->r1,region->c2,region->r2,force_scale);
        }
    /*
    ** Tag blank rows and columns and trim the blank margins off
    ** trimflags = 0xf for all margin trim.
    ** trimflags = 0xc for just top and bottom margins.
    */
    bmpregion_trim_margins(newregion,colcount,rowcount,trim_flags);
#if (WILLUSDEBUGX & 1)
printf("    After trim:  (%d,%d) - (%d,%d)\n",newregion->c1,newregion->r1,newregion->c2,newregion->r2);
#endif
    nc=newregion->c2-newregion->c1+1;
    nr=newregion->r2-newregion->r1+1;
// printf("nc=%d, nr=%d\n",nc,nr);
    if (verbose)
        {
        printf("    row range adjusted to %d - %d\n",newregion->r1,newregion->r2);
        printf("    col range adjusted to %d - %d\n",newregion->c1,newregion->c2);
        }
    if (nc<=5 || nr<=1)
        return;
    region_width_inches = (double)nc/src_dpi;
// printf("regwidth = %g in\n",region_width_inches);
    /* Use untrimmed region left/right if possible */
    if (caller_id==1 && region_width_inches <= max_region_width_inches)
        {
        int trimleft,trimright;
        int maxpix,dpix;

        maxpix = (int)(max_region_width_inches*src_dpi+.5);
#if (WILLUSDEBUGX & 1)
printf("    Trimming.  C's = %4d %4d %4d %4d\n",region->c1,newregion->c1,newregion->c2,region->c2);
printf("    maxpix = %d, regwidth = %d\n",maxpix,region->c2-region->c1+1);
#endif
        if (maxpix > (region->c2-region->c1+1))
            maxpix = region->c2-region->c1+1;
// printf("    maxpix = %d\n",maxpix);
        dpix = (region->c2-region->c1+1 - maxpix)/2;
// printf("    dpix = %d\n",dpix);
        trimright = region->c2-newregion->c2;
        trimleft = newregion->c1-region->c1;
        if (trimleft<trimright)
            {
            if (trimleft > dpix)
                newregion->c1 = region->c1+dpix;
            newregion->c2 = newregion->c1+maxpix-1;
            }
        else
            {
            if (trimright > dpix)
                newregion->c2 = region->c2-dpix;
            newregion->c1 = newregion->c2-maxpix+1;
            }
        if (newregion->c1 < region->c1)
            newregion->c1 = region->c1;
        if (newregion->c2 > region->c2)
            newregion->c2 = region->c2;
        nc=newregion->c2-newregion->c1+1;
#if (WILLUSDEBUGX & 1)
printf("    Post Trim.  C's = %4d %4d %4d %4d\n",region->c1,newregion->c1,newregion->c2,region->c2);
#endif
        region_width_inches = (double)nc/src_dpi;
        }
        
    /*
    region_height_inches = (double)nr/src_dpi;
    if (force_scale < -1.5 && region_width_inches > MIN_REGION_WIDTH_INCHES
                         && region_width_inches/max_region_width_inches < 1.25
                         && region_height_inches > 0.5)
        {
        revert=1;
        force_scale = -1.0;
        fit_column_to_screen(region_width_inches);
        }
    else
        revert=0;
    */
        
    /*
    ** Try breaking the region into smaller horizontal pieces (wrap text lines)
    */
/*
printf("allow_text_wrapping=%d, region_width_inches=%g, max_region_width_inches=%g\n",
allow_text_wrapping,region_width_inches,max_region_width_inches);
*/
    /* New in v1.50, if allow_text_wrapping==2, unwrap short lines. */
    if (allow_text_wrapping==2 
         || (allow_text_wrapping==1 && region_width_inches > max_region_width_inches))
        {
        bmpregion_analyze_justification_and_line_spacing(newregion,breakinfo,masterinfo,
                                           colcount,rowcount,pageinfo,1,force_scale);
        return;
        }

    /*
    ** If allowed, re-submit each vertical region individually
    */
    if (allow_vertical_breaks)
        {
        bmpregion_analyze_justification_and_line_spacing(newregion,breakinfo,masterinfo,
                                            colcount,rowcount,pageinfo,0,force_scale);
#ifdef COMMENT
        int i0,c;
        for (i=0;i<breakinfo->n;i++)
            if (breakinfo->textrow[i].r1 <= region->r2
                  && breakinfo->textrow[i].r2 >= region->r1)
                break;
        if (i>=breakinfo->n)
             return;
        for (c=0,i0=i;i<breakinfo->n;i++)
            {
            BMPREGION xregion;
            int allow_full_justify;

            if (breakinfo->textrow[i].r1 > region->r2)
                break;
            if (c==0)
                wrapbmp_flush(masterinfo,0,pageinfo);
            c++;
            xregion=(*newregion);
            xregion.r1=breakinfo->textrow[i].r1;
            xregion.r2=breakinfo->textrow[i].r2;
            if (i>i0)
                dst_add_gap_src_pixels("Vert break L2",masterinfo,breakinfo->textrow[i-1].gap);
            allow_full_justify = ((double)(xregion.r2-xregion.r1+1)/src_dpi 
                                               <= no_wrap_height_limit_inches);
            /* No further breaking or trimming allowed */
            /* caller ID? */
            bmpregion_add(&xregion,breakinfo,masterinfo,0,0,0,force_scale,
                          0,caller_id,colcount,rowcount,pageinfo,mark_flags,-1);
            }
#endif
        return;
        }

    /* Green Marks */
    /*
    if (mark_flags)
        mark_source_page(newregion,2,0xf);
    */

    /* AT THIS POINT, BITMAP IS NOT TO BE BROKEN UP HORIZONTALLY OR VERTICALLY */
    /* (IT CAN STILL BE FULLY JUSTIFIED IF ALLOWED.) */

    /*
    ** Scale region to fit the destination device width and add to the master bitmap.
    **
    **
    ** Start by copying source region to new bitmap 
    **
    */

// printf("c1=%d\n",newregion->c1);
    /* If centered region, re-trim left and right */
    if ((trim_flags&0x80)==0 && ((trim_flags&3)!=3 && ((justification_flags&3)==1
          || ((justification_flags&3)==3
              && (dst_justify==1
                    || (dst_justify<0 && (justification_flags&0xc)==4))))))
        {
        bmpregion_trim_margins(newregion,colcount,rowcount,0x3);
        nc=newregion->c2-newregion->c1+1;
        }
#if (WILLUSDEBUGX & 1)
    aprintf("atomic region:  " ANSI_CYAN "%.2f x %.2f in" ANSI_NORMAL " c1=%d, (%d x %d) (rbdel=%d) just=0x%02X\n",
                   (double)(newregion->c2-newregion->c1+1)/src_dpi,
                   (double)(newregion->r2-newregion->r1+1)/src_dpi,
                   newregion->c1,
                   (newregion->c2-newregion->c1+1),
                   (newregion->r2-newregion->r1+1),
                   rowbase_delta,justification_flags);
#endif
    bmp=&_bmp;
    bmp_init(bmp);
    bmp->width=nc;
    bmp->height=nr;
    if (dst_color)
        bmp->bpp=24;
    else
        {
        bmp->bpp=8;
        for (i=0;i<256;i++)
            bmp->red[i]=bmp->blue[i]=bmp->green[i]=i;
        }
    bmp_alloc(bmp);
    bpp = dst_color ? 3 : 1;
// printf("r1=%d, r2=%d\n",newregion->r1,newregion->r2);
    for (i=newregion->r1;i<=newregion->r2;i++)
        {
        unsigned char *psrc,*pdst;

        pdst=bmp_rowptr_from_top(bmp,i-newregion->r1);
        psrc=bmp_rowptr_from_top(dst_color ? newregion->bmp : newregion->bmp8,i)+bpp*newregion->c1;
        memcpy(pdst,psrc,nc*bpp);
        }
    /*
    ** Now scale to appropriate destination size.
    **
    ** force_scale is used to maintain uniform scaling so that
    ** most of the regions are scaled at the same value.
    **
    ** force_scale = -2.0 : Fit column width to display width
    ** force_scale = -1.0 : Use output dpi unless the region doesn't fit.
    **                      In that case, scale it down until it fits.
    ** force_scale > 0.0  : Scale region by force_scale.
    **
    */
    if (force_scale > 0.)
        sr = force_scale;
    else
        {
        if (region_width_inches < max_region_width_inches)
            sr=(double)masterinfo->bmp.width/(display_width_inches*src_dpi);
        else
            sr=(double)(masterinfo->bmp.width-(dst_marleft+dst_marright)*dst_dpi)/bmp->width;
        }
    w=sr*bmp->width;
    h=(int)(sr*bmp->height+.5);

    /*
    ** If scaled dimensions are finite, add to master bitmap.
    */
    if (w>0 && h>0)
        {
        WILLUSBITMAP *tmp,_tmp;

#ifdef HAVE_OCR
        if (dst_ocr)
            {
            nocr=(int)((1./sr)+0.5);
            if (nocr < 1)
                nocr=1;
            if (nocr > 10)
                nocr=10;
            w *= nocr;
            h *= nocr;
            }
        else
#endif
            nocr=1;
        tmp=&_tmp;
        bmp_init(tmp);
        bmp_resample(tmp,bmp,(double)0.,(double)0.,(double)bmp->width,(double)bmp->height,w,h);
        bmp_free(bmp);
        last_scale_factor_internal=sr;
/*
{
static int nn=0;
char filename[256];
sprintf(filename,"xxx%02d.png",nn++);
bmp_write(tmp,filename,stdout,100);
}
*/
        /*
        ** Add scaled bitmap to destination.
        */
        /* Allocate more rows if necessary */
        while (masterinfo->rows+tmp->height/nocr > masterinfo->bmp.height)
            bmp_more_rows(&masterinfo->bmp,1.4,255);
        bmp_src_to_dst(masterinfo,tmp,justification_flags,region->bgcolor,nocr);
        bmp_free(tmp);
        }

    /* Store delta to base of text row (used by wrapbmp_flush()) */
    last_rowbase_internal = rowbase_delta;
    /* .05 was .072 in v1.35 */
    /* dst_add_gap(&masterinfo->bmp,&masterinfo->rows,0.05); */
    /*
    if (revert)
        restore_output_dpi();
    */
    }


static void dst_add_gap_src_pixels(char *caller,MASTERINFO *masterinfo,int pixels)

    {
    double gap_inches;

#ifdef WILLUSDEBUG
aprintf("%s " ANSI_GREEN "dst_add" ANSI_NORMAL " %.3f in (%d pix)\n",caller,(double)pixels/src_dpi,pixels);
#endif
    if (last_scale_factor_internal < 0.)
        gap_inches=(double)pixels/src_dpi;
    else
        gap_inches=(double)pixels*last_scale_factor_internal/dst_dpi;
    gap_inches *= vertical_multiplier;
    if (gap_inches > max_vertical_gap_inches)
        gap_inches=max_vertical_gap_inches;
    dst_add_gap(masterinfo,gap_inches);
    }


static void dst_add_gap(MASTERINFO *masterinfo,double inches)

    {
    int n,bw;
    unsigned char *p;

    n=(int)(inches*dst_dpi+.5);
    if (n<1)
        n=1;
    while (masterinfo->rows+n > masterinfo->bmp.height)
        bmp_more_rows(&masterinfo->bmp,1.4,255);
    bw=bmp_bytewidth(&masterinfo->bmp)*n;
    p=bmp_rowptr_from_top(&masterinfo->bmp,masterinfo->rows);
    memset(p,255,bw);
    masterinfo->rows += n;
    }


/*
**
** Add already-scaled source bmp to destination bmp.
** Source bmp may be narrower than destination--if so, it may be fully justifed.
** dst = destination bitmap
** src = source bitmap
** dst and src bpp must match!
** All rows of src are applied to masterinfo->bmp starting at row masterinfo->rows
** Full justification is done if requested.
**
*/
static void bmp_src_to_dst(MASTERINFO *masterinfo,WILLUSBITMAP *src,int justification_flags,
                           int whitethresh,int nocr)

    {
    WILLUSBITMAP *src1,_src1;
    WILLUSBITMAP *tmp;
#ifdef HAVE_OCR
    WILLUSBITMAP _tmp;
    OCRWORDS _words,*words;
#endif
    int dw,dw2;
    int i,srcbytespp,srcbytewidth,go_full;
    int destwidth,destx0,just;

    if (src->width<=0 || src->height<=0)
        return;
/*
if (fulljust && dst_fulljustify)
printf("@bmp_src_to_dst.  dst->bpp=%d, src->bpp=%d, src=%d x %d\n",masterinfo->bmp.bpp,src->bpp,src->width,src->height);
*/
/*
{
static int count=0;
static char filename[256];

printf("    @bmp_src_to_dst...\n");
sprintf(filename,"src%05d.png",count++);
bmp_write(src,filename,stdout,100);
}
*/
/*
if (fulljust && dst_fulljustify)
printf("srcbytespp=%d, srcbytewidth=%d, destwidth=%d, destx0=%d, destbytewidth=%d\n",
srcbytespp,srcbytewidth,destwidth,destx0,dstbytewidth);
*/

     /* Determine what justification to use */
    /* Left? */
    if ((justification_flags&3)==0  /* Mandatory left just */
          || ((justification_flags&3)==3  /* Use user settings */
              && (dst_justify==0  
                    || (dst_justify<0 && (justification_flags&0xc)==0))))
        just=0;
    else if ((justification_flags&3)==2
          || ((justification_flags&3)==3
              && (dst_justify==2
                    || (dst_justify<0 && (justification_flags&0xc)==8))))
        just=2;
    else
        just=1;

    /* Full justification? */
    destwidth=(int)(masterinfo->bmp.width-(dst_marleft+dst_marright)*dst_dpi+.5);
    go_full = (destwidth*nocr > src->width 
               && (((justification_flags&0x30)==0x10)
                   || ((justification_flags&0x30)==0 // Use user settings
                       && (dst_fulljustify==1
                            || (dst_fulljustify<0 && (justification_flags&0xc0)==0x40)))));

    /* Put fully justified text into src1 bitmap */
    if (go_full)
        {
        src1=&_src1;
        bmp_init(src1);
        bmp_fully_justify(src1,src,nocr*destwidth,whitethresh,just);
        }
    else
        src1=src;

#if (WILLUSDEBUGX & 1)
printf("@bmp_src_to_dst:  jflags=0x%02X just=%d, go_full=%d\n",justification_flags,just,go_full);
printf("    destx0=%d, destwidth=%d, src->width=%d\n",destx0,destwidth,src->width);
#endif
#ifdef HAVE_OCR
    if (dst_ocr)
        {
        /* Run OCR on the bitmap */
        words=&_words;
        ocrwords_init(words);
        ocrwords_fill_in(words,src1,whitethresh);
        /* Scale bitmap and word positions to destination size */
        if (nocr>1)
            {
            tmp=&_tmp;
            bmp_init(tmp);
            bmp_integer_resample(tmp,src1,nocr);
            ocrwords_int_scale(words,nocr);
            }
        else
            tmp=src1;
        }
    else
#endif
        tmp=src1;
/*
printf("writing...\n");
ocrwords_box(words,tmp);
bmp_write(tmp,"out.png",stdout,100);
exit(10);
*/
    destx0=(int)(dst_marleft*dst_dpi+.5);
    if (just==0)
        dw=destx0;
    else if (just==1)
        dw=destx0+(destwidth-tmp->width)/2;
    else
        dw=destx0+destwidth-tmp->width;
    if (dw<0)
        dw=0;
    /* Add OCR words to destination list */
#ifdef HAVE_OCR
    if (dst_ocr)
        {
        ocrwords_offset(words,dw,masterinfo->rows);
        ocrwords_concatenate(dst_ocrwords,words);
        ocrwords_free(words);
        }
#endif

    /* Add tmp bitmap to dst */
    srcbytespp = tmp->bpp==24 ? 3 : 1;
    srcbytewidth = tmp->width*srcbytespp;
    dw2=masterinfo->bmp.width-tmp->width-dw;
    dw *= srcbytespp;
    dw2 *= srcbytespp;
    for (i=0;i<tmp->height;i++,masterinfo->rows++)
        {
        unsigned char *pdst,*psrc;

        psrc=bmp_rowptr_from_top(tmp,i);
        pdst=bmp_rowptr_from_top(&masterinfo->bmp,masterinfo->rows);
        memset(pdst,255,dw);
        pdst += dw;
        memcpy(pdst,psrc,srcbytewidth);
        pdst += srcbytewidth;
        memset(pdst,255,dw2);
        }

#ifdef HAVE_OCR
    if (dst_ocr && nocr>1)
        bmp_free(tmp);
#endif
    if (go_full)
        bmp_free(src1);
    }


/*
** Spread words out in src and put into jbmp at scaling nocr
** In case the text can't be expanded enough,
**     just=0 (left justify), 1 (center), 2 (right justify)
*/
static void bmp_fully_justify(WILLUSBITMAP *jbmp,WILLUSBITMAP *src,int jbmpwidth,int whitethresh,
                              int just)

    {
    BMPREGION srcregion;
    BREAKINFO *colbreaks,_colbreaks;
    WILLUSBITMAP gray;
    int *gappos,*gapsize;
    int i,srcbytespp,srcbytewidth,jbmpbytewidth,newwidth,destx0,ng;
    static char *funcname="bmp_fully_justify";

/*
{
char filename[256];
count++;
sprintf(filename,"out%03d.png",count);
bmp_write(src,filename,stdout,100);
}
*/
    /* Init/allocate destination bitmap */
    jbmp->width = jbmpwidth;
    jbmp->height = src->height;
    jbmp->bpp = src->bpp;
    if (jbmp->bpp==8)
        for (i=0;i<256;i++)
            jbmp->red[i]=jbmp->green[i]=jbmp->blue[i]=i;
    bmp_alloc(jbmp);

    /* Find breaks in the text row */
    colbreaks=&_colbreaks;
    colbreaks->textrow=NULL;
    srcregion.bgcolor=whitethresh;
    srcregion.c1=0;
    srcregion.c2=src->width-1;
    srcregion.r1=0;
    srcregion.r2=src->height-1;
    srcbytespp=src->bpp==24 ? 3 : 1;
    if (srcbytespp==3)
        {
        srcregion.bmp=src;
        srcregion.bmp8=&gray;
        bmp_init(srcregion.bmp8);
        bmp_convert_to_greyscale_ex(srcregion.bmp8,src);
        }
    else
        {
        srcregion.bmp=src;
        srcregion.bmp8=src;
        }
    breakinfo_alloc(103,colbreaks,src->width);
    {
    int *colcount,*rowcount;

    colcount=rowcount=NULL;
    willus_dmem_alloc_warn(8,(void **)&colcount,sizeof(int)*(src->width+src->height),funcname,10);
    rowcount=&colcount[src->width];
    bmpregion_one_row_find_breaks(&srcregion,colbreaks,colcount,rowcount,1);
    willus_dmem_free(8,(double **)&colcount,funcname);
    }
    if (srcbytespp==3)
        bmp_free(srcregion.bmp8);
    ng=colbreaks->n-1;
    gappos=NULL;
    if (ng>0)
        {
        int maxsize,ms2,mingap,j;

        willus_dmem_alloc_warn(9,(void **)&gappos,(2*sizeof(int))*ng,funcname,10);
        gapsize=&gappos[ng];
        for (i=0;i<ng;i++)
            {
            gappos[i]=colbreaks->textrow[i].c2+1;
            gapsize[i]=colbreaks->textrow[i].gap;
            }
        
        /* Take only the largest group of gaps */
        for (maxsize=i=0;i<ng;i++)
            if (maxsize<gapsize[i])
                maxsize=gapsize[i];
        mingap = srcregion.lcheight*word_spacing;
        if (mingap < 2)
            mingap = 2; 
        if (maxsize > mingap)
            maxsize = mingap;
        ms2 = maxsize/2;
        for (i=j=0;i<ng;i++)
            if (gapsize[i] > ms2)
                {
                if (j!=i)
                    {
                    gapsize[j]=gapsize[i];
                    gappos[j]=gappos[i];
                    }
                j++;
                }
        ng=j;

        /* Figure out total pixel expansion */
        newwidth = src->width*1.25;
        if (newwidth > jbmp->width)
            newwidth=jbmp->width;
        }
    else
        newwidth=src->width;
    breakinfo_free(103,colbreaks);

    /* Starting column in destination bitmap */
    if (just==1)
        destx0 = (jbmp->width-newwidth)/2;
    else if (just==2)
        destx0 = (jbmp->width-newwidth);
    else
        destx0 = 0;

    jbmpbytewidth = bmp_bytewidth(jbmp);
    srcbytewidth = bmp_bytewidth(src);

    /* Clear entire fully justified bitmap */
    memset(bmp_rowptr_from_top(jbmp,0),255,jbmpbytewidth*jbmp->height);

    /* Spread out source pieces to fully justify them */
    for (i=0;i<=ng;i++)
        {
        int j,dx0,dx,sx0;
        unsigned char *pdst,*psrc;

        dx = i<ng ? (i>0 ? gappos[i]-gappos[i-1] : gappos[i]+1)
                  : (i>0 ? src->width-(gappos[i-1]+1) : src->width);
        dx *= srcbytespp;
        sx0= i==0 ? 0 : (gappos[i-1]+1);
        dx0= destx0 + sx0 + (i==0 ? 0 : (newwidth-src->width)*i/ng);
        psrc=bmp_rowptr_from_top(src,0)+sx0*srcbytespp;
        pdst=bmp_rowptr_from_top(jbmp,0)+dx0*srcbytespp;
        for (j=0;j<src->height;j++,pdst+=jbmpbytewidth,psrc+=srcbytewidth)
            memcpy(pdst,psrc,dx);
        }
    if (gappos!=NULL)
        willus_dmem_free(9,(double **)&gappos,funcname);
    }


#ifdef HAVE_OCR
/*
** Find words in src bitmap and put into words structure.
*/
static void ocrwords_fill_in(OCRWORDS *words,WILLUSBITMAP *src,int whitethresh)

    {
    BMPREGION region;
    BREAKINFO *rowbreaks,_rowbreaks;
    BREAKINFO *colbreaks,_colbreaks;
    WILLUSBITMAP *gray,_gray;
    int *colcount,*rowcount;
    int i;
    static char *funcname="ocrwords_fill_in";

#if (WILLUSDEBUGX & 32)
printf("@ocrwords_fill_in...\n");
#endif
/*
{
char filename[256];
count++;
sprintf(filename,"out%03d.png",count);
bmp_write(src,filename,stdout,100);
}
*/
    if (src->width<=0 || src->height<=0)
        return;

    /* Create grayscale version of bitmap */
    if (src->bpp==8)
        gray=src;
    else
        {
        gray=&_gray;
        bmp_init(gray);
        bmp_convert_to_grayscale_ex(gray,src);
        }

    /* Find breaks in the text row */
    region.bgcolor=whitethresh;
    region.c1=0;
    region.c2=gray->width-1;
    region.r1=0;
    region.r2=gray->height-1;
    region.bmp8=gray;
    region.bmp=src; /* May not be 24-bit! */
    colcount=rowcount=NULL;
    willus_dmem_alloc_warn(25,(void **)&colcount,sizeof(int)*(gray->width+gray->height),funcname,10);
    rowcount=&colcount[src->width];
    /* Allocate rowbreaks and colbreaks structures */
    rowbreaks=&_rowbreaks;
    rowbreaks->textrow=NULL;
    breakinfo_alloc(104,rowbreaks,gray->height);
    colbreaks=&_colbreaks;
    colbreaks->textrow=NULL;
    breakinfo_alloc(105,colbreaks,gray->width);

    /* Find rows of text */
    bmpregion_find_vertical_breaks(&region,rowbreaks,colcount,rowcount,column_row_gap_height_in);
    /* Go text row by text row */
    for (i=0;i<rowbreaks->n;i++)
        {
        BMPREGION _newregion,*newregion;
        int j,rowbase,r1,r2;
        double lcheight;

        newregion=&_newregion;
        (*newregion)=region;
        r1=newregion->r1=rowbreaks->textrow[i].r1;
        r2=newregion->r2=rowbreaks->textrow[i].r2;
        rowbase=rowbreaks->textrow[i].rowbase;
        lcheight=rowbreaks->textrow[i].lcheight;
        /* Sanity check on rowbase, lcheight */
        if ((double)(rowbase-r1)/(r2-r1) < .5)
            rowbase = r1+(r2-r1)*0.7;
        if (lcheight/(r2-r1) < .33)
            lcheight = 0.33;
        /* Break text row into words (also establishes lcheight) */
        bmpregion_one_row_find_breaks(newregion,colbreaks,colcount,rowcount,0);
        /* Add each word */
        for (j=0;j<colbreaks->n;j++)
            {
            char wordbuf[256];

            /* Don't OCR if "word" height exceeds spec */
            if ((double)(colbreaks->textrow[j].r2-colbreaks->textrow[j].r1+1)/src_dpi
                     > ocr_max_height_inches)
                continue;
#if (WILLUSDEBUGX & 32)
printf("j=%d of %d\n",j,colbreaks->n);
{
static int counter=1;
int i;
char filename[256];
WILLUSBITMAP *bmp,_bmp;
bmp=&_bmp;
bmp_init(bmp);
bmp->width=colbreaks->textrow[j].c2-colbreaks->textrow[j].c1+1;
bmp->height=colbreaks->textrow[j].r2-colbreaks->textrow[j].r1+1;
bmp->bpp=8;
bmp_alloc(bmp);
for (i=0;i<256;i++)
bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
for (i=0;i<bmp->height;i++)
{
unsigned char *s,*d;
s=bmp_rowptr_from_top(newregion->bmp8,colbreaks->textrow[j].r1+i)+colbreaks->textrow[j].c1;
d=bmp_rowptr_from_top(bmp,i);
memcpy(d,s,bmp->width);
}
sprintf(filename,"word%04d.png",counter);
bmp_write(bmp,filename,stdout,100);
bmp_free(bmp);
printf("%5d. ",counter);
fflush(stdout);
#endif
            wordbuf[0]='\0';
#ifdef HAVE_TESSERACT
#ifdef HAVE_GOCR
            if (dst_ocr=='t' && !ocrtess_status)
#else
            if (!ocrtess_status)
#endif
                ocrtess_single_word_from_bmp8(wordbuf,255,newregion->bmp8,
                                          colbreaks->textrow[j].c1,
                                          colbreaks->textrow[j].r1,
                                          colbreaks->textrow[j].c2,
                                          colbreaks->textrow[j].r2,3,0,1,NULL);
#ifdef HAVE_GOCR
            else
#endif
#endif
#ifdef HAVE_GOCR
                jocr_single_word_from_bmp8(wordbuf,255,newregion->bmp8,
                                            colbreaks->textrow[j].c1,
                                            colbreaks->textrow[j].r1,
                                            colbreaks->textrow[j].c2,
                                            colbreaks->textrow[j].r2,0,1);
#endif
#if (WILLUSDEBUGX & 32)
if (wordbuf[0]!='\0')
printf("%s\n",wordbuf);
else
printf("(OCR failed)\n");
counter++;
}
#endif
            if (wordbuf[0]!='\0')
                {
                OCRWORD word;
                word.c=colbreaks->textrow[j].c1;
                word.r=rowbase;
                word.maxheight=rowbase-colbreaks->textrow[j].r1;
                word.w=colbreaks->textrow[j].c2-colbreaks->textrow[j].c1+1;
                word.h=colbreaks->textrow[j].r2-colbreaks->textrow[j].r1+1;
                word.lcheight=lcheight;
                word.rot=0;
                word.text=wordbuf;
#if (WILLUSDEBUGX & 32)
printf("'%s', r1=%d, rowbase=%d, h=%d\n",wordbuf,
                             colbreaks->textrow[j].r1,
                             colbreaks->textrow[j].rowbase,
                             colbreaks->textrow[j].r2-colbreaks->textrow[j].r1+1);
#endif
                /* Use same rowbase for whole row */
                ocrwords_add_word(words,&word);
                }
            }
        }
    breakinfo_free(105,colbreaks);
    breakinfo_free(104,rowbreaks);
    willus_dmem_free(25,(double **)&colcount,funcname);
    if (src->bpp!=8)
        bmp_free(gray);
    }
#endif /* HAVE_OCR */


/*
** flags&1  : trim c1
** flags&2  : trim c2
** flags&4  : trim r1
** flags&8  : trim r2
** flags&16 : Find rowbase, font size, etc.
**
** Row base is where row dist crosses 50% on r2 side.
** Font size is where row dist crosses 5% on other side (r1 side).
** Lowercase font size is where row dist crosses 50% on r1 side.
**
** For 12 pt font:
**     Single spacing is 14.66 pts (Calibri), 13.82 pts (Times), 13.81 pts (Arial)
**     Size of cap letter is 7.7 pts (Calibri), 8.1 pts (Times), 8.7 pts (Arial)
**     Size of small letter is 5.7 pts (Calibri), 5.6 pts (Times), 6.5 pts (Arial)
** Mean line spacing = 1.15 - 1.22 (~1.16)
** Mean cap height = 0.68
** Mean small letter height = 0.49
** 
*/
static void bmpregion_trim_margins(BMPREGION *region,int *colcount0,int *rowcount0,int flags)

    {
    int i,j,n; /* ,r1,r2,dr1,dr2,dr,vtrim,vspace; */
    int *colcount,*rowcount;
    static char *funcname="bmpregion_trim_margins";

    /* To detect a hyphen, we need to trim and calc text base row */
    if (flags&32)
        flags |= 0x1f;
    if (colcount0==NULL)
        willus_dmem_alloc_warn(10,(void **)&colcount,sizeof(int)*(region->c2+1),funcname,10);
    else
        colcount=colcount0;
    if (rowcount0==NULL)
        willus_dmem_alloc_warn(11,(void **)&rowcount,sizeof(int)*(region->r2+1),funcname,10);
    else
        rowcount=rowcount0;
    n=region->c2-region->c1+1;
/*
printf("Trim:  reg=(%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
if (region->c2+1 > cca || region->r2+1 > rca)
{
printf("A ha 0!\n");
exit(10);
}
*/
    memset(colcount,0,(region->c2+1)*sizeof(int));
    memset(rowcount,0,(region->r2+1)*sizeof(int));
    for (j=region->r1;j<=region->r2;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(region->bmp8,j)+region->c1;
        for (i=0;i<n;i++,p++)
            if (p[0]<region->bgcolor)
                {
                rowcount[j]++;
                colcount[i+region->c1]++;
                }
        }
    /*
    ** Trim excess margins
    */
    if (flags&1)
        trim_to(colcount,&region->c1,region->c2,src_left_to_right ? 2.0 : 4.0);
    if (flags&2)
        trim_to(colcount,&region->c2,region->c1,src_left_to_right ? 4.0 : 2.0);
    if (colcount0==NULL)
        willus_dmem_free(10,(double **)&colcount,funcname);
    if (flags&4)
        trim_to(rowcount,&region->r1,region->r2,4.0);
    if (flags&8)
        trim_to(rowcount,&region->r2,region->r1,4.0);
    if (flags&16)
        {
        int maxcount,mc2,h2;
        double f;

        maxcount=0;
        for (i=region->r1;i<=region->r2;i++)
            if (rowcount[i] > maxcount)
                maxcount = rowcount[i];
        mc2 = maxcount / 2;
        for (i=region->r2;i>=region->r1;i--)
            if (rowcount[i] > mc2)
                break;
        region->rowbase = i;
        for (i=region->r1;i<=region->r2;i++)
            if (rowcount[i] > mc2)
                break;
        region->h5050 = region->lcheight = region->rowbase-i+1;
        mc2 = maxcount / 20;
        for (i=region->r1;i<=region->r2;i++)
            if (rowcount[i] > mc2)
                break;
        region->capheight = region->rowbase-i+1;
        /*
        ** Sanity check capheight and lcheight
        */
        h2=height2_calc(&rowcount[region->r1],region->r2-region->r1+1);
#if (WILLUSDEBUGX & 8)
if (region->c2-region->c1 > 1500)
printf("reg %d x %d (%d,%d) - (%d,%d) h2=%d ch/h2=%g\n",region->c2-region->c1+1,region->r2-region->r1+1,region->c1,region->r1,region->c2,region->r2,h2,(double)region->capheight/h2);
#endif
        if (region->capheight < h2*0.75)
            region->capheight = h2;
        f=(double)region->lcheight/region->capheight;
        if (f<0.55)
            region->lcheight = (int)(0.72*region->capheight+.5);
        else if (f>0.85)
            region->lcheight = (int)(0.72*region->capheight+.5);
#if (WILLUSDEBUGX & 8)
if (region->c2-region->c1 > 1500)
printf("    lcheight final = %d\n",region->lcheight);
#endif
#if (WILLUSDEBUGX & 10)
if (region->c2-region->c1 > 1500 && region->r2-region->r1 < 100)
{
static int append=0;
FILE *f;
int i;
f=fopen("textrows.ep",append==0?"w":"a");
append=1;
for (i=region->r1;i<=region->r2;i++)
fprintf(f,"%d %g\n",region->rowbase-i,(double)rowcount[i]/maxcount);
fprintf(f,"//nc\n");
fclose(f);
}
#endif
        }
    else
        {
        region->h5050 = region->r2-region->r1+1;
        region->capheight = 0.68*(region->r2-region->r1+1);
        region->lcheight = 0.5*(region->r2-region->r1+1);
        region->rowbase = region->r2;
        }
#if (WILLUSDEBUGX & 2)
printf("trim:\n    reg->c1=%d, reg->c2=%d\n",region->c1,region->c2);
printf("    reg->r1=%d, reg->r2=%d, reg->rowbase=%d\n\n",region->r1,region->r2,region->rowbase);
#endif
    if (rowcount0==NULL)
        willus_dmem_free(11,(double **)&rowcount,funcname);
    }


/*
** Does region end in a hyphen?  If so, fill in HYPHENINFO structure.
*/
static void bmpregion_hyphen_detect(BMPREGION *region)

    {
    int i,j; /* ,r1,r2,dr1,dr2,dr,vtrim,vspace; */
    int width;
    int *r0,*r1,*r2,*r3;
    int rmin,rmax,rowbytes,nrmid,rsum;
    int cstart,cend,cdir;
    unsigned char *p;
    static char *funcname="bmpregion_hyphen_detect";

#if (WILLUSDEBUGX & 16)
static int count=0;
char pngfile[256];
FILE *out;

count++;
printf("@bmpregion_hyphen_detect count=%d\n",count);
sprintf(pngfile,"word%04d.png",count);
bmpregion_write(region,pngfile);
sprintf(pngfile,"word%04d.txt",count);
out=fopen(pngfile,"w");
fprintf(out,"c1=%d, c2=%d, r1=%d, r2=%d\n",region->c1,region->c2,region->r1,region->r2);
fprintf(out,"lcheight=%d\n",region->lcheight);
#endif

    region->hyphen.ch = -1;
    region->hyphen.c2 = -1;
    if (!k2_hyphen_detect)
        return;
    width=region->c2-region->c1+1;
    if (width<2)
        return;
    willus_dmem_alloc_warn(27,(void **)&r0,sizeof(int)*4*width,funcname,10);
    r1=&r0[width];
    r2=&r1[width];
    r3=&r2[width];
    for (i=0;i<width;i++)
        r0[i]=r1[i]=r2[i]=r3[i]=-1;
    rmin=region->rowbase-region->capheight-region->lcheight*.04;
    if (rmin < region->r1)
        rmin = region->r1;
    rmax=region->rowbase+region->lcheight*.04;
    if (rmax > region->r2)
        rmax = region->r2;
    rowbytes=bmp_bytewidth(region->bmp8);
    p=bmp_rowptr_from_top(region->bmp8,0);
    nrmid=rsum=0;
    if (src_left_to_right)
        {
        cstart=region->c2;
        cend=region->c1-1;
        cdir=-1;
        }
    else
        {
        cstart=region->c1;
        cend=region->c2+1;
        cdir=1;
        }
#if (WILLUSDEBUGX & 16)
fprintf(out,"   j     r0     r1     r2     r3\n");
#endif
    for (j=cstart;j!=cend;j+=cdir)
        {
        int r,rmid,dr,drmax;

// printf("j=%d\n",j);
        rmid=(rmin+rmax)/2;
// printf("   rmid=%d\n",rmid);
        drmax=region->r2+1-rmid > rmid-region->r1+1 ? region->r2+1-rmid : rmid-region->r1+1;
        /* Find dark region closest to center line */
        for (dr=0;dr<drmax;dr++)
            {
            if (rmid+dr<=region->r2 && p[(rmid+dr)*rowbytes+j]<region->bgcolor)
                break;
            if (rmid-dr>=region->r1 && p[(rmid-dr)*rowbytes+j]<region->bgcolor)
                {
                dr=-dr;
                break;
                }
            }
#if (WILLUSDEBUGX & 16)
fprintf(out,"    dr=%d/%d, rmid+dr=%d, rmin=%d, rmax=%d, nrmid=%d\n",dr,drmax,rmid+dr,rmin,rmax,nrmid);
#endif
        /* No dark detected or mark is outside hyphen region? */
        /* Termination criterion #1 */
        if (dr>=drmax || (nrmid>2 && (double)nrmid/region->lcheight>.1 
                               && (rmid+dr<rmin || rmid+dr>rmax)))
            {
            if (region->hyphen.ch>=0 && dr>=drmax)
                continue;
            if (nrmid>2 && (double)nrmid/region->lcheight > .35)
                {
                region->hyphen.ch = j-cdir;
                region->hyphen.r1 = rmin;
                region->hyphen.r2 = rmax;
                }
            if (dr<drmax)
                {
                region->hyphen.c2=j;
                break;
                }
            continue;
            }
        if (region->hyphen.ch>=0)
            {
            region->hyphen.c2=j;
            break;
            }
        nrmid++;
        rmid += dr;
        /* Dark spot is outside expected hyphen area */
        /*
        if (rmid<rmin || rmid>rmax)
            {
            if (nrmid>0)
                break;
            continue;
            }
        */
        for (r=rmid;r>=region->r1;r--)
            if (p[r*rowbytes+j]>=region->bgcolor)
                break;
        r1[j-region->c1]=r+1;
        r0[j-region->c1]=-1;
        if (r>=region->r1)
            {
            for (;r>=region->r1;r--)
                if (p[r*rowbytes+j]<region->bgcolor)
                    break;
            if (r>=region->r1)
                r0[j-region->c1]=r;
            }
        for (r=rmid;r<=region->r2;r++)
            if (p[r*rowbytes+j]>=region->bgcolor)
                break;
        r2[j-region->c1]=r-1;
        r3[j-region->c1]=-1;
        if (r<=region->r2)
            {
            for (;r<=region->r2;r++)
                if (p[r*rowbytes+j]<region->bgcolor)
                    break;
            if (r<=region->r2)
                r3[j-region->c1]=r;
            }
#if (WILLUSDEBUGX & 16)
fprintf(out," %4d  %4d  %4d  %4d  %4d\n",j,r0[j-region->c1],r1[j-region->c1],r2[j-region->c1],r3[j-region->c1]);
#endif
        if (region->hyphen.c2<0 && (r0[j-region->c1]>=0 || r3[j-region->c1]>=0))
            region->hyphen.c2=j;
        /* Termination criterion #2 */
        if (nrmid>2 && (double)nrmid/region->lcheight > .35
                && (r1[j-region->c1] > rmax || r2[j-region->c1] < rmin))
            {
            region->hyphen.ch = j-cdir;
            region->hyphen.r1 = rmin;
            region->hyphen.r2 = rmax;
            if (region->hyphen.c2<0)
                region->hyphen.c2=j;
            break;
            }
        // rc=(r1[j-region->c1]+r2[j-region->c1])/2;
        /* DQ possible hyphen if r1/r2 out of range */
        if (nrmid>1)
           {
           /* Too far away from last values? */
           if ((double)(rmin-r1[j-region->c1])/region->lcheight > .1
               || (double)(r2[j-region->c1]-rmax)/region->lcheight > .1)
               break;
           if ((double)nrmid/region->lcheight > .1 && nrmid>1)
               {
               if ((double)fabs(rmin-r1[j-region->c1])/region->lcheight > .1
                   || (double)(rmax-r2[j-region->c1])/region->lcheight > .1)
                   break;
               }
           }
        if (nrmid==1 || r1[j-region->c1]<rmin)
            rmin=r1[j-region->c1];
        if (nrmid==1 || r2[j-region->c1]>rmax)
            rmax=r2[j-region->c1];
        if ((double)nrmid/region->lcheight > .1 && nrmid>1)
            {
            double rmean;

            /* Can't be too thick */
            if ((double)(rmax-rmin)/region->lcheight > .55
                    || (double)(rmax-rmin)/region->lcheight < .08)
                break;
            /* Must be reasonably well centered above baseline */
            rmean=(double)(rmax+rmin)/2;
            if ((double)(region->rowbase-rmean)/region->lcheight < 0.35
                  || (double)(region->rowbase-rmean)/region->lcheight > 0.85)
                break;
            if ((double)(region->rowbase-rmax)/region->lcheight < 0.2
                  || (double)(region->rowbase-rmin)/region->lcheight > 0.92)
                break;
            }
        }
#if (WILLUSDEBUGX & 16)
fprintf(out,"   ch=%d, c2=%d, r1=%d, r2=%d\n",region->hyphen.ch,region->hyphen.c2,region->hyphen.r1,region->hyphen.r2);
fclose(out);
#endif
    /* More sanity checks--better to miss a hyphen than falsely detect it. */
    if (region->hyphen.ch>=0)
        {
        double ar;
        /* If it's only a hyphen, then it's probably actually a dash--don't detect it. */
        if (region->hyphen.c2<0)
            region->hyphen.ch = -1;
        /* Check aspect ratio */
        ar=(double)(region->hyphen.r2-region->hyphen.r1)/nrmid;
        if (ar<0.08 || ar > 0.75)
            region->hyphen.ch = -1;
        }
    willus_dmem_free(27,(double **)&r0,funcname);
#if (WILLUSDEBUGX & 16)
if (region->hyphen.ch>=0)
printf("\n\n   GOT HYPHEN.\n\n");
printf("   Exiting bmpregion_hyphen_detect\n");
#endif
    }


#if (defined(WILLUSDEBUGX) || defined(WILLUSDEBUG))
static void bmpregion_write(BMPREGION *region,char *filename)

    {
    int i,bpp;
    WILLUSBITMAP *bmp,_bmp;

    bmp=&_bmp;
    bmp_init(bmp);
    bmp->width=region->c2-region->c1+1;
    bmp->height=region->r2-region->r1+1;
    bmp->bpp=region->bmp->bpp;
    bpp=bmp->bpp==8?1:3;
    bmp_alloc(bmp);
    for (i=0;i<256;i++)
        bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
    for (i=0;i<bmp->height;i++)
        {
        unsigned char *s,*d;
        s=bmp_rowptr_from_top(region->bmp,region->r1+i)+region->c1*bpp;
        d=bmp_rowptr_from_top(bmp,i);
        memcpy(d,s,bmp->width*bpp);
        }
    bmp_write(bmp,filename,stdout,97);
    bmp_free(bmp);
    }
#endif

#if (WILLUSDEBUGX & 6)
static void breakinfo_echo(BREAKINFO *breakinfo)

    {
    int i;
    printf("@breakinfo_echo...\n");
    for (i=0;i<breakinfo->n;i++)
        printf("    %2d.  r1=%4d, rowbase=%4d, r2=%4d, c1=%4d, c2=%4d\n",
             i+1,breakinfo->textrow[i].r1,
             breakinfo->textrow[i].rowbase,
             breakinfo->textrow[i].r2,
             breakinfo->textrow[i].c1,
             breakinfo->textrow[i].c2);
    }
#endif


/*
** Calculate weighted height of a rectangular region.
** This weighted height is intended to be close to the height of
** a capital letter, or the height of the majority of the region.
**
*/
static int height2_calc(int *rc,int n)

    {
    int i,thresh,i1,h2;
    int *c;
    static char *funcname="height2_calc";
#if (WILLUSDEBUGX & 8)
    int cmax;
#endif

    if (n<=0)
        return(1);
    willus_dmem_alloc_warn(12,(void **)&c,sizeof(int)*n,funcname,10);
    memcpy(c,rc,n*sizeof(int));
    sorti(c,n);
#if (WILLUSDEBUGX & 8)
    cmax=c[n-1];
#endif
    for (i=0;i<n-1 && c[i]==0;i++);
    thresh=c[(i+n)/3];
    willus_dmem_free(12,(double **)&c,funcname);
    for (i=0;i<n-1;i++)
        if (rc[i]>=thresh)
            break;
    i1=i;
    for (i=n-1;i>i1;i--)
        if (rc[i]>=thresh)
            break;
#if (WILLUSDEBUGX & 8)
// printf("thresh = %g, i1=%d, i2=%d\n",(double)thresh/cmax,i1,i);
#endif
    h2=i-i1+1; /* Guaranteed to be >=1 */
    return(h2);
    }


static void trim_to(int *count,int *i1,int i2,double gaplen)

    {
    int del,dcount,igaplen,clevel,dlevel,defect_start,last_defect;

    igaplen=(int)(gaplen*src_dpi/72.);
    if (igaplen<1)
        igaplen=1;
    /* clevel=(int)(defect_size_pts*src_dpi/72./3.); */
    clevel=0;
    dlevel=(int)(pow(defect_size_pts*src_dpi/72.,2.)*PI/4.+.5);
    del=i2>(*i1) ? 1 : -1;
    defect_start=-1;
    last_defect=-1;
    dcount=0;
    for (;(*i1)!=i2;(*i1)=(*i1)+del)
        {
        if (count[(*i1)]<=clevel)
            {
            dcount=0;  /* Reset defect size */
            continue;
            }
        /* Mark found */
        if (dcount==0)
            {
            if (defect_start>=0)
                last_defect=defect_start;
            defect_start=(*i1);
            }
        dcount += count[(*i1)];
        if (dcount >= dlevel)
            {
            if (last_defect>=0 && abs(defect_start-last_defect)<=igaplen)
                (*i1)=last_defect;
            else
                (*i1)=defect_start;
            return;
            }
        }
    if (defect_start<0)
        return;
    if (last_defect<0)
        {
        (*i1)=defect_start;
        return;
        }
    if (abs(defect_start-last_defect)<=igaplen)
        (*i1)=last_defect;
    else
        (*i1)=defect_start;
    }


/*
** A region that needs its line spacing and justification analyzed.
**
** The region may be wider than the max desirable region width.
**
** Input:  breakinfo should be valid row-break information for the region.
**
** Calls bmpregion_one_row_wrap_and_add() for each text row from the
** breakinfo structure that is within the region.
**
*/
static void bmpregion_analyze_justification_and_line_spacing(BMPREGION *region,
                                   BREAKINFO *breakinfo,MASTERINFO *masterinfo,
                                   int *colcount,int *rowcount,PAGEINFO *pageinfo,
                                   int allow_text_wrapping,double force_scale)

    {
    int i,i1,i2,ntr,mean_row_gap,maxgap,line_spacing,nls,nch;
    BMPREGION *newregion,_newregion;
    double *id,*c1,*c2,*ch,*lch,*ls;
    int *just,*indented,*short_line;
    double capheight,lcheight,fontsize;
    int textheight,ragged_right,src_line_spacing;
    static char *funcname="bmpregion_analyze_justification_and_line_spacing";

#if (WILLUSDEBUGX & 1)
printf("@bmpregion_analyze_justification_and_line_spacing");
printf("    (%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
printf("    centering = %d\n",breakinfo->centered);
#endif
#if (WILLUSDEBUGX & 2)
breakinfo_echo(breakinfo);
#endif

    /* Locate the vertical part indices in the breakinfo structure */
    newregion=&_newregion;
    breakinfo_sort_by_row_position(breakinfo);
    for (i=0;i<breakinfo->n;i++)
        {
        TEXTROW *textrow;
        textrow=&breakinfo->textrow[i];
        if ((textrow->r1+textrow->r2)/2 >= region->r1)
            break;
        }
    if (i>=breakinfo->n)
        return;
    i1=i;
    for (;i<breakinfo->n;i++)
        {
        TEXTROW *textrow;
        textrow=&breakinfo->textrow[i];
        if ((textrow->r1+textrow->r2)/2 > region->r2)
            break;
        }
    i2=i-1;
    if (i2<i1)
        return;
    ntr=i2-i1+1;
#if (WILLUSDEBUGX & 1)
printf("    i1=%d, i2=%d, ntr=%d\n",i1,i2,ntr);
#endif

    willus_dmem_alloc_warn(13,(void **)&c1,sizeof(double)*6*ntr,funcname,10);
    willus_dmem_alloc_warn(14,(void **)&just,sizeof(int)*3*ntr,funcname,10);
    c2=&c1[ntr];
    ch=&c2[ntr];
    lch=&ch[ntr];
    ls=&lch[ntr];
    id=&ls[ntr];
    indented=&just[ntr];
    short_line=&indented[ntr];
    for (i=0;i<ntr;i++)
        id[i]=i;

    /* Find baselines / font size */
    capheight=lcheight=0.;
    maxgap=-1;
    for (nch=nls=0,i=i1;i<=i2;i++)
        {
        TEXTROW *textrow;
        double ar,rh;
        int marking_flags;

        textrow=&breakinfo->textrow[i];
        c1[i-i1]=(double)textrow->c1;
        c2[i-i1]=(double)textrow->c2;
        if (i<i2 && maxgap < textrow->gap)
            {
            maxgap = textrow->gap;
            if (maxgap < 2)
                maxgap=2;
            }
        if (textrow->c2<textrow->c1)
            ar = 100.;
        else
            ar = (double)(textrow->r2-textrow->r1+1)/(double)(textrow->c2-textrow->c1+1);
        rh = (double)(textrow->r2-textrow->r1+1)/src_dpi;
        if (i<i2 && ar <= no_wrap_ar_limit && rh <= no_wrap_height_limit_inches)
            ls[nls++]=breakinfo->textrow[i+1].r1-textrow->r1;
        if (ar <= no_wrap_ar_limit && rh <= no_wrap_height_limit_inches)
            {
            ch[nch] = textrow->capheight;
            lch[nch] = textrow->lcheight;
            nch++;
            }

        /* Mark region w/gray, mark rowbase also */
        marking_flags=(i==i1?0:1)|(i==i2?0:2);
        if (i<i2 || textrow->r2-textrow->rowbase>1)
            marking_flags |= 0x10;
        (*newregion)=(*region);
        newregion->r1=textrow->r1;
        newregion->r2=textrow->r2;
        newregion->c1=textrow->c1;
        newregion->c2=textrow->c2;
        newregion->rowbase=textrow->rowbase;
        mark_source_page(newregion,5,marking_flags);
#if (WILLUSDEBUGX & 1)
printf("   Row %2d: (%4d,%4d) - (%4d,%4d) rowbase=%4d, lch=%d, h5050=%d, rh=%d\n",i-i1+1,textrow->c1,textrow->r1,textrow->c2,textrow->r2,textrow->rowbase,textrow->lcheight,textrow->h5050,textrow->rowheight);
#endif
        }
    wrapbmp_set_maxgap(maxgap);
    if (nch<1)
        capheight = lcheight = 2; // Err on the side of too small
    else
        {
        capheight = median_val(ch,nch);
        lcheight = median_val(lch,nch);
        }
// printf("capheight = %g, lcheight = %g\n",capheight,lcheight);
    bmpregion_is_centered(region,breakinfo,i1,i2,&textheight);
    /*
    ** For 12 pt font:
    **     Single spacing is 14.66 pts (Calibri), 13.82 pts (Times), 13.81 pts (Arial)
    **     Size of cap letter is 7.7 pts (Calibri), 8.1 pts (Times), 8.7 pts (Arial)
    **     Size of small letter is 5.7 pts (Calibri), 5.6 pts (Times), 6.5 pts (Arial)
    ** Mean line spacing = 1.15 - 1.22 (~1.16)
    ** Mean cap height = 0.68
    ** Mean small letter height = 0.49
    */
    fontsize = (capheight+lcheight)/1.17;
// printf("font size = %g pts.\n",(fontsize/src_dpi)*72.);
    /*
    ** Set line spacing for this region
    */
    if (nls>0)
        src_line_spacing = median_val(ls,nls);
    else
        src_line_spacing = fontsize*1.2;
    if (vertical_line_spacing < 0 
              && src_line_spacing <= fabs(vertical_line_spacing)*fontsize*1.16)
        line_spacing = src_line_spacing;
    else
        line_spacing = fabs(vertical_line_spacing)*fontsize*1.16;
#if (WILLUSDEBUGX & 1)
printf("   font size = %.2f pts = %d pixels\n",(fontsize/src_dpi)*72.,(int)(fontsize+.5));
printf("   src_line_spacing = %d, line_spacing = %d\n",src_line_spacing,line_spacing);
#endif
    /*
    if (ntr==1)
        rheight=  (int)((breakinfo->textrow[i1].r2 - breakinfo->textrow[i1].r1)*1.25+.5);
    else
        rheight = (int)((double)(breakinfo->textrow[i2].rowbase - breakinfo->textrow[i1].rowbase)/(ntr-1)+.5);
    */
    mean_row_gap = line_spacing - textheight;
    if (mean_row_gap <= 1)
        mean_row_gap = 1;


    /* Try to figure out if we have a ragged right edge */
    if (ntr<3)
        ragged_right=1;
    else
        {
        int flushcount;

        if (src_left_to_right)
            {
            for (flushcount=i=0;i<ntr;i++)
                {
#if (WILLUSDEBUGX & 1)
printf("    flush_factors[%d] = %g (<.5), %g in (<.1)\n",
i,(double)(region->c2-c2[i])/textheight,(double)(region->c2-c2[i])/src_dpi);
#endif
                if ((double)(region->c2-c2[i])/textheight < 0.5
                      && (double)(region->c2-c2[i])/src_dpi < 0.1)
                    flushcount++;
                }
            }
        else
            {
            for (flushcount=i=0;i<ntr;i++)
                {
#if (WILLUSDEBUGX & 1)
printf("    flush_factors[%d] = %g (<.5), %g in (<.1)\n",
i,(double)(c1[i]-region->c1)/textheight,(double)(c1[i]-region->c1)/src_dpi);
#endif
                if ((double)(c1[i]-region->c1)/textheight < 0.5
                      && (double)(c1[i]-region->c1)/src_dpi < 0.1)
                    flushcount++;
                }
            }
        ragged_right = (flushcount <= ntr/2);
        /*
        if (src_left_to_right)
            {
            sortxyd(c2,id,ntr);
            del = region->c2 - c2[ntr-1-ntr/3];
            sortxyd(id,c2,ntr);
            }
        else
            {
            sortxyd(c1,id,ntr);
            del = c1[ntr/3] - region->c1;
            sortxyd(id,c1,ntr);
            }
        del /= textheight;
printf("del=%g\n",del);
        ragged_right = (del > 0.5);
        */
        }
#if (WILLUSDEBUGX & 1)
printf("ragged_right=%d\n",ragged_right);
#endif

    /* Store justification and other info line by line */
    for (i=i1;i<=i2;i++)
        {
        double indent1,del;
        double i1f,ilfi,i2f,ilf,ifmin,dif;
        int centered;

        TEXTROW *textrow;
        textrow=&breakinfo->textrow[i];
        i1f = (double)(c1[i-i1]-region->c1)/(region->c2-region->c1+1);
        i2f = (double)(region->c2-c2[i-i1])/(region->c2-region->c1+1);
        ilf = src_left_to_right ? i1f : i2f;
        ilfi = ilf*(region->c2-region->c1+1)/src_dpi; /* Indent in inches */
        ifmin = i1f<i2f ? i1f : i2f;
        dif=fabs(i1f-i2f);
        if (ifmin < .01)
            ifmin=0.01;
        if (src_left_to_right)
            indent1 = (double)(c1[i-i1]-region->c1) / textheight;
        else
            indent1 = (double)(region->c2 - c2[i-i1]) / textheight;
// printf("    row %2d:  indent1=%g\n",i-i1,indent1);
        if (!breakinfo->centered)
            {
            indented[i-i1]=(indent1 > 0.5 && ilfi < 1.2 && ilf < .25);
            centered= (!indented[i-i1] && indent1 > 1.0 && dif/ifmin<0.5);
            }
        else
            {
            centered= (dif<0.1 || dif/ifmin<0.5);
            indented[i-i1]=(indent1 > 0.5 && ilfi < 1.2 && ilf < .25 && !centered);
            }
#if (WILLUSDEBUGX & 1)
printf("Indent %d:  %d.  indent1=%g, ilf=%g, centered=%d\n",i-i1+1,indented[i-i1],indent1,ilf,centered);
printf("    indent1=%g, i1f=%g, i2f=%g\n",indent1,i1f,i2f);
#endif
        if (centered)
            just[i-i1] = 4;
        else
            {
            /*
            ** The .01 favors left justification over right justification in
            ** close cases.
            */
            if (src_left_to_right)
                just[i-i1] = indented[i-i1] || (i1f < i2f+.01) ? 0 : 8;
            else
                just[i-i1] = indented[i-i1] || (i2f < i1f+.01) ? 8 : 0;
            }
        if (src_left_to_right)
            del = (double)(region->c2 - textrow->c2);
        else
            del = (double)(textrow->c1 - region->c1);
        /* Should we keep wrapping after this line? */
        if (!ragged_right)
            short_line[i-i1] = (del/textheight > 0.5);
        else
            short_line[i-i1] = (del/(region->c2-region->c1) > 0.25);
        /* If this row is a bigger/smaller row (font) than the next row, don't wrap. */
        if (!short_line[i-i1] && i<i2)
            {
            TEXTROW *t1;
            t1=&breakinfo->textrow[i+1];
            if ((textrow->h5050 > t1->h5050*1.5 || textrow->h5050*1.5 < t1->h5050)
                  && (i==0 || (i>0 && (textrow->rowheight > t1->rowheight*1.5
                                        || textrow->rowheight*1.5 < t1->rowheight))))
                short_line[i-i1] = 1;
            }
        if (!ragged_right)
            just[i-i1] |= 0x40;
#if (WILLUSDEBUGX & 1)
printf("        just[%d]=0x%02X, shortline[%d]=%d\n",i-i1,just[i-i1],i-i1,short_line[i-i1]);
printf("        textrow->c2=%d, region->c2=%d, del=%g, textheight=%d\n",textrow->c2,region->c2,del,textheight);
#endif
        /* If short line, it should still be fully justified if it is wrapped. */
        /*
        if (short_line[i-i1])
            just[i-i1] = (just[i-i1]&0xf)|0x60;
        */
        }
/*        
{
double mean1,mean2,stdev1,stdev2;
array_mean(c1,ntr,&mean1,&stdev1);
array_mean(c2,ntr,&mean2,&stdev2);
printf("Mean c1, c2 = %g, %g; stddevs = %g, %g\n",mean1,mean2,stdev1,stdev2);
printf("textheight = %d, line_spacing = %d\n",textheight,line_spacing);
}
*/
    for (i=i1;i<=i2;i++)
        {
        TEXTROW *textrow;
        int justflags,trimflags,centered,marking_flags,gap;

#if (WILLUSDEBUGX & 1)
aprintf("Row " ANSI_YELLOW "%d of %d" ANSI_NORMAL " (wrap=%d)\n",i-i1+1,i2-i1+1,allow_text_wrapping);
#endif
        textrow=&breakinfo->textrow[i];
        (*newregion)=(*region);
        newregion->r1=textrow->r1;
        newregion->r2=textrow->r2;

        /* The |3 tells it to use the user settings for left/right/center */
        justflags = just[i-i1]|0x3;
        centered=((justflags&0xc)==4);
#if (WILLUSDEBUGX & 1)
printf("    justflags[%d]=0x%2X, centered=%d, indented=%d\n",i-i1,justflags,centered,indented[i-i1]);
#endif
        if (allow_text_wrapping)
            {
            /* If this line is indented or if the justification has changed, */
            /* then start a new line.                                        */
            if (centered || indented[i-i1] || (i>i1 && (just[i-i1]&0xc)!=(just[i-i1-1]&0xc)))
{
#ifdef WILLUSDEBUG
printf("wrapflush4\n");
#endif
                wrapbmp_flush(masterinfo,0,pageinfo,1);
}
#ifdef WILLUSDEBUG
printf("    c1=%d, c2=%d\n",newregion->c1,newregion->c2);
#endif
            marking_flags=0xc|(i==i1?0:1)|(i==i2?0:2);
            bmpregion_one_row_wrap_and_add(newregion,breakinfo,i,i1,i2,
                                       masterinfo,justflags,colcount,rowcount,pageinfo,
                                       line_spacing,mean_row_gap,textrow->rowbase,
                                       marking_flags,indented[i-i1]);
            if (centered || short_line[i-i1])
{
#ifdef WILLUSDEBUG
printf("wrapflush5\n");
#endif
                wrapbmp_flush(masterinfo,0,pageinfo,2);
}
            continue;
            }
#ifdef WILLUSDEBUG
printf("wrapflush5a\n");
#endif
        wrapbmp_flush(masterinfo,0,pageinfo,1);
        /* If default justifications, ignore all analysis and just center it. */
        if (dst_justify<0 && dst_fulljustify<0)
            {
            newregion->c1 = region->c1;
            newregion->c2 = region->c2;
            justflags=0xad; /* Force centered region, no justification */
            trimflags=0x80;
            }
        else
            trimflags=0;
        /* No wrapping:  text wrap, trim flags, vert breaks, fscale, just */
        bmpregion_add(newregion,breakinfo,masterinfo,0,trimflags,0,force_scale,
                      justflags,5,colcount,rowcount,pageinfo,0,
                      textrow->r2-textrow->rowbase);
        if (vertical_line_spacing < 0)
            {
            int gap1;
            gap1 = line_spacing - (textrow->r2-textrow->r1+1);
            if (i<i2)
                gap = textrow->gap > gap1 ? gap1 : textrow->gap;
            else
                {
                gap = textrow->rowheight - (textrow->rowbase + last_rowbase_internal);
                if (gap < mean_row_gap/2.)
                    gap = mean_row_gap;
                }
            }
        else
            {
            gap = line_spacing - (textrow->r2-textrow->r1+1);
            if (gap < mean_row_gap/2.)
                gap = mean_row_gap;
            }
        if (i<i2)
            dst_add_gap_src_pixels("No-wrap line",masterinfo,gap);
        else
            {
            last_h5050_internal = textrow->h5050;
            beginning_gap_internal = gap;
            }
        }
    willus_dmem_free(14,(double **)&just,funcname);
    willus_dmem_free(13,(double **)&c1,funcname);
#ifdef WILLUSDEBUG
printf("Done wrap_and_add.\n");
#endif
    }


static int bmpregion_is_centered(BMPREGION *region,BREAKINFO *breakinfo,int i1,int i2,int *th)

    {
    int j,i,cc,n1,ntr;
    int textheight;

#if (WILLUSDEBUGX & 1)
printf("@bmpregion_is_centered:  region=(%d,%d) - (%d,%d)\n",region->c1,region->r1,region->c2,region->r2);
printf("    nrows = %d\n",i2-i1+1);
#endif
    ntr=i2-i1+1;
    for (j=0;j<3;j++)
        {
        for (n1=textheight=0,i=i1;i<=i2;i++)
            {
            TEXTROW *textrow;
            double ar,rh;

            textrow=&breakinfo->textrow[i];
            if (textrow->c2<textrow->c1)
                ar = 100.;
            else
                ar = (double)(textrow->r2-textrow->r1+1)/(double)(textrow->c2-textrow->c1+1);
            rh = (double)(textrow->r2-textrow->r1+1)/src_dpi;
            if (j==2 || (j>=1 && rh<=no_wrap_height_limit_inches)
                     || (j==0 && rh<=no_wrap_height_limit_inches && ar<=no_wrap_ar_limit))
                {
                textheight += textrow->rowbase - textrow->r1+1;
                n1++;
                }
            }
        if (n1>0)
            break;
        }
    textheight = (int)((double)textheight/n1+.5);
    if (th!=NULL)
        {
        (*th)=textheight;
#if (WILLUSDEBUGX & 1)
printf("    textheight assigned (%d)\n",textheight);
#endif
        return(breakinfo->centered);
        }

    /*
    ** Does region appear to be centered?
    */
    for (cc=0,i=i1;i<=i2;i++)
        {
        double indent1,indent2;

#if (WILLUSDEBUGX & 1)
printf("    tr[%d].c1,c2 = %d, %d\n",i,breakinfo->textrow[i].c1,breakinfo->textrow[i].c2);
#endif
        indent1 = (double)(breakinfo->textrow[i].c1-region->c1) / textheight;
        indent2 = (double)(region->c2 - breakinfo->textrow[i].c2) / textheight;
#if (WILLUSDEBUGX & 1)
printf("    tr[%d].indent1,2 = %g, %g\n",i,indent1,indent2);
#endif
        /* If only one line and it spans the entire region, call it centered */
        /* Sometimes this won't be the right thing to to. */
        if (i1==i2 && indent1<.5 && indent2<.5)
{
#if (WILLUSDEBUGX & 1)
printf("    One line default to bigger region (%s).\n",breakinfo->centered?"not centered":"centered");
#endif
            return(1);
}
        if (fabs(indent1-indent2) > 1.5)
{
#if (WILLUSDEBUGX & 1)
printf("    Region not centered.\n");
#endif
            return(0);
}
        if (indent1 > 1.0)
            cc++;
        }
#if (WILLUSDEBUGX & 1)
printf("Region centering:  i=%d, i2=%d, cc=%d, ntr=%d\n",i,i2,cc,ntr);
#endif
    if (cc>ntr/2)
{
#if (WILLUSDEBUGX & 1)
printf("    Region is centered (enough obviously centered lines).\n");
#endif
        return(1);
}
#if (WILLUSDEBUGX & 1)
printf("    Not centered (not enough obviously centered lines).\n");
#endif
    return(0);
    }


/*
** CAUTION:  This function re-orders the x[] array!
*/
static double median_val(double *x,int n)

    {
    int i1,n1;

    if (n<4)
        return(array_mean(x,n,NULL,NULL));
    sortd(x,n);
    if (n==4)
        {
        n1=2;
        i1=1;
        }
    else if (n==5)
        {
        n1=3;
        i1=1;
        }
    else
        {
        n1=n/3;
        i1=(n-n1)/2;
        }
    return(array_mean(&x[i1],n1,NULL,NULL));
    }


/*
**
** Searches the region for vertical break points and stores them into
** the BREAKINFO structure.
**
** apsize_in = averaging aperture size in inches.  Use -1 for dynamic aperture.
**
*/
static void bmpregion_find_vertical_breaks(BMPREGION *region,BREAKINFO *breakinfo,
                                           int *colcount,int *rowcount,double apsize_in)

    {
    static char *funcname="bmpregion_find_vertical_breaks";
    int nr,i,brc,brcmin,dtrc,trc,aperture,aperturemax,figrow,labelrow;
    int ntr,rhmin_pix;
    BMPREGION *newregion,_newregion;
    int *rowthresh;
    double min_fig_height,max_fig_gap,max_label_height;

    min_fig_height=0.75;
    max_fig_gap=0.16;
    max_label_height=0.5;
    /* Trim region and populate colcount/rowcount arrays */
    bmpregion_trim_margins(region,colcount,rowcount,0xf);
    newregion=&_newregion;
    (*newregion)=(*region);
    if (debug)
        printf("@bmpregion_find_vertical_breaks:  (%d,%d) - (%d,%d)\n",
                region->c1,region->r1,region->c2,region->r2);
    /*
    ** brc = consecutive blank pixel rows
    ** trc = consecutive non-blank pixel rows
    ** dtrc = number of non blank pixel rows since last dump
    */
    nr=region->r2-region->r1+1;
    willus_dmem_alloc_warn(15,(void **)&rowthresh,sizeof(int)*nr,funcname,10);
    brcmin = max_vertical_gap_inches*src_dpi;
    aperturemax = (int)(src_dpi/72.+.5);
    if (aperturemax < 2)
        aperturemax = 2;
    aperture=(int)(src_dpi*apsize_in+.5);
/*
for (i=region->r1;i<=region->r2;i++)
printf("rowcount[%d]=%d\n",i,rowcount[i]);
*/
    breakinfo->rhmean_pixels=0; // Mean text row height
    ntr=0; // Number of text rows
    /* Fill rowthresh[] array */
    for (dtrc=0,i=region->r1;i<=region->r2;i++)
        {
        int ii,i1,i2,sum,pt;

        if (apsize_in < 0.)
            {
            aperture=(int)(dtrc/13.7+.5);
            if (aperture > aperturemax)
                aperture=aperturemax;
            if (aperture < 2)
                aperture=2;
            }
        i1=i-aperture/2;
        i2=i1+aperture-1;
        if (i1<region->r1)
            i1=region->r1;
        if (i2>region->r2)
            i2=region->r2;
        pt=(int)((i2-i1+1)*gtr_in*src_dpi+.5); /* pixel count threshold */
        if (pt<1)
            pt=1;
        /* Sum over row aperture */
        for (sum=0,ii=i1;ii<=i2;sum+=rowcount[ii],ii++);
        /* Does row have few enough black pixels to be considered blank? */
        if ((rowthresh[i-region->r1]=10*sum/pt)<=40)
            {
            if (dtrc>0)
                {
                breakinfo->rhmean_pixels += dtrc;
                ntr++;
                }
            dtrc=0;
            }
        else
            dtrc++;
        }
    if (dtrc>0)
        {
        breakinfo->rhmean_pixels += dtrc;
        ntr++;
        }
    if (ntr>0)
        breakinfo->rhmean_pixels /= ntr;
/*
printf("rhmean=%d (ntr=%d)\n",breakinfo->rhmean_pixels,ntr);
{
FILE *f;
static int count=0;
f=fopen("rthresh.ep",count==0?"w":"a");
count++;
for (i=region->r1;i<=region->r2;i++)
nprintf(f,"%d\n",rowthresh[i-region->r1]);
nprintf(f,"//nc\n");
fclose(f);
}
*/
    /* Minimum text row height required (pixels) */
    rhmin_pix = breakinfo->rhmean_pixels/3;
    if (rhmin_pix < .04*src_dpi)
        rhmin_pix = .04*src_dpi;
    if (rhmin_pix > .13*src_dpi)
        rhmin_pix = .13*src_dpi;
    if (rhmin_pix < 1)
        rhmin_pix = 1;
    /*
    for (rmax=region->r2;rmax>region->r1;rmax--)
        if (rowthresh[rmax-region->r1]>10)
            break;
    */
    /* Look for "row" gaps in the region so that it can be broken into */
    /* multiple "rows".                                                */
    breakinfo->n=0;
    for (labelrow=figrow=-1,dtrc=trc=brc=0,i=region->r1;i<=region->r2;i++)
        {
        /* Does row have few enough black pixels to be considered blank? */
        if (rowthresh[i-region->r1]<=10) 
            {
            trc=0;
            brc++;
            /*
            ** Max allowed white space between rows = max_vertical_gap_inches
            */
            if (dtrc==0)
                {
                if (brc > brcmin)
                    newregion->r1++;
                continue;
                }
            /*
            ** Big enough blank gap, so add one row / line
            */
            if (dtrc+brc >= rhmin_pix)
                {
                int i0,iopt;
                double region_height_inches;
                double gap_inches;

                if (dtrc<src_dpi*0.02)
                    dtrc=src_dpi*0.02;
                if (dtrc<2)
                    dtrc=2;
                /* Look for more optimum point */
                for (i0=iopt=i;i<=region->r2 && i-i0<dtrc;i++)
                    {
                    if (rowthresh[i-region->r1]<rowthresh[iopt-region->r1])
                        {
                        iopt=i;
                        if (rowthresh[i-region->r1]==0)
                            break;
                        }
                    if (rowthresh[i-region->r1]>100)
                        break;
                    }
                /* If at end of region and haven't found perfect break, stay at end */
                if (i>region->r2 && rowthresh[iopt-region->r1]>0)
                    i=region->r2;
                else
                    i=iopt;
                newregion->r2=i-1;
                region_height_inches = (double)(newregion->r2-newregion->r1+1)/src_dpi;

                /* Could this region be a figure? */
                if (figrow < 0 && region_height_inches >= min_fig_height)
                    {
                    /* If so, set figrow and don't process it yet. */
                    figrow = newregion->r1;
                    labelrow = -1;
                    newregion->r1=i;
                    dtrc=trc=0;
                    brc=1;
                    continue;
                    }
                /* Are we processing a figure? */
                if (figrow >= 0)
                    {
                    /* Compute most recent gap */
                    if (labelrow>=0)
                        gap_inches = (double)(labelrow-newregion->r1)/src_dpi;
                    else
                        gap_inches = -1.;
                    /* If gap and region height are small enough, tack them on to the figure. */
                    if (region_height_inches < max_label_height && gap_inches>0. 
                                  && gap_inches<max_fig_gap)
                        newregion->r1=figrow;
                    else
                        {
                        /* Not small enough--dump the previous figure. */
                        newregion->r2=newregion->r1-1;
                        newregion->r1=figrow;
                        newregion->c1=region->c1;
                        newregion->c2=region->c2;
                        bmpregion_trim_margins(newregion,colcount,rowcount,0x1f);
                        if (newregion->r2>newregion->r1)
                            textrow_assign_bmpregion(&breakinfo->textrow[breakinfo->n++],newregion);
                        if (gap_inches>0. && gap_inches<max_fig_gap)
                            {
                            /* This new region might be a figure--set it as the new figure */
                            /* and don't dump it yet.                                      */
                            figrow = newregion->r2+1;
                            labelrow = -1;
                            newregion->r1=i;
                            dtrc=trc=0;
                            brc=1;
                            continue;
                            }
                        else
                            {
                            newregion->r1=newregion->r2+1;
                            newregion->r2=i-1;
                            }
                        }
                    /* Cancel figure processing */
                    figrow=-1;
                    labelrow=-1;
                    }
                /*
                if (newregion->r2 >= rmax)
                    i=newregion->r2=region->r2;
                */
                newregion->c1=region->c1;
                newregion->c2=region->c2;
                bmpregion_trim_margins(newregion,colcount,rowcount,0x1f);
                if (newregion->r2>newregion->r1)
                    textrow_assign_bmpregion(&breakinfo->textrow[breakinfo->n++],newregion);
                newregion->r1=i;
                dtrc=trc=0;
                brc=1;
                }
            }
        else
            {
            if (figrow>=0 && labelrow<0)
                labelrow=i;
            dtrc++;
            trc++;
            brc=0;
            }
        }
    newregion->r2=region->r2;
    if (dtrc>0 && newregion->r2-newregion->r1+1 > 0)
        {
        /* If we were processing a figure, include it. */
        if (figrow>=0)
            newregion->r1=figrow;
        newregion->c1=region->c1;
        newregion->c2=region->c2;
        bmpregion_trim_margins(newregion,colcount,rowcount,0x1f);
        if (newregion->r2>newregion->r1)
            textrow_assign_bmpregion(&breakinfo->textrow[breakinfo->n++],newregion);
        }
    /* Compute gaps between rows and row heights */
    breakinfo_compute_row_gaps(breakinfo,region->r2);
    willus_dmem_free(15,(double **)&rowthresh,funcname);
    }


static void textrow_assign_bmpregion(TEXTROW *textrow,BMPREGION *region)

    {
    textrow->r1=region->r1;
    textrow->r2=region->r2;
    textrow->c1=region->c1;
    textrow->c2=region->c2;
    textrow->rowbase=region->rowbase;
    textrow->lcheight=region->lcheight;
    textrow->capheight=region->capheight;
    textrow->h5050=region->h5050;
    }


static void breakinfo_compute_row_gaps(BREAKINFO *breakinfo,int r2)

    {
    int i,n;

    n=breakinfo->n;
    if (n<=0)
        return;
    breakinfo->textrow[0].rowheight = breakinfo->textrow[0].r2 - breakinfo->textrow[0].r1;
    for (i=0;i<n-1;i++)
        breakinfo->textrow[i].gap = breakinfo->textrow[i+1].r1 - breakinfo->textrow[i].rowbase - 1;
        /*
        breakinfo->textrow[i].rowheight = breakinfo->textrow[i+1].r1 - breakinfo->textrow[i].r1;
        */
    for (i=1;i<n;i++)
        breakinfo->textrow[i].rowheight = breakinfo->textrow[i].rowbase 
                                            - breakinfo->textrow[i-1].rowbase;
    breakinfo->textrow[n-1].gap = r2 - breakinfo->textrow[n-1].rowbase;
    }


static void breakinfo_compute_col_gaps(BREAKINFO *breakinfo,int c2)

    {
    int i,n;

    n=breakinfo->n;
    if (n<=0)
        return;
    for (i=0;i<n-1;i++)
        {
        breakinfo->textrow[i].gap = breakinfo->textrow[i+1].c1 - breakinfo->textrow[i].c2 - 1;
        breakinfo->textrow[i].rowheight = breakinfo->textrow[i+1].c1 - breakinfo->textrow[i].c1;
        }
    breakinfo->textrow[n-1].gap = c2 - breakinfo->textrow[n-1].c2;
    breakinfo->textrow[n-1].rowheight = breakinfo->textrow[n-1].c2 - breakinfo->textrow[n-1].c1;
    }


static void breakinfo_remove_small_col_gaps(BREAKINFO *breakinfo,int lcheight,double mingap)

    {
    int i,j;

    if (mingap < word_spacing)
        mingap = word_spacing;
    for (i=0;i<breakinfo->n-1;i++)
        {
        double gap;

        gap=(double)breakinfo->textrow[i].gap / lcheight;
        if (gap >= mingap)
            continue;
        breakinfo->textrow[i].c2 = breakinfo->textrow[i+1].c2;
        breakinfo->textrow[i].gap = breakinfo->textrow[i+1].gap;
        if (breakinfo->textrow[i+1].r1 < breakinfo->textrow[i].r1)
            breakinfo->textrow[i].r1 = breakinfo->textrow[i+1].r1;
        if (breakinfo->textrow[i+1].r2 > breakinfo->textrow[i].r2)
            breakinfo->textrow[i].r2 = breakinfo->textrow[i+1].r2;
        for (j=i+1;j<breakinfo->n-1;j++)
            breakinfo->textrow[j] = breakinfo->textrow[j+1];
        breakinfo->n--;
        i--;
        }
    }


static void breakinfo_remove_small_rows(BREAKINFO *breakinfo,double fracrh,double fracgap,
                                        BMPREGION *region,int *colcount,int *rowcount)

    {
    int i,j,mg,mh,mg0,mg1;
    int c1,c2,nc;
    int *rh,*gap;
    static char *funcname="breakinfo_remove_small_rows";

#if (WILLUSDEBUGX & 2)
printf("@breakinfo_remove_small_rows(fracrh=%g,fracgap=%g)\n",fracrh,fracgap);
#endif
    if (breakinfo->n<2)
        return;
    c1=region->c1;
    c2=region->c2;
    nc=c2-c1+1;
    willus_dmem_alloc_warn(16,(void **)&rh,2*sizeof(int)*breakinfo->n,funcname,10);
    gap=&rh[breakinfo->n];
    for (i=0;i<breakinfo->n;i++)
        {
        rh[i]=breakinfo->textrow[i].r2-breakinfo->textrow[i].r1+1;
        if (i<breakinfo->n-1)
            gap[i]=breakinfo->textrow[i].gap;
        }
    sorti(rh,breakinfo->n);
    sorti(gap,breakinfo->n-1);
    mh=rh[breakinfo->n/2];
    mh *= fracrh;
    if (mh<1)
        mh=1;
    mg0=gap[(breakinfo->n-1)/2];
    mg = mg0*fracgap;
    mg1 = mg0*0.7;
    if (mg<1)
        mg=1;
#if (WILLUSDEBUGX & 2)
printf("mh = %d x %g = %d\n",rh[breakinfo->n/2],fracrh,mh);
printf("mg = %d x %g = %d\n",gap[breakinfo->n/2],fracgap,mg);
#endif
    for (i=0;i<breakinfo->n;i++)
        {
        TEXTROW *textrow;
        int trh,gs1,gs2,g1,g2,gap_is_big,row_too_small;
        double m1,m2,row_width_inches;

        textrow=&breakinfo->textrow[i];
        trh=textrow->r2-textrow->r1+1;
        if (i==0)
            {
            g1=mg0+1;
            gs1=mg+1;
            }
        else
            {
            g1=textrow->r1-breakinfo->textrow[i-1].r2-1;
            gs1=breakinfo->textrow[i-1].gap;
            }
        if (i==breakinfo->n-1)
            {
            g2=mg0+1;
            gs2=mg+1;
            }
        else
            {
            g2=breakinfo->textrow[i+1].r1-textrow->r2-1;
            gs2=breakinfo->textrow[i].gap;
            }
#if (WILLUSDEBUGX & 2)
printf("   rowheight[%d] = %d, mh=%d, gs1=%d, gs2=%d\n",i,trh,gs1,gs2);
#endif
        gap_is_big = (trh >= mh || (gs1 >= mg && gs2 >= mg));
        /*
        ** Is the row width small and centered?  If so, it should probably
        ** be attached to its nearest neighbor--it's usually a fragment of
        ** an equation or a table/figure.
        */
        row_width_inches = (double)(textrow->c2-textrow->c1+1)/src_dpi;
        m1 = fabs(textrow->c1-c1)/nc;
        m2 = fabs(textrow->c2-c2)/nc;
        row_too_small = m1 > 0.1 && m2 > 0.1 
                            && row_width_inches < little_piece_threshold_inches
                            && (g1<=mg1 || g2<=mg1);
#if (WILLUSDEBUGX & 2)
printf("       m1=%g, m2=%g, rwi=%g, g1=%d, g2=%d, mg0=%d\n",m1,m2,row_width_inches,g1,g2,mg0);
#endif
        if (gap_is_big && !row_too_small)
            continue;
#if (WILLUSDEBUGX & 2)
printf("   row[%d] to be combined w/next row.\n",i);
#endif
        if (row_too_small)
            {
            if (g1<g2)
                i--;
            }
        else
            {
            if (gs1<gs2)
                i--;
            }
/*
printf("Removing row.  nrows=%d, rh=%d, gs1=%d, gs2=%d\n",breakinfo->n,trh,gs1,gs2);
printf("    mh = %d, mg = %d\n",rh[breakinfo->n/2],gap[(breakinfo->n-1)/2]);
*/
        breakinfo->textrow[i].r2 = breakinfo->textrow[i+1].r2;
        if (breakinfo->textrow[i+1].c2 > breakinfo->textrow[i].c2)
            breakinfo->textrow[i].c2 = breakinfo->textrow[i+1].c2;
        if (breakinfo->textrow[i+1].c1 < breakinfo->textrow[i].c1)
            breakinfo->textrow[i].c1 = breakinfo->textrow[i+1].c1;
        /* Re-compute rowbase, capheight, lcheight */
        {
        BMPREGION newregion;
        newregion=(*region);
        newregion.c1=breakinfo->textrow[i].c1;
        newregion.c2=breakinfo->textrow[i].c2;
        newregion.r1=breakinfo->textrow[i].r1;
        newregion.r2=breakinfo->textrow[i].r2;
        bmpregion_trim_margins(&newregion,colcount,rowcount,0x1f);
        newregion.c1=breakinfo->textrow[i].c1;
        newregion.c2=breakinfo->textrow[i].c2;
        newregion.r1=breakinfo->textrow[i].r1;
        newregion.r2=breakinfo->textrow[i].r2;
        textrow_assign_bmpregion(&breakinfo->textrow[i],&newregion);
        }
        for (j=i+1;j<breakinfo->n-1;j++)
            breakinfo->textrow[j] = breakinfo->textrow[j+1];
        breakinfo->n--;
        i--;
        }
    willus_dmem_free(16,(double **)&rh,funcname);
    }
            

static void breakinfo_alloc(int index,BREAKINFO *breakinfo,int nrows)

    {
    static char *funcname="breakinfo_alloc";

    willus_dmem_alloc_warn(index,(void **)&breakinfo->textrow,sizeof(TEXTROW)*(nrows/2+2),
                        funcname,10);
    }


static void breakinfo_free(int index,BREAKINFO *breakinfo)

    {
    static char *funcname="breakinfo_free";

    willus_dmem_free(index,(double **)&breakinfo->textrow,funcname);
    }


static void breakinfo_sort_by_gap(BREAKINFO *breakinfo)

    {
    int n,top,n1;
    TEXTROW *x,x0;

    x=breakinfo->textrow;
    n=breakinfo->n;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child].gap<x[child+1].gap)
                child++;
            if (x0.gap<x[child].gap)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    }


static void breakinfo_sort_by_row_position(BREAKINFO *breakinfo)

    {
    int n,top,n1;
    TEXTROW *x,x0;

    x=breakinfo->textrow;
    n=breakinfo->n;
    if (n<2)
        return;
    top=n/2;
    n1=n-1;
    while (1)
        {
        if (top>0)
            {
            top--;
            x0=x[top];
            }
        else
            {
            x0=x[n1];
            x[n1]=x[0];
            n1--;
            if (!n1)
                {
                x[0]=x0;
                return;
                }
            }
        {
        int parent,child;

        parent=top;
        child=top*2+1;
        while (child<=n1)
            {
            if (child<n1 && x[child].r1<x[child+1].r1)
                child++;
            if (x0.r1<x[child].r1)
                {
                x[parent]=x[child];
                parent=child;
                child+=(parent+1);
                }
            else
                break;
            }
        x[parent]=x0;
        }
        }
    }


/*
** Add a vertically-contiguous rectangular region to the destination bitmap.
** The rectangular region may be broken up horizontally (wrapped).
*/
static void bmpregion_one_row_find_breaks(BMPREGION *region,BREAKINFO *breakinfo,
                                          int *colcount,int *rowcount,int add_to_dbase)

    {
    int nc,i,mingap,col0,dr,thlow,thhigh;
    int *bp;
    BMPREGION *newregion,_newregion;
    static char *funcname="bmpregion_one_row_find_breaks";

    if (debug)
        printf("@bmpregion_one_row_find_breaks(%d,%d)-(%d,%d)\n",
               region->c1,region->r1,region->c2,region->r2);
    newregion=&_newregion;
    (*newregion)=(*region);
    bmpregion_trim_margins(newregion,colcount,rowcount,0x1f);
    region->lcheight=newregion->lcheight;
    region->capheight=newregion->capheight;
    region->rowbase=newregion->rowbase;
    region->h5050=newregion->h5050;
    nc=newregion->c2-newregion->c1+1;
    breakinfo->n=0;
    if (nc<6)
        return;
    /*
    ** Look for "space-sized" gaps, i.e. gaps that would occur between words.
    ** Use this as pixel counting aperture.
    */
    dr=newregion->lcheight;
    mingap = dr*word_spacing*0.8;
    if (mingap < 2)
        mingap = 2;

    /*
    ** Find places where there are gaps (store in bp array)
    ** Could do this more intelligently--maybe calculate a histogram?
    */
    willus_dmem_alloc_warn(18,(void **)&bp,sizeof(int)*nc,funcname,10);
    for (i=0;i<nc;i++)
        bp[i]=0;
    if (src_left_to_right)
        {
        for (i=newregion->c1;i<=newregion->c2;i++)
            {
            int i1,i2,pt,sum,ii;
            i1=i-mingap/2;
            i2=i1+mingap-1;
            if (i1<newregion->c1)
                i1=newregion->c1;
            if (i2>newregion->c2)
                i2=newregion->c2;
            pt=(int)((i2-i1+1)*gtw_in*src_dpi+.5);
            if (pt<1)
                pt=1;
            for (sum=0,ii=i1;ii<=i2;ii++,sum+=colcount[ii]);
            bp[i-newregion->c1]=10*sum/pt;
            }
        }
    else
        {
        for (i=newregion->c2;i>=newregion->c1;i--)
            {
            int i1,i2,pt,sum,ii;
            i1=i-mingap/2;
            i2=i1+mingap-1;
            if (i1<newregion->c1)
                i1=newregion->c1;
            if (i2>newregion->c2)
                i2=newregion->c2;
            pt=(int)((i2-i1+1)*gtw_in*src_dpi+.5);
            if (pt<1)
                pt=1;
            for (sum=0,ii=i1;ii<=i2;ii++,sum+=colcount[ii]);
            bp[i-newregion->c1]=10*sum/pt;
            }
        }
#if (WILLUSDEBUGX & 4)
if (region->r1 > 3699 && region->r1<3750)
{
static int a=0;
FILE *f;
f=fopen("outbp.ep",a==0?"w":"a");
a++;
fprintf(f,"/sa l \"(%d,%d)-(%d,%d) lch=%d\" 2\n",region->c1,region->r1,region->c2,region->r2,region->lcheight);
for (i=0;i<nc;i++)
fprintf(f,"%d\n",bp[i]);
fprintf(f,"//nc\n");
fclose(f);
}
#endif
    thlow=10;
    thhigh=50;
    /*
    ** Break into pieces
    */
    for (col0=newregion->c1;col0<=newregion->c2;col0++)
        {
        int copt,c0;
        BMPREGION xregion;

        xregion=(*newregion);
        xregion.c1=col0;
        for (;col0<=newregion->c2;col0++)
            if (bp[col0-newregion->c1]>=thhigh)
                break;
        if (col0>newregion->c2)
            break;
        for (col0++;col0<=newregion->c2;col0++)
            if (bp[col0-newregion->c1]<thlow)
                break;
        for (copt=c0=col0;col0<=newregion->c2 && col0-c0<=dr;col0++)
            {
            if (bp[col0-newregion->c1] <  bp[copt-newregion->c1])
                copt=col0;
            if (bp[col0-newregion->c1] > thhigh)
                break;
            }
        if (copt>newregion->c2)
            copt=newregion->c2;
        xregion.c2=copt;
        if (xregion.c2-xregion.c1 < 2)
            continue;
        bmpregion_trim_margins(&xregion,colcount,rowcount,0x1f);
        textrow_assign_bmpregion(&breakinfo->textrow[breakinfo->n++],&xregion);
        col0=copt;
        if (copt==newregion->c2)
            break;
        }
    breakinfo_compute_col_gaps(breakinfo,newregion->c2);
    willus_dmem_free(18,(double **)&bp,funcname);

    /* Remove small gaps */
    {
    double median_gap;
    word_gaps_add(add_to_dbase ? breakinfo : NULL,region->lcheight,&median_gap);
    breakinfo_remove_small_col_gaps(breakinfo,region->lcheight,median_gap/1.9);
    }
    }


/*
** pi = preserve indentation
*/
static void bmpregion_one_row_wrap_and_add(BMPREGION *region,BREAKINFO *rowbreakinfo,
                                           int index,int i1,int i2,
                                           MASTERINFO *masterinfo,int justflags,
                                           int *colcount,int *rowcount,PAGEINFO *pageinfo,
                                           int line_spacing,int mean_row_gap,int rowbase,
                                           int marking_flags,int pi)

    {
    int nc,nr,i,i0,gappix;
    double aspect_ratio,region_height;
    BREAKINFO *colbreaks,_colbreaks;
    BMPREGION *newregion,_newregion;

#if (WILLUSDEBUGX & 4)
printf("@bmpregion_one_row_wrap_and_add, index=%d, i1=%d, i2=%d\n",index,i1,i2);
#endif
    newregion=&_newregion;
    (*newregion)=(*region);
    bmpregion_trim_margins(newregion,colcount,rowcount,0xf);
    nc=newregion->c2-newregion->c1+1;
    nr=newregion->r2-newregion->r1+1;
    if (nc<6)
        return;
    aspect_ratio = (double)nr/nc;
    region_height = (double)nr/src_dpi;
    if (aspect_ratio > no_wrap_ar_limit && region_height > no_wrap_height_limit_inches)
        {
        newregion->r1=region->r1;
        newregion->r2=region->r2;
#ifdef WILLUSDEBUG
printf("wrapflush6\n");
#endif
        wrapbmp_flush(masterinfo,0,pageinfo,1);
        if (index>i1)
            dst_add_gap_src_pixels("Tall region",masterinfo,rowbreakinfo->textrow[index-1].gap);
        bmpregion_add(newregion,rowbreakinfo,masterinfo,0,0xf,0,-1.0,0,2,colcount,rowcount,
                      pageinfo,0xf,
                      rowbreakinfo->textrow[index].r2-rowbreakinfo->textrow[index].rowbase);
        if (index<i2)
            gap_override_internal=rowbreakinfo->textrow[index].gap;
        return;
        }
    colbreaks=&_colbreaks;
    colbreaks->textrow=NULL;
    breakinfo_alloc(106,colbreaks,newregion->c2-newregion->c1+1);
    bmpregion_one_row_find_breaks(newregion,colbreaks,colcount,rowcount,1);
    if (pi && colbreaks->n>0)
        {
        if (src_left_to_right)
            colbreaks->textrow[0].c1=region->c1;
        else
            colbreaks->textrow[colbreaks->n-1].c2=region->c2;
        }
    /*
    hs=0.;
    for (i=0;i<colbreaks->n;i++)
        hs += (colbreaks->textrow[i].r2-colbreaks->textrow[i].r1);
    hs /= colbreaks->n;
    */
    /*
    ** Find appropriate letter height to use for word spacing
    */
    {
    double median_gap;
    word_gaps_add(NULL,newregion->lcheight,&median_gap);
    gappix = (int)(median_gap*newregion->lcheight+.5);
    }
#if (WILLUSDEBUGX & 4)
printf("Before small gap removal, column breaks:\n");
breakinfo_echo(colbreaks);
#endif
#if (WILLUSDEBUGX & 4)
printf("After small gap removal, column breaks:\n");
breakinfo_echo(colbreaks);
#endif
    if (show_marked_source)
        for (i=0;i<colbreaks->n;i++)
            {
            BMPREGION xregion;
            xregion=(*newregion);
            xregion.c1=colbreaks->textrow[i].c1;
            xregion.c2=colbreaks->textrow[i].c2;
            mark_source_page(&xregion,2,marking_flags);
            }
#if (WILLUSDEBUGX & 4)
for (i=0;i<colbreaks->n;i++)
printf("    colbreak[%d] = %d - %d\n",i,colbreaks->textrow[i].c1,colbreaks->textrow[i].c2);
#endif
    /* Maybe skip gaps < 0.5*median_gap or collect gap/rowheight ratios and skip small gaps */
    /* (Could be thrown off by full-justified articles where some lines have big gaps.)     */
    /* Need do call a separate function that removes these gaps. */
    for (i0=0;i0<colbreaks->n;)
        {
        int i1,i2,toolong,rw,remaining_width_pixels;
        BMPREGION reg;

        toolong=0; /* Avoid compiler warning */
        for (i=i0;i<colbreaks->n;i++)
            {
            int wordgap;

            wordgap = wrapbmp_ends_in_hyphen() ? 0 : gappix;
            i1=src_left_to_right ? i0 : colbreaks->n-1-i;
            i2=src_left_to_right ? i : colbreaks->n-1-i0;
            rw=(colbreaks->textrow[i2].c2-colbreaks->textrow[i1].c1+1);
            remaining_width_pixels = wrapbmp_remaining();
            toolong = (rw+wordgap > remaining_width_pixels);
#if (WILLUSDEBUGX & 4)
printf("    i1=%d, i2=%d, rw=%d, rw+gap=%d, remainder=%d, toolong=%d\n",i1,i2,rw,rw+wordgap,remaining_width_pixels,toolong);
#endif
            /*
            ** If we're too long with just one word and there is already
            ** stuff on the queue, then flush it and re-evaluate.
            */
            if (i==i0 && toolong && wrapbmp_width()>0)
                {
#ifdef WILLUSDEBUG
printf("wrapflush8\n");
#endif
                wrapbmp_flush(masterinfo,1,pageinfo,0);
                i--;
                continue;
                }
            /*
            ** If we're not too long and we're not done yet, add another word.
            */
            if (i < colbreaks->n-1 && !toolong)
                continue;
            /*
            ** Add the regions from i0 to i (or i0 to i-1)
            */
            break;
            }
        if (i>i0 && toolong)
            i--;
        i1=src_left_to_right ? i0 : colbreaks->n-1-i;
        i2=src_left_to_right ? i : colbreaks->n-1-i0;
        reg=(*newregion);
        reg.c1=colbreaks->textrow[i1].c1;
        reg.c2=colbreaks->textrow[i2].c2;
#if (WILLUSDEBUGX & 4)
printf("    Adding i1=%d to i2=%d\n",i1,i2);
#endif
        /* Trim the word top/bottom */
        bmpregion_trim_margins(&reg,colcount,rowcount,0xc);
        reg.c1=colbreaks->textrow[i1].c1;
        reg.c2=colbreaks->textrow[i2].c2;
        reg.lcheight=newregion->lcheight;
        reg.capheight=newregion->capheight;
        reg.rowbase=newregion->rowbase;
        reg.h5050=newregion->h5050;
        if (reg.r1 > reg.rowbase)
            reg.r1 = reg.rowbase;
        if (reg.r2 < reg.rowbase)
            reg.r2=reg.rowbase;
        /* Add it to the existing line queue */
        wrapbmp_add(&reg,gappix,line_spacing,rowbase,mean_row_gap,justflags);
        if (toolong)
{
#ifdef WILLUSDEBUG
printf("wrapflush7\n");
#endif
            wrapbmp_flush(masterinfo,1,pageinfo,0);
}
        i0=i+1;
        }
    breakinfo_free(106,colbreaks);
    }


static WILLUSBITMAP _wrapbmp,*wrapbmp;
static int wrapbmp_base;
static int wrapbmp_line_spacing;
static int wrapbmp_gap;
static int wrapbmp_bgcolor;
static int wrapbmp_just;
static int wrapbmp_rhmax;
static int wrapbmp_thmax;
static int wrapbmp_maxgap=2;
static int wrapbmp_height_extended;
static HYPHENINFO wrapbmp_hyphen;

static void wrapbmp_init(void)

    {
    wrapbmp=&_wrapbmp;
    bmp_init(wrapbmp);
    wrapbmp_set_color(dst_color);
    wrapbmp->width=0;
    wrapbmp->height=0;
    wrapbmp_base=0;
    wrapbmp_line_spacing=-1;
    wrapbmp_gap=-1;
    wrapbmp_bgcolor=-1;
    wrapbmp_height_extended=0;
    wrapbmp_just=0x8f;
    wrapbmp_rhmax=-1;
    wrapbmp_thmax=-1;
    wrapbmp_hyphen.ch=-1;
    just_flushed_internal=0;
    beginning_gap_internal=-1;
    last_h5050_internal=-1;
    }


static int wrapbmp_ends_in_hyphen(void)

    {
    return(wrapbmp_hyphen.ch>=0);
    }


static void wrapbmp_set_color(int is_color)

    {
    if (is_color)
        wrapbmp->bpp=24;
    else
        {
        int i;

        wrapbmp->bpp=8;
        for (i=0;i<256;i++)
            wrapbmp->red[i]=wrapbmp->blue[i]=wrapbmp->green[i]=i;
        }
    }


static void wrapbmp_free(void)

    {
    bmp_free(wrapbmp);
    }


static void wrapbmp_set_maxgap(int value)

    {
    wrapbmp_maxgap=value;
    }


static int wrapbmp_width(void)

    {
    return(wrapbmp->width);
    }


static int wrapbmp_remaining(void)

    {
    int maxpix,w;
    maxpix=max_region_width_inches*src_dpi;
    /* Don't include hyphen if wrapbmp ends in a hyphen */
    if (wrapbmp_hyphen.ch<0)
        w=wrapbmp->width;
    else
        if (src_left_to_right)
            w=wrapbmp_hyphen.c2+1;
        else
            w=wrapbmp->width - wrapbmp_hyphen.c2;
    return(maxpix-w);
    }


/*
** region = bitmap region to add to line
** gap = horizontal pixel gap between existing region and region being added
** line_spacing = desired spacing between lines of text (pixels)
** rbase = position of baseline in region
** gio = gap if over--gap above top of text if it goes over line_spacing.
*/
// static int bcount=0;
static void wrapbmp_add(BMPREGION *region,int gap,int line_spacing,int rbase,int gio,
                                          int just_flags)

    {
    WILLUSBITMAP *tmp,_tmp;
    int i,rh,th,bw,new_base,h2,bpp;
// static char filename[256];

#ifdef WILLUSDEBUG
printf("@wrapbmp_add %d x %d (w=%d).\n",region->c2-region->c1+1,region->r2-region->r1+1,wrapbmp->width);
#endif
    bmpregion_hyphen_detect(region); /* Figure out if what we're adding ends in a hyphen */
    if (wrapbmp_ends_in_hyphen())
        gap=0;
    wrapbmp_hyphen_erase();
    just_flushed_internal=0;  // Reset "just flushed" flag
    beginning_gap_internal = -1; // Reset top-of-page or top-of-column gap
    last_h5050_internal = -1; // Reset last row font size
    if (line_spacing > wrapbmp_line_spacing)
        wrapbmp_line_spacing = line_spacing;
    if (gio > wrapbmp_gap)
        wrapbmp_gap = gio;
    wrapbmp_bgcolor=region->bgcolor;
    wrapbmp_just=just_flags;
/*
printf("    c1=%d, c2=%d, r1=%d, r2=%d\n",region->c1,region->c2,region->r1,region->r2);
printf("    gap=%d, line_spacing=%d, rbase=%d, gio=%d\n",gap,line_spacing,rbase,gio);
*/
    bpp=dst_color?3:1;
    rh=rbase-region->r1+1;
    if (rh > wrapbmp_rhmax)
        wrapbmp_rhmax=rh;
    th = rh + (region->r2-rbase);
    if (th > wrapbmp_thmax)
        wrapbmp_thmax=th;
/*
{
WILLUSBITMAP *bmp,_bmp;

bmp=&_bmp;
bmp_init(bmp);
bmp->height=region->r2-region->r1+1;
bmp->width=region->c2-region->c1+1;
bmp->bpp=bpp*8;
if (bpp==1)
for (i=0;i<256;i++)
bmp->red[i]=bmp->blue[i]=bmp->green[i]=i;
bmp_alloc(bmp);
bw=bmp_bytewidth(bmp);
memset(bmp_rowptr_from_top(bmp,0),255,bw*bmp->height);
for (i=region->r1;i<=region->r2;i++)
{
unsigned char *d,*s;
d=bmp_rowptr_from_top(bmp,i-region->r1);
s=bmp_rowptr_from_top(dst_color?region->bmp:region->bmp8,i)+bpp*region->c1;
if (i==rbase)
memset(d,0,bw);
else
memcpy(d,s,bw);
}
sprintf(filename,"out%05d.png",bcount++);
bmp_write(bmp,filename,stdout,100);
bmp_free(bmp);
}
*/
    if (wrapbmp->width==0)
        {
        /* Put appropriate gap in */
        if (last_rowbase_internal>=0 && rh < wrapbmp_line_spacing-last_rowbase_internal)
            {
            rh = wrapbmp_line_spacing - last_rowbase_internal;
            if (rh<2)
                rh=2;
            th = rh + (region->r2-rbase);
            wrapbmp_height_extended=0;
            }
        else
            wrapbmp_height_extended=(last_rowbase_internal>=0);
        wrapbmp_base = rh-1;
        wrapbmp->height = th;
#ifdef WILLUSDEBUG
printf("@wrapbmp_add:  bmpheight set to %d (wls=%d, lrbi=%d)\n",wrapbmp->height,wrapbmp_line_spacing,last_rowbase_internal);
#endif
        wrapbmp->width=region->c2-region->c1+1;
        bmp_alloc(wrapbmp);
        bw=bmp_bytewidth(wrapbmp);
        memset(bmp_rowptr_from_top(wrapbmp,0),255,bw*wrapbmp->height);
        for (i=region->r1;i<=region->r2;i++)
            {
            unsigned char *d,*s;
            d=bmp_rowptr_from_top(wrapbmp,wrapbmp_base+(i-rbase));
            s=bmp_rowptr_from_top(dst_color?region->bmp:region->bmp8,i)+bpp*region->c1;
            memcpy(d,s,bw);
            }
#ifdef WILLUSDEBUG
if (wrapbmp->height<=wrapbmp_base)
{
printf("1. SCREEECH!\n");
printf("wrapbmp = %d x %d, base=%d\n",wrapbmp->width,wrapbmp->height,wrapbmp_base);
exit(10);
}
#endif
        /* Copy hyphen info from added region */
        wrapbmp_hyphen = region->hyphen;
        if (wrapbmp_ends_in_hyphen())
            {
            wrapbmp_hyphen.r1 += (wrapbmp_base-rbase);
            wrapbmp_hyphen.r2 += (wrapbmp_base-rbase);
            wrapbmp_hyphen.ch -= region->c1;
            wrapbmp_hyphen.c2 -= region->c1;
            }
        return;
        }
    tmp=&_tmp;
    bmp_init(tmp);
    bmp_copy(tmp,wrapbmp);
    tmp->width += gap+region->c2-region->c1+1;
    if (rh > wrapbmp_base)
        {
        wrapbmp_height_extended=1;
        new_base = rh-1;
        }
    else
        new_base = wrapbmp_base;
    if (region->r2-rbase > wrapbmp->height-1-wrapbmp_base)
        h2=region->r2-rbase;
    else
        h2=wrapbmp->height-1-wrapbmp_base;
    tmp->height = new_base + h2 + 1;
    bmp_alloc(tmp);
    bw=bmp_bytewidth(tmp);
    memset(bmp_rowptr_from_top(tmp,0),255,bw*tmp->height);
    bw=bmp_bytewidth(wrapbmp);
/*
printf("3.  wbh=%d x %d, tmp=%d x %d x %d, new_base=%d, wbbase=%d\n",wrapbmp->width,wrapbmp->height,tmp->width,tmp->height,tmp->bpp,new_base,wrapbmp_base);
*/
    for (i=0;i<wrapbmp->height;i++)
        {
        unsigned char *d,*s;
        d=bmp_rowptr_from_top(tmp,i+new_base-wrapbmp_base)
                 + (src_left_to_right ? 0 : tmp->width-1-wrapbmp->width)*bpp;
        s=bmp_rowptr_from_top(wrapbmp,i);
        memcpy(d,s,bw);
        }
    bw=bpp*(region->c2-region->c1+1);
    if (region->r1+new_base-rbase<0 || region->r2+new_base-rbase>tmp->height-1)
        {
        aprintf(ANSI_YELLOW "INTERNAL ERROR--TMP NOT DIMENSIONED PROPERLY.\n");
        aprintf("(%d-%d), tmp->height=%d\n" ANSI_NORMAL,
            region->r1+new_base-rbase,
            region->r2+new_base-rbase,tmp->height);
        exit(10);
        }
    for (i=region->r1;i<=region->r2;i++)
        {
        unsigned char *d,*s;

        d=bmp_rowptr_from_top(tmp,i+new_base-rbase)
                 + (src_left_to_right ? wrapbmp->width+gap : 0)*bpp;
        s=bmp_rowptr_from_top(dst_color?region->bmp:region->bmp8,i)+bpp*region->c1;
        memcpy(d,s,bw);
        }
    bmp_copy(wrapbmp,tmp);
    bmp_free(tmp);
    /* Copy region's hyphen info */
    wrapbmp_hyphen = region->hyphen;
    if (wrapbmp_ends_in_hyphen())
        {
        wrapbmp_hyphen.r1 += (new_base-rbase);
        wrapbmp_hyphen.r2 += (new_base-rbase);
        if (src_left_to_right)
            {
            wrapbmp_hyphen.ch += wrapbmp->width+gap-region->c1;
            wrapbmp_hyphen.c2 += wrapbmp->width+gap-region->c1;
            }
        else
            {
            wrapbmp_hyphen.ch -= region->c1;
            wrapbmp_hyphen.c2 -= region->c1;
            }
        }
    wrapbmp_base=new_base;
#ifdef WILLUSDEBUG
if (wrapbmp->height<=wrapbmp_base)
{
printf("2. SCREEECH!\n");
printf("wrapbmp = %d x %d, base=%d\n",wrapbmp->width,wrapbmp->height,wrapbmp_base);
exit(10);
}
#endif
    }


static void wrapbmp_flush(MASTERINFO *masterinfo,int allow_full_justification,
                          PAGEINFO *pageinfo,int use_bgi)

    {
    BMPREGION region;
    WILLUSBITMAP *bmp8,_bmp8;
    int gap,just,nomss,dh;
    int *colcount,*rowcount;
    static char *funcname="wrapbmp_flush";
// char filename[256];

    if (wrapbmp->width<=0)
        {
        if (use_bgi==1 && beginning_gap_internal > 0)
            dst_add_gap_src_pixels("wrapbmp_bgi0",masterinfo,beginning_gap_internal);
        beginning_gap_internal=-1;
        last_h5050_internal=-1;
        if (use_bgi)
            just_flushed_internal=1;
        return;
        }
#ifdef WILLUSDEBUG
printf("@wrapbmp_flush()\n");
#endif
/*
{
char filename[256];
int i;
static int bcount=0;
for (i=0;i<wrapbmp->height;i++)
{
unsigned char *p;
int j;
p=bmp_rowptr_from_top(wrapbmp,i);
for (j=0;j<wrapbmp->width;j++)
if (p[j]>240)
    p[j]=192;
}
sprintf(filename,"out%05d.png",bcount++);
bmp_write(wrapbmp,filename,stdout,100);
}
*/
    colcount=rowcount=NULL;
    willus_dmem_alloc_warn(19,(void **)&colcount,(wrapbmp->width+16)*sizeof(int),funcname,10);
    willus_dmem_alloc_warn(20,(void **)&rowcount,(wrapbmp->height+16)*sizeof(int),funcname,10);
    region.c1=0;
    region.c2=wrapbmp->width-1;
    region.r1=0;
    region.r2=wrapbmp->height-1;
    region.rowbase=wrapbmp_base;
    region.bmp=wrapbmp;
    region.bgcolor=wrapbmp_bgcolor;
#ifdef WILLUSDEBUG
printf("Bitmap is %d x %d (baseline=%d)\n",wrapbmp->width,wrapbmp->height,wrapbmp_base);
#endif

    /* Sanity check on row spacing -- don't let it be too large. */
    nomss = wrapbmp_rhmax*1.7; /* Nominal single-spaced height for this row */
    if (last_rowbase_internal<0)
        dh = 0;
    else
        {
        dh=(int)(wrapbmp_line_spacing-last_rowbase_internal 
                           - 1.2*fabs(vertical_line_spacing)*nomss +.5);
        if (vertical_line_spacing < 0.)
            {
            int dh1;
            if (wrapbmp_maxgap > 0)
                dh1 = region.rowbase+1-wrapbmp_rhmax-wrapbmp_maxgap;
            else
                dh1=(int)(wrapbmp_line_spacing-last_rowbase_internal- 1.2*nomss+.5);
            if (dh1 > dh)
                dh =dh1;
            }
        }
    if (dh>0)
{
#ifdef WILLUSDEBUG
aprintf(ANSI_YELLOW "dh > 0 = %d" ANSI_NORMAL "\n",dh);
printf("    wrapbmp_line_spacing=%d\n",wrapbmp_line_spacing);
printf("    nomss = %d\n",nomss);
printf("    vls = %g\n",vertical_line_spacing);
printf("    lrbi=%d\n",last_rowbase_internal);
printf("    wrapbmp_maxgap=%d\n",wrapbmp_maxgap);
printf("    wrapbmp_rhmax=%d\n",wrapbmp_rhmax);
#endif
        region.r1 = dh;
/*
if (dh>200)
{
bmp_write(wrapbmp,"out.png",stdout,100);
exit(10);
}
*/
}
    if (wrapbmp->bpp==24)
        {
        bmp8=&_bmp8;
        bmp_init(bmp8);
        bmp_convert_to_greyscale_ex(bmp8,wrapbmp);
        region.bmp8=bmp8;
        }
    else
        region.bmp8=wrapbmp;
    if (gap_override_internal > 0)
        {
        region.r1=wrapbmp_base-wrapbmp_rhmax+1;
        if (region.r1<0)
            region.r1=0;
        if (region.r1>wrapbmp_base)
            region.r1=wrapbmp_base;
        gap=gap_override_internal;
        gap_override_internal = -1;
        }
    else
        {
        if (wrapbmp_height_extended)
            gap = wrapbmp_gap;
        else
            gap = 0;
        }
#ifdef WILLUSDEBUG
printf("wf:  gap=%d\n",gap);
#endif
    if (gap>0)
        dst_add_gap_src_pixels("wrapbmp",masterinfo,gap);
    if (!allow_full_justification)
        just = (wrapbmp_just & 0xcf) | 0x20;
    else
        just = wrapbmp_just;
    bmpregion_add(&region,NULL,masterinfo,0,0,0,-1.0,just,2,
                  colcount,rowcount,pageinfo,0xf,wrapbmp->height-1-wrapbmp_base);
    if (wrapbmp->bpp==24)
        bmp_free(bmp8);
    willus_dmem_free(20,(double **)&rowcount,funcname);
    willus_dmem_free(19,(double **)&colcount,funcname);
    wrapbmp->width=0;
    wrapbmp->height=0;
    wrapbmp_line_spacing=-1;
    wrapbmp_gap=-1;
    wrapbmp_rhmax=-1;
    wrapbmp_thmax=-1;
    wrapbmp_hyphen.ch=-1;
    if (use_bgi==1 && beginning_gap_internal > 0)
        dst_add_gap_src_pixels("wrapbmp_bgi1",masterinfo,beginning_gap_internal);
    beginning_gap_internal = -1;
    last_h5050_internal = -1;
    if (use_bgi)
        just_flushed_internal=1;
    }


static void wrapbmp_hyphen_erase(void)

    {
    WILLUSBITMAP *bmp,_bmp;
    int bw,bpp,c0,c1,c2,i;

    if (wrapbmp_hyphen.ch<0)
        return;
#if (WILLUSDEBUGX & 16)
printf("@hyphen_erase, bmp=%d x %d x %d\n",wrapbmp->width,wrapbmp->height,wrapbmp->bpp);
printf("    ch=%d, c2=%d, r1=%d, r2=%d\n",wrapbmp_hyphen.ch,wrapbmp_hyphen.c2,wrapbmp_hyphen.r1,wrapbmp_hyphen.r2);
#endif
    bmp=&_bmp;
    bmp_init(bmp);
    bmp->bpp = wrapbmp->bpp;
    if (bmp->bpp==8)
        for (i=0;i<256;i++)
            bmp->red[i]=bmp->blue[i]=bmp->green[i]=i;
    bmp->height = wrapbmp->height;
    if (src_left_to_right)
        {
        bmp->width = wrapbmp_hyphen.c2+1;
        c0=0;
        c1=wrapbmp_hyphen.ch;
        c2=bmp->width-1;
        }
    else
        {
        bmp->width = wrapbmp->width - wrapbmp_hyphen.c2;
        c0=wrapbmp_hyphen.c2;
        c1=0;
        c2=wrapbmp_hyphen.ch-wrapbmp_hyphen.c2;
        }
    bmp_alloc(bmp);
    bpp=bmp->bpp==24 ? 3 : 1;
    bw=bpp*bmp->width;
    for (i=0;i<bmp->height;i++)
        memcpy(bmp_rowptr_from_top(bmp,i),bmp_rowptr_from_top(wrapbmp,i)+bpp*c0,bw);
    bw=(c2-c1+1)*bpp;
    if (bw>0)
        for (i=wrapbmp_hyphen.r1;i<=wrapbmp_hyphen.r2;i++)
            memset(bmp_rowptr_from_top(bmp,i)+bpp*c1,255,bw);
#if (WILLUSDEBUGX & 16)
{
static int count=1;
char filename[256];
sprintf(filename,"be%04d.png",count);
bmp_write(wrapbmp,filename,stdout,100);
sprintf(filename,"ae%04d.png",count);
bmp_write(bmp,filename,stdout,100);
count++;
}
#endif
    bmp_copy(wrapbmp,bmp);
    bmp_free(bmp);
    }


/*
** Publish pages from the master bitmap by finding break points that will
** fit the device page.
**
** As of 9/2012, it appears to be guaranteed that dst_width = masterinfo->bmp.width
**
*/
static void publish_master(MASTERINFO *masterinfo,PAGEINFO *pageinfo,int flushall)

    {
    /* bmp = full-sized bitmap w/padding and margins */
    WILLUSBITMAP *bmp,_bmp;
    /* bmp1 = viewable contents bitmap (smaller than bmp) */
    WILLUSBITMAP *bmp1,_bmp1;
    int rr,maxsize,r,r0,bytespp,size_reduction;
    int pl,pr,pt,pb;
    /* Local DPI, width, height */
    int ldpi;
    int lwidth,lheight,ltotheight;
#ifdef HAVE_OCR
    OCRWORDS *ocrwords_pub,_ocrwords_pub;

    ocrwords_pub=&_ocrwords_pub;
#endif
    if (debug)
        printf("@publish_master(page %d)\n",masterinfo->published_pages);
    if (masterinfo->bmp.width != dst_width)
        {
        aprintf("\n\n\a" TTEXT_WARN "!! Internal error, masterinfo->bmp.width=%d != dst_width=%d.\n"
               "Contact author." TTEXT_NORMAL "\n\n",masterinfo->bmp.width,dst_width);
        sys_enter_to_exit(NULL);
        exit(10);
        }
    bmp=&_bmp;
    bmp_init(bmp);
    bmp1=&_bmp1;
    bmp_init(bmp1);
    /* dh = viewable height in pixels */
    maxsize=dst_height-(int)(dst_dpi*(dst_marbot+dst_martop)+.5);
    if (maxsize>dst_height)
        maxsize=dst_height;
    r0=(int)(dst_dpi*dst_martop+.5);
    if (r0+maxsize > dst_height)
        r0=dst_height-maxsize;
    rr= flushall ? 0 : maxsize;
    if (verbose)
        printf("rows=%d, maxsize=%d, rr=%d\n",masterinfo->rows,maxsize,rr);
    if (dst_landscape)
        {
        pl=pad_bottom;
        pr=pad_top;
        pt=pad_left;
        pb=pad_right;
        }
    else
        {
        pb=pad_bottom;
        pt=pad_top;
        pl=pad_left;
        pr=pad_right;
        }
#ifdef HAVE_OCR
    if (dst_ocr)
        ocrwords_init(ocrwords_pub);
#endif
    /* While enough pixel rows in dest bitmap, break into pages */
    while (masterinfo->rows>rr)
        {
        int i,bp,bw;
        /* Get a suitable breaking point for the next page */
        bp=break_point(masterinfo,maxsize);
        if (verbose)
            printf("bp: maxsize=%d, bp=%d, r0=%d\n",maxsize,bp,r0);
        bmp1->bpp=masterinfo->bmp.bpp;
        for (i=0;i<256;i++)
            bmp1->red[i]=bmp1->green[i]=bmp1->blue[i]=i;
        // h=bp*dst_width/masterinfo->bmp.width;
        /* If too tall, shrink to fit */
        if (bp>maxsize)
            {
            lheight=bp;
            ltotheight=(int)((double)dst_height*lheight/maxsize+.5);
            lwidth=(int)((double)masterinfo->bmp.width*lheight/maxsize+.5);
            ldpi=(int)((double)dst_dpi*lheight/maxsize+.5);
            }
        else
            {
            lheight=maxsize; /* useable height */
            ltotheight=dst_height;
            lwidth=masterinfo->bmp.width;
            ldpi=dst_dpi;
            }
        r0=(int)(ldpi*dst_martop+.5);
        bmp1->width=lwidth;
        bmp1->height=lheight;
        bmp_alloc(bmp1);
        bmp_fill(bmp1,255,255,255);

        /* Create list of OCR'd words on this page and move */
        /* up positions of remaining OCR'd words.           */
#ifdef HAVE_OCR
        if (dst_ocr)
            {
            for (i=0;i<dst_ocrwords->n;i++)
                if (dst_ocrwords->word[i].r - dst_ocrwords->word[i].maxheight 
                                            + dst_ocrwords->word[i].h/2 < bp)
                    {
                    ocrwords_add_word(ocrwords_pub,&dst_ocrwords->word[i]);
                    ocrwords_remove_words(dst_ocrwords,i,i);
                    i--;
                    }
            ocrwords_offset(dst_ocrwords,0,-bp);
            }
#endif
            
        /* Center masterinfo->bmp into bmp1 (horizontally) */
        {
        int bpp,w1,bw,bw1;
        bpp=bmp1->bpp==24?3:1;
        w1=(bmp1->width-masterinfo->bmp.width)/2;
        bw=bmp_bytewidth(&masterinfo->bmp);
        bw1=w1*bpp;
        for (i=0;i<bp;i++)
            memcpy(bmp_rowptr_from_top(bmp1,i)+bw1,bmp_rowptr_from_top(&masterinfo->bmp,i),bw);
#ifdef HAVE_OCR
        if (dst_ocr)
            ocrwords_offset(dst_ocrwords,w1,0);
#endif
        }

        /* Gamma correct */
        if (fabs(dst_gamma-1.0)>.001)
            bmp_gamma_correct(bmp1,bmp1,dst_gamma);

        /* Sharpen */
        if (dst_sharpen)
            {
            WILLUSBITMAP tmp;
            bmp_init(&tmp);
            bmp_copy(&tmp,bmp1);
            bmp_sharpen(bmp1,&tmp);
            bmp_free(&tmp);
            }

        /* Put into full-sized destination bitmap with appropriate padding and margins */
        bmp->bpp=masterinfo->bmp.bpp;
        for (i=0;i<256;i++)
            bmp->red[i]=bmp->green[i]=bmp->blue[i]=i;
        bmp->width=bmp1->width+pl+pr;
        bmp->height=ltotheight+pt+pb;
        bmp_alloc(bmp);
        bmp_fill(bmp,255,255,255);
        bw=bmp_bytewidth(bmp1);
        bytespp=bmp->bpp==8?1:3;
        for (r=0;r<bmp1->height && r+r0+pt<bmp->height;r++)
            {
            unsigned char *psrc,*pdst;
            psrc=bmp_rowptr_from_top(bmp1,r);
            pdst=bmp_rowptr_from_top(bmp,r+r0+pt)+pl*bytespp;
            memcpy(pdst,psrc,bw);
            }
#ifdef HAVE_OCR
        if (dst_ocr)
            ocrwords_offset(ocrwords_pub,pl,r0+pt);
#endif
        /* Can't save grayscale as JPEG yet. */
        if (bmp->bpp==8 && jpeg_quality>=0)
            bmp_promote_to_24(bmp);
        masterinfo->published_pages++;
        if (mark_corners)
            {
            unsigned char *p;

            if (pt<bmp->height)
                {
                p=bmp_rowptr_from_top(bmp,pt);
                if (pl<bmp->width)
                    p[pl]=0;
                if (pr<bmp->width)
                    p[bmp->width-1-pr]=0;
                }
            if (pb<bmp->height)
                {
                p=bmp_rowptr_from_top(bmp,bmp->height-1-pb);
                if (pl<bmp->width)
                    p[pl]=0;
                if (pr<bmp->width)
                    p[bmp->width-1-pr]=0;
                }
            }
        if (dst_landscape)
            {
#ifdef HAVE_OCR
            /* Rotate OCR'd words list */
            if (dst_ocr)
                {
                for (i=0;i<ocrwords_pub->n;i++)
                    {
                    int cnew,rnew;
                    ocrwords_pub->word[i].rot=90;
                    cnew = ocrwords_pub->word[i].r;
                    rnew = bmp->width-1 - ocrwords_pub->word[i].c;
                    ocrwords_pub->word[i].c = cnew;
                    ocrwords_pub->word[i].r = rnew;
                    }
                }
#endif
            bmp_rotate_right_angle(bmp,90);
            }
        if (debug)
            {
            char basename[32];
            char opbmpfile[512];
            static int filecount=0;

            sprintf(basename,"outpage%05d.%s",filecount+1,jpeg_quality>0?"jpg":"png");
            wfile_fullname(opbmpfile,masterinfo->debugfolder,basename);
            bmp_write(bmp,opbmpfile,stdout,jpeg_quality<0?100:jpeg_quality);
#ifdef HAVE_OCR
            if (dst_ocr)
                {
                FILE *f;
                sprintf(basename,"wordlist%05d.txt",filecount+1);
                f=fopen(basename,"w");
                if (f!=NULL)
                    {
                    for (i=0;i<ocrwords_pub->n;i++)
                        fprintf(f,"%s\n",ocrwords_pub->word[i].text);
                    fclose(f);
                    }
                }
#endif
            filecount++;
            }
        if (dst_bpc==8 || jpeg_quality>=0)
            size_reduction=0;
        else if (dst_bpc==4)
            size_reduction=1;
        else if (dst_bpc==2)
            size_reduction=2;
        else
            size_reduction=3;
        if (dst_dither && dst_bpc<8 && jpeg_quality<0)
            bmp_dither_to_bpc(bmp,dst_bpc);
/*
{
static int count=0;
count++;
if (count==1)
{
*/
#ifdef HAVE_OCR
        if (dst_ocr)
            {
            int wordcolor;

            if ((dst_ocr_wordcolor&1)==0)
                wordcolor=3; /* Invisible */
            else
                wordcolor=4;
            if (dst_ocr_wordcolor&2)
                wordcolor|=0x40;
            if (dst_ocr_wordcolor&4)
                wordcolor|=0x80;
            pdffile_add_bitmap_with_ocrwords(gpdf,bmp,ldpi,jpeg_quality,size_reduction,
                                             ocrwords_pub,wordcolor);
/*
{
static int count=1;
char filename[256];
sprintf(filename,"page%04d.png",count++);
bmp_write(bmp,filename,stdout,100);
}
*/
            masterinfo->wordcount += ocrwords_pub->n;
            ocrwords_free(ocrwords_pub);
            }
        else
#endif
            pdffile_add_bitmap(gpdf,bmp,ldpi,jpeg_quality,size_reduction);
/*
}
else
{
if (dst_ocr)
    ocrwords_free(ocrwords_pub);
}
}
*/
        bw=bmp_bytewidth(&masterinfo->bmp);
        for (i=bp;i<masterinfo->rows;i++)
            {
            unsigned char *psrc,*pdst;
            pdst=bmp_rowptr_from_top(&masterinfo->bmp,i-bp);
            psrc=bmp_rowptr_from_top(&masterinfo->bmp,i);
            memcpy(pdst,psrc,bw);
            }
        masterinfo->rows -= bp;
        }
    bmp_free(bmp);
    bmp_free(bmp1);
    }


/*
** Find gaps in the master bitmap so that it can be broken into regions
** which go onto separate pages.
*/
static int break_point(MASTERINFO *masterinfo,int maxsize)

    {
    static char *funcname="break_point";
    int *rowcount;
    int fig,fc,figend,cw,bp1,bp2,i,j,goodsize,figure,bp,scanheight,nwc;
    int bp1f,bp2f;
    int bp1e,bp2e;

    /* masterinfo->fit_to_page==-2 means user specified -f2p -2 which means */
    /* flush entire contents of master to single page every time.   */
    if (masterinfo->rows<maxsize || masterinfo->fit_to_page==-2)
        return(masterinfo->rows);

    /* scanheight tells how far down the master bitmap to scan */
    if (masterinfo->fit_to_page==-1)
        scanheight=masterinfo->rows;
    else if (masterinfo->fit_to_page>0)
        scanheight=(int)(((1.+masterinfo->fit_to_page/100.)*maxsize)+.5);
    else
        scanheight=maxsize;
    if (scanheight > masterinfo->rows)
        scanheight=masterinfo->rows;
    goodsize=masterinfo->bmp.width/100;
    figure=masterinfo->bmp.width/10;
    willus_dmem_alloc_warn(29,(void **)&rowcount,masterinfo->rows*sizeof(int),funcname,10);
    for (j=0;j<masterinfo->rows;j++)
        {
        unsigned char *p;
        p=bmp_rowptr_from_top(&masterinfo->bmp,j);
        rowcount[j]=0;
        if (masterinfo->bmp.bpp==24)
            {
            for (i=0;i<masterinfo->bmp.width;i++,p+=3)
                if (GRAYLEVEL(p[0],p[1],p[2])<masterinfo->bgcolor)
                    rowcount[j]++;
            }
        else
            {
            for (i=0;i<masterinfo->bmp.width;i++,p++)
                if (p[0]<masterinfo->bgcolor)
                    rowcount[j]++;
            }
        }
    /* cw = consecutive all-white rows */
    /* fc = consecutive non-all-white rows */
    bp1f=bp2f=0; /* bp1f,bp2f = max break points that fit within maxsize */
    bp1e=bp2e=0; /* bp1e,bp2e = min break points that exceed maxsize */
    for (figend=fc=fig=cw=i=bp=bp1=bp2=nwc=0;i<scanheight;i++)
        {
        if (rowcount[i]==0)
            {
// if (cw==0)
// printf("%d black\n",fc);
            cw++;
            if (fc>figure)
                {
                fig=i-fc;
                figend=i;
                }
            fc=0;
            if (fig && i-figend > fc/2)
                fig=0;
            if (fig)
                continue;
            if (nwc==0)
                continue;
            bp1=i-cw/2;
            if (bp1<=maxsize)
                bp1f=bp1;
            if (bp1>maxsize && bp1e==0)
                bp1e=bp1;
            if (cw>=goodsize)
                {
                bp2=i-cw/2;
                if (bp2<=maxsize)
                    bp2f=bp2;
                if (bp2>maxsize && bp2e==0)
                    bp2e=bp2;
                }
            }
        else
            {
// if (fc==0)
// printf("%d white\n",cw);
            cw=0;
            nwc++;
            fc++;
            }
        }
/*
{
static int count=0;
FILE *out;
count++;
printf("rows=%d, gs=%d, scanheight=%d, bp1=%d, bp2=%d\n",masterinfo->rows,goodsize,scanheight,bp1,bp2);
printf("     bp1f=%d, bp2f=%d, bp1e=%d, bp2e=%d\n",bp1f,bp2f,bp1e,bp2e);
bmp_write(&masterinfo->bmp,"master.png",stdout,100);
out=fopen("rc.dat","w");
for (i=0;i<scanheight;i++)
fprintf(out,"%d\n",rowcount[i]);
fclose(out);
if (count==2)
exit(10);
}
*/
    willus_dmem_free(29,(double **)&rowcount,funcname);
    if (masterinfo->fit_to_page==0)
        {
        if (bp2 > maxsize*.8)
            return(bp2);
        if (bp1 < maxsize*.25)
            bp1=scanheight;
        return(bp1);
        }
    if (bp1f==0 && bp1e==0)
        return(scanheight);
    if (bp2f > 0)
        return((bp1f>0 && bp2f < maxsize*.8) ? bp1f : bp2f);
    if (bp1f > 0)
        return(bp1f);
    if (masterinfo->fit_to_page<0)
        return(bp1e);
    if (bp2e > 0)
        return(bp2e);
    return(bp1e);
    }


/*
** src is only allocated if dst_color != 0
*/
static void white_margins(WILLUSBITMAP *src,WILLUSBITMAP *srcgrey)

    {
    int i,n;
    BMPREGION *region,_region;

    region=&_region;
    region->bmp = srcgrey;
    get_white_margins(region);
    n=region->c1;
    for (i=0;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (dst_color)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,n*3);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,n);
        }
    n=srcgrey->width-1-region->c2;
    for (i=0;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (dst_color)
            {
            p=bmp_rowptr_from_top(src,i)+3*(src->width-n);
            memset(p,255,n*3);
            }
        p=bmp_rowptr_from_top(srcgrey,i)+srcgrey->width-n;
        memset(p,255,n);
        }
    n=region->r1;
    for (i=0;i<n;i++)
        {
        unsigned char *p;
        if (dst_color)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,src->width*3);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,srcgrey->width);
        }
    n=srcgrey->height-1-region->r2;
    for (i=srcgrey->height-n;i<srcgrey->height;i++)
        {
        unsigned char *p;
        if (dst_color)
            {
            p=bmp_rowptr_from_top(src,i);
            memset(p,255,src->width*3);
            }
        p=bmp_rowptr_from_top(srcgrey,i);
        memset(p,255,srcgrey->width);
        }
    }


static void get_white_margins(BMPREGION *region)

    {
    int n;
    double defval;

    defval=0.25;
    if (mar_left < 0.)
        mar_left=defval;
    n=(int)(0.5+mar_left*src_dpi);
    if (n>region->bmp->width)
        n=region->bmp->width;
    region->c1=n;
    if (mar_right < 0.)
        mar_right=defval;
    n=(int)(0.5+mar_right*src_dpi);
    if (n>region->bmp->width)
        n=region->bmp->width;
    region->c2=region->bmp->width-1-n;
    if (mar_top < 0.)
        mar_top=defval;
    n=(int)(0.5+mar_top*src_dpi);
    if (n>region->bmp->height)
        n=region->bmp->height;
    region->r1=n;
    if (mar_bot < 0.)
        mar_bot=defval;
    n=(int)(0.5+mar_bot*src_dpi);
    if (n>region->bmp->height)
        n=region->bmp->height;
    region->r2=region->bmp->height-1-n;
    }


static int pagelist_page_by_index(char *pagelist,int index,int maxpages)

    {
    int n1,n2,i,j,s;

// printf("@pagelist_page_by_index('%s',%d,%d)\n",pagelist,index,maxpages);
    if (pagelist[0]=='\0')
        return(index+1);
    i=0;
    while (pagelist_next_pages(pagelist,maxpages,&i,&n1,&n2))
        {
        if (n1<=0 && n2<=0)
            continue;
        s = (n2>=n1) ? 1 : -1;
        n2 += s;
        for (j=n1;j!=n2;j+=s)
            {
            if (j<1)
                continue;
            if (maxpages>0 && j>maxpages)
                continue;
            if (index==0)
                return(j);
            index--;
            }
        }
    return(-1);
    }


static int pagelist_count(char *pagelist,int maxpages)

    {
    int n1,n2,i,count;

// printf("@pagelist_count('%s',%d)\n",pagelist,maxpages);
    if (pagelist[0]=='\0')
        return(maxpages);
    count=0;
    i=0;
    while (pagelist_next_pages(pagelist,maxpages,&i,&n1,&n2))
        {
        if (n1<=0 && n2<=0)
            continue;
        if (n1>n2)
            {
            int t;
            t=n1;
            n1=n2;
            n2=t;
            }
        if ((maxpages>0 && n1>maxpages) || n2<1)
            continue;
        if (n1<1)
            n1=1;
        if (maxpages>0 && n2>maxpages)
            n2=maxpages;
        count += n2-n1+1;
        }
    return(count);
    }


static int pagelist_next_pages(char *pagelist,int maxpages,int *index,
                               int *n1,int *n2)

    {
    int i,j;
    char buf[128];

    i=(*index);
    for (j=0;j<126 && pagelist[i]>='0' && pagelist[i]<='9';i++)
        buf[j++]=pagelist[i];
    buf[j]='\0';
    if (buf[0]=='\0')
        {
        if (pagelist[i]=='-')
            (*n1)=1;
        else
            {
            (*n1)=-1;
            (*n2)=-1;
            (*index)=i;
            if (pagelist[i]=='\0')
                return(0);
            (*index)=(*index)+1;
            return(1);
            }
        }
    else
        (*n1)=atoi(buf);
    if (pagelist[i]!='-')
        (*n2)=(*n1);
    else
        {
        for (i++,j=0;j<126 && pagelist[i]>='0' && pagelist[i]<='9';i++)
            buf[j++]=pagelist[i];
        buf[j]='\0';
        if (buf[0]=='\0')
            (*n2)=maxpages;
        else
            (*n2)=atoi(buf);
        }
    if (pagelist[i]!='\0')
        i++;
    (*index)=i;
    return(1);
    }


/*
** bitmap_orientation()
**
** 1.0 means neutral
**
** >> 1.0 means document is likely portrait (no rotation necessary)
**    (max is 100.)
**
** << 1.0 means document is likely landscape (need to rotate it)
**    (min is 0.01)
**
*/
static double bitmap_orientation(WILLUSBITMAP *bmp)

    {
    int i,ic,wtcalc;
    double hsum,vsum,rat;

    wtcalc=-1;
    for (vsum=0.,hsum=0.,ic=0,i=20;i<=85;i+=5,ic++)
        {
        double nv,nh;
        int wth,wtv;

#ifdef DEBUG
printf("h %d:\n",i);
#endif
        if (ic==0)
            wth=-1;
        else
            wth=wtcalc;
wth=-1;
        nh=bmp_inflections_horizontal(bmp,8,i,&wth);
#ifdef DEBUG
{
FILE *f;
f=fopen("inf.ep","a");
fprintf(f,"/ag\n");
fclose(f);
}
printf("v %d:\n",i);
#endif
        if (ic==0)
            wtv=-1;
        else
            wtv=wtcalc;
wtv=-1;
        nv=bmp_inflections_vertical(bmp,8,i,&wtv);
        if (ic==0)
            {
            if (wtv > wth)
                wtcalc=wtv;
            else
                wtcalc=wth;
            continue;
            }
// exit(10);
        hsum += nh*i*i*i;
        vsum += nv*i*i*i;
        }
    if (vsum==0. && hsum==0.)
        rat=1.0;
    else if (hsum<vsum && hsum/vsum<.01)
        rat=100.;
    else
        rat=vsum/hsum;
    if (rat < .01)
        rat = .01;
    // printf("    page %2d:  %8.4f\n",pagenum,rat);
    // fprintf(out,"\t%8.4f",vsum/hsum);
    // fprintf(out,"\n");
    return(rat);
    }


static double bmp_inflections_vertical(WILLUSBITMAP *srcgrey,int ndivisions,int delta,int *wthresh)

    {
    int y0,y1,ny,i,nw,nisum,ni,wt,wtmax;
    double *g;
    char *funcname="bmp_inflections_vertical";

    nw=srcgrey->width/ndivisions;
    y0=srcgrey->height/6;
    y1=srcgrey->height-y0;
    ny=y1-y0;
    willus_dmem_alloc_warn(21,(void **)&g,ny*sizeof(double),funcname,10);
    wtmax=-1;
    for (nisum=0,i=0;i<10;i++)
        {
        int x0,x1,nx,j;

        x0=(srcgrey->width-nw)*(i+2)/13;
        x1=x0+nw;
        if (x1>srcgrey->width)
            x1=srcgrey->width;
        nx=x1-x0; 
        for (j=y0;j<y1;j++)
            {
            int k,rsum;
            unsigned char *p;

            p=bmp_rowptr_from_top(srcgrey,j)+x0;
            for (rsum=k=0;k<nx;k++,p++)
                rsum+=p[0];
            g[j-y0]=(double)rsum/nx;
            }
        wt=(*wthresh);
        ni=inflection_count(g,ny,delta,&wt);
        if ((*wthresh)<0 && ni>=3 && wt>wtmax)
            wtmax=wt;
        if (ni>nisum)
            nisum=ni;
        }
    willus_dmem_free(21,&g,funcname);
    if ((*wthresh)<0)
        (*wthresh)=wtmax;
    return(nisum);
    }


static double bmp_inflections_horizontal(WILLUSBITMAP *srcgrey,int ndivisions,int delta,int *wthresh)

    {
    int x0,x1,nx,bw,i,nh,nisum,ni,wt,wtmax;
    double *g;
    char *funcname="bmp_inflections_vertical";

    nh=srcgrey->height/ndivisions;
    x0=srcgrey->width/6;
    x1=srcgrey->width-x0;
    nx=x1-x0;
    bw=bmp_bytewidth(srcgrey);
    willus_dmem_alloc_warn(22,(void **)&g,nx*sizeof(double),funcname,10);
    wtmax=-1;
    for (nisum=0,i=0;i<10;i++)
        {
        int y0,y1,ny,j;

        y0=(srcgrey->height-nh)*(i+2)/13;
        y1=y0+nh;
        if (y1>srcgrey->height)
            y1=srcgrey->height;
        ny=y1-y0; 
        for (j=x0;j<x1;j++)
            {
            int k,rsum;
            unsigned char *p;

            p=bmp_rowptr_from_top(srcgrey,y0)+j;
            for (rsum=k=0;k<ny;k++,p+=bw)
                rsum+=p[0];
            g[j-x0]=(double)rsum/ny;
            }
        wt=(*wthresh);
        ni=inflection_count(g,nx,delta,&wt);
        if ((*wthresh)<0 && ni>=3 && wt>wtmax)
            wtmax=wt;
        if (ni>nisum)
            nisum=ni;
        }
    willus_dmem_free(22,&g,funcname);
    if ((*wthresh)<0)
        (*wthresh)=wtmax;
    return(nisum);
    }


static int inflection_count(double *x,int n,int delta,int *wthresh)

    {
    int i,i0,ni,ww,c,ct,wt,mode;
    double meandi,meandisq,f1,f2,stdev;
    double *xs;
    static int hist[256];
    static char *funcname="inflection_count";

    /* Find threshold white value that peaks must exceed */
    if ((*wthresh)<0)
        {
        for (i=0;i<256;i++)
            hist[i]=0;
        for (i=0;i<n;i++)
            {
            i0=floor(x[i]);
            if (i0>255)
                i0=255;
            hist[i0]++;
            }
        ct=n*.15;
        for (c=0,i=255;i>=0;i--)
            {
            c+=hist[i];
            if (c>ct)
                break;
            }
        wt=i-10;
        if (wt<192)
            wt=192;
#ifdef DEBUG
printf("wt=%d\n",wt);
#endif
        (*wthresh)=wt;
        }
    else
        wt=(*wthresh);
    ww=n/150;
    if (ww<1)
        ww=1;
    willus_dmem_alloc_warn(23,(void **)&xs,sizeof(double)*n,funcname,10);
    for (i=0;i<n-ww;i++)
        {
        int j;
        for (xs[i]=0.,j=0;j<ww;j++,xs[i]+=x[i+j]);
        xs[i] /= ww;
        }
    meandi=meandisq=0.;
    if (xs[0]<=wt-delta)
        mode=1;
    else if (xs[0]>=wt)
        mode=-1;
    else
        mode=0;
    for (i0=0,ni=0,i=1;i<n-ww;i++)
        {
        if (mode==1 && xs[i]>=wt)
            {
            if (i0>0)
                {
                meandi+=i-i0;
                meandisq+=(i-i0)*(i-i0);
                ni++;
                }
            i0=i;
            mode=-1;
            continue;
            }
        if (xs[i]<=wt-delta)
            mode=1;
        }
    stdev = 1.0; /* Avoid compiler warning */
    if (ni>0)
        {
        meandi /= ni;
        meandisq /= ni;
        stdev = sqrt(fabs(meandi*meandi-meandisq));
        }
    f1=meandi/n;
    if (f1>.15)
        f1=.15;
    if (ni>2)
        {
        if (stdev/meandi < .05)
            f2=20.;
        else
            f2=meandi/stdev;
        }
    else
        f2=1.;
#ifdef DEBUG
printf("    ni=%3d, f1=%8.4f, f2=%8.4f, f1*f2*ni=%8.4f\n",ni,f1,f2,f1*f2*ni);
{
static int count=0;
FILE *f;
int i;
f=fopen("inf.ep",count==0?"w":"a");
count++;
fprintf(f,"/sa l \"%d\" 1\n",ni);
for (i=0;i<n-ww;i++)
fprintf(f,"%g\n",xs[i]);
fprintf(f,"//nc\n");
fclose(f);
}
#endif /* DEBUG */
    willus_dmem_free(23,&xs,funcname);
    return(f1*f2*ni);
    }


static void pdfboxes_init(PDFBOXES *boxes)

    {
    boxes->n=boxes->na=0;
    boxes->box=NULL;
    }


static void pdfboxes_free(PDFBOXES *boxes)

    {
    static char *funcname="pdfboxes_free";
    willus_dmem_free(24,(double **)&boxes->box,funcname);
    }

#ifdef COMMENT
static void pdfboxes_add_box(PDFBOXES *boxes,PDFBOX *box)

    {
    static char *funcname="pdfboxes_add_box";

    if (boxes->n>=boxes->na)
        {
        int newsize;

        newsize = boxes->na < 1024 ? 2048 : boxes->na*2;
        /* Just calls willus_mem_alloc if oldsize==0 */
        willus_mem_realloc_robust_warn((void **)&boxes->box,newsize*sizeof(PDFBOX),
                                   boxes->na*sizeof(PDFBOX),funcname,10);
        boxes->na=newsize;
        }
    boxes->box[boxes->n++]=(*box);
    }


static void pdfboxes_delete(PDFBOXES *boxes,int n)

    {
    if (n>0 && n<boxes->n)
        {
        int i;
        for (i=0;i<boxes->n-n;i++)
            boxes->box[i]=boxes->box[i+n];
        }
    boxes->n -= n;
    if (boxes->n < 0)
        boxes->n = 0;
    }
#endif


/*
** Track gaps between words so that we can tell when one is out of family.
** lcheight = height of a lowercase letter.
*/
static void word_gaps_add(BREAKINFO *breakinfo,int lcheight,double *median_gap)

    {
    static int nn=0;
    static double gap[1024];
    static char *funcname="word_gaps_add";

    if (breakinfo!=NULL && breakinfo->n>1)
        {
        int i;

        for (i=0;i<breakinfo->n-1;i++)
            {
            double g;
            g = (double)breakinfo->textrow[i].gap / lcheight;
            if (g>=word_spacing)
                {
                gap[nn&0x3ff]= g;
                nn++;
                }
            }
        }
    if (median_gap!=NULL)
        {
        if (nn>0)
            {
            int n;
            static double *gap_sorted;

            n = (nn>1024) ? 1024 : nn;
            willus_dmem_alloc_warn(28,(void **)&gap_sorted,sizeof(double)*n,funcname,10);
            memcpy(gap_sorted,gap,n*sizeof(double));
            sortd(gap_sorted,n);
            (*median_gap)=gap_sorted[n/2];
            willus_dmem_free(28,&gap_sorted,funcname);
            }
        else
            (*median_gap)=0.7;
        }
    }

/*
** bmp must be grayscale! (cbmp = color, can be null)
*/
static void bmp_detect_vertical_lines(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,
                                      double dpi,double minwidth_in,
                                      double maxwidth_in,double minheight_in,double anglemax_deg,
                                      int white_thresh)

    {
    int tc,iangle,irow,icol;
    int rowstep,na,angle_sign,ccthresh;
    int pixmin,halfwidth,bytewidth;
    int bs1,nrsteps,dp;
    double anglestep;
    WILLUSBITMAP *tmp,_tmp;
    unsigned char *p0;

    if (debug)
        printf("At bmp_detect_vertical_lines...\n");
    if (!bmp_is_grayscale(bmp))
        {
        printf("Internal error.  bmp_detect_vertical_lines passed a non-grayscale bitmap.\n");
        exit(10);
        }
    tmp=&_tmp;
    bmp_init(tmp);
    bmp_copy(tmp,bmp);
    dp = bmp_rowptr_from_top(tmp,0) - bmp_rowptr_from_top(bmp,0);
    bytewidth=bmp_bytewidth(bmp);
    pixmin = (int)(minwidth_in*dpi+.5);
    if (pixmin<1)
        pixmin=1;
    halfwidth=pixmin/4;
    if (halfwidth<1)
        halfwidth=1; 
    anglestep=atan2((double)halfwidth/dpi,minheight_in);
    na=(int)((anglemax_deg*PI/180.)/anglestep+.5);
    if (na<1)
        na=1;
    rowstep=(int)(dpi/40.+.5);
    if (rowstep<2)
        rowstep=2;
    nrsteps=bmp->height/rowstep;
    bs1=bytewidth*rowstep;
    ccthresh=(int)(minheight_in*dpi/rowstep+.5);
    if (ccthresh<2)
        ccthresh=2;
    if (debug && verbose)
        printf("    na = %d, rowstep = %d, ccthresh = %d, white_thresh = %d, nrsteps=%d\n",na,rowstep,ccthresh,white_thresh,nrsteps);
/*
bmp_write(bmp,"out.png",stdout,97);
wfile_written_info("out.png",stdout);
*/
    p0=bmp_rowptr_from_top(bmp,0);
    for (tc=0;tc<100;tc++)
        {
        int ccmax,ic0max,ir0max;
        double tanthmax;

        ccmax=-1;
        ic0max=ir0max=0;
        tanthmax=0.;
        for (iangle=0;iangle<=na;iangle++)
            {
            for (angle_sign=1;angle_sign>=-1;angle_sign-=2)
                {
                double th,tanth,tanthx;
                int ic1,ic2;

                if (iangle==0 && angle_sign==-1)
                    continue;
                th=(PI/180.)*iangle*angle_sign*fabs(anglemax_deg)/na;
                tanth=tan(th);
                tanthx=tanth*rowstep;
                if (angle_sign==1)
                    {
                    ic1=-(int)(bmp->height*tanth+1.);
                    ic2=bmp->width-1;
                    }
                else
                    {
                    ic1=(int)(-bmp->height*tanth+1.);
                    ic2=bmp->width-1+(int)(-bmp->height*tanth+1.);
                    }
// printf("iangle=%2d, angle_sign=%2d, ic1=%4d, ic2=%4d\n",iangle,angle_sign,ic1,ic2);
                for (icol=ic1;icol<=ic2;icol++)
                    {
                    unsigned char *p;
                    int cc,ic0,ir0;
                    p=p0;
                    if (icol<0 || icol>bmp->width-1)
                        for (irow=0;irow<nrsteps;irow++,p+=bs1)
                            {
                            int ic;
                            ic=icol+irow*tanthx;
                            if (ic>=0 && ic<bmp->width)
                                break;
                            }
                    else
                        irow=0;
                    for (ir0=ic0=cc=0;irow<nrsteps;irow++,p+=bs1)
                        {
                        int ic;
                        ic=icol+irow*tanthx;
                        if (ic<0 || ic>=bmp->width)
                            break;
                        if ((p[ic]<white_thresh || p[ic+bytewidth]<white_thresh)
                            && (p[ic+dp]<white_thresh || p[ic+bytewidth+dp]<white_thresh))
                            {
                            if (cc==0)
                                {
                                ic0=ic;
                                ir0=irow*rowstep;
                                }
                            cc++;
                            if (cc>ccmax)
                                {
                                ccmax=cc;
                                tanthmax=tanth;
                                ic0max=ic0;
                                ir0max=ir0;
                                }
                            }
                        else
                            cc=0;
                        }
                    }
                }
            }
        if (ccmax<ccthresh)
            break;
        if (debug)
            printf("    Vert line detected:  ccmax=%d (pix=%d), tanthmax=%g, ic0max=%d, ir0max=%d\n",ccmax,ccmax*rowstep,tanthmax,ic0max,ir0max);
        if (!vert_line_erase(bmp,cbmp,tmp,ir0max,ic0max,tanthmax,minheight_in,
                             minwidth_in,maxwidth_in,white_thresh))
            break;
        }
/*
bmp_write(tmp,"outt.png",stdout,95);
wfile_written_info("outt.png",stdout);
bmp_write(bmp,"out2.png",stdout,95);
wfile_written_info("out2.png",stdout);
exit(10);
*/
    }


/*
** Calculate max vert line length.  Line is terminated by nw consecutive white pixels
** on either side.
*/
static int vert_line_erase(WILLUSBITMAP *bmp,WILLUSBITMAP *cbmp,WILLUSBITMAP *tmp,
                           int row0,int col0,double tanth,double minheight_in,
                           double minwidth_in,double maxwidth_in,int white_thresh)

    {
    int lw,cc,maxdev,nw,dir,i,n;
    int *c1,*c2,*w;
    static char *funcname="vert_line_erase";

    willus_dmem_alloc_warn(26,(void **)&c1,sizeof(int)*3*bmp->height,funcname,10);
    c2=&c1[bmp->height];
    w=&c2[bmp->height];
    /*
    maxdev = (int)((double)bmp->height / minheight_in +.5);
    if (maxdev < 3)
        maxdev=3;
    */
    nw = (int)((double)src_dpi/100.+.5);
    if (nw<2)
        nw=2;
    maxdev=nw;
    for (i=0;i<bmp->height;i++)
        c1[i]=c2[i]=-1;
    n=0;
    for (dir=-1;dir<=1;dir+=2)
        {
        int del,brc;

        brc = 0;
        for (del=(dir==-1)?0:1;1;del++)
            {
            int r,c;
            unsigned char *p;

            r=row0+dir*del;
            if (r<0 || r>bmp->height-1)
                break;
            c=col0+(r-row0)*tanth;
            if (c<0 || c>bmp->width-1)
                break;
            p=bmp_rowptr_from_top(bmp,r);
            for (i=c;i<=c+maxdev && i<bmp->width;i++)
                if (p[i]<white_thresh)
                    break;
            if (i>c+maxdev || i>=bmp->width)
                {
                for (i=c-1;i>=c-maxdev && i>=0;i--)
                    if (p[i]<white_thresh)
                        break;
                if (i<c-maxdev || i<0)
                    {
                    brc++;
                    if (brc>=nw)
                        break;
                    continue;
                    }
                }
            brc=0;
            for (c=i,cc=0;i<bmp->width;i++)
                if (p[i]<white_thresh)
                    cc=0;
                else
                    {
                    cc++;
                    if (cc>=nw)
                        break;
                    }
            c2[r]=i-cc;
            if (c2[r]>bmp->width-1)
                c2[r]=bmp->width-1;
            for (cc=0,i=c;i>=0;i--)
                if (p[i]<white_thresh)
                    cc=0;
                else
                    {
                    cc++;
                    if (cc>=nw)
                        break;
                    }
            c1[r]=i+cc;
            if (c1[r]<0)
                c1[r]=0;
            w[n++]=c2[r]-c1[r]+1;
            c1[r]-=cc;
            if (c1[r]<0)
                c1[r]=0;
            c2[r]+=cc;
            if (c2[r]>bmp->width-1)
                c2[r]=bmp->width-1;
            }
        }
    if (n>1)
        sorti(w,n);
    if (n < 10 || n < minheight_in*src_dpi
               || w[n/4] < minwidth_in*src_dpi 
               || w[3*n/4] > maxwidth_in*src_dpi
               || (erase_vertical_lines==1 && w[n-1] > maxwidth_in*src_dpi))
        {
        /* Erase area in temp bitmap */
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p;
            int cmax;

            if (c1[i]<0 || c2[i]<0)
                continue;
            cmax=(c2[i]-c1[i])+1;
            p=bmp_rowptr_from_top(tmp,i)+c1[i];
            for (;cmax>0;cmax--,p++)
                (*p)=255;
            }
        }
    else
        {
        /* Erase line width in source bitmap */
        lw=w[3*n/4]+nw*2;
        if (lw > maxwidth_in*src_dpi/2)
            lw=maxwidth_in*src_dpi/2; 
        for (i=0;i<bmp->height;i++)
            {
            unsigned char *p;
            int c0,cmin,cmax,count,white;

            if (c1[i]<0 || c2[i]<0)
                continue;
            c0=col0+(i-row0)*tanth;
            cmin=c0-lw-1;
            if (cmin<c1[i])
                cmin=c1[i];
            cmax=c0+lw+1;
            if (cmax>c2[i])
                cmax=c2[i];
            p=bmp_rowptr_from_top(bmp,i);
            c0 = (p[cmin] > p[cmax]) ? cmin : cmax;
            white=p[c0];
            if (white <= white_thresh)
                white = white_thresh+1;
            if (white>255)
                white=255;
            count=(cmax-cmin)+1;
            p=&p[cmin];
            for (;count>0;count--,p++)
                (*p)=white;
            if (cbmp!=NULL)
                {
                unsigned char *p0;
                p=bmp_rowptr_from_top(cbmp,i);
                p0=p+c0*3;
                p=p+cmin*3;
                count=(cmax-cmin)+1;
                for (;count>0;count--,p+=3)
                    {
                    p[0]=p0[0];
                    p[1]=p0[1];
                    p[2]=p0[2];
                    }
                }
            }
        }
    willus_dmem_free(26,(double **)&c1,funcname);
    return(1);
    }

/*
** mem_index... controls which memory allocactions get a protective margin
** around them.
*/
static int mem_index_min = 999;
static int mem_index_max = 999;
static void willus_dmem_alloc_warn(int index,void **ptr,int size,char *funcname,
                                int exitcode)

    {
    if (index>=mem_index_min && index<=mem_index_max)
        {
        char *ptr1;
        void *x;
        willus_mem_alloc_warn((void **)&ptr1,size+2048,funcname,exitcode);
        ptr1 += 1024;
        x=(void *)ptr1;
        (*ptr) = x;
        }
    else
        willus_mem_alloc_warn(ptr,size,funcname,exitcode);
    }


static void willus_dmem_free(int index,double **ptr,char *funcname)

    {
    if ((*ptr)==NULL)
        return;
    if (index>=mem_index_min && index<=mem_index_max)
        { 
        double *x;
        char *ptr1;
        x=(*ptr);
        ptr1=(char *)x;
        ptr1 -= 1024;
        x=(double *)ptr1;
        willus_mem_free(&x,funcname);
        (*ptr)=NULL;
        }
    else
        willus_mem_free(ptr,funcname);
    }
