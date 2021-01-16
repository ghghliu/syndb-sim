#ifndef DEVTESTS_H
#define DEVTESTS_H

#include "utils/types.hpp"

/* 
Iterates over all links (ToR and Network).
For each link, shows next_idle_time in both directions.
Independent of topology. But too many links give cumbersome output.
*/
void checkRemainingQueuingAtLinks();

/* 
Prints OWD between for delivered pkt between two hosts h0 <--> h1. 
Also compares IPG seen at (h0 Tx vs. h1 RX) and (h1 Tx vs. h0 Rx).

Note: Use with small total runtime as it records entry in global 
syndbSim.pktLatencyMap for every generated pkt. 

Ues glue code in Host->generateNextPkt() and Simulation->processNormalPktEvents().
*/
void testNormalPktLatencies(host_id_t h0, host_id_t h1);

/* 
Quick test for shared_ptr destruction.
Independent of any topology.
*/
void testSharedPtrDestruction();




#endif
