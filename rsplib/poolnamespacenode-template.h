/*
 * An Efficient RSerPool Pool Namespace Management Implementation
 * Copyright (C) 2004 by Thomas Dreibholz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Contact: dreibh@exp-math.uni-essen.de
 *
 */

#ifndef INTERNAL_POOLTEMPLATE
#error Do not include this file directly, use poolnamespacemanagement.h
#endif


#ifdef __cplusplus
extern "C" {
#endif


struct ST_CLASS(PoolNamespaceNode)
{
   struct ST_CLASSNAME PoolIndexStorage;             /* Pools                          */
   struct ST_CLASSNAME PoolElementTimerStorage;      /* PEs with timer event scheduled */
   struct ST_CLASSNAME PoolElementConnectionStorage; /* PEs by connection              */
   struct ST_CLASSNAME PoolElementOwnershipStorage;  /* PEs by ownership               */
   ENRPIdentifierType  HomeNSIdentifier;             /* This NS's Identifier           */
   size_t              PoolElements;                 /* Number of Pool Elements        */
};


void ST_CLASS(poolNamespaceNodeNew)(struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                    const ENRPIdentifierType            homeNSIdentifier);
void ST_CLASS(poolNamespaceNodeDelete)(struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
size_t ST_CLASS(poolNamespaceNodeGetTimerNodes)(
                 struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetFirstPoolElementTimerNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetLastPoolElementTimerNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetPrevPoolElementTimerNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetNextPoolElementTimerNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
size_t ST_CLASS(poolNamespaceNodeGetOwnershipNodes)(
          struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
size_t ST_CLASS(poolNamespaceNodeGetOwnershipNodesForIdentifier)(
          struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
          const ENRPIdentifierType            homeNSIdentifier);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetFirstPoolElementOwnershipNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetLastPoolElementOwnershipNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeFindNearestPrevPoolElementOwnershipNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const ENRPIdentifierType            homeNSIdentifier,
                                     const struct PoolHandle*            poolHandle,
                                     const PoolElementIdentifierType     poolElementIdentifier);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeFindNearestNextPoolElementOwnershipNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const ENRPIdentifierType            homeNSIdentifier,
                                     const struct PoolHandle*            poolHandle,
                                     const PoolElementIdentifierType     poolElementIdentifier);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetPrevPoolElementOwnershipNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetNextPoolElementOwnershipNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetFirstPoolElementOwnershipNodeForIdentifier)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const ENRPIdentifierType            homeNSIdentifier);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetLastPoolElementOwnershipNodeForIdentifier)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const ENRPIdentifierType            homeNSIdentifier);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetPrevPoolElementOwnershipNodeForSameIdentifier)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetNextPoolElementOwnershipNodeForSameIdentifier)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
size_t ST_CLASS(poolNamespaceNodeGetConnectionNodes)(
          struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
size_t ST_CLASS(poolNamespaceNodeGetConnectionNodesForConnection)(
          struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
          const int                           connectionSocketDescriptor,
          const sctp_assoc_t                  assocID);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetFirstPoolElementConnectionNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetLastPoolElementConnectionNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeFindNearestPrevPoolElementConnectionNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const int                           connectionSocketDescriptor,
                                     const sctp_assoc_t                  assocID,
                                     const struct PoolHandle*            poolHandle,
                                     const PoolElementIdentifierType     poolElementIdentifier);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeFindNearestNextPoolElementConnectionNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const int                           connectionSocketDescriptor,
                                     const sctp_assoc_t                  assocID,
                                     const struct PoolHandle*            poolHandle,
                                     const PoolElementIdentifierType     poolElementIdentifier);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetPrevPoolElementConnectionNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetNextPoolElementConnectionNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetFirstPoolElementConnectionNodeForConnection)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const int                           connectionSocketDescriptor,
                                     const sctp_assoc_t                  assocID);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetLastPoolElementConnectionNodeForConnection)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const int                           connectionSocketDescriptor,
                                     const sctp_assoc_t                  assocID);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetPrevPoolElementConnectionNodeForSameConnection)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeGetNextPoolElementConnectionNodeForSameConnection)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
size_t ST_CLASS(poolNamespaceNodeGetPoolElementNodes)(
          const struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
size_t ST_CLASS(poolNamespaceNodeGetPoolNodes)(
          const struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
size_t ST_CLASS(poolNamespaceNodeGetPoolElementNodesOfPool)(
          struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
          const struct PoolHandle*            poolHandle);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeGetFirstPoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeGetLastPoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeGetNextPoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                              struct ST_CLASS(PoolNode)*          poolNode);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeGetPrevPoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                              struct ST_CLASS(PoolNode)*          poolNode);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeAddPoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                              struct ST_CLASS(PoolNode)*          poolNode);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeFindPoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                              const struct PoolHandle*            poolHandle);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeFindNearestPrevPoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                              const struct PoolHandle*            poolHandle);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeFindNearestNextPoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                              const struct PoolHandle*            poolHandle);
struct ST_CLASS(PoolNode)* ST_CLASS(poolNamespaceNodeRemovePoolNode)(
                              struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                              struct ST_CLASS(PoolNode)*          poolNode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeAddPoolElementNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolNode)*          poolNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode,
                                     unsigned int*                       errorCode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeFindPoolElementNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     const struct PoolHandle*            poolHandle,
                                     const PoolElementIdentifierType     poolElementIdentifier);
void ST_CLASS(poolNamespaceNodeUpdateOwnershipOfPoolElementNode)(
        struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
        struct ST_CLASS(PoolElementNode)*   poolElementNode,
        const ENRPIdentifierType            newHomeNSIdentifier);
void ST_CLASS(poolNamespaceNodeUpdateConnectionOfPoolElementNode)(
        struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
        struct ST_CLASS(PoolElementNode)*   poolElementNode,
        const int                           connectionSocketDescriptor,
        const sctp_assoc_t                  connectionAssocID);
void ST_CLASS(poolNamespaceNodeUpdatePoolElementNode)(
        struct ST_CLASS(PoolNamespaceNode)*     poolNamespaceNode,
        struct ST_CLASS(PoolElementNode)*       poolElementNode,
        const struct ST_CLASS(PoolElementNode)* source,
        unsigned int*                           errorCode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeAddOrUpdatePoolElementNode)(
                                    struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                    struct ST_CLASS(PoolNode)**         poolNode,
                                    struct ST_CLASS(PoolElementNode)**  poolElementNode,
                                    unsigned int*                       errorCode);
struct ST_CLASS(PoolElementNode)* ST_CLASS(poolNamespaceNodeRemovePoolElementNode)(
                                     struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                     struct ST_CLASS(PoolElementNode)*   poolElementNode);
void ST_CLASS(poolNamespaceNodeGetDescription)(
        struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
        char*                               buffer,
        const size_t                        bufferSize);
void ST_CLASS(poolNamespaceNodePrint)(struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                      FILE*                               fd,
                                      const unsigned int                  fields);
void ST_CLASS(poolNamespaceNodeActivateTimer)(
        struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
        struct ST_CLASS(PoolElementNode)*   poolElementNode,
        const unsigned int                  timerCode,
        const unsigned long long            timerTimeStamp);
void ST_CLASS(poolNamespaceNodeDeactivateTimer)(
        struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
        struct ST_CLASS(PoolElementNode)*   poolElementNode);
void ST_CLASS(poolNamespaceNodeVerify)(struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode);
void ST_CLASS(poolNamespaceNodeClear)(struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
                                      void                                (*poolNodeDisposer)(void* poolNode, void* userData),
                                      void                                (*poolElementNodeDisposer)(void* poolElementNode, void* userData),
                                      void*                               userData);
size_t ST_CLASS(poolNamespaceNodeSelectPoolElementNodesByPolicy)(
          struct ST_CLASS(PoolNamespaceNode)* poolNamespaceNode,
          const struct PoolHandle*            poolHandle,
          struct ST_CLASS(PoolElementNode)**  poolElementNodeArray,
          const size_t                        maxPoolElementNodes,
          const size_t                        maxIncrement,
          unsigned int*                       errorCode);


#ifdef __cplusplus
}
#endif
