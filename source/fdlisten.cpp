/* $Id: //depot/ida/tvision/source/fdlisten.cpp#1 $ */
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
#define Uses_TEventQueue
#include <tv.h>
#include <map>
#include <list>

#include "fdlisten.h"

static void evaluatePollFlags(void);

struct fd_callback_request {
  bool add;
  int fd;
  void *cookie;
  void (*fn)(void *, int, int);
};
typedef std::list<FdListener *> FdListenerList;
typedef std::map<int, FdListenerList *> FdListenerMap;
typedef std::list<struct fd_callback_request> FdActionList;

//
// A registry of callbacks to make when activity occurs on certain
// file descriptors.
//
static FdListenerMap fdListeners;

//
// A list of callbacks to register/unregister after the current callback
// loop completes.
//
// (This list allows us to safely defer requests to remove and
// add callbacks while still in the middle of a callback issuing loop).
//
static FdActionList queuedFdActions;

//
// A simple mutex that allows us to determine if we're in the middle
// of a callback loop.
//
int issuingFdCallbacks;

//
// Add a listener for events on a specific file descriptor.
//
// This facility allows for event-driven callbacks on UNIX file descriptors
// in a consistent manner that prevents the need for separate threads.
//
// The callback function provided will be called whenever there is
// activity on the file descriptor.  The file descriptor, cookie and
// a description of the activity will be provided as arguments.
//
// Returns 0: ok - Callback registered
//        <0: failure - Could not register callback
//
int TEventQueue::addFdListener(int fd, void (*cb)(void *, int, int),
                               void *cookie )
{
  //
  // Is the callback registry currently busy?
  //
  if (issuingFdCallbacks > 0) {
    //
    // The callback registry is currently busy.
    // Queue this request for later.
    //
    struct fd_callback_request request;
    request.add = true;
    request.fd = fd;
    request.fn = cb;
    request.cookie = cookie;
    queuedFdActions.push_back(request);
    return 0;
  }

  FdListenerList *l;
  
  //
  // Is there an existing listener list for this file descriptor?
  //
  FdListenerMap::iterator i = fdListeners.find(fd);
  if (i != fdListeners.end()) {
    //
    // Found an existing listener list, use it.
    //
    l = i->second;
  } else {
    //
    // No existing listener list for this descriptor, create one
    // and insert it into the map.
    //
    l = new std::list<FdListener *>;
    fdListeners[fd] = l;
    
    //
    // Likewise, extend the global polling event list to include
    // this new file descriptor.
    //
    struct pollfd newfd;
    newfd.fd = fd;
    newfd.events = 0;
    pollList->push_back(newfd);
  }
  
  //
  // Add the listener to the list.
  //
  l->push_front(new FdListener(cb, fd, cookie));
  
  //
  // Evaluate the polling flags to use for each file
  // descriptor since there is a new listener.
  //
  evaluatePollFlags();
  return 0;
}


//
// Remove a callback for a file-descriptor and cookie pair.
//
int TEventQueue::removeFdListener(int fd, void *cookie)
{
  //
  // Is the callback registry currently busy?
  //
  if (issuingFdCallbacks > 0) {
    //
    // The callback registry is currently busy.
    // Queue this request for later.
    //
    struct fd_callback_request request;
    request.add = false;
    request.fd = fd;
    request.cookie = cookie;
    queuedFdActions.push_back(request);
    return 0;
  }

  //
  // Callback registry is open for modification.
  //
  FdListenerList *l;
  bool found = false;

  //
  // Find the listener list for this file descriptor.
  //
  FdListenerMap::iterator i = fdListeners.find(fd);
  if (i == fdListeners.end())
    //
    // Not found.
    //
    return -1;
  l = i->second;
  
  //
  // Remove all listeners matching the cookie.
  //
  FdListenerList::iterator j = l->begin();
  while (j != l->end()) {
    //
    // Save the next entry, because we may delete the
    // current one.
    //
    FdListener *listener = *j;
    
    if (listener->getCookie() == cookie) {
      //
      // Found a matching entry.  Delete the FdListener
      // behind it.
      //
      found = true;
      delete listener;
      
      //
      // Remove the entry.
      //
      l->erase(j);
      
      //
      // Since we've modified the list, start again.
      //
      j = l->begin();
    }
  }
  
  //
  // Did we find anything?
  //
  if (!found)
    return -1;
  
  //
  // Did we just remove the last listener for a file descriptor?
  //
  if (l->size() == 0) {
    //
    // We did.
    //
    
    //
    // Remove the now-empty list from the map.
    //
    fdListeners.erase(i);
    
    //
    // Destroy the list.
    //
    delete l;
    
    //
    // Remove this fd from the global polling list.
    //
    for (int c; c < pollList->size(); c++) {
      if ((*pollList)[c].fd == fd) {
        pollList->erase(pollList->begin() + c);
        break;
      }
    }
  } else {
    //
    // No.  There are other listeners still listening to this descriptor.
    // Recalculate the polling flags, they may have changed with the
    // absence of this listener.
    //
    evaluatePollFlags();
  }
}

//
// Change the events that a listener is interested in.
//
int TEventQueue::setFdListenerFlags(int fd, void *cookie, int newflags)
{
  FdListenerList *l;
  bool found = false;

  //
  // Find the listener list for this file descriptor.
  //
  FdListenerMap::iterator i = fdListeners.find(fd);
  if (i == fdListeners.end())
    //
    // Not found.
    //
    return -1;
  l = i->second;
  
  //
  // Find the listener(s) matching the cookie and set their
  // flags.
  //
  FdListenerList::iterator j = l->begin();
  for (;j != l->end(); j++) {
    FdListener *listener = *j;
    if (listener->getCookie() == cookie) {
      //
      // Found a matching entry.  Set its flags
      //
      found = true;
      listener->setFlags(newflags);
    }
  }
  
  //
  // Did we find anything?
  //
  if (!found)
    return -1;
  
  //
  // Recalculate the polling flags as they may have changed with this
  // listener's request.
  //
  evaluatePollFlags();
}
        
void notifyFdListeners(int fd, int events)
{
  //
  // Is anyone else interested in this descriptor?
  //
  FdListenerMap::iterator entry = fdListeners.find(fd);
  if (entry == fdListeners.end())
    return;
        
  //
  // Iterate through all interested parties and notify them.
  //
  FdListenerList *list = entry->second;
  FdListenerList::iterator list_entry = list->begin();
  int revents = 0;
  if (events & POLLIN)
    revents |= TEventQueue::FLAGS_READ;
  if (events & POLLOUT)
    revents |= TEventQueue::FLAGS_WRITE;
  for (;list_entry != list->end(); list_entry++)
    (*list_entry)->callback(revents);
}

//
// Evaluate the set of listeners waiting for events on each file descriptor
// and determine globally what events to listen for for that file descriptor.
//
static void evaluatePollFlags(void)
{
  //
  // Iterate over each file descriptor in the listeners map.
  //
  FdListenerMap::iterator fdList = fdListeners.begin();
  for (;fdList != fdListeners.end(); fdList++) {
    //
    // Gather up the the total event flags desired by all listeners
    // on this descriptor.
    //
    int totalFlags = 0;
    int fd = fdList->first;
    FdListenerList::iterator listener = fdList->second->begin();
    for (;listener != fdList->second->end(); listener++)
      totalFlags |= (*listener)->getFlags();
    //
    // Translate TEventQueue flags to poll() flags.
    //
    int pflags = 0;
    if (totalFlags & TEventQueue::FLAGS_READ)
      pflags |= POLLIN;
    if (totalFlags & TEventQueue::FLAGS_WRITE)
      pflags |= POLLOUT;
    //
    // Find the poll entry for this descriptor and set
    // its flags.
    //
    for (int i = 0; i < pollList->size(); i++) {
      if ((*pollList)[i].fd == fd) {
        (*pollList)[i].events = pflags;
        break;
      }
    }
  }
}

//
// Process any file-descriptor callback addition or removal
// requests that were queued up while we were issuing callbacks from
// the registry itself.
//
void processQueuedFdActions(void)
{
  FdActionList::iterator action = queuedFdActions.begin();
  for (;action != queuedFdActions.end(); action++) {
    if (action->add)
      TEventQueue::addFdListener(action->fd, action->fn, action->cookie);
    else
      TEventQueue::removeFdListener(action->fd, action->cookie);
  }
  
  //
  // Empty the list.
  //
  queuedFdActions.erase(queuedFdActions.begin(), queuedFdActions.end());
}
