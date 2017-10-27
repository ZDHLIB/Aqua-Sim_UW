#include "protoname_rtable.h"

protoname_rtable::protoname_rtable(){}

void protoname_rtable::print(Trace* out){
    sprintf(out->pt_->buffer(), "P\tdest\tnext");
    out->pt_->dump();
    for (rtable_t::iterator it = rt_.begin(); it != rt_.end(); it++) {
        sprintf(out->pt_->buffer(), "P\t%d\t%d",(*it).first,(*it).second);
        out->pt_->dump();
    }
}

/**
 * Clear the routing table
 */
void protoname_rtable::clear(){
    rt_.clear();
}

/**
 * Remove an entry of routing table
 */
void protoname_rtable::rm_entry(nsaddr_t dest){
    rt_.erase(dest);
}

/**
 * Add an entry into routing table
 */
void protoname_rtable::add_entry(nsaddr_t dest, nsaddr_t next){
    rt_[dest] = next;
}

/**
 * Get next hop addr or return IP_BROADCAST
 */
nsaddr_t protoname_rtable::lookup(nsaddr_t dest){
    rtable_t::iterator it = rt_.find(dest);
    if( it == rt_.end() ){
        return IP_BROADCAST;
    }
    else {
        return (*it).second;
    }
}   


/**
 * Return the number of entries in the routing table
 */
u_int32_t protoname_rtable::size(){
    return rt_.size();
}





