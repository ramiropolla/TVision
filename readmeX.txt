$Id: readmeX.txt,v 1.4 2004/08/28 23:02:18 jeremy Exp $

TVision ported to Linux X Windows
-----------------------------------------------------------

This distribution includes a module which offers support for running
TVision applications natively under X11.  To use this version of the
library you must build the 'libtvisionx.so' target, install it
as 'libtvision.so' with your application, and install a "VGA" font on your
X server.

Release notes
-----------------------------------------------------------

Release JSC-1.2 notes:

* Added support for numeric keypad 'multiply' key (*); it was previously
  missing and not translated.  Also fixed translation of Tab, Enter,
  Backspace, and Escape keys that was botched with the introduction of
  scancode translation in release 1.1.

Installation
-----------------------------------------------------------

To build, run 'make libtvisionx.so' in the source/ directory.

Install the 'libtvisionx.so' file as 'libtvision.so' with your application.

TVision requires an IBM-compatible PC X server font (commonly called a 'VGA'
font around the web).  There are several freely available high-quality fonts
which fit this description.  I recommend 'vga.pcf' and 'sabvga.pcf', both of
which can currently be found at

http://www.shelluser.net/~giles/bashprompt/xfonts/

Please consult your X server's documentation for information on font
installation.  In general, these fonts should be installed in the
/usr/X11R6/lib/X11/fonts/ directory, run 'mkfontdir' in the directory in which
you install the fonts, then run 'xset fp rehash'.  Finally, if you are running
a font server, (such as "xfs") force it to restart.  If all else fails, just
run 'mkfontdir' and restart your X server.

Running
-----------------------------------------------------------

This X port of TVision obeys the X11 resource specification as closely as
possible.  It will obey the following resource settings (settable in your
.Xdefaults file):

  Colors
  ======
 
TVision applications support a text color model based on the color
text mode of an IBM-PC VGA display card.  Under this model text can be
displayed in any one of eight background colors and sixteen foreground colors.
The exact colors used by the application can be modified by setting the
following resources.  The values for these colors can be specified by well
known name (such as "blue", or "grey70") or in 24 bit hexadecimal RGB
notation (#120ac7).

tvision.text.bgcolor0  - Background color 0 (black)
             bgcolor1  - Background color 1 (dark blue)
             bgcolor2  - Background color 2 (dark green)
             bgcolor3  - Background color 3 (dark cyan)
             bgcolor4  - Background color 4 (dark red)
             bgcolor5  - Background color 5 (dark magenta)
             bgcolor6  - Background color 6 (brown)
             bgcolor7  - Background color 7 (dark white/grey)
             fgcolor0  - Foreground color 0 (black)
             fgcolor1  - Foreground color 1 (dark blue)
             fgcolor2  - Foreground color 2 (dark green)
             fgcolor3  - Foreground color 3 (dark cyan)
             fgcolor4  - Foreground color 4 (dark red)
             fgcolor5  - Foreground color 5 (dark magenta)
             fgcolor6  - Foreground color 6 (brown)
             fgcolor7  - Foreground color 7 (dark white/grey)
             fgcolor8  - Foreground color 8 (high intensity black/dark grey)
             fgcolor9  - Foreground color 9 (high intensity blue)
             fgcolor10  - Foreground color 10 (high intensity green)
             fgcolor11  - Foreground color 11 (high intensity cyan)
             fgcolor12  - Foreground color 12 (high intensity red)
             fgcolor13  - Foreground color 13 (high intensity magenta)
             fgcolor14  - Foreground color 14 (high intensity yellow)
             fgcolor15  - Foreground color 15 (high intensity white)

  Fonts
  =====

The TVision application will use any font you specify.  For best results,
the font you use must be fixed with and should be encoded in the IBM-PC
style (mostly for drawing line characters).  The font specified in the

tvision.text.font

resource will be used.  (By default, TVision attempts to use a font
named "vga").


  Geometry
  ========

The initial size and position of the TVision "virtual screen" provided by this
library is bounded by the

tvision.geometry

resource specification.  The value for this resource is a standard X11
"geometry" specification, of the format

<width>[x<height>[(-|+)<x-position>][(-|+)<y-position>]

The width and height specified will be interpreted as the number of columns
and rows, respectively, to display in the virtual screen.

  Example
  =======

Here's an example .Xdefaults file, with example entries for the TVision
library.

tvision.text.font: vga
tvision.geometry: 100x40
tvision.text.bgcolor1: blue40
tvision.text.bgcolor6: #404000

Enjoy!
Jeremy Cooper <jeremy, at baymoo.org>
