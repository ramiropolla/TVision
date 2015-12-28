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

MAC_EXPORT_ALL=1 # export all symbols under MAC OS X
_CFLAGS = -O -I..\include -D__NOROW__ -D__RUS__ -D__IDA__ -DNO_TV_STREAMS
				# see tscreen.cpp for the use of __NOROW__
				# __RUS__ changes some characters to russian

!ifndef IDA
!include ../../../../allmake.mak
!else
!include $(IDA)allmake.mak
!endif

!ifdef __LINUX__
__UNIX__=1
!endif
!ifdef __BSD__
__UNIX__=1
!endif
!ifdef __MAC__
__UNIX__=1
INSTALL_NAME=-install_name @executable_path/libtvision$(DLLSUFF)
!endif

!ifdef __X11__
_CFLAGS+= -D__X11__
!endif

OBJS=   $(F)colorsel$(O) \
        $(F)drivers$(O)  \
        $(F)edits2$(O)   \
        $(F)editstat$(O) \
        $(F)frmline2$(O) \
        $(F)grp$(O)      \
        $(F)help$(O)     \
        $(F)helpbase$(O) \
        $(F)histlist$(O) \
        $(F)mapcolor$(O) \
        $(F)menu$(O)     \
        $(F)misc$(O)     \
        $(F)msgbox$(O)   \
        $(F)names$(O)    \
        $(F)new$(O)      \
        $(F)newstr$(O)   \
        $(F)os2handl$(O) \
        $(F)palette$(O)  \
        $(F)sall$(O)     \
        $(F)stddlg$(O)   \
        $(F)swapst2$(O)  \
        $(F)syserr$(O)   \
        $(F)tapplica$(O) \
        $(F)tbkgrnd$(O)  \
        $(F)tbutton$(O)  \
        $(F)tchdrdlg$(O) \
        $(F)tcheckbo$(O) \
        $(F)tcluster$(O) \
        $(F)tcmdset$(O)  \
        $(F)tcollect$(O) \
        $(F)tdesktop$(O) \
        $(F)tdialog$(O)  \
        $(F)tdircoll$(O) \
        $(F)tdirlist$(O) \
        $(F)teditor1$(O) \
        $(F)teditor2$(O) \
        $(F)teditwnd$(O) \
        $(F)tevent$(O)   \
        $(F)textview$(O) \
        $(F)tfildlg$(O)  \
        $(F)tfilecol$(O) \
        $(F)tfiledtr$(O) \
        $(F)tfillist$(O) \
        $(F)tframe$(O)   \
        $(F)tgrmv2$(O)   \
        $(F)tgroup$(O)   \
        $(F)thistory$(O) \
        $(F)thistwin$(O) \
        $(F)thstview$(O) \
        $(F)tindictr$(O) \
        $(F)tinputli$(O) \
        $(F)tlabel$(O)   \
        $(F)tlistbox$(O) \
        $(F)tlstview$(O) \
        $(F)tmemo$(O)    \
        $(F)tmenubar$(O) \
        $(F)tmenubox$(O) \
        $(F)tmnuview$(O) \
        $(F)tmouse$(O)   \
        $(F)tobject$(O)  \
        $(F)tobjstrm$(O) \
        $(F)tparamte$(O) \
        $(F)tpoint$(O)   \
        $(F)tprogram$(O) \
        $(F)tradiobu$(O) \
        $(F)trescoll$(O) \
        $(F)tresfile$(O) \
        $(F)tscreen$(O)  \
        $(F)tscrlbar$(O) \
        $(F)tscrolle$(O) \
        $(F)tsortcol$(O) \
        $(F)tstatict$(O) \
        $(F)tstatusl$(O) \
        $(F)tstrcoll$(O) \
        $(F)tstrlist$(O) \
        $(F)ttprvln2$(O) \
        $(F)tvcursr2$(O) \
        $(F)tvexpsd2$(O) \
        $(F)tview$(O)    \
        $(F)tvtext1$(O)  \
        $(F)tvtext2$(O)  \
        $(F)tvwrite2$(O) \
        $(F)twindow$(O)

OBJS_XTERM= $(OBJS) linux$(O) fdlisten$(O)
OBJS_XWIN=  $(OBJS) linuxx$(O) fdlisten$(O)

!ifdef __UNIX__
!ifndef __BSD__
DL=-ldl
!endif
!ifdef __MAC__
UNDEFS=-undefined dynamic_lookup
!else
EXPORTS_DEF=tvision.script
!endif
all: $(R)libtvision$(DLLSUFF) $(R)libtvisionx$(DLLSUFF)
$(R)libtvision$(DLLSUFF): $(OBJS_XTERM) $(EXPORTS_DEF)
	$(CCL) $(OUTDLL) $(OUTSW)$@ $(EXPORTS_DEF) $(OBJS_XTERM) $(DL) $(UNDEFS) $(INSTALL_NAME)
$(R)libtvisionx$(DLLSUFF): $(OBJS_XWIN) $(EXPORTS_DEF)
	$(CCL) $(OUTDLL) $(OUTSW)$@ $(EXPORTS_DEF) $(OBJS_XWIN) $(DL) $(UNDEFS) $(INSTALL_NAME) -L/usr/X11R6/lib -lX11
!else
all: objdir tv$(T) $(TL)tv$(A)
!endif

!ifdef __NT__
hello:	.\hello$(B) $(TL)tv$(A)
.\hello$(B): tv$(T) $(F)hello$(O)
!else
./hello$(B): tv$(T) $(F)hello$(O)
!endif
	$(CCL) $(OUTSW)$@ $(F)hello$(O) $(OBJS_XTERM) $(L)pro$(A) $(DL)
./hellox$(B): tv$(T) $(F)hello$(O)
	$(CCL) $(OUTSW)$@ $(F)hello$(O) $(OBJS_XWIN) $(L)pro$(A) $(DL)

$(TL)tv$(A) tv$(T): $(OBJS)
!ifdef TL
        @if not exist "$(TL)$(CHKDIR)" mkdir $(TL)
!endif
	@$(AR) $(TL)tv$(A) $(OBJS)

!include $(IDA)objdir.mak
#-----------------------------------------------------------------------
$(F)hello$(O):          hello.cpp

$(F)colorsel$(O):       colorsel.cpp
$(F)drivers$(O):        drivers.cpp
$(F)edits2$(O):         edits2.cpp
$(F)editstat$(O):       editstat.cpp
$(F)frmline2$(O):       frmline2.cpp
$(F)grp$(O):            grp.cpp
$(F)help$(O):           help.cpp
$(F)helpbase$(O):       helpbase.cpp
$(F)histlist$(O):       histlist.cpp
$(F)linux$(O):          linux.cpp
$(F)linuxx$(O):         linuxx.cpp
$(F)fdlisten$(O):       fdlisten.cpp
$(F)mapcolor$(O):       mapcolor.cpp
$(F)menu$(O):           menu.cpp
$(F)misc$(O):           misc.cpp
$(F)msgbox$(O):         msgbox.cpp
$(F)names$(O):          names.cpp
$(F)new$(O):            new.cpp
$(F)newstr$(O):         newstr.cpp
$(F)os2handl$(O):       os2handl.cpp
$(F)palette$(O):        palette.cpp
$(F)sall$(O):           sall.cpp
$(F)stddlg$(O):         stddlg.cpp
$(F)swapst2$(O):        swapst2.cpp
$(F)syserr$(O):         syserr.cpp
$(F)tapplica$(O):       tapplica.cpp
$(F)tbkgrnd$(O):        tbkgrnd.cpp
$(F)tbutton$(O):        tbutton.cpp
$(F)tchdrdlg$(O):       tchdrdlg.cpp
$(F)tcheckbo$(O):       tcheckbo.cpp
$(F)tcluster$(O):       tcluster.cpp
$(F)tcmdset$(O):        tcmdset.cpp
$(F)tcollect$(O):       tcollect.cpp
$(F)tdesktop$(O):       tdesktop.cpp
$(F)tdialog$(O):        tdialog.cpp
$(F)tdircoll$(O):       tdircoll.cpp
$(F)tdirlist$(O):       tdirlist.cpp
$(F)teditor1$(O):       teditor1.cpp
$(F)teditor2$(O):       teditor2.cpp
$(F)teditwnd$(O):       teditwnd.cpp
$(F)tevent$(O):         tevent.cpp
$(F)textview$(O):       textview.cpp
$(F)tfildlg$(O):        tfildlg.cpp
$(F)tfilecol$(O):       tfilecol.cpp
$(F)tfiledtr$(O):       tfiledtr.cpp
$(F)tfillist$(O):       tfillist.cpp
$(F)tframe$(O):         tframe.cpp
$(F)tgrmv2$(O):         tgrmv2.cpp
$(F)tgroup$(O):         tgroup.cpp
$(F)thistory$(O):       thistory.cpp
$(F)thistwin$(O):       thistwin.cpp
$(F)thstview$(O):       thstview.cpp
$(F)tindictr$(O):       tindictr.cpp
$(F)tinputli$(O):       tinputli.cpp
$(F)tlabel$(O):         tlabel.cpp
$(F)tlistbox$(O):       tlistbox.cpp
$(F)tlstview$(O):       tlstview.cpp
$(F)tmemo$(O):          tmemo.cpp
$(F)tmenubar$(O):       tmenubar.cpp
$(F)tmenubox$(O):       tmenubox.cpp
$(F)tmnuview$(O):       tmnuview.cpp
$(F)tmouse$(O):         tmouse.cpp
$(F)tobject$(O):        tobject.cpp
$(F)tobjstrm$(O):       tobjstrm.cpp
$(F)tparamte$(O):       tparamte.cpp
$(F)tpoint$(O):         tpoint.cpp
$(F)tprogram$(O):       tprogram.cpp
$(F)tradiobu$(O):       tradiobu.cpp
$(F)trescoll$(O):       trescoll.cpp
$(F)tresfile$(O):       tresfile.cpp
$(F)tscreen$(O):        tscreen.cpp
$(F)tscrlbar$(O):       tscrlbar.cpp
$(F)tscrolle$(O):       tscrolle.cpp
$(F)tsortcol$(O):       tsortcol.cpp
$(F)tstatict$(O):       tstatict.cpp
$(F)tstatusl$(O):       tstatusl.cpp
$(F)tstrcoll$(O):       tstrcoll.cpp
$(F)tstrlist$(O):       tstrlist.cpp
$(F)ttprvln2$(O):       ttprvln2.cpp
$(F)tvcursr2$(O):       tvcursr2.cpp
$(F)tvexpsd2$(O):       tvexpsd2.cpp
$(F)tview$(O):          tview.cpp
$(F)tvtext1$(O):        tvtext1.cpp
$(F)tvtext2$(O):        tvtext2.cpp
$(F)tvwrite2$(O):       tvwrite2.cpp
$(F)twindow$(O):        twindow.cpp
