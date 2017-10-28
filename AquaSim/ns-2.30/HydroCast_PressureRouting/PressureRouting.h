#ifndef PRESSUREROUTING_H_INCLUDED
#define PRESSUREROUTING_H_INCLUDED

#include "PressureRoutingTable.h"
#include "node.h"
#include "underwatersensor/uw_common/underwatersensornode.h"
#include "uw_datastructure.h"

#include <stdio.h>
#include <limits>
#include <packet.h>
#include <agent.h>
#include <scheduler.h>
#include <classifier-port.h>
#include <address.h>
#include <trace.h>
#include <math.h>


#define HDR_DBR_PKT(p)     hdr_dbr_pkt::access(p)

struct hdr_dbr_pkt
{
    ns_addr_t myId_;
    u_int8_t  pkt_seq_num_;
    u_int16_t hops_;
    u_int16_t depth_;
    double ts_;

    struct uw_extra_info info;

    static int offset_;
    inline static int& offset()
    {
        return offset_;
    }
    inline static hdr_dbr_pkt* access (const Packet* p)
    {
        return (hdr_dbr_pkt*) p->access(offset_);
    }
};


class PressureRoutingAgent : public Agent
{

protected:
    u_int16_t num_hops_;
    u_int16_t next_hop_;

    ns_addr_t myId_;  //node id
    u_int16_t depth_;  //current depth
    u_int16_t depth_threshold;   //depth_threshold

    PressureRoutingTable pressureRoutingTable;

    PortClassifier* dmux_;
    Trace *logtarget;
    PressureRoutingTable prTable;
    UnderwaterSensorNode *node;

public:
    PressureRoutingAgent(nsaddr_t);
    int command(int, const char*const*);
    Packet* create_packet()
    calculatePosition(Packet*)
    double distance(Packet* pkt)
    void recv(Packet*, Handler*);
    void sendMsg();
    //void sendBcast();
};

#endif // PRESSUREROUTING_H_INCLUDED
