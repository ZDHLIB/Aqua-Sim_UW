/*
 This program is the modified version of phy.cc, it supports the periodic operation of sensor nodes---modified by xp at 2007
 */

#include <HydroCast_PressureRouting/underwaterEPAphy.h>
#include <math.h>
#include <packet.h>
#include <phy.h>
#include "underwatersensor/uw_mac/underwaterpropagation.h"
#include "underwatersensor/uw_common/underwatersensornode.h"
#include <modulation.h>

#include <packet.h>
#include "random.h"

void Underwater_Idle_Timer::expire(Event *) {
	a_->UpdateIdleEnergy();
}

/* ======================================================================
 UnderwaterPhy Interface
 ====================================================================== */
static class UnderwaterPhyClass: public TclClass {
public:
	UnderwaterPhyClass() :
			TclClass("Phy/UnderwaterEPAPhy") {
	}
	TclObject* create(int, const char* const *) {
		return (new UnderwaterPhy);
	}
} class_UnderwaterPhy;

UnderwaterPhy::UnderwaterPhy() :
		Phy(), idle_timer_(this)
/*,sense_handler(this)*/
{
	/*
	 *  It sounds like 10db should be the capture threshold.
	 *
	 *  If a node is presently receiving a packet a a power level
	 *  Pa, and a packet at power level Pb arrives, the following
	 *  comparion must be made to determine whether or not capture
	 *  occurs:
	 *
	 *    10 * log(Pa) - 10 * log(Pb) > 10db
	 *
	 *  OR equivalently
	 *
	 *    Pa/Pb > 10.>ResetSenseStatus();
	 *
	 */
	initTransitTime();

	bind("CPThresh_", &CPThresh_);
	bind("CSThresh_", &CSThresh_);
	bind("RXThresh_", &RXThresh_);
	bind("Pt_", &Pt_);
	bind("freq_", &freq_);
	bind("L_", &L_);
	bind("K_", &K_);
	bind("TurnOnEnergy", &Energy_turn_on_);
	bind("TurnOffEnergy", &Energy_turn_off_);
	bind("EnergyPerBit", &Energy_perbit);
	bind("NoisePwDensity", &NoisePwDensity);

	bind("sync_hdr_len", &sync_hdr_len_);
	bind("fowarding_delay", &fowarding_delay_);

	/*
	 bind("time_idle2send", &TransitTime[IDLE][SEND]);
	 bind("time_send2idle", &TransitTime[SEND][IDLE]);
	 bind("time_idle2recv", &TransitTime[IDLE][RECV]);
	 bind("time_recv2idle", &TransitTime[RECV][IDLE]);
	 bind("time_send2recv", &TransitTime[SEND][RECV]);
	 bind("time_recv2send", &TransitTime[RECV][SEND]);
	 */

	bind("bit_error_rate", &bit_error_rate_);

	node_ = 0;
	propagation_ = 0;
	modulation_ = 0;
	ant_ = 0;

	Pt_consume_ = 0.660;  // 1.6 W drained power for transmission
	Pr_consume_ = 0.395;  // 1.2 W drained power for reception
	P_idle_ = 0.0;  // 0 W drained power for idle

	update_energy_time_ = NOW;

	power_status = 1;
	// 	printf("underwaterphy: schedule update energy at %f\n",NOW);
	idle_timer_.resched(1.0);
	//Energy_turn_on_ = 0.0;
	//Energy_turn_off_ = 0.0;
}

int UnderwaterPhy::command(int argc, const char* const * argv) {
	TclObject *obj;

	if (argc == 2) {
		if (strcasecmp(argv[1], "PowerOn") == 0) {
			if (em() == NULL)
				return TCL_OK;
			power_on();
			return TCL_OK;
		} else if (strcasecmp(argv[1], "PowerOff") == 0) {
			if (em() == NULL)
				return TCL_OK;
			power_off();
			return TCL_OK;
		}

	} else if (argc == 3) {
		if (strcasecmp(argv[1], "setTxPower") == 0) {
			Pt_consume_ = atof(argv[2]);
			//   printf("wireless-phy.cc: Pt_consume_=%f\n",Pt_consume_);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setRxPower") == 0) {
			Pr_consume_ = atof(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setIdlePower") == 0) {
			P_idle_ = atof(argv[2]);
			return TCL_OK;
		} else if ((obj = TclObject::lookup(argv[2])) == 0) {
			fprintf(stderr, "UnderwaterPhy: %s lookup of %s failed\n", argv[1],
					argv[2]);
			return TCL_ERROR;
		} else if (strcmp(argv[1], "propagation") == 0) {
			assert(propagation_ == 0);
			propagation_ = (Propagation*) obj;
			return TCL_OK;
		} else if (strcmp(argv[1], "antenna") == 0) {
			assert(ant_ == 0);
			ant_ = (Antenna*) obj;
			return TCL_OK;
		} else if (strcasecmp(argv[1], "node") == 0) {
			assert(node_ == 0);
			node_ = (Node *) obj;
			return TCL_OK;
		}
	}
	return Phy::command(argc, argv);
}

void UnderwaterPhy::sendDown(Packet *p) {
	UnderwaterSensorNode* n1;
	//int it=hdr_cmn::access(p)->next_hop();
	//printf("ok, node %d is underwaterphy senddown to node %d at %f pidel is%f pt=%f and pr=%f\n",node_->address(),it,NOW,P_idle_,Pt_consume_,Pr_consume_);

	//hdr_cmn* cmnh=HDR_CMN(p);

	// printf("underwater phy: node %d, got the down packet  ptytype=%d at %f\n", node_->address(),cmnh->ptype_,NOW);   

	/*
	 * Sanity Check
	 */

	assert(initialized());

	n1 = (UnderwaterSensorNode*) node_;

	// this part is moved from routing protocol by peng xie at 12-20-06

	if (n1->failure_status() == 1) {
		printf("underwaterphy %d: fails!!!!\n ", node_->address());
		Packet::free(p);
		return;
	}

	TransmissionStatus status = n1->TransmissionStatus();
	// printf("underwater phy node %d: sendDown status is %d\n",
	//         node_->address(),status);
	if (em()) {
		if ((status == SLEEP) || (em()->energy() <= 0)) {
			Packet::free(p);
			return;
		}
	}
	// node is power-on and energy is greater 
	switch (status) {

	case SEND: {
		//  printf("underwater phy node %d: I am in sending state\n",node_->address());
		double txtime = hdr_cmn::access(p)->txtime();

		double start_time = NOW;
		double end_time = NOW+txtime;

		// update energy
		if (start_time >= update_energy_time_) {
			em()->DecrIdleEnergy(start_time - update_energy_time_, P_idle_);
			update_energy_time_ = start_time;
		}

		//minus the forwarding delay part
		//suppose modem cannot recv when the packet is experiencing forwarding delay
		em()->DecrIdleEnergy(forwarding_delay(), P_idle_);

		em()->DecrTxEnergy(txtime - forwarding_delay(), Pt_consume_);
		update_energy_time_ = end_time;
		break;
	}
	case IDLE: {
		/* something wrong in this case since the transmitter is
		 transmitting some packet while a new packet arrives.
		 In this simulator, we can't handle this case
		 */
		printf(
				"UnderwaterPhy node(%d):mac forget to change the status at time %f\n",
				node_->address(), NOW);

		break;
	}
	case SLEEP: printf("underwaterphy: I am sleeping!!\n");
	break;
	default: printf("underwaterphy: no such default at all\n");
}			  //end of switch

/*
 *  Stamp the packet with the interface arguments
 */
	if (!ant_)
		printf("ok, there is no antenna\n");
	p->txinfo_.stamp((MobileNode*) node(), ant_->copy(), Pt_, lambda_);

	channel_->recv(p, this);
}

int UnderwaterPhy::sendUp(Packet *p) {
	/*
	 * Sanity Check
	 */

	UnderwaterSensorNode* n1;
	hdr_cmn* cmnh = HDR_CMN(p);

	//  printf("underwater phy: node %d, got the upgoing packet  ptytype=%d at %f\n", node_->address(),cmnh->ptype_,NOW);
	assert(initialized());

	double rcvtime = hdr_cmn::access(p)->txtime();

	n1 = (UnderwaterSensorNode*) node_;

	// this part is moved from routing protocol by peng xie at 12-20-06 

	if (n1->failure_status() == 1) {
		printf("underwaterphy %d: fails!!!!\n ", node_->address());
		Packet::free(p);
		return 0;
	}
	// mark the error on the incoming packet with probability recv_pro
	/*here only set the percent of error packets*/
	double recv_pro = n1->failure_pro();
	double error_pro = Random::uniform();
	if (error_pro <= recv_pro)
		cmnh->error() = 1;

	TransmissionStatus status = n1->TransmissionStatus();
	PacketStamp s;
	double Pr;
	double d;
	int pkt_recvd = 0;
	double pkt_error_pro = 0;
	hdr_cmn* hdr = HDR_CMN(p);
	hdr_dbr* dbr = hdr_dbr::access(p);

	// if the node is in sleeping mode, drop the packet simply

	if (em())
		if ((status == SEND || status == SLEEP) || (em()->energy() <= 0)) {
			printf("receive failure: SEND, SLEEP or NO-ENERGY\n");
			return pkt_recvd;
		}

	if (propagation_) {
		printf("underwaterphy: propagation_\n");
		s.stamp((MobileNode*) node(), ant_, 0, lambda_);
		Pr = propagation_->Pr(&p->txinfo_, &s, this);
		if (Pr < CSThresh_) {
			printf("underwaterphy: Pr<CSThresh_,signal is too weak\n");
			return pkt_recvd;
		}

	}

	if (modulation_) {
		hdr->error() = modulation_->BitError(Pr);
	} else {
//			hdr->error() = DefaultBitError(hdr->size());
		d = calDistance(&p->txinfo_, &s);
		pkt_error_pro = epaErrorProb(d, freq_);
		if (Random::uniform() < pkt_error_pro )
			hdr->error() = 1;
		else
			hdr->error() = 0;

	}

	p->txinfo_.RxPr = Pr;
	p->txinfo_.CPThresh = CPThresh_;

	double start_time = NOW;
	double end_time = NOW+rcvtime;

	if (start_time > update_energy_time_) {
		em()->DecrIdleEnergy(start_time - update_energy_time_, P_idle_);
		em()->DecrRcvEnergy(rcvtime, Pr_consume_);
		update_energy_time_ = end_time;
	} else {
		/* In this case, this node is receiving some packet*/
		if (end_time > update_energy_time_) {
			em()->DecrRcvEnergy(end_time - update_energy_time_, Pr_consume_);
			update_energy_time_ = end_time;
		}
	}

	double epaValue = epaSuccessProb(pkt_error_pro, hdr->size(), d, freq_);
	if (epaValue == 0) {
		printf("underwaterphy: epa transmit failed\n");
		return pkt_recvd;
	} else {
		dbr->epaValue = epaValue;
	}

//		if (Pr < RXThresh_) {
//
//			/*
//			* We can detect, but not successfully receive
//			* this packet.
//			*/
//			printf("underwaterphy: Pr<RXThresh_,signal is too weak\n");
//			return pkt_recvd;
//		}

	if (em()->energy() <= 0) {
		// saying node died
		em()->setenergy(0);
		((MobileNode*) node())->log_energy(0);
	}
	//	printf("underwaterphy: received!!!!\n");

	return 1;
}

double UnderwaterPhy::calDistance(PacketStamp * t, PacketStamp * r) {
	double Xt, Yt, Zt;		// location of transmitter
	double Xr, Yr, Zr;		// location of receiver

	t->getNode()->getLoc(&Xt, &Yt, &Zt);
	r->getNode()->getLoc(&Xr, &Yr, &Zr);

	double dX = Xr - Xt;
	double dY = Yr - Yt;
	double dZ = Zr - Zt;
	return sqrt(dX * dX + dY * dY + dZ * dZ);
}

int UnderwaterPhy::DefaultBitError(int pktsize) {
	double pkt_correct_pro = pow(1 - bit_error_rate_, pktsize);

	if (Random::uniform() < pkt_correct_pro)
		return 0;
	else
		return 1;
}

double UnderwaterPhy::epaErrorProb(double d, double f) {
	PacketStamp s;
	s.stamp((MobileNode*) node(), ant_, 0, lambda_);
	double srn = calSRN(d, f);
	double pkt_error_pro = (1 - (sqrt(srn / (1 + srn)))) / 2;
	return pkt_error_pro;
}

double UnderwaterPhy::epaSuccessProb(double pkt_error_pro, int pktsize, double d, double f) {
	double successCorrect = pow((1 - pkt_error_pro), pktsize);
	if (Random::uniform() < successCorrect)
		return successCorrect;
	else
		return 0;
}

double UnderwaterPhy::calSRN(double d, double f) {
	return (Energy_perbit / (NoisePwDensity * d * Thorp(f)));
}

double UnderwaterPhy::Thorp(double f) {
	return 0.11 * ((f * f) / (1.0 + (f * f)))
			+ 44.0 * ((f * f) / (4100.0 + (f * f))) + 2.75 * ((f * f) / 10000.0)
			+ 0.0003;
}

void UnderwaterPhy::ResetSenseStatus() {
	return;

	/*
	 since we move the sense carrier to mac layer, this function
	 does nothing.

	 UnderwaterSensorNode* n1=(UnderwaterSensorNode*) node_;
	 n1->ResetCarrierSense();
	 */
}

void UnderwaterPhy::power_on() {
	power_status = 1;
	if (em() == NULL)
		return;
	//minus the energy consumed by power on, Added by Yibo
	if (em()->energy() < Energy_turn_on_)
		em()->setenergy(0.0);
	else
		em()->setenergy(em()->energy() - Energy_turn_on_);

	if (NOW> update_energy_time_) {
		update_energy_time_ = NOW;
	}
}

void UnderwaterPhy::power_off() {
	power_status = 0;
	if (em() == NULL)
		return;

	//minus the energy consumed by power on. Added by Yibo
	if (em()->energy() < Energy_turn_off_)
		em()->setenergy(0.0);
	else
		em()->setenergy(em()->energy() - Energy_turn_off_);

	if (NOW> update_energy_time_) {
		em()->DecrIdleEnergy(NOW-update_energy_time_,
		P_idle_);
		update_energy_time_ = NOW;
	}

	printf("node %d is poweroff energy is %f at %f\n", node_->address(),
			em()->energy(), NOW);

}

void UnderwaterPhy::dump(void) const {
	Phy::dump();
	fprintf(stdout, "\tPt: %f, Gt: %f, Gr: %f, lambda: %f, L: %f\n", Pt_,
			ant_->getTxGain(0, 0, 0, lambda_),
			ant_->getRxGain(0, 0, 0, lambda_), lambda_, L_);
	//fprintf(stdout, "\tbandwidth: %f\n", bandwidth_);
	fprintf(stdout, "--------------------------------------------------\n");
}

void UnderwaterPhy::UpdateIdleEnergy() {
	if (power_status == 0)
		return;
	if (em() == NULL) {
		return;
	}
	if (NOW> update_energy_time_ && em()->node_on()) {
		em()-> DecrIdleEnergy(NOW-update_energy_time_,
		P_idle_);
		update_energy_time_ = NOW;
	}

	// log node energy
	if (em()->energy() > 0) {
		((MobileNode *) node_)->log_energy(1);
	} else {
		((MobileNode *) node_)->log_energy(0);
	}

	//    printf("underphy: schedule energy update at %f \n",NOW);
	idle_timer_.resched(1.0);
}

void UnderwaterPhy::status_shift(double txtime) {

	//   double txtime = hdr_cmn::access(p)->txtime();
	double end_time = NOW+txtime;
	/*  The receiver is receiving a packet when the 
	 transmitter begins to transmit a data.
	 We assume the half-duplex mode, the transmitter
	 stops the receiving process and begins the sending
	 process.
	 */

	if(update_energy_time_<end_time)
	{
		double overlap_time=update_energy_time_-NOW;
		double actual_txtime=end_time-update_energy_time_;
		em()->DecrTxEnergy(overlap_time,
				Pt_consume_-Pr_consume_);
		em()->DecrTxEnergy(actual_txtime,Pt_consume_);
		update_energy_time_=end_time;
	}
	else {
		double overlap_time=txtime;
		em()->DecrTxEnergy(overlap_time,
				Pt_consume_-Pr_consume_);
	}
}

