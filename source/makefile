#
#   Borland C++ - (C) Copyright 1991, 1992 by Borland International
#   Makefile for building Turbo Vision library
#

#
#	Please note that default .CFG files define NO_TV_STREAMS for
#	TVision w/o streams.
#

#
#	__IDA__ changes behaviour of TVision in some places:
#		- color selector allows to select bright backgrounds
#		- color display shows "black on black" as is (was: blinking on red)
#		- use snprintf() function (see msgbox.cpp)
#

CCPLUS=g++
O=.o
B=
A=.a
CFLAGS = -D__LINUX__ -I/usr/X11R6/include -I../include -D__NOROW__ -D__RUS__ -D__IDA__ -DNO_TV_STREAMS
				# see tscreen.cpp for the use of __NOROW__
				# __RUS__ changes some characters to russian
				#
# Uncomment to build under Apple's MacOS X.
#CFLAGS+=-D__MACOSX__ 


OBJS=   prodir$(O)   \
        colorsel$(O) \
        drivers$(O)  \
        edits2$(O)   \
        editstat$(O) \
        frmline2$(O) \
        grp$(O)      \
        help$(O)     \
        helpbase$(O) \
        histlist$(O) \
        mapcolor$(O) \
        menu$(O)     \
        misc$(O)     \
        msgbox$(O)   \
        names$(O)    \
        new$(O)      \
        newstr$(O)   \
        os2handl$(O) \
        palette$(O)  \
        sall$(O)     \
        stddlg$(O)   \
        swapst2$(O)  \
        syserr$(O)   \
        tapplica$(O) \
        tbkgrnd$(O)  \
        tbutton$(O)  \
        tchdrdlg$(O) \
        tcheckbo$(O) \
        tcluster$(O) \
        tcmdset$(O)  \
        tcollect$(O) \
        tdesktop$(O) \
        tdialog$(O)  \
        tdircoll$(O) \
        tdirlist$(O) \
        teditor1$(O) \
        teditor2$(O) \
        teditwnd$(O) \
        tevent$(O)   \
        textview$(O) \
        tfildlg$(O)  \
        tfilecol$(O) \
        tfiledtr$(O) \
        tfillist$(O) \
        tframe$(O)   \
        tgrmv2$(O)   \
        tgroup$(O)   \
        thistory$(O) \
        thistwin$(O) \
        thstview$(O) \
        tindictr$(O) \
        tinputli$(O) \
        tlabel$(O)   \
        tlistbox$(O) \
        tlstview$(O) \
        tmemo$(O)    \
        tmenubar$(O) \
        tmenubox$(O) \
        tmnuview$(O) \
        tmouse$(O)   \
        tobject$(O)  \
        tobjstrm$(O) \
        tparamte$(O) \
        tpoint$(O)   \
        tprogram$(O) \
        tradiobu$(O) \
        trescoll$(O) \
        tresfile$(O) \
        tscreen$(O)  \
        tscrlbar$(O) \
        tscrolle$(O) \
        tsortcol$(O) \
        tstatict$(O) \
        tstatusl$(O) \
        tstrcoll$(O) \
        tstrlist$(O) \
        ttprvln2$(O) \
        tvcursr2$(O) \
        tvexpsd2$(O) \
        tview$(O)    \
        tvtext1$(O)  \
        tvtext2$(O)  \
        tvwrite2$(O) \
        twindow$(O)

OBJS_XTERM= $(OBJS) linux$(O)
OBJS_XWIN=  $(OBJS) linuxx$(O)


.cpp.o:
	$(CCPLUS) -c $(CFLAGS) $<

EXPORTS_DEF=tvision.script
all: $(R)libtvision.so
$(R)libtvision.so: $(OBJS_XTERM) $(EXPORTS_DEF)
	$(CCPLUS) -shared -o $@ $(EXPORTS_DEF) $(OBJS_XTERM) -ldl --no-undefined
$(R)libtvisionx.so: $(OBJS_XWIN) $(EXPORTS_DEF)
	$(CCPLUS) -shared -o $@ $(EXPORTS_DEF) $(OBJS_XWIN) -lX11 -ldl --no-undefined \
	-L/usr/X11R6/lib

./hello: hello$(O) $(OBJS_XTERM)
	$(CCPLUS) -o $@ hello$(O) $(OBJS_XTERM) -ldl

./hellox: hello$(O) $(OBJS_XWIN)
	$(CCPLUS) -o $@ hello$(O) $(OBJS_XWIN) -L/usr/X11R6/lib -lX11

#-----------------------------------------------------------------------
hello$(O):          hello.cpp

prodir$(O):         prodir.cpp
colorsel$(O):       colorsel.cpp
drivers$(O):        drivers.cpp
edits2$(O):         edits2.cpp
editstat$(O):       editstat.cpp
frmline2$(O):       frmline2.cpp
grp$(O):            grp.cpp
help$(O):           help.cpp
helpbase$(O):       helpbase.cpp
histlist$(O):       histlist.cpp
linux$(O):          linux.cpp
linuxx$(O):         linuxx.cpp
mapcolor$(O):       mapcolor.cpp
menu$(O):           menu.cpp
misc$(O):           misc.cpp
msgbox$(O):         msgbox.cpp
names$(O):          names.cpp
new$(O):            new.cpp
newstr$(O):         newstr.cpp
os2handl$(O):       os2handl.cpp
palette$(O):        palette.cpp
sall$(O):           sall.cpp
stddlg$(O):         stddlg.cpp
swapst2$(O):        swapst2.cpp
syserr$(O):         syserr.cpp
tapplica$(O):       tapplica.cpp
tbkgrnd$(O):        tbkgrnd.cpp
tbutton$(O):        tbutton.cpp
tchdrdlg$(O):       tchdrdlg.cpp
tcheckbo$(O):       tcheckbo.cpp
tcluster$(O):       tcluster.cpp
tcmdset$(O):        tcmdset.cpp
tcollect$(O):       tcollect.cpp
tdesktop$(O):       tdesktop.cpp
tdialog$(O):        tdialog.cpp
tdircoll$(O):       tdircoll.cpp
tdirlist$(O):       tdirlist.cpp
teditor1$(O):       teditor1.cpp
teditor2$(O):       teditor2.cpp
teditwnd$(O):       teditwnd.cpp
tevent$(O):         tevent.cpp
textview$(O):       textview.cpp
tfildlg$(O):        tfildlg.cpp
tfilecol$(O):       tfilecol.cpp
tfiledtr$(O):       tfiledtr.cpp
tfillist$(O):       tfillist.cpp
tframe$(O):         tframe.cpp
tgrmv2$(O):         tgrmv2.cpp
tgroup$(O):         tgroup.cpp
thistory$(O):       thistory.cpp
thistwin$(O):       thistwin.cpp
thstview$(O):       thstview.cpp
tindictr$(O):       tindictr.cpp
tinputli$(O):       tinputli.cpp
tlabel$(O):         tlabel.cpp
tlistbox$(O):       tlistbox.cpp
tlstview$(O):       tlstview.cpp
tmemo$(O):          tmemo.cpp
tmenubar$(O):       tmenubar.cpp
tmenubox$(O):       tmenubox.cpp
tmnuview$(O):       tmnuview.cpp
tmouse$(O):         tmouse.cpp
tobject$(O):        tobject.cpp
tobjstrm$(O):       tobjstrm.cpp
tparamte$(O):       tparamte.cpp
tpoint$(O):         tpoint.cpp
tprogram$(O):       tprogram.cpp
tradiobu$(O):       tradiobu.cpp
trescoll$(O):       trescoll.cpp
tresfile$(O):       tresfile.cpp
tscreen$(O):        tscreen.cpp
tscrlbar$(O):       tscrlbar.cpp
tscrolle$(O):       tscrolle.cpp
tsortcol$(O):       tsortcol.cpp
tstatict$(O):       tstatict.cpp
tstatusl$(O):       tstatusl.cpp
tstrcoll$(O):       tstrcoll.cpp
tstrlist$(O):       tstrlist.cpp
ttprvln2$(O):       ttprvln2.cpp
tvcursr2$(O):       tvcursr2.cpp
tvexpsd2$(O):       tvexpsd2.cpp
tview$(O):          tview.cpp
tvtext1$(O):        tvtext1.cpp
tvtext2$(O):        tvtext2.cpp
tvwrite2$(O):       tvwrite2.cpp
twindow$(O):        twindow.cpp
