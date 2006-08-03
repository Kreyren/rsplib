/*
 *  $Id$
 *
 * RSerPool implementation.
 *
 * Realized in co-operation between Siemens AG
 * and University of Essen, Institute of Computer Networking Technology.
 *
 * Acknowledgement
 * This work was partially funded by the Bundesministerium fr Bildung und
 * Forschung (BMBF) of the Federal Republic of Germany (Förderkennzeichen 01AK045).
 * The authors alone are responsible for the contents.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * There are two mailinglists available at http://www.sctp.de/rserpool.html
 * which should be used for any discussion related to this implementation.
 *
 * Contact: rsplib-discussion@sctp.de
 *          dreibh@exp-math.uni-essen.de
 *
 * Purpose: Registrar Table
 *
 */


#include "tdtypes.h"
#include "registrartable.h"
#include "rserpool-internals.h"
#include "rserpoolmessage.h"
#include "asapinstance.h"
#include "leaflinkedredblacktree.h"
#include "netutilities.h"
#include "randomizer.h"
#include "loglevel.h"

#include <ext_socket.h>
#include <sys/time.h>



/* Maximum number of simultaneous registrar connection trials */
#define MAX_SIMULTANEOUS_REQUESTS 3


struct RegistrarAssocIDNode
{
   struct SimpleRedBlackTreeNode Node;
   sctp_assoc_t                      AssocID;
};


/* ###### Registrar announce callback  ######################################### */
static void handleRegistrarAnnounceCallback(struct RegistrarTable* registrarTable,
                                            int                    sd)
{
   struct RSerPoolMessage*        message;
   struct ST_CLASS(PeerListNode)* peerListNode;
   union sockaddr_union           senderAddress;
   socklen_t                      senderAddressLength;
   char                           buffer[1024];
   ssize_t                        received;
   unsigned int                   result;
   size_t                         i;

   LOG_VERBOSE
   fputs("Trying to receive registrar announce...\n",  stdlog);
   LOG_END

   senderAddressLength = sizeof(senderAddress);
   received = ext_recvfrom(sd,
                           (char*)&buffer , sizeof(buffer), 0,
                           (struct sockaddr*)&senderAddress,
                           &senderAddressLength);
   if(received > 0) {
      result = rserpoolPacket2Message((char*)&buffer,
                                      &senderAddress,
                                      0,
                                      PPID_ASAP,
                                      received, sizeof(buffer),
                                      &message);
      if(message != NULL) {
         if(result == RSPERR_OKAY) {
            if(message->Type == AHT_SERVER_ANNOUNCE) {
               if(message->Error == RSPERR_OKAY) {
                  LOG_VERBOSE3
                  fputs("RegistrarAnnounce from ",  stdlog);
                  address2string((struct sockaddr*)&senderAddress,
                                 (char*)&buffer, sizeof(buffer), true);
                  fputs(buffer, stdlog);
                  fputs(" received\n",  stdlog);
                  LOG_END

                  result = ST_CLASS(peerListManagementRegisterPeerListNode)(
                              &registrarTable->RegistrarList,
                              message->RegistrarIdentifier,
                              PLNF_DYNAMIC,
                              message->TransportAddressBlockListPtr,
                              getMicroTime(),
                              &peerListNode);
                  if(result == RSPERR_OKAY) {
                     registrarTable->LastAnnounceHeard = getMicroTime();
                     ST_CLASS(peerListManagementRestartPeerListNodeExpiryTimer)(
                        &registrarTable->RegistrarList,
                        peerListNode,
                        registrarTable->RegistrarAnnounceTimeout);
                  }
                  else {
                     LOG_ERROR
                     fputs("Unable to add new peer: ",  stdlog);
                     rserpoolErrorPrint(result, stdlog);
                     fputs("\n",  stdlog);
                     LOG_END
                  }

                  i = ST_CLASS(peerListManagementPurgeExpiredPeerListNodes)(
                        &registrarTable->RegistrarList,
                        getMicroTime());
                  LOG_VERBOSE4
                  fprintf(stdlog, "Purged %u out-of-date peer list nodes. Peer List:\n",  (unsigned int)i);
                  ST_CLASS(peerListManagementPrint)(&registrarTable->RegistrarList, stdlog, PLPO_PEERS_INDEX|PLNPO_TRANSPORT);
                  LOG_END
               }
            }
         }
         rserpoolMessageDelete(message);
      }
   }
}


/* ###### Registrar announce callback for FDCallback ######################## */
/* This function is only called to "flush" the socket's buffer into the
   cache when there is *no* PR hunt in progress. The PR hunt itself may
   *not* use the Dispatcher's select() procedure -> This may case
   recursive invocation! Therefore, registrarTableFindRegistrar() simply
   calls ext_select() and manually calls handleRegistrarAnnounceCallback()
   to read from the announce socket. */
static void registrarAnnouceFDCallback(struct Dispatcher* dispatcher,
                                       int                fd,
                                       unsigned int       eventMask,
                                       void*              userData)
{
   LOG_VERBOSE4
   fputs("Entering registrarAnnouceFDCallback()...\n", stdlog);
   LOG_END
   handleRegistrarAnnounceCallback((struct RegistrarTable*)userData, fd);
   LOG_VERBOSE4
   fputs("Leaving registrarAnnouceFDCallback()\n", stdlog);
   LOG_END
}


/* ###### Compare two registrar association IDs ########################## */
static int registrarAssocIDNodeComparison(const void* node1, const void* node2)
{
   const struct RegistrarAssocIDNode* a1 = (const struct RegistrarAssocIDNode*)node1;
   const struct RegistrarAssocIDNode* a2 = (const struct RegistrarAssocIDNode*)node2;
   if(a1->AssocID < a2->AssocID) {
      return(-1);
   }
   else if(a1->AssocID > a2->AssocID) {
      return(1);
   }
   return(0);
}


/* ###### Print registrar association ID ################################# */
static void registrarAssocIDNodePrint(const void* node, FILE* fd)
{
   const struct RegistrarAssocIDNode* assocIDNode = (const struct RegistrarAssocIDNode*)node;
   fprintf(fd, "%u ", (unsigned int)assocIDNode->AssocID);
}


/* ###### Constructor ####################################################### */
struct RegistrarTable* registrarTableNew(struct Dispatcher*          dispatcher,
                                         const union sockaddr_union* registrarAnnounceAddress,
                                         struct TagItem*             tags)
{
   union sockaddr_union*  announceAddress;
   union sockaddr_union   defaultAnnounceAddress;
   struct RegistrarTable* registrarTable = (struct RegistrarTable*)malloc(sizeof(struct RegistrarTable));

   if(registrarTable != NULL) {
      registrarTable->Dispatcher          = dispatcher;
      registrarTable->LastAnnounceHeard   = 0;
      registrarTable->OutstandingConnects = 0;
      registrarTable->AnnounceSocket      = -1;
      ST_CLASS(peerListManagementNew)(&registrarTable->RegistrarList, NULL, 0, NULL, NULL);
      simpleRedBlackTreeNew(&registrarTable->RegistrarAssocIDList, registrarAssocIDNodePrint, registrarAssocIDNodeComparison);


      /* ====== ASAP Instance settings ==================================== */
      registrarTable->RegistrarConnectMaxTrials = tagListGetData(tags, TAG_RspLib_RegistrarConnectMaxTrials,
                                                              ASAP_DEFAULT_REGISTRAR_CONNECT_MAXTRIALS);
      registrarTable->RegistrarConnectTimeout = (unsigned long long)tagListGetData(tags, TAG_RspLib_RegistrarConnectTimeout,
                                                                     ASAP_DEFAULT_REGISTRAR_CONNECT_TIMEOUT);
      registrarTable->RegistrarAnnounceTimeout = (unsigned long long)tagListGetData(tags, TAG_RspLib_RegistrarAnnounceTimeout,
                                                                      ASAP_DEFAULT_REGISTRAR_ANNOUNCE_TIMEOUT);

      if(registrarAnnounceAddress) {
         memcpy(&registrarTable->AnnounceAddress, registrarAnnounceAddress, sizeof(registrarTable->AnnounceAddress));
      }
      else {
         CHECK(string2address(ASAP_DEFAULT_REGISTRAR_ANNOUNCE_ADDRESS, &defaultAnnounceAddress) == true);
         memcpy(&registrarTable->AnnounceAddress, &defaultAnnounceAddress, sizeof(registrarTable->AnnounceAddress));
      }

      /* ====== Show results ================================================ */
      LOG_VERBOSE3
      fputs("New RegistrarTable's configuration:\n",  stdlog);
      fprintf(stdlog, "registrar.announce.timeout  = %llu [us]\n", registrarTable->RegistrarAnnounceTimeout);
      fputs("registrar.announce.address  = ",  stdlog);
      fputaddress((struct sockaddr*)&registrarTable->AnnounceAddress, true, stdlog);
      fputs("\n",  stdlog);
      fprintf(stdlog, "registrar.connect.maxtrials = %u\n",        registrarTable->RegistrarConnectMaxTrials);
      fprintf(stdlog, "registrar.connect.timeout   = %llu [us]\n", registrarTable->RegistrarConnectTimeout);
      LOG_END


      /* ====== Join announce multicast groups ============================ */
      registrarTable->AnnounceSocket = ext_socket(registrarTable->AnnounceAddress.sa.sa_family,
                                                  SOCK_DGRAM, IPPROTO_UDP);
      if(registrarTable->AnnounceSocket >= 0) {
         fdCallbackNew(&registrarTable->AnnounceSocketFDCallback,
                       registrarTable->Dispatcher,
                       registrarTable->AnnounceSocket,
                       FDCE_Read,
                       registrarAnnouceFDCallback,
                       registrarTable);

         setReusable(registrarTable->AnnounceSocket, 1);
         if(bindplus(registrarTable->AnnounceSocket,
                     &registrarTable->AnnounceAddress,
                     1) == false) {
            LOG_ERROR
            fputs("Unable to bind multicast socket to address ",  stdlog);
            fputaddress((struct sockaddr*)&registrarTable->AnnounceAddress.sa, true, stdlog);
            fputs("\n",  stdlog);
            LOG_END
            registrarTableDelete(registrarTable);
            return(false);
         }

         if(!joinOrLeaveMulticastGroup(registrarTable->AnnounceSocket,
                                       &registrarTable->AnnounceAddress,
                                       true)) {
            LOG_WARNING
            fputs("Joining multicast group ",  stdlog);
            fputaddress(&registrarTable->AnnounceAddress.sa, true, stdlog);
            fputs(" failed. Check routing (is default route set?) and firewall settings!\n",  stdlog);
            LOG_END
         }
      }
      else {
         LOG_ERROR
         fputs("Creating a socket for registrar announces failed\n",  stdlog);
         LOG_END
         registrarTableDelete(registrarTable);
         return(false);
      }
   }
   return(registrarTable);
}


/* ###### Add registrar assoc ID to list ################################# */
static void addRegistrarAssocID(struct RegistrarTable* registrarTable,
                                int                    registrarHuntFD,
                                sctp_assoc_t           assocID)
{
   struct RegistrarAssocIDNode* node = (struct RegistrarAssocIDNode*)malloc(sizeof(struct RegistrarAssocIDNode));
   if(node != NULL) {
      simpleRedBlackTreeNodeNew(&node->Node);
      node->Node.Value = 1;
      node->AssocID    = assocID;

      CHECK(simpleRedBlackTreeInsert(&registrarTable->RegistrarAssocIDList, &node->Node) == &node->Node);

      LOG_VERBOSE3
      fprintf(stdlog, "Added assoc %u to registrar assoc ID list.\n" , (unsigned int)assocID);
      fputs("RegistrarAssocIDList: ", stdlog);
      simpleRedBlackTreePrint(&registrarTable->RegistrarAssocIDList, stdlog);
      LOG_END
   }
   else {
      sendabort(registrarHuntFD, assocID);
   }
}


/* ###### Remove registrar assoc ID to list ############################## */
static void removeRegistrarAssocID(struct RegistrarTable* registrarTable,
                                   int                    registrarHuntFD,
                                   sctp_assoc_t           assocID)
{
   struct RegistrarAssocIDNode        cmpNode;
   struct SimpleRedBlackTreeNode* node;

   cmpNode.AssocID = assocID;
   node = simpleRedBlackTreeFind(&registrarTable->RegistrarAssocIDList, &cmpNode.Node);
   if(node != NULL) {
      CHECK(simpleRedBlackTreeRemove(&registrarTable->RegistrarAssocIDList, node) == node);
      free(node);

      LOG_VERBOSE3
      fprintf(stdlog, "Removed assoc %u from registrar assoc ID list.\n" , (unsigned int)assocID);
      fputs("RegistrarAssocIDList: ", stdlog);
      simpleRedBlackTreePrint(&registrarTable->RegistrarAssocIDList, stdlog);
      LOG_END
   }
   else {
      LOG_WARNING
      fprintf(stderr, "Tried to remove not-existing assoc %u from registrar assoc ID list.\n", (unsigned int)assocID);
      LOG_END
   }
}


/* ###### Get registrar assoc ID from list ############################### */
static sctp_assoc_t selectRegistrarAssocID(struct RegistrarTable* registrarTable)
{
   size_t                         elements;
   RedBlackTreeNodeValueType      value;
   struct SimpleRedBlackTreeNode* node;

   elements = simpleRedBlackTreeGetElements(&registrarTable->RegistrarAssocIDList);
   if(elements > 0) {
      value = random32() % elements;
      node = simpleRedBlackTreeGetNodeByValue(&registrarTable->RegistrarAssocIDList, value);
      CHECK(node);
      return(((struct RegistrarAssocIDNode*)node)->AssocID);
   }
   return(0);
}


/* ###### Get registrar assoc ID from list ############################### */
static int selectRegistrar(struct RegistrarTable*   registrarTable,
                           int                      registrarHuntFD,
                           struct MessageBuffer*    registrarHuntMessageBuffer,
                           RegistrarIdentifierType* registrarIdentifier)
{
   sctp_assoc_t                   assocID;
   struct ST_CLASS(PeerListNode)* peerListNode;
   union sockaddr_union*          peerAddressArray;
   int                            sd;
   size_t                         n;

   sd                   = -1;
   *registrarIdentifier = UNDEFINED_REGISTRAR_IDENTIFIER;

   assocID = selectRegistrarAssocID(registrarTable);
   if(assocID != 0) {
      /* ====== Registrar selected, get some information about it ======== */
      n = getpaddrsplus(registrarHuntFD, assocID, &peerAddressArray);
      if(n > 0) {
         LOG_VERBOSE2
         fprintf(stdlog, "Assoc %u connected to registrar at ", (unsigned int)assocID);
         fputaddress((struct sockaddr*)&peerAddressArray[0], true, stdlog);
         fputs("\n", stdlog);
         LOG_END

         /* ====== Get registrar's entry ======================== */
         struct TransportAddressBlock* registrarAddressBlock = (struct TransportAddressBlock*)malloc(transportAddressBlockGetSize(n));
         if(registrarAddressBlock) {
            transportAddressBlockNew(registrarAddressBlock,
                                     IPPROTO_SCTP,
                                     getPort(&peerAddressArray[0].sa),
                                     0,
                                     peerAddressArray, n);
            peerListNode = ST_CLASS(peerListManagementFindPeerListNode)(
                              &registrarTable->RegistrarList,
                              0,
                              registrarAddressBlock);
            if(peerListNode) {
               *registrarIdentifier = peerListNode->Identifier;
            }
            free(registrarAddressBlock);
         }
         free(peerAddressArray);
      }

      sd = registrarTablePeelOffRegistrarAssocID(registrarTable,
                                                 registrarHuntFD,
                                                 registrarHuntMessageBuffer,
                                                 assocID);
   }
   return(sd);
}


/* ###### Peel off registrar assoc ####################################### */
int registrarTablePeelOffRegistrarAssocID(struct RegistrarTable* registrarTable,
                                          int                    registrarHuntFD,
                                          struct MessageBuffer*  registrarHuntMessageBuffer,
                                          sctp_assoc_t           assocID)
{
   int sd = sctp_peeloff(registrarHuntFD, assocID);
   if(sd >= 0) {
      LOG_VERBOSE2
      fprintf(stdlog, "Assoc %u peeled off from registrar hunt socket\n", (unsigned int)assocID);
      LOG_END
      removeRegistrarAssocID(registrarTable, registrarHuntFD, assocID);
   }
   return(sd);
}


/* ###### Handle notification on registrar hunt socket ################### */
void registrarTableHandleNotificationOnRegistrarHuntSocket(struct RegistrarTable*         registrarTable,
                                                           int                            registrarHuntFD,
                                                           struct MessageBuffer*          registrarHuntMessageBuffer,
                                                           const union sctp_notification* notification)
{
   union sockaddr_union* peerAddressArray;
   size_t                n;

   /* ====== Association change notification ============================= */
   if(notification->sn_header.sn_type == SCTP_ASSOC_CHANGE) {
      if(notification->sn_assoc_change.sac_state == SCTP_COMM_UP) {
         n = getpaddrsplus(registrarHuntFD,
                           notification->sn_assoc_change.sac_assoc_id,
                           &peerAddressArray);
         if(n > 0) {
            LOG_VERBOSE2
            fprintf(stdlog, "Assoc %u connected to registrar at ",
                    (unsigned int)notification->sn_assoc_change.sac_assoc_id);
            fputaddress((struct sockaddr*)&peerAddressArray[0], true, stdlog);
            fputs("\n", stdlog);
            LOG_END
            free(peerAddressArray);
         }

         /* ====== Add registrar assoc ID to list ======================== */
         addRegistrarAssocID(registrarTable,
                             registrarHuntFD,
                             notification->sn_assoc_change.sac_assoc_id);
      }
      else if((notification->sn_assoc_change.sac_state == SCTP_COMM_LOST) ||
              (notification->sn_assoc_change.sac_state == SCTP_SHUTDOWN_COMP) ) {
         LOG_VERBOSE2
         fprintf(stdlog, "Assoc %u disconnected from registrar (communication lost or shutdown complete)\n",
                 (unsigned int)notification->sn_assoc_change.sac_assoc_id);
         LOG_END
         removeRegistrarAssocID(registrarTable,
                                registrarHuntFD,
                                notification->sn_assoc_change.sac_assoc_id);
      }
   }
   else if(notification->sn_header.sn_type == SCTP_SHUTDOWN_EVENT) {
      LOG_VERBOSE2
      fprintf(stdlog, "Assoc %u disconnected from registrar (shutdown)\n",
              (unsigned int)notification->sn_shutdown_event.sse_assoc_id);
      LOG_END
      removeRegistrarAssocID(registrarTable,
                             registrarHuntFD,
                             notification->sn_shutdown_event.sse_assoc_id);
   }
}


/* ###### Destructor ##################################################### */
void registrarTableDelete(struct RegistrarTable* registrarTable)
{
   if(registrarTable != NULL) {
      if(registrarTable->AnnounceSocket >= 0) {
         fdCallbackDelete(&registrarTable->AnnounceSocketFDCallback);
         ext_close(registrarTable->AnnounceSocket);
         registrarTable->AnnounceSocket = -1;
      }
      simpleRedBlackTreeDelete(&registrarTable->RegistrarAssocIDList);
      ST_CLASS(peerListManagementDelete)(&registrarTable->RegistrarList);
      free(registrarTable);
   }
}


/* ###### Add static registrar entry ################################### */
unsigned int registrarTableAddStaticEntry(
                struct RegistrarTable*              registrarTable,
                const struct TransportAddressBlock* transportAddressBlock)
{
   struct ST_CLASS(PeerListNode)* peerListNode;
   unsigned int                   result;

   result = ST_CLASS(peerListManagementRegisterPeerListNode)(
               &registrarTable->RegistrarList,
               0,
               PLNF_STATIC,
               transportAddressBlock,
               getMicroTime(),
               &peerListNode);
   if(result == RSPERR_OKAY) {
      LOG_VERBOSE3
      fputs("Added static entry to registrar table: ", stdlog);
      ST_CLASS(peerListNodePrint)(peerListNode, stdlog, PLPO_FULL);
      fputs("\n", stdlog);
      LOG_END
   }
   else {
      LOG_WARNING
      fputs("Unable to add static entry to registrar table: ", stdlog);
      transportAddressBlockPrint(transportAddressBlock, stdlog);
      fputs("\n", stdlog);
      LOG_END
   }
   return(result);
}


/* ###### Try more registrars ########################################### */
static void tryNextBlock(struct RegistrarTable*         registrarTable,
                         int                            registrarHuntFD,
                         struct MessageBuffer*          registrarHuntMessageBuffer,
                         RegistrarIdentifierType*       lastRegistrarIdentifier,
                         struct TransportAddressBlock** lastTransportAddressBlock)
{
   struct TransportAddressBlock*  transportAddressBlock;
   struct ST_CLASS(PeerListNode)* peerListNode;
   int                            result;
   size_t                         i;

   i = ST_CLASS(peerListManagementPurgeExpiredPeerListNodes)(
         &registrarTable->RegistrarList,
         getMicroTime());
   LOG_VERBOSE4
   fprintf(stdlog, "Purged %u out-of-date peer list nodes. Peer List:\n",  (unsigned int)i);
   ST_CLASS(peerListManagementPrint)(&registrarTable->RegistrarList, stdlog, PLPO_PEERS_INDEX|PLNPO_TRANSPORT);
   LOG_END

   for(i = 0;i < MAX_SIMULTANEOUS_REQUESTS;i++) {
      peerListNode = ST_CLASS(peerListManagementFindNearestNextPeerListNode)(
                        &registrarTable->RegistrarList,
                        *lastRegistrarIdentifier,
                        *lastTransportAddressBlock);

      if(peerListNode != NULL) {
         *lastRegistrarIdentifier = peerListNode->Identifier;
         if(*lastTransportAddressBlock) {
            transportAddressBlockDelete(*lastTransportAddressBlock);
            free(*lastTransportAddressBlock);
         }
         *lastTransportAddressBlock = transportAddressBlockDuplicate(peerListNode->AddressBlock);

         transportAddressBlock = peerListNode->AddressBlock;
         while(transportAddressBlock != NULL) {
            if(transportAddressBlock->Protocol == IPPROTO_SCTP) {
               break;
            }
            transportAddressBlock = transportAddressBlock->Next;
         }
         if(transportAddressBlock != NULL) {
            LOG_VERBOSE1
            fputs("Trying registrar at ",  stdlog);
            transportAddressBlockPrint(transportAddressBlock, stdlog);
            fputs("...\n",  stdlog);
            LOG_END

            result = connectplus(registrarHuntFD,
                                 transportAddressBlock->AddressArray,
                                 transportAddressBlock->Addresses);
            if((result < 0) &&
               (errno != EINPROGRESS) && (errno != EISCONN)) {
               LOG_WARNING
               fputs("Connection to registrar ",  stdlog);
               transportAddressBlockPrint(transportAddressBlock, stdlog);
               fputs(" failed\n",  stdlog);
               LOG_END
            }
            else {
               registrarTable->OutstandingConnects++;
            }
         }
      }
      else {
         if(*lastTransportAddressBlock) {
            free(*lastTransportAddressBlock);
         }
         *lastTransportAddressBlock = NULL;
         *lastRegistrarIdentifier   = UNDEFINED_REGISTRAR_IDENTIFIER;
         break;
      }
   }
}


/* ###### Find registrar ################################################### */
int registrarTableGetRegistrar(struct RegistrarTable*   registrarTable,
                               int                      registrarHuntFD,
                               struct MessageBuffer*    registrarHuntMessageBuffer,
                               RegistrarIdentifierType* registrarIdentifier)
{
   struct timeval                 selectTimeout;
   const union sctp_notification* notification;
   RegistrarIdentifierType        lastRegistrarIdentifier;
   struct TransportAddressBlock*  lastTransportAddressBlock;
   struct ST_CLASS(PeerListNode)* peerListNode;
   sctp_assoc_t                   assocID;
   ssize_t                        received;
   unsigned long long             start;
   unsigned long long             nextTimeout;
   unsigned long long             lastTrialTimeStamp;
   fd_set                         rfdset;
   unsigned int                   trials;
   int                            result;
   int                            flags;
   int                            sd;
   int                            n;

   *registrarIdentifier = 0;
   if(registrarTable == NULL) {
      return(-1);
   }
   LOG_VERBOSE1
   fputs("Looking for registrar...\n",  stdlog);
   LOG_END


   /* ====== Is there already a connection to a registrar? =============== */
   sd = selectRegistrar(registrarTable, registrarHuntFD, registrarHuntMessageBuffer,
                        registrarIdentifier);
   if(sd >= 0) {
      return(sd);
   }


   /* ====== Do registrar hunt =========================================== */
   trials                    = 0;
   lastRegistrarIdentifier   = 0;
   lastTransportAddressBlock = NULL;
   start                     = 0;

   registrarTable->OutstandingConnects = 0;

   peerListNode = ST_CLASS(peerListGetRandomPeerNode)(&registrarTable->RegistrarList.List);
   if(peerListNode) {
      lastRegistrarIdentifier   = peerListNode->Identifier;
      lastTransportAddressBlock = transportAddressBlockDuplicate(peerListNode->AddressBlock);
      LOG_VERBOSE3
      fputs("Randomized registrar hunt start: ", stdlog);
      ST_CLASS(peerListNodePrint)(peerListNode, stdlog, PLPO_FULL);
      fputs("\n", stdlog);
      LOG_END
   }

   lastTrialTimeStamp = 0;
   for(;;) {
      /* ====== Start new round ========================================== */
      if((lastRegistrarIdentifier == 0) && (lastTransportAddressBlock == NULL)) {
         /*
            Start new trial, when
            - First time
            - A new registrar announce has been added to the list AND
                 there are no outstanding connects
            - The current trial has been running for at least
               registrarTable->RegistrarConnectTimeout
         */
         if( (start == 0) ||
             ((registrarTable->LastAnnounceHeard >= start) && (registrarTable->OutstandingConnects == 0)) ||
             (start + registrarTable->RegistrarConnectTimeout < getMicroTime()) ) {
            trials++;
            if(trials > registrarTable->RegistrarConnectMaxTrials) {
               LOG_VERBOSE
               fputs("Registrar hunt procedure did not find any registrar!\n", stdlog);
               LOG_END
               if(lastTransportAddressBlock) {
                  transportAddressBlockDelete(lastTransportAddressBlock);
                  free(lastTransportAddressBlock);
               }
               return(-1);
            }

            lastTrialTimeStamp = 0;
            start = getMicroTime();
            LOG_VERBOSE2
            fprintf(stdlog, "Trial #%u...\n", trials);
            LOG_END

            if(!joinOrLeaveMulticastGroup(registrarTable->AnnounceSocket,
                                          &registrarTable->AnnounceAddress,
                                          true)) {
               LOG_WARNING
               fputs("Joining multicast group ",  stdlog);
               fputaddress(&registrarTable->AnnounceAddress.sa, true, stdlog);
               fputs(" failed. Check routing (is default route set?) and firewall settings!\n",  stdlog);
               LOG_END
            }
         }
      }

      /* ====== Try next block of addresses ================================= */
      if(lastTrialTimeStamp + registrarTable->RegistrarConnectTimeout < getMicroTime()) {
         /* Try next block of addresses */
         tryNextBlock(registrarTable,
                      registrarHuntFD,
                      registrarHuntMessageBuffer,
                      &lastRegistrarIdentifier,
                      &lastTransportAddressBlock);
         lastTrialTimeStamp = getMicroTime();
      }

      /* ====== Wait for event =========================================== */
      FD_ZERO(&rfdset);
      FD_SET(registrarHuntFD, &rfdset);
      n = registrarHuntFD;
      if(registrarTable->AnnounceSocket >= 0) {
         FD_SET(registrarTable->AnnounceSocket, &rfdset);
         n = max(n, registrarTable->AnnounceSocket);
      }
      nextTimeout = registrarTable->RegistrarConnectTimeout;
      selectTimeout.tv_sec  = nextTimeout / (unsigned long long)1000000;
      selectTimeout.tv_usec = nextTimeout % (unsigned long long)1000000;

      LOG_VERBOSE4
      fprintf(stdlog, "select() with timeout %lld\n", nextTimeout);
      LOG_END
      result = ext_select(n + 1,
                          &rfdset, NULL, NULL,
                          &selectTimeout);
      LOG_VERBOSE4
      fprintf(stdlog, "select() result=%d\n", result);
      LOG_END

      if((result < 0) && (errno == EINTR)) {
         LOG_VERBOSE3
         fputs("Interrupted select() call -> returning immediately!\n",  stdlog);
         LOG_END
         if(lastTransportAddressBlock) {
            transportAddressBlockDelete(lastTransportAddressBlock);
            free(lastTransportAddressBlock);
         }
         return(-1);
      }

      /* ====== Handle events ============================================ */
      if(result != 0) {
         if( (registrarTable->AnnounceSocket > 0) &&
             (FD_ISSET(registrarTable->AnnounceSocket, &rfdset)) ) {
            handleRegistrarAnnounceCallback(registrarTable, registrarTable->AnnounceSocket);
         }
         if(FD_ISSET(registrarHuntFD, &rfdset)) {
            flags    = MSG_PEEK;
            received = messageBufferRead(registrarHuntMessageBuffer, registrarHuntFD, &flags,
                                         NULL, 0,
                                         NULL, &assocID, NULL, 0);
            if(received > 0) {
               if(flags & MSG_NOTIFICATION) {
                  notification = (const union sctp_notification*)registrarHuntMessageBuffer->Buffer;
                  /* ====== Notification received ======================== */
                  flags = 0;
                  if( (messageBufferRead(registrarHuntMessageBuffer, registrarHuntFD, &flags,
                                         NULL, 0,
                                         NULL, NULL, NULL, 0) > 0) &&
                      (flags & MSG_NOTIFICATION) ) {

                     registrarTableHandleNotificationOnRegistrarHuntSocket(registrarTable,
                                                                           registrarHuntFD,
                                                                           registrarHuntMessageBuffer,
                                                                           notification);
                     if((notification->sn_assoc_change.sac_state == SCTP_COMM_LOST) ||
                        (notification->sn_assoc_change.sac_state == SCTP_CANT_STR_ASSOC) ||
                        (notification->sn_assoc_change.sac_state == SCTP_SHUTDOWN_COMP)) {
                        if(registrarTable->OutstandingConnects > 0) {
                           registrarTable->OutstandingConnects--;
                        }
                        LOG_VERBOSE3
                        fprintf(stdlog, "Failed to establish registrar connection, outstanding=%u\n",
                                 (unsigned int)registrarTable->OutstandingConnects);
                        LOG_END
                     }
                  }
                  else {
                     LOG_ERROR
                     fputs("Peeked notification but failed to read it\n", stdlog);
                     LOG_END_FATAL
                  }

                  /* ====== Is there a connection to a registrar? ======== */
                  sd = selectRegistrar(registrarTable, registrarHuntFD, registrarHuntMessageBuffer,
                                       registrarIdentifier);
                  if(sd >= 0) {
                     if(lastTransportAddressBlock) {
                        transportAddressBlockDelete(lastTransportAddressBlock);
                        free(lastTransportAddressBlock);
                     }
                     return(sd);
                  }
               }
               else {
                  LOG_VERBOSE
                  fprintf(stdlog, "Peeked data on assoc %u, but no association is listed\n",
                          (unsigned int)assocID);
                  LOG_END

                  sd = registrarTablePeelOffRegistrarAssocID(registrarTable, registrarHuntFD, registrarHuntMessageBuffer,
                                                             assocID);
                  if(sd >= 0) {
                     LOG_ACTION
                     fprintf(stdlog, "Taking assoc %u as new registrar => socket %d\n", (unsigned int)assocID, sd);
                     LOG_END
                     return(sd);
                  }

                  LOG_WARNING
                  fprintf(stdlog, "Got crap via assoc %u -> aborting it\n", (unsigned int)assocID);
                  LOG_END
                  flags = 0;
                  messageBufferRead(registrarHuntMessageBuffer, registrarHuntFD, &flags,
                                    NULL, 0,
                                    NULL, &assocID, NULL, 0);
                  sendabort(registrarHuntFD, assocID);
               }
            }
         }
      }
   }
}
