#include "PressureRoutingTable.h"

PressureRoutingTable::PressureRoutingTable() {}


void PressureRoutingTable::add_entry(nsaddr_t sendId, u_int8_t sqe_num)
{
    rt_history[nsaddr_t] = sqe_num;
}

void PressureRoutingTable::add_entry(Packet* p, u_int16_t scheduleTime)
{
    rt_quality[p] = scheduleTime;
}


void PressureRoutingTable::rm_entry(Packet* p)
{
    rt_quality.erase(p);
}

void PressureRoutingTable::rm_entry(nsaddr_t sendID)
{
    rt_history.erase(sendID);
}

bool PressureRoutingTable::isExist(Packet* p)
{
    if(rt_quality.count(p) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


u_int16_t PressureRoutingTable::getValue(Packet* p)
{
    return rt_quality[p];
}


void PressureRoutingTable::update(Packet* p, u_int16_t scheduleTime)
{
    rt_quality[p] = scheduleTime;
}


void PressureRoutingTable::print(int type, Packet* p, nsaddr_t sendID)
{
    if(type == 0)
    {
        //print Q1 quality table
        hdr_dbr_pkt* packet = HDR_DBR_PKT(p);
        printf("%f node %d scheduled sending time:%d\n", NOW, packet->myId_, rt_quality[p]);
    }
    else
    {
        //print Q2 history table
        printf("%f node %d sequence number:%d\n", NOW, packet->myId_, rt_history[sendID]);
    }
}


void PressureRoutingTable::clear(int type)
{
    if(type == 0)
    {
        rt_quality.clear();
    }
    else
    {
        rt_history.clear();
    }
}


