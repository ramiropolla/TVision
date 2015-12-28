/* $Id$ */
/*
 * fdlisten.cpp
 *
 * Copyright (c) 2009 Jeremy Cooper.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _TVISION_FDLISTEN_H
#define _TVISION_FDLISTEN_H

#include <poll.h>
#include <vector>

//
// A simple class for keeping track of file-descriptor listeners.
//
class FdListener
{
public:
  FdListener(void (*fn)(void *, int, int), int fd_, void *cookie_) :
    fd(fd_), cookie(cookie_), callbackFn(fn),
    flags(TEventQueue::FLAGS_READ|TEventQueue::FLAGS_WRITE) {};
  inline int getFd(void) { return fd; }
  inline void *getCookie(void) { return cookie; }
  inline void setFlags(int flags_) { flags = flags_; }
  inline int getFlags(void) { return flags; }
  inline void callback(int event_type) {
    if (event_type & flags)
       callbackFn(cookie, fd, event_type);
   }

protected:
  int fd;
  int flags;
  void *cookie;
  void (*callbackFn)(void *, int, int);
};

void processQueuedFdActions(void);
void notifyFdListeners(int fd, int events);

extern std::vector<struct pollfd> *pollList;
extern int issuingFdCallbacks;

#endif