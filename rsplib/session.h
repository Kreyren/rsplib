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

#ifndef SESSION_H
#define SESSION_H

#include "tdtypes.h"
#include "rserpool.h"
#include "poolhandle.h"
#include "tagitem.h"
#include "leaflinkedredblacktree.h"

#include <ext_socket.h>


#ifdef __cplusplus
extern "C" {
#endif


struct Session
{
   struct LeafLinkedRedBlackTreeNode SessionIDNode;
   struct LeafLinkedRedBlackTreeNode AssocIDNode;

   sctp_assoc_t                      AssocID;
   rserpool_session_t                SessionID;

   struct PoolHandle                 Handle;
   uint32_t                          ConnectedPE;
   bool                              IsIncoming;
   bool                              IsFailed;
   unsigned long long                ConnectionTimeStamp;

   unsigned int                      PendingNotifications;
   unsigned int                      EventMask;

   void*                             Cookie;
   size_t                            CookieSize;
   void*                             CookieEcho;
   size_t                            CookieEchoSize;

   unsigned long long                ConnectTimeout;
   unsigned long long                HandleResolutionRetryDelay;

   struct TagItem*                   Tags;

   char                              StatusText[128];
};

#define SNT_FAILOVER_NECESSARY ((unsigned int)1 << 0)
#define SNT_FAILOVER_COMPLETE  ((unsigned int)1 << 1)
#define SNT_SESSION_ADD        ((unsigned int)1 << 2)
#define SNT_SESSION_REMOVE     ((unsigned int)1 << 3)

#define SNT_NOTIFICATION_MASK (SNT_FAILOVER_NECESSARY|SNT_FAILOVER_COMPLETE|SNT_SESSION_ADD|SNT_SESSION_REMOVE)


#ifdef __cplusplus
}
#endif

#endif
