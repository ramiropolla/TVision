
TVision by Borland ported to OS/2, MS Windows, DOS32, Linux
-----------------------------------------------------------

                                                Version Level I (beta)

This port is partially based on the port of TVision to FreeBSD made by
Sergio Sigala.

I stripped down the source code and removed the curses stuff. Also
FreeBSD and GPM stuff was removed. In fact there is a lot removed from
the modifications made by Sergio Sigala, but since I have used his texts
as the starting point, I kept his copyright.

This version of TVision supports only XTERM emulation. It will happily
try to run on any terminal and might fail miserably on terminals not
compatible with XTERM. But since the source code is available, feel free
to modify the code and contribute the changes, I'll be glad to add them.

I also removed GNU configurator, 16-bit MS DOS support and various
obsolete stuff. The code is supposed to run on:

        MS Windows
        Linux
        DOS32

at least this was my goal.

This port is also based on my own version of TVision (ported to OS/2).
The old version is available on the net:

http://www.rosprombank.ru/~ig/other.html

Since this is the very first release of the source code, I expect that
there will be many problems with it. Feel free to report them or even
better, to fix the found bugs :)

With best regards,
Ilfak Guilfanov at Datarescue <ig@datarescue.be>

Level I news
------------

The linux part has been completely rewritten by Yury Haron <yjh@styx.cabel.net>.
Now the curses and X11 support are back.
Russian font/keyboard support has been added.
The TVOPT environment variable is added - it contains parameters to fine-tune
TVision.
See tvtuning.txt for information.
