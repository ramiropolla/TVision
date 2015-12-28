/*------------------------------------------------------------*/
/* filename -       tbutton.cpp                               */
/*                                                            */
/* function(s)                                                */
/*          TButton member functions                          */
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
/*                                                            */
/*    Turbo Vision -  Version 1.0                             */
/*                                                            */
/*                                                            */
/*    Copyright (c) 1991 by Borland International             */
/*    All Rights Reserved.                                    */
/*                                                            */
/*------------------------------------------------------------*/

#define Uses_TButton
#define Uses_TDrawBuffer
#define Uses_TEvent
#define Uses_TRect
#define Uses_TGroup
#define Uses_opstream
#define Uses_ipstream
#include <tv.h>

#define cpButton "\x0A\x0B\x0C\x0D\x0E\x0E\x0E\x0F"

TButton::TButton( const TRect& bounds,
                  const char *aTitle,
                  ushort aCommand,
                  ushort aFlags) :
    TView( bounds ),
    title( newStr( aTitle ) ),
    command( aCommand ),
    flags( uchar(aFlags) ),
    amDefault( Boolean( (aFlags & bfDefault) != 0 ) )
{
    options |= ofSelectable | ofFirstClick | ofPreProcess | ofPostProcess;
    eventMask |= evBroadcast;
    if( !commandEnabled(aCommand) )
        state |= sfDisabled;
}

TButton::~TButton()
{
    delete[] (char*)title;
}

void TButton::draw()
{
    drawState(False);
}

void TButton::drawTitle( TDrawBuffer &b,
                         int s,
                         int i,
                         ushort cButton,
                         Boolean down
                       )
{
    int l, scOff;
    if( (flags & bfLeftJust) != 0 )
        l = 1;
    else
        {
        l = (s - cstrlen(title) - 1)/2;
        if( l < 1 && (flags & bfNoShadows) == 0)
            l = 1;
        }
    b.moveCStr( ushort(i+l), title, cButton );

    if( showMarkers == True && !down )
        {
        if( (state & sfSelected) != 0 )
            scOff = 0;
        else if( amDefault )
            scOff = 2;
        else
            scOff = 4;
        b.putChar( 0, specialChars[scOff] );
        b.putChar( ushort(s), specialChars[scOff+1] );
        }
}

void TButton::drawState(Boolean down)
{
    ushort cButton, cShadow;
    char   ch = ' ';
    int    i;
    TDrawBuffer b;

    if( (state & sfDisabled) != 0 )
        cButton = getColor(0x0404);
    else
        {
        cButton = getColor(0x0501);
        if( (state & sfActive) != 0 )
            if( (state & sfSelected) != 0 )
                cButton = getColor(0x0703);
            else if( amDefault )
                cButton = getColor(0x0602);
        }
    cShadow = cButton;
    int s = size.x;
    int T = size.y / 2;
    int maxy = size.y - 1;
    if ( (flags & bfNoShadows) == 0 )
    {
      s--;
      T--;
      maxy--;
      cShadow = getColor(8);
    }

    for( int y = 0; y <= maxy; y++ )
    {
      b.moveChar(0, ' ', cButton, size.x);
      b.putAttribute(0, cShadow);
      if( down )
      {
        b.putAttribute(1, cShadow);
        i = 2;
      }
      else
      {
        b.putAttribute( ushort(s), cShadow );
        if( showMarkers == False )
        {
          if( y == 0 )
              b.putChar( ushort(s), shadows[0] );
          else
              b.putChar( ushort(s), shadows[1] );
          ch = shadows[2];
        }
        i =  1;
      }

      if( y == T && title != 0 )
          drawTitle(b, s, i, cButton, down);

      if( showMarkers && !down )
      {
        b.putChar(1, markers[0]);
        b.putChar(ushort(s-1), markers[1]);
      }
      writeLine( 0, y, size.x, 1, b );
    }
    if ( (flags & bfNoShadows) == 0 )
    {
      b.moveChar(0, ' ', cShadow, 2);
      b.moveChar(2, ch, cShadow, s-1);
      writeLine(0, size.y-1, size.x, 1, b);
    }
}

TPalette& TButton::getPalette() const
{
    static TPalette palette( cpButton, sizeof( cpButton )-1 );
    return palette;
}

void TButton::handleEvent( TEvent& event )
{
    TPoint mouse;
    TRect clickRect;

    clickRect = getExtent();
    if ( (flags & bfNoShadows) == 0 )
    {
      clickRect.a.x++;
      clickRect.b.x--;
      clickRect.b.y--;
    }

    if( event.what == evMouseDown )
        {
        mouse = makeLocal( event.mouse.where );
        if( !clickRect.contains(mouse) )
            clearEvent( event );
        }
    TView::handleEvent(event);

    switch( event.what )
        {
        case evMouseDown: {
            clickRect.b.x++;
            Boolean down = False;
            do  {
                mouse = makeLocal( event.mouse.where );
                if( down != clickRect.contains( mouse ) )
                    {
                    down = Boolean( !down );
                    drawState( down );
                    }
                } while( mouseEvent( event, evMouseMove ) );
            if( down )
                {
                press();
                drawState( False );
                }
            clearEvent( event );
          }
          break;

        case evKeyDown: {
            char c = hotKey( title );
            if( event.keyDown.keyCode == getAltCode(c) ||
                ( owner->phase == phPostProcess &&
                  c != 0 &&
                  toupper(event.keyDown.charScan.charCode) == c
                ) ||
                ( (state & sfFocused) != 0 &&
                  event.keyDown.charScan.charCode == ' '
                )
              )
                {
                press();
                clearEvent( event );
                }
          }
          break;

        case evBroadcast:
            switch( event.message.command )
                {
                case cmDefault:
                    if( amDefault && !(state & sfDisabled) )
                        {
                        press();
                        clearEvent(event);
                        }
                    break;

                case cmGrabDefault:
                case cmReleaseDefault:
                    if( (flags & bfDefault) != 0 )
                        {
                        amDefault = Boolean(event.message.command == cmReleaseDefault);
                        drawView();
                        }
                    break;

                case cmCommandSetChanged:
                    if (((state & sfDisabled) && commandEnabled(command)) ||
                        (!(state & sfDisabled) && !commandEnabled(command)))
                    {
                      setState(sfDisabled,Boolean(!commandEnabled(command)));
                      drawView();
                    }
                    break;
                }
        break;
        }
}

void TButton::makeDefault( Boolean enable )
{
    if( (flags & bfDefault) == 0 )
        {
        message( owner,
                 evBroadcast,
                 ushort((enable == True) ? cmGrabDefault : cmReleaseDefault),
                 this
               );
        amDefault = enable;
        drawView();
        }
}

void TButton::setState( ushort aState, Boolean enable )
{
    if ( ((state & aState) != 0) == (enable != 0) )
      return;
      
    TView::setState(aState, enable);
    if( aState & (sfSelected | sfActive) )
    {
        if(!enable)
        {                          
            state &= ~sfFocused;
            makeDefault(False);
        }
        drawView();
    }
    if( (aState & sfFocused) != 0 )
        makeDefault( enable );
}

void TButton::press()
{
    message( owner, evBroadcast, cmRecordHistory, 0 );
    if( (flags & bfBroadcast) != 0 )
        message( owner, evBroadcast, command, this );
    else
        {
        TEvent e;
        e.what = evCommand;
        e.message.command = command;
        e.message.infoPtr = this;
        putEvent( e );
        }
}

#ifndef NO_TV_STREAMS
void TButton::write( opstream& os )
{
    TView::write( os );
    os.writeString( title );
    os << command << flags << (int)amDefault;
}

void *TButton::read( ipstream& is )
{
    TView::read( is );
    title = is.readString();
    int temp;
    is >> command >> flags >> temp;
    amDefault = Boolean(temp);
    if( TButton::commandEnabled( command ) )
        state &= ~sfDisabled;
    else
        state |= sfDisabled;
    return this;
}

TStreamable *TButton::build()
{
    return new TButton( streamableInit );
}
#endif  // ifndef NO_TV_STREAMS
