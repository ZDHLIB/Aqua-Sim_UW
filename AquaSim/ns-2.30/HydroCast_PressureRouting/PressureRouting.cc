#include "PressureRouting.h"

int hdr_dbr_pkt::offset_;
static class DBRHeaderClass : public PacketHeaderClass
{

public:
    MyProtHeaderClass() : PacketHeaderClass("PacketHeader/DBRHeader", sizeof(hdr_dbr_pkt))
    {
        bind_offset(&hdr_dbr_pkt::offset_);
    }
} class_DBR_hdr;


static class DBRClass : public TclClass
{
public:
    DBRClass() : TclClass("Agent/DBR") {}
    TclObject* create (int argc, const char*const* argv)
    {
        assert(argc == 5);
        return (new PressureRoutingAgent((nsaddr_t)Address::instance().str2addr(argv[4])));
    }
} class_myprot;


PressureRoutingAgent::PressureRoutingAgent(nsaddr_t id) : Agent(PT_DBR)
{
    node = NULL;
    id_ = id;
    num_hops_=numeric_limits<int>::max();
    bind("depth_threshold", &depth_threshold);
}


int PressureRoutingAgent::command(int argc, const char*const* argv)
{
    if (argc == 2)
    {
        if (strcmp(argv[1],"cal-distance") == 0)
        {
            calDistance();
            return TCL_OK;
        }
        else if (strcmp(argv[1], "send2Sink") == 0)
        {
            sendMsg();
            return TCL_OK;
        }
        else if (strcmp(argv[1], "start") == 0)
        {
            return TCL_OK;
        }
        else if (strcmp(argv[1], "print-dist") == 0)
        {
            printf("Node [%d] - distance to sink [%d] - next hop[%d]\n", id_, num_hops_, next_hop_);
            return TCL_OK;
        }
    }
    else if (argc == 3)
    {
        if (strcmp(argv[1], "port-dmux") == 0)
        {
            dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
            if (dmux_ == 0)
            {
                fprintf (stderr, "%s: %s lookup of %s failed\n", __FILE__,argv[1], argv[2]);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        else if(strcmp(argv[1], "log-target") == 0 || strcmp(argv[1], "tracetarget") == 0)
        {
            logtarget = (Trace*) TclObject::lookup(argv[2]);
            if(logtarget == 0)
                return TCL_ERROR;
            return TCL_OK;

        }
    }
    return Agent::command(argc, argv);
}


void PressureRoutingAgent::recv(Packet* p, Handler*)
{
    struct hdr_cmn* ch = HDR_CMN(p);
    struct hdr_ip* ih = HDR_IP(p);
    struct hdr_dbr_pkt* pkt = HDR_DBR_PKT(p);

    u_int16_t pre_hop_depth = pkt->depth_;
    u_int16_t current_depth = depth_;

    u_int16_t d = pre_hop_depth - current_depth;

    if (fabs(d) < depth_threshold )
    {
        if(pressureRoutingTable.isExist(p))
        {
            pressureRoutingTable.rm_entry(p);
            Packet::free(p);
            return;
        }
    }

    if(pressureRoutingTable.isExist(p))
    {
        Packet::free(p);
        return;
    }

    pkt->depth_ = current_depth;

    // Compute holding time HT
    u_int16_t ST = NOW;

    if(pressureRoutingTable.isExist(p) )
    {
        ST = min(ST, pressureRoutingTable.getValue(p));
        pressureRoutingTable.update(p,ST);
    }
    else
    {
        pressureRoutingTable.add_entry(p, ST);
    }

    if (pkt->hops+1 < num_hops_)
    {
        num_hops_ = pkt->hops +1;
        next_hop_ = ih->saddr();
        sendMsg();
    }

    Packet::free(p);

}


void PressureRoutingAgent::sendMsg()
{

}


Packet* PressureRoutingAgent::create_packet()
{
    Packet *pkt = allocpkt();

    if (pkt==NULL) return NULL;

    hdr_cmn*  cmh = HDR_CMN(pkt);
    cmh->size() = 36;

    hdr_dbr_pkt* dbr = HDR_DBR_PKT(pkt);
    dbr->ts_ = NOW;

    dbr->info.ox=node->CX();
    dbr->info.oy=node->CY();
    dbr->info.oz=node->CZ();
    dbr->info.fx=node->CX();
    dbr->info.fy=node->CY();
    dbr->info.fz=node->CZ();

    return pkt;
}


void PressureRoutingAgent::calculatePosition(Packet* pkt)
{
    hdr_dbr_pkt* dbr = HDR_DBR_PKT(pkt);

    double fx = dbr->info.fx;
    double fy = dbr->info.fy;
    double fz = dbr->info.fz;

    double dx = dbr->info.dx;
    double dy = dbr->info.dy;
    double dz = dbr->info.dz;

    node->CX_ = fx + dx;
    node->CY_ = fy + dy;
    node->CZ_ = fz + dz;
    printf("%d position is computed as (%f,%f,%f)\n",dbr->myId_, node->CX_, node->CY_,node->CZ_);
}


double PressureRoutingAgent::distance(Packet* pkt)
{
    hdr_dbr_pkt* dbr = HDR_DBR_PKT(pkt);

    double tx = dbr->info.fx;
    double ty = dbr->info.fy;
    double tz = dbr->info.fz;
    printf("The target is %lf,%lf,%lf \n",tx,ty,tz);

    double x = node->CX();
    double y = node->CY();
    double z = node->CZ();
    printf("the target is %lf,%lf,%lf and my coordinates are %lf,%lf,%lf\n",tx,ty,tz,x,y,z);

    return sqrt((tx-x)*(tx-x)+(ty-y)*(ty-y)+ (tz-z)*(tz-z));
}


int main()
{
    printf("SAFASFASDdsdfsadfaefedfasfawefdc");
    return 0;
}

