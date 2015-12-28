/*
  NO_TV_STREAMS WORKS WITH THE HELP FIXED BY URI BECHAR
*/

/*------------------------------------------------------------*/
/* filename -       HelpBase.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  Member function(s) of following classes   */
/*                      THelpTopic                            */
/*                      THelpIndex                            */
/*                      THelpFile                             */
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

#define Uses_TStreamableClass
#define Uses_TPoint
#define Uses_TStreamable
#define Uses_ipstream
#define Uses_opstream
#define Uses_fpstream
#define Uses_TRect
#include <tv.h>

#include "helpbase.h"
#include "util.h"
#include <sys/stat.h>
#ifdef __IDA__
#include <help.h>
#endif

const char *tv_dynhelp;   // 02.08.98: dynamic help message for forms

TCrossRefHandler crossRefHandler = notAssigned;

// THelpTopic
#ifndef NO_TV_STREAMS
const char * const THelpTopic::name = "THelpTopic";

TStreamable *THelpTopic::build()
{
    return new THelpTopic( streamableInit );
}


TStreamableClass RHelpTopic( THelpTopic::name,
                                  THelpTopic::build,
                                  __DELTA(THelpTopic)
                                );
#endif  // ifndef NO_TV_STREAMS

THelpTopic::THelpTopic() : TObject()
{
    paragraphs = 0;
    numRefs = 0;
    crossRefs = 0;
    width = 0;
    lastOffset = 0;
    lastLine = INT_MAX;
    lastParagraph = 0;
};

void THelpTopic::disposeParagraphs()
{
    TParagraph *p, *t;

    p = paragraphs;
    while (p != 0)
        {
        t = p;
        p = p->next;
        delete[] t->text;
        delete t;
        }
}


THelpTopic::~THelpTopic()
{
    TCrossRef *crossRefPtr;

    disposeParagraphs();
    if (crossRefs != 0)
       {
       crossRefPtr = (TCrossRef *)crossRefs;
       delete [] crossRefPtr;
       }
}

void THelpTopic::addCrossRef( TCrossRef ref )
{
    TCrossRef *p;
    TCrossRef *crossRefPtr;

    p =  new TCrossRef[numRefs+1];
    if (numRefs > 0)
        {
        crossRefPtr = crossRefs;
        memmove(p, crossRefPtr, numRefs * sizeof(TCrossRef));
        delete [] crossRefPtr;
        }
    crossRefs = p;
    crossRefPtr = crossRefs + numRefs;
    *crossRefPtr = ref;
    crossRefPtr->loc.x = -1;
    crossRefPtr->loc.y = -1;
    ++numRefs;
}


void THelpTopic::addParagraph( TParagraph *p )
{
    TParagraph  *pp, *back;

    if (paragraphs == 0)
        paragraphs = p;
    else
        {
        pp = paragraphs;
        back = pp;
        while (pp != 0)
            {
            back = pp;
            pp = pp->next;
            }
        back->next = p;
        }
    p->next = 0;
}

void THelpTopic::getCrossRef( int i, TPoint& loc, uchar& length,
         int& ref )
{
    TCrossRef *crossRefPtr;

    if ( crossRefs[0].loc.x == -1 ) {   // build cross-ref locations
      int paraOffset = 0;
      int curOffset = 0;
      int oldOffset = 0;
      int line = 0;
      TParagraph *p = paragraphs;
      crossRefPtr = crossRefs;
      for ( int j=0; j < numRefs; j++,crossRefPtr++ ) {
        int offset = crossRefPtr->offset;
        while (paraOffset + curOffset < offset) {
        char buffer[maxViewWidth];
          oldOffset = paraOffset + curOffset;
        wrapText(p->text, p->size, curOffset, p->wrap, buffer, sizeof(buffer));
          ++line;
          if (curOffset >= p->size) {
            paraOffset += p->size;
            p = p->next;
            curOffset = 0;
          }
        }
        crossRefPtr->loc.x = offset - oldOffset - 1;
        crossRefPtr->loc.y = line;
      }
    }

    crossRefPtr = crossRefs + i;
    length = crossRefPtr->length;
    ref = crossRefPtr->ref;
    loc = crossRefPtr->loc;
}

char *THelpTopic::getLine( int line, char *buffer, size_t bufsize )
{
    int offset, i;
    TParagraph *p;

    if (lastLine < line)
        {
        i = line;
        line -= lastLine;
        lastLine = i;
        offset = lastOffset;
        p = lastParagraph;
        }
    else
        {
        p = paragraphs;
        offset = 0;
        lastLine = line;
        }
    buffer[0] = 0;
    while (p != 0)
    {
        while (offset < p->size)
        {
            --line;
            wrapText(p->text, p->size, offset, p->wrap, buffer, bufsize);
            if (line == 0)
                {
                lastOffset = offset;
                lastParagraph = p;
                return buffer;
                }
        }
        p = p->next;
        offset = 0;
    }
    buffer[0] = 0;
    return buffer;
}

int THelpTopic::getNumCrossRefs()
{
    return numRefs;
}

int THelpTopic::numLines()
{
    int offset, lines;
    TParagraph *p;

    offset = 0;
    lines = 0;
    p = paragraphs;
    while (p != 0) {
      offset = 0;
      while (offset < p->size) {
        char buffer[maxViewWidth];
        ++lines;
        wrapText(p->text, p->size, offset, p->wrap, buffer, sizeof(buffer));
      }
      p = p->next;
    }
    return lines;
}

void THelpTopic::setCrossRef( int i, TCrossRef& ref )
{
    TCrossRef *crossRefPtr;

    if (i < numRefs)
        {
        crossRefPtr = crossRefs + i;
        *crossRefPtr = ref;
        }
}

void THelpTopic::setNumCrossRefs( int i )
{
    TCrossRef  *p, *crossRefPtr;

    if (numRefs == i)
        return;
    p = new TCrossRef[i];
    if (numRefs > 0)
        {
        crossRefPtr = crossRefs;
        if (i > numRefs)
            memmove(p, crossRefPtr, numRefs * sizeof(TCrossRef));
        else
            memmove(p, crossRefPtr, i * sizeof(TCrossRef));

        delete [] crossRefPtr;
        }
    crossRefs = p;
    numRefs = (ushort)i;
}


void THelpTopic::setWidth( int aWidth )
{
    if ( numRefs > 0 ) crossRefs[0].loc.x = -1;
    width = aWidth;
}

inline Boolean isBlank( char ch )
{
    if (isspace(uchar(ch)))
        return True;
    else
        return False;
}

inline int scan( char *p, int offset, char c)
{
    char *temp1, *temp2;

    temp1 = p + offset;
    temp2 = strchr(temp1, c);
    if (temp2 == 0)
       return maxViewWidth;
    else
       {
       if ((temp2 - temp1) <= maxViewWidth )
         return uint32((temp2 - temp1) + 1);
       else
         return maxViewWidth;
       }
}

char *THelpTopic::wrapText( char *text, int size,
          int& offset, Boolean wrap, char *line, size_t lineBufLen )
{
    int i;

    i = scan(text, offset, '\n');
    if (i + offset > size ) i = size - offset;
    if ( (i > width) && wrap ) {
      i = offset + width;
      if ( i > size ) {
        i = size;
      } else {
        while( (i > offset) && !(isspace((uchar)text[i])) )
          --i;
        if ( i == offset ) {
          i = offset + width;
          while ( (i < size) && !isspace((uchar)text[i]) )
            ++i;
          if ( i < size ) ++i;        // skip Blank
        } else {
          ++i;
        }
      }
      if ( i == offset ) i = offset + width;
      i -= offset;
    }
    qstrncpy(line, text+offset, qmin((size_t)(i+1),lineBufLen));
    // remove the last '\n'
    uint32 len = (uint32)strlen(line);
    offset += len;
    if ( len > 0 && line[len-1] == '\n' )
      line[--len] = '\0';
    return line;
}

// THelpIndex

#ifndef NO_TV_STREAMS
const char * const THelpIndex::name = "THelpIndex";

void THelpIndex::write( opstream& os )
{
    int32 *indexArrayPtr;

    os << size;
    for (int i = 0; i < size; ++i)
        {
        indexArrayPtr = index + i;
        os << *indexArrayPtr;
        }
}

void *THelpIndex::read( ipstream& is )
{
    int32 *indexArrayPtr;

    is >> size;
    if (size == 0)
        index = 0;
    else
        {
        index =  new int32[size];
        for(int i = 0; i < size; ++i)
            {
            indexArrayPtr = index + i;
            is >> *indexArrayPtr;
            }
        }
    return this;
}

TStreamable *THelpIndex::build()
{
    return new THelpIndex( streamableInit );
}

TStreamableClass RHelpIndex( THelpIndex::name,
                                  THelpIndex::build,
                                  __DELTA(THelpIndex)
                            );

#else
//Added to file by Uri bechar
void THelpIndex::write( FILE *fp ){

    int32 *indexArrayPtr;

    fwrite(&size,sizeof(size),1,fp);


    for(int i = 0; i < size; ++i)
    {
        indexArrayPtr = index + i;
        fwrite(indexArrayPtr, sizeof(int32),1,fp);
    }


   }

void THelpIndex::read( FILE *fp ){

    int32 *indexArrayPtr;

    fread(&size,sizeof(size),1,fp);

    if (size == 0)
        index = 0;
    else
        {
        index =  new int32[size];

        for(int i = 0; i < size; ++i)
            {
            indexArrayPtr = index + i;
            fread(indexArrayPtr, sizeof(int32),1,fp);
            }
        }

   }


#endif  // ifndef NO_TV_STREAMS

THelpIndex::~THelpIndex()
{
       delete [] index;
}

THelpIndex::THelpIndex(void): TObject ()
{
    size = 0;
    index = 0;
}

int32 THelpIndex::position(int i)
{
    int32 *indexArrayPtr;

    if (i < size)
        {
        indexArrayPtr = index + i;
        return (*indexArrayPtr);
        }
    else
        return -1;
}

void THelpIndex::add( int i, int32 val )
{
    int delta = 10;
    int32 *p;
    int newSize;
    int32 *indexArrayPtr;

    if (i >= size)
        {
        newSize = (i + delta) / delta * delta;
        p = new int32[newSize];
        if (p != 0)
            {
            memmove(p, index, size * sizeof(int32));
            memset(p+size, 0xFF, (newSize - size) * sizeof(int32));
            }
        if (size > 0)
            {
            delete [] index;
            }
        index = p;
        size = (ushort)newSize;
        }
    indexArrayPtr = index + i;
    *indexArrayPtr = val;
}

// THelpFile

//--------------------------------------------------------------------------
#ifndef NO_TV_STREAMS
void THelpTopic::readParagraphs( ipstream& s )
{
    ushort i, size;
    TParagraph **pp;
    ushort temp;

    s >> i;
    pp = &paragraphs;
    while ( i > 0)
    {
        s >> size;
        *pp = new TParagraph;
        (*pp)->text = new char[size];
        (*pp)->size = (ushort) size;
    s >> temp;
        (*pp)->wrap = Boolean(temp);
        s.readBytes((*pp)->text, (*pp)->size);
        pp = &((*pp)->next);
        --i;
    }
    *pp = 0;
}

void THelpTopic::readCrossRefs( ipstream& s )
{
    int i;
    TCrossRef *crossRefPtr;

    s >> numRefs;
    crossRefs = new TCrossRef[numRefs];
    for (i = 0; i < numRefs; ++i)
        {
        crossRefPtr = (TCrossRef *)crossRefs + i;
        s.readBytes(crossRefPtr, sizeof(TCrossRef));
        }
}

void THelpTopic::writeParagraphs( opstream& s )
{
    ushort i;
    TParagraph  *p;
    ushort temp;

    p = paragraphs;
    for (i = 0; p != 0; ++i)
        p = p->next;
    s << i;
    for(p = paragraphs; p != 0; p = p->next)
        {
        s << p->size;
        temp = int(p->wrap);
        s << temp;
        s.writeBytes(p->text, p->size);
        }
}


void THelpTopic::writeCrossRefs( opstream& s )
{
    int i;
    TCrossRef *crossRefPtr;

    s << numRefs;
    if (crossRefHandler == notAssigned)
        {
        for(i = 0; i < numRefs; ++i)
            {
            crossRefPtr = crossRefs + i;
            s << crossRefPtr->ref << crossRefPtr->offset << crossRefPtr->length;
            }
        }
    else
        for (i = 0; i < numRefs; ++i)
            {
            crossRefPtr = crossRefs + i;
            crossRefHandler(s, crossRefPtr->ref);
            s << crossRefPtr->offset << crossRefPtr->length;
            }
}

THelpFile::THelpFile( fpstream&  s )
{
    int32 magic;
    int handle;
    int32 size;

    magic = 0;
    s.seekg(0);
    handle = s.rdbuf()->fd();
    size = filelength(handle);
    if (size > sizeof(magic))
        s >> magic;
    if (magic != magicHeader)
        {
        indexPos = 12;
        s.seekg(indexPos);
        index =  new THelpIndex;
        modified = True;
        }
    else
        {
        s.seekg(8);
        s >> indexPos;
        s.seekg(indexPos);
        s >> index;
        modified = False;
        }
    stream = &s;
}

THelpFile::~THelpFile(void)
{
    int32 magic, size;
    int handle;

    if (modified == True)
        {
        stream->seekp(indexPos);
        *stream << index;
        stream->seekp(0);
        magic = magicHeader;
        handle = stream->rdbuf()->fd();
        size = filelength(handle) - 8;
        *stream << magic;
        *stream << size;
        *stream << indexPos;
        }
    delete stream;
    delete index;
}

THelpTopic *THelpFile::getTopic( int i )
{
    int32 pos;
    THelpTopic *topic;

    pos = index->position(i);
    if (pos > 0 )
        {
        stream->seekg(pos);
        *stream >> topic;
        return topic;
        }
    else return(invalidTopic());
}

void THelpFile::putTopic( THelpTopic *topic )
{
    stream->seekp(indexPos);
    *stream << topic;
    indexPos = stream->tellp();
    modified = True;
}

void THelpTopic::write( opstream& os )
{
    writeParagraphs( os );
    writeCrossRefs( os );

}

void *THelpTopic::read( ipstream& is )
{
    readParagraphs( is );
    readCrossRefs( is );
    width = 0;
    lastLine = INT_MAX;
    return this;
}

void notAssigned( opstream& , int )
{
}

//--------------------------------------------------------------------------
#else   // ifndef NO_TV_STREAMS

#ifndef __IDA__

void THelpTopic::readParagraphs(FILE *fp)
{
    ushort i, size;
    TParagraph **pp;
    ushort temp;

    fread(&i,sizeof(i),1,fp);
    pp = &paragraphs;
    while ( i > 0)
    {
        fread(&size,sizeof(size),1,fp);
        *pp = new TParagraph;
        (*pp)->text = new char[size];
        (*pp)->size = (ushort) size;
        fread(&temp,sizeof(temp),1,fp);
        (*pp)->wrap = Boolean(temp);
        fread((*pp)->text,(*pp)->size,1,fp);
        pp = &((*pp)->next);
        --i;
    }
    *pp = 0;
}

void THelpTopic::readCrossRefs(FILE *fp)
{
    int i;
    TCrossRef *crossRefPtr;

    fread(&numRefs,sizeof(numRefs),1,fp);
    crossRefs = new TCrossRef[numRefs];
    for (i = 0; i < numRefs; ++i)
        {
        crossRefPtr = (TCrossRef *)crossRefs + i;
        fread(crossRefPtr,sizeof(TCrossRef),1,fp);
        }
}

void THelpTopic::writeParagraphs(FILE *fp)
{
    ushort i;
    TParagraph  *p;
    ushort temp;

    p = paragraphs;
    for (i = 0; p != 0; ++i)
        p = p->next;
    fwrite(&i,sizeof(i),1,fp);
    for(p = paragraphs; p != 0; p = p->next)
        {
        fwrite(&p->size,sizeof(p->size),1,fp);
        temp = ushort(p->wrap);
        fwrite(&temp,sizeof(temp),1,fp);
        fwrite(p->text,p->size,1,fp);
        }
}


void THelpTopic::writeCrossRefs(FILE *fp)
{
    int i;
    TCrossRef *crossRefPtr;

    fwrite(&numRefs,sizeof(numRefs),1,fp);
    if (crossRefHandler == notAssigned)
        {
        for(i = 0; i < numRefs; ++i)
            {
            crossRefPtr = crossRefs + i;
            fwrite(&crossRefPtr->ref,   sizeof(crossRefPtr->ref),   1,fp);
            fwrite(&crossRefPtr->offset,sizeof(crossRefPtr->offset),1,fp);
            fwrite(&crossRefPtr->length,sizeof(crossRefPtr->length),1,fp);
            }
        }
    else
        for (i = 0; i < numRefs; ++i)
            {
            crossRefPtr = crossRefs + i;
            crossRefHandler(fp, crossRefPtr->ref);
            fwrite(&crossRefPtr->offset,sizeof(crossRefPtr->offset),1,fp);
            fwrite(&crossRefPtr->length,sizeof(crossRefPtr->length),1,fp);
            }
}

THelpFile::THelpFile( FILE *fp )
{
    int32 magic;
    int32 size;

    magic = 0;
    fseek(fp,0,SEEK_END);
    size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    if (size > (int)sizeof(magic))
        fread(&magic,sizeof(magic),1,fp);
    if (magic != magicHeader)
        {
        indexPos = 12;
        fseek(fp,indexPos,SEEK_SET);
        index =  new THelpIndex;
        modified = True;
        }
    else
        {
        fseek(fp,8,SEEK_SET);
        fread(&indexPos,sizeof(indexPos),1,fp);
        fseek(fp,indexPos+13,SEEK_SET); //must add 13 for the rigth position

        index =  new THelpIndex;  //patched by Uri Bechar
        index->read(fp);
        modified = False;
        }
    hfp = fp;
}

THelpFile::~THelpFile(void)
{
    int32 magic, size;

    if (modified == True)
        {
        fseek(hfp,indexPos,SEEK_SET);
        fwrite(&index,sizeof(index),1,hfp);
        fseek(hfp,0,SEEK_END);
        size = ftell(hfp) - 8;
        fseek(hfp,0,SEEK_SET);
        magic = magicHeader;
        fwrite(&magic,sizeof(magic),1,hfp);
        fwrite(&size,sizeof(size),1,hfp);
        fwrite(&indexPos,sizeof(indexPos),1,hfp);
        }
    fclose(hfp);
    delete index;
}

void THelpTopic::write(FILE *fp)
{
    writeParagraphs(fp);
    writeCrossRefs(fp);

}

void * THelpTopic::read(FILE *fp)
{
    readParagraphs(fp);
    readCrossRefs(fp);
    width = 0;
    lastLine = INT_MAX;
    return this;
}

THelpTopic *THelpFile::getTopic( int i )
{
    int32 pos;
    THelpTopic *topic;

    pos = index->position(i);
    if (pos > 0 )
        {
        fseek(hfp,pos + 13,SEEK_SET); //must add 13 for the rigth position
        topic = new THelpTopic;
        topic->read(hfp);
        return topic;
        }
    else return(invalidTopic());
}

void THelpFile::putTopic( THelpTopic *topic )
{
    fseek(hfp,indexPos,SEEK_SET);
    topic->write(hfp);
    indexPos = ftell(hfp);
    modified = True;
}

#else   // The following code is for IDA which does not use TVision help files

THelpFile::THelpFile( FILE * )
{
}

THelpFile::~THelpFile(void)
{
}

extern "C" void ActionKey(const char *str, char *buf, size_t bufsize);

// return the new text (possibly reallocated, if not enough space in the original buffer)
static char *addxrefs( THelpTopic *topic, char *text, size_t textsize ) { /* ig 22.04.93 */
  char *base = text;
  char *tend = text + textsize;
  int base_size = (uint32)strlen(base) + 1;
  while ( (text=strchr(text,'@')) != NULL ) {
    int y;
    text++;
    if ( text[0] == '<' ) {                     /* User function, 07.11.93 */
      char *end = strchr(text,'>');
      if ( end != NULL )
      {
        *end++ = '\0';
        char actionkey[MAXSTR];
        ActionKey(text+1, actionkey, sizeof(actionkey));
        int resize = uint32(text + strlen(actionkey) - end); // how many bytes must we add to the string ?
        if (resize > 0) // if the buffer needs resizing,
        {
          char *base_old = base;
          // resize it
          base_size += resize;
          base = (char*) realloc(base, base_size);
          // adjust all pointers to point in this new resized buffer
          text = base + (text - base_old);
          end  = base + (end  - base_old);
          memmove(end+resize, end, strlen(end)+1); // shift to the right, to create some space in the buffer
          memcpy(text-1, actionkey, strlen(actionkey)); // and insert the string (without its '\0')
        }
        else
        {
          text = qstpncpy(text-1,actionkey,tend-text+1);
          memmove(text,end,strlen(end)+1);
        }
        continue;
      }
    }
    if ( sscanf(text,"0:%d",&y) != 1 ) continue;
    char *start = strchr(text,'[');
    char *end = strchr(text,']');
    if ( start == NULL || end == NULL ) continue;
    int len = (int)(end-start)-1;
    if ( len <= 0 || len > 80 ) continue;
    start++;
    end++;
    memmove(text-1,start,len);
    memmove(text-1+len,end,strlen(end)+1);
    TCrossRef ref;
    ref.ref    = (ushort)y;
    ref.offset = (ushort)(int)(text-base);
    ref.length = (uchar)len;
    topic->addCrossRef(ref);
  }
  return base;
}

THelpTopic *THelpFile::getTopic( int i )
{
    char *text;
    while ( 1 ) {
      switch ( i ) {
        case 0:
          text = NULL;
          break;
        case 0xFFFE:
          text = qstrdup(tv_dynhelp);
          break;
        default:
          text = qivalue(i);
          break;
      }
      if ( text == NULL )
      {
        char buf[80];
        qsnprintf(buf, sizeof(buf),
                  "\nNo help available for this context.\n"
                  "  (context number: %d)",
                  i);
        text = qstrdup(buf);
      }
      int tmp;
      if ( sscanf(text,"@%d:%d",&tmp,&i) != 2 ) break;
      qfree(text);
    }
    THelpTopic *topic = new THelpTopic;
    TParagraph *para  = new TParagraph;
    size_t len = strlen(text);
    Boolean Wrap = False;
    if ( strnicmp(text,"WRAP",4) == 0 ) {
      int ext = ( text[4] == '\n' );
      memmove(text,text+4+ext,len-4-ext+1);
      Wrap = True;
      char *ptr = text;
      while ( (ptr=strchr(ptr+1,'\n')) != NULL ) {
        if ( ptr[1] != ' ' && ptr[1] != '\n' && ptr[-1] != '\n' ) ptr[0] = ' ';
      }
    }
    text = addxrefs(topic, text, len+1);
    para->text = text;
    para->size = (ushort)strlen(text);
    para->wrap = Wrap;
    para->next = 0;
    topic->addParagraph(para);
    return topic;
}
#endif

void notAssigned( FILE * , int )
{
}

#endif  // ifndef NO_TV_STREAMS


THelpTopic *THelpFile::invalidTopic()
{
    THelpTopic *topic;
    TParagraph *para;
    static const char invalidText[] = "\n No help available in this context.";

    topic =  new THelpTopic;
    para =  new TParagraph;
    para->text = newStr(invalidText);
    para->size = (ushort)strlen(invalidText);
    para->wrap = False;
    para->next = 0;
    topic->addParagraph(para);
    return topic;
}

void THelpFile::recordPositionInIndex( int i )
{
    index->add(i, indexPos);
    modified = True;
}


