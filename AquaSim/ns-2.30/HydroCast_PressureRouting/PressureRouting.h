#ifndef PRESSUREROUTING_H_INCLUDED
#define PRESSUREROUTING_H_INCLUDED

#include <stdio.h>
#include <limits>
#include <packet.h>
#include <agent.h>
#include <scheduler.h>
#include <classifier-port.h>
#include <address.h>
#include <trace.h>


#define HDR_DBR_PKT(p)     hdr_dbr_pkt::access(p)

struct hdr_dbr_pkt {
   ns_addr_t myId_;
   u_int8_t  pkt_seq_num_;
   u_int16_t hops_;
   u_int16_t depth_;

   static int offset_;
   inline static int& offset() { return offset_;}
   inline static hdr_dbr_pkt* access (const Packet* p) {
      return (hdr_dbr_pkt*) p->access(offset_);
   }
};


#endif // PRESSUREROUTING_H_INCLUDED
