#ifndef PRESSUREROUTINGTABLE_H_INCLUDED
#define PRESSUREROUTINGTABLE_H_INCLUDED

#include <trace.h>
#include <map>
#include <ip.h>


typedef std::map<nsaddr_t, u_int8_t> rtable_history;  //sender id, sequence number  Q2
typedef std::map<Packet*, u_int16_t> rtable_quality;  //packet, scheduled sending time  Q1

class PressureRoutingTable
{
public:
    rtable_history rt_history;
    rtable_quality rt_quality;

    PressureRoutingTable();

    void print(int type, Packet* p, nsaddr_t sendID);

    void clear(int type);

    void rm_entry(nsaddr_t);

    void rm_entry(Packet*);

    void add_entry(nsaddr_t, u_int8_t);

    void add_entry(Packet*, u_int16_t);

    bool isExist(nsaddr_t);

    bool isExist(Packet*);

    u_int16_t getValue(Packet*);

    //void update(rtable_t*, nsaddr_t);

    void update(Packet*, u_int16_t);

    u_int32_t size();
};

#endif // PRESSUREROUTINGTABLE_H_INCLUDED
