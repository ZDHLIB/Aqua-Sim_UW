/*
 * EPARouting.cpp
 *
 *  Created on: Nov 7, 2017
 *      Author: ZDH
 */

#include <HydroCast_PressureRouting/EPARoutingAgent.h>

EPARoutingAgent::EPARoutingAgent() {
	// TODO Auto-generated constructor stub

}

EPARoutingAgent::~EPARoutingAgent() {
	// TODO Auto-generated destructor stub
}

static class EPARoutingAgentClass: public TclClass {
public:
	EPARoutingAgentClass() :
			TclClass("Agent/EPARouting") {
	}
	TclObject* create(int, const char* const *) {
		return (new EPARoutingAgent);
	}
} class_EPARoutingAgent;


class EPAHeaderClass: public PacketHeaderClass {
public:
	EPAHeaderClass() :
			PacketHeaderClass("PacketHeader/EPARouting", sizeof(packet_hdr)) {
		bind_offset(&packet_hdr::offset_);
	}
} class_EPAHeader;

//=============================NeighbTable=========================================
void NeighbTable::delete_ent(const NeighbEnt *e) {
	if(num_ents<=0)
		return;
	if( neighbTable.find(e->net_id) != neighbTable.end() ){
		delete neighbTable[e->net_id];
		neighbTable.erase(e->net_id);
		num_ents--;
	}
	return;
}

void NeighbTable::add_ent(const NeighbEnt *e) {
	if( neighbTable.find(e->net_id) != neighbTable.end() ){
		neighbTable.erase(e->net_id);
	}
	neighbTable[e->net_id] = e;
	return;
}



//=============================EPARoutingAgent=========================================
void EPARoutingAgent::init(void){
	BEACON_RESCHED;
}

Packet* EPARoutingAgent::makeBeacon(){
	Packet *p = allocpkt();
	assert(p != NULL);

	hdr_ip *iph = hdr_ip::access(p);
	hdr_cmn *cmh = hdr_cmn::access(p);
	packet_hdr *packetHrd = packet_hdr::access(p);

	cmh->next_hop_ = IP_BROADCAST;
	cmh->direction() = hdr_cmn::DOWN;
	cmh->size() = packetHrd->size() + IP_HDR_LEN;
//	cmh->ptype = PT_MAX_EPA;

	iph->daddr() = IP_BROADCAST << Address::instance().nodeshift();
	iph->saddr() = mn_->address();
	iph->daddr() = IP_BROADCAST;
	iph->dport() = EPA_PORT;

	mn_->getLoc(&(packetHrd->x), &(packetHrd->y), &(packetHrd->z));
	packetHrd->mode = EPA_BEACON;
	packetHrd->mhops = 1;
	packetHrd->neighb_1hop = getNeighbors_1hop();
	return p;
}

void EPARoutingAgent::sendBeacon(void) {
	Packet *p = makeBeacon();
	hdr_cmn *cmh = hdr_cmn::access(p);

	if (p) {
		assert(!HDR_CMN(p)->xmit_failure_);
		if (cmh->direction() == hdr_cmn::UP)
			cmh->direction() = hdr_cmn::DOWN;
		Scheduler::instance().schedule(ll, p, 0);
	} else {
		fprintf(stderr, "ERROR: Can't make new beacon!\n");
		abort();
	}
}

std::vector<NeighbEnt> EPARoutingAgent::getNeighbors_1hop(){
	std::vector<NeighbEnt> neighb_1hop;
	map<nsaddr_t,NeighbEnt*>::iterator iter;
	for(iter = neighbTable_1hop.neighbTable.begin(); iter != neighbTable_1hop.neighbTable.end(); ++iter){
		neighb_1hop.push_back(*(iter->second));
	}
	return neighb_1hop;
}


std::map<nsaddr_t, NeighbTable> EPARoutingAgent::filterNodes(){
	map<nsaddr_t, NeighbTable>::iterator iter;
	for(iter = neighbTable_2hop.begin(); iter != neighbTable_2hop.end(); ++iter){
		nsaddr_t neighb = iter->first;
		NeighbTable neighTable = iter->second;
		map<nsaddr_t, NeighbEnt*>::iterator iter2;
		for(iter2 = neighTable.neighbTable.begin(); iter2 != neighTable.neighbTable.end(); ++iter2) {
			nsaddr_t addr = iter2->first;
			NeighbEnt* neighEnt = iter2->second;
			double dis = distance(neighb, neighEnt);
			if( dis > EPA_RANGE ){
				neighTable.delete_ent(neighEnt);
			}
		}
	}
	return neighbTable_2hop;
}


void EPARoutingAgent::recv(Packet *p, Handler *){
	hdr_ip *iph = hdr_ip::access(p);
	hdr_cmn *cmh = hdr_cmn::access(p);
	packet_hdr *packetHrd = packet_hdr::access(p);

//	nsaddr_t src = Address::instance().get_nodeaddr(iph->saddr());
//	nsaddr_t dst = Address::instance().get_nodeaddr(iph->daddr());

	switch(packetHrd->mode){
	case EPA_BEACON:
		nsaddr_t sender_id = packetHrd->node_id;
		NeighbEnt* newEnt = new NeighbEnt(sender_id);
		newEnt->x = packetHrd->x;
		newEnt->y = packetHrd->y;
		newEnt->z = packetHrd->z;
		newEnt->hop = 1;
		newEnt->expireTime = EXPIRE_TIME;
		neighbTable_1hop.add_ent(newEnt);

		//Add 2-hop nodes
		if( neighbTable_2hop.find(sender_id) == neighbTable_2hop.end() ){
			NeighbTable *neighbTalbe = new NeighbTable();
			int len = packetHrd->neighb_1hop.size();

			for(int i = 0; i < len; i++){
				NeighbEnt* newEnt = new NeighbEnt(packetHrd->neighb_1hop[i].net_id);
				newEnt->x = packetHrd->neighb_1hop[i].x;
				newEnt->y = packetHrd->neighb_1hop[i].y;
				newEnt->z = packetHrd->neighb_1hop[i].z;
				newEnt->hop = 2;
				newEnt->expireTime = EXPIRE_TIME;
				neighbTalbe->add_ent(newEnt);
			}

			neighbTable_2hop[sender_id] = neighbTalbe;
		}
		break;

	case EPA_MSG:
		filterNodes();

	}
}


double EPARoutingAgent::distance(NeighbEnt e1, NeighbEnt e2){
	double dX = e1.x-e2.x;
	double dY = e1.y-e2.y;
	double dZ = e1.z-e2.z;
	return sqrt(dX * dX + dY * dY + dZ * dZ);
}

void EPARoutingAgent::calNeighEPA(){
	map<nsaddr_t, NeighbTable>::iterator iter;
	for(iter = neighbTable_2hop.begin(); iter != neighbTable_2hop.end(); ++iter){
		nsaddr_t neighb = iter->first;
		NeighbTable neighTable = iter->second;
		map<nsaddr_t, NeighbEnt*>::iterator iter2;
		for(iter2 = neighTable.neighbTable.begin(); iter2 != neighTable.neighbTable.end(); ++iter2) {

		}
	}
}





void BeaconHandler::handle(Event *e) {
	a_->sendBeacon();
}






