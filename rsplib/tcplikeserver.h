/*
 * The rsplib Prototype -- An RSerPool Implementation.
 * Copyright (C) 2005 by Thomas Dreibholz, dreibh@exp-math.uni-essen.de
 *
 * $Id$
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Contact: rsplib-discussion@sctp.de
 *          dreibh@iem.uni-due.de
 *
 */

#ifndef TCPLIKESERVER_H
#define TCPLIKESERVER_H

#include "rserpool.h"
#include "cpprspserver.h"
#include "thread.h"


struct TagItem;
class TCPLikeServer;

class TCPLikeServerList : public TDMutex
{
   public:
   TCPLikeServerList();
   ~TCPLikeServerList();
   void add(TCPLikeServer* thread);
   void remove(TCPLikeServer* thread);
   void removeFinished();
   void removeAll();

   size_t getThreads();
   double getTotalLoad();

   private:
   friend class TCPLikeServer;
   struct ThreadListEntry {
      ThreadListEntry* Next;
      TCPLikeServer*  Object;
   };
   ThreadListEntry*   ThreadList;
   size_t             Threads;
   unsigned long long LoadSum;
};


class TCPLikeServer : public TDThread
{
   public:
   TCPLikeServer(int rserpoolSocketDescriptor);
   ~TCPLikeServer();

   inline bool hasFinished() const {
      return(Finished);
   }
   inline bool isShuttingDown() const {
      return(Shutdown);
   }
   inline TCPLikeServerList* getServerList() const {
      return(ServerList);
   }
   inline void setServerList(TCPLikeServerList* serverList) {
      ServerList = serverList;
   }
   void shutdown();

   double getLoad() const;
   void setLoad(double load);

   static void poolElement(const char*          programTitle,
                           const char*          poolHandle,
                           struct rsp_info*     info,
                           struct rsp_loadinfo* loadinfo,
                           void*                userData,
                           TCPLikeServer*       (*threadFactory)(int sd, void* userData),
                           struct TagItem*      tags = NULL);

   protected:
   int RSerPoolSocketDescriptor;

   protected:
   virtual EventHandlingResult startup();
   virtual void finish(EventHandlingResult result);
   virtual EventHandlingResult handleMessage(const char* buffer,
                                             size_t      bufferSize,
                                             uint32_t    ppid,
                                             uint16_t    streamID);
   virtual EventHandlingResult handleCookieEcho(const char* buffer, size_t bufferSize);
   virtual EventHandlingResult handleNotification(const union rserpool_notification* notification);

   private:
   void run();

   TCPLikeServerList* ServerList;
   unsigned int       Load;
   bool               IsNewSession;
   bool               Shutdown;
   bool               Finished;
};


#endif
