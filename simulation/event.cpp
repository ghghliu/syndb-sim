#include <fmt/core.h>
#include "simulation/event.hpp"
#include "simulation/simulation.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"

template<typename T>
PktEvent<T>::PktEvent(){
    // debug_print(fmt::format("New empty PktEvent constructed!"));
}

template<>
PktEvent<normalpkt_p>::~PktEvent(){
    /* if(this->pkt != NULL)
        debug_print(fmt::format("Normal Pkt Event with packet id {} destructed!", this->pkt->id));
    else
        debug_print(fmt::format("Normal Pkt Event (empty) destructed!")); */
}

template<>
PktEvent<triggerpkt_p>::~PktEvent(){
    // debug_print(fmt::format("Trigger Pkt Event with trigger id {} destructed!", this->pkt->triggerId));
}


HostPktEvent::HostPktEvent(Host* host, normalpkt_p &pkt){
    this->host = host;
    this->pkt = pkt;
}

/* 
    To avoid linking error for the template's methods,
    instantiating the template class with the two possible types
*/

template class PktEvent<normalpkt_p>;
template class PktEvent<triggerpkt_p>;
