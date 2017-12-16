/*
 * EPARouting.h
 *
 *  Created on: Nov 7, 2017
 *      Author: ZDH
 */

#ifndef EPAROUTINGAGENT_H_
#define EPAROUTINGAGENT_H_

#include <deque>
#include <map>
#include <vector>
#include <stdarg.h>
#include <math.h>

#include <object.h>
#include <agent.h>
#include <trace.h>
#include <packet.h>
#include <mac.h>
#include <mobilenode.h>
#include <classifier-port.h>
#include <scheduler.h>
#include <random.h>
#include <tcl.h>
#include <ll.h>
#include <cmu-trace.h>



#define	EPA_PORT		0xFF
#define	EPA_BEACON		1
#define EPA_MSG         2
#define EXPIRE_TIME     120
#define EPA_RANGE       240

#define	BEACON_RESCHED  beacon_timer_->resched(30);

class EPARoutingAgent;

//============================Timer Start=======================================
class AgentTimer : public TimerHandler {
public:
	AgentTimer(EPARoutingAgent *a) { a_ = a; }
	virtual void expire(Event *e) = 0;

protected:
	EPARoutingAgent *a_;
};

class BeaconHandler : public Handler {
public:
	BeaconHandler(EPARoutingAgent *a) { a_ = a; }
	virtual void handle(Event *e);

private:
	EPARoutingAgent *a_;
};

class BeaconTimer : public AgentTimer {
public:
	BeaconTimer(EPARoutingAgent *a) : AgentTimer(a) {}
	virtual void expire(Event *e);
};

//============================Timer end=========================================


//=============================NeighbTable Start====================================
class NeighbEnt {
public:
	NeighbEnt(nsaddr_t net_id) : x(0.0), y(0.0), z(0.0), hop(1), expireTime(0) {
		this->net_id = net_id;
	}
	// the agent is used for timer object

	double x, y, z;		// location of neighbor, actually we only need depth info
	nsaddr_t net_id;    // IP of neighbor
	int hop;		    // indicates the hop of neighbor
	double expireTime;  // timer for expiration of neighbor
};

class NeighbTable {
public:
	NeighbTable();
	~NeighbTable();

	void dump(void);
	void delete_ent(const NeighbEnt *e);        	// delete an neighbor
	void add_ent(const NeighbEnt *e);     	// add an neighbor

private:
	int num_ents;       // number of entries in use
	std::map<nsaddr_t, NeighbEnt*> neighbTable;
};
//=============================NeighbTable End====================================


//=============================Packet header Start================================
class packet_hdr {
public:
	static int offset_;		// offset of this header
	static int& offset() { return offset_; }
	static packet_hdr* access(const Packet *p) {
		return (packet_hdr*)p->access(offset_);
	}

	nsaddr_t source;    //the source id of message
	nsaddr_t node_id;   // IP
	double x, y, z;     // location
	int packetID;       // unique id for packet
	int mode;           //packet mode
	int mhops;		    // max # of hops to broadcast
	std::vector<NeighbEnt> neighb_1hop;

	// get the header size
	int size()
	{ 	int sz;
		sz = 4 * sizeof(int);
		sz += 3 * sizeof(double);
		return sz;
	}
};
//=============================Packet header End====================================

//=============================EPARoutingAgent Start================================
class EPARoutingAgent : public Agent {
public:
	EPARoutingAgent();
	virtual ~EPARoutingAgent();

	virtual int command(int argc, const char*const* argv);
	virtual void recv(Packet *, Handler *);

protected:
	NeighbTable neighbTable_1hop;
	std::map<nsaddr_t, NeighbTable> neighbTable_2hop;  //store my neighbor's neighbors
	std::map<nsaddr_t, double> epaNeighb;
	NsObject *ll;				// our link layer output
	MobileNode* mn_;			// MobileNode associated with the agent
	BeaconTimer *beacon_timer_;		// beacon timer

	void init(void);
	void sendBeacon(void);
	void calNeighEPA();
	std::map<nsaddr_t, NeighbTable> filterNodes();   //filter nodes with distance >= R/2
	double distance(NeighbEnt e1, NeighbEnt e2);
	Packet* makeBeacon();    //make beacon header
	std::vector<NeighbEnt> getNeighbors_1hop();  //neighbors with 1 hop
};
//=============================EPARoutingAgent End====================================

#endif /* EPAROUTINGAGENT_H_ */







