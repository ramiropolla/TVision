$Id: readmeX.txt,v 1.7 2005/10/11 02:52:27 jeremy Exp $

TVision ported to Linux X Windows
-----------------------------------------------------------

This package is a module which offers support for running IDA natively under
X11.  To use this version of the library you must build the library, install
it as 'libtvision.so' in your IDA application directory, and finally, install
a "VGA" font on your X server.

Release notes
-----------------------------------------------------------

Release JSC-1.5 notes:

* Incorporated changes in DataRescue's TVision L release.  Compatability
  with IDA 4.9.

Installation
-----------------------------------------------------------

To build this library you must first obtain and unpack the IDA SDK.  Once
you have unpacked the SDK, run 'make IDA=<your-sdk-dir>/' in
the source/ directory, where <your-sdk-dir> is a relative or absolute path
to the unpacked SDK.  Don't forget the trailing slash.

When the build process has completed, you will find two shared libraries in
in the 'bin' directory of your IDA SDK distribution:  A 'libtvision.so' file
(xterm mode library) and a 'libtvisionx.so' file (native X11 library).  To use
the native X11 library copy it to your IDA application directory and rename it
to 'libtvision.so'.  To switch back to using xterm mode just do the same with
the built 'libtvision.so'.

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

To use the library, simply run IDA as you normally would (for linux this
is the 'idal' binary).  Since the X11 version of the library makes IDA a native
X11 application, your 'DISPLAY' environment variable must be set to a valid
X server specification.  (This happens automatically if you invoke IDA from
within an existing XTerm).

This X port of TVision obeys the X11 resource specification as closely as
possible.  It will obey the following resource settings (settable in your
.Xdefaults file):

  Colors
  ======
 
TVision applications support a text color model based on the color
text mode of an IBM-PC VGA display card, with some additional features.
Under this model text can be displayed in any one of sixteen background colors
and sixteen foreground colors.

The exact colors used by the application can be modified by setting the
following X resources.  The values for these colors can be specified by well
known name (such as "blue", or "grey70") or in 24 bit hexadecimal RGB
notation (#120ac7).  The default setting for each color appears in parentheses.

tvision.text.bgcolor0  - Background color 0 (black)
             bgcolor1  - Background color 1 (dark blue)
             bgcolor2  - Background color 2 (dark green)
             bgcolor3  - Background color 3 (dark cyan)
             bgcolor4  - Background color 4 (dark red)
             bgcolor5  - Background color 5 (dark magenta)
             bgcolor6  - Background color 6 (brown)
             bgcolor7  - Background color 7 (dark white/grey)
             bgcolor8  - Background color 8 (high intensity black/dark grey)
             bgcolor9  - Background color 9 (high intensity blue)
             bgcolor10  - Background color 10 (high intensity green)
             bgcolor11  - Background color 11 (high intensity cyan)
             bgcolor12  - Background color 12 (high intensity red)
             bgcolor13  - Background color 13 (high intensity magenta)
             bgcolor14  - Background color 14 (high intensity yellow)
             bgcolor15  - Background color 15 (high intensity white)
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
the font you use must be fixed-width and should be encoded in the IBM-PC
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

Troubleshooting
-----------------------------------------------------------

This section has solutions to common errors you might encounter when
using the library.

* No DISPLAY defined.

  If you see this error then your DISPLAY environment variable has not been
  set.  IDA uses this variable to find the address of the X server to which
  it must connect.

  Normally the DISPLAY environment variable is populated automatically
  by your shell application (such as Xterm) or by your SSH application (if
  X11 forwarding is turned on).  The most common source of this error is
  using SSH without turning on X11 forwarding, or a missing 'xauth' application
  on the remote host.

* Font '...' is not fixed-width.

  The font specified in your resource file (or the default, 'vga')
  doesn't appear to be a fixed-width font; that is, not all of the characters
  in the font occupy the same imaginary bounding box.  IDA depends on its
  font being fixed-width to ensure that items on the screen line up properly.

  While running IDA with a variable-width font will not cause any serious
  errors, you will find that lines of text don't align properly, and you will
  see strange behavior when IDA makes partial line updates (rather than
  redrawing a full line).

Enjoy!
Jeremy Cooper <jeremy, at baymoo.org>
