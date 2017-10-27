/*
 * Copyright (c) 2001-2004 by the Protocol Engineering Lab, U of Delaware
 * All rights reserved.
 *
 * Armando L. Caro Jr. <acaro@@cis,udel,edu>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#) $Header: /cvsroot/nsnam/ns-2/sctp/sctp-mfrTimestamp.h,v 1.1 2005/10/07 06:14:38 tomh Exp $ (UD/PEL)
 */

/* This extension combines MultipleFastRtx and Timestamp extensions.
 *
 * MultipleFastRtx extension implements the Caro Multiple Fast Retransmit
 * Algorithm. Caro's Algorithm introduces a fastRtxRecover state variable
 * per TSN in the send buffer. Any time a TSN is retransmitted, its
 * fastRtxRecover is set to the highest TSN outstanding at the time of
 * retransmit. That way, only missing reports triggered by TSNs beyond
 * fastRtxRecover may trigger yet another fast retransmit.
 *
 * Timestamp extension adds a TIMESTAMP chunk into every packet with DATA
 * or SACK chunk(s). The timestamps allow RTT measurements to be made per
 * packet on both original transmissiosn and retransmissions. Thus, Karn's
 * algorithm is no longer needed.
 */

#ifndef ns_sctp_mfrTimestamp_h
#define ns_sctp_mfrTimestamp_h

#include "sctp-timestamp.h"

class MfrTimestampSctpAgent : public virtual TimestampSctpAgent 
{
public:
  MfrTimestampSctpAgent();

protected:
  virtual void  delay_bind_init_all();
  virtual int   delay_bind_dispatch(const char *varName, const char *localName,
				    TclObject *tracer);

  /* tracing functions
   */
  virtual void  TraceVar(const char*);
  virtual void  TraceAll();

  /* sending functions
   */
  virtual void  AddToSendBuffer(SctpDataChunkHdr_S *, int, u_int, SctpDest_S *);
  virtual void  SendBufferDequeueUpTo(u_int);
  
  /* processing functions
   */
  virtual Boolean_E  ProcessGapAckBlocks(u_char *, Boolean_E);

  TracedInt        tiMfrCount;        // trace each time MFR gets triggered
};

#endif
