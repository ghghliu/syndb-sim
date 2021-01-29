#include <string>
#include <fmt/core.h>

#include "simulation/config.hpp"
#include "simulation/simulation.hpp"
#include "utils/logger.hpp"
#include "utils/utils.hpp"
#include "topology/fattree_topology.hpp"


Simulation syndbSim;

Simulation::Simulation(){

    this->currTime = 0;
    this->timeIncrement = syndbConfig.timeIncrementNs;
    this->totalTime = (sim_time_t)(syndbConfig.totalTimeMSecs * (float)1000000);

    if(syndbConfig.topoType == TopologyType::Simple){
        this->topo = std::shared_ptr<Topology>(new SimpleTopology());
    }
    else if (syndbConfig.topoType == TopologyType::FatTree){
        this->topo = std::shared_ptr<Topology>(new FattreeTopology(syndbConfig.fatTreeTopoK));
    }

    this->nextPktId = 0;
    this->nextTriggerPktId = 0;

}

void Simulation::initTriggerGen(){
    switch(syndbConfig.topoType){
        case TopologyType::Simple:
            this->triggerGen = std::shared_ptr<TriggerGenerator>(new TriggerGeneratorSimpleTopo());
            break;
        case TopologyType::FatTree:
            this->triggerGen = std::shared_ptr<TriggerGenerator>(new TriggerGeneratorFatTreeTopo());
            break;
    }
}

void Simulation::initHosts(){

    auto it = this->topo->hostIDMap.begin();

    while (it != this->topo->hostIDMap.end() )
    {
        host_p h = it->second;
        h->generateNextPkt();

        it++;
    }

    debug_print(fmt::format("Initialized {} hosts!", this->topo->hostIDMap.size()));
    
}

void Simulation::processHosts(){

    // debug_print_yellow("Inside process hosts");
    // debug_print("Num hosts: {}", this->topo->hostIDMap.size());

    auto it = this->topo->hostIDMap.begin();

    while (it != this->topo->hostIDMap.end() )
    {
        host_p h = it->second;

        if(this->currTime >= h->nextPktTime){
            h->sendPkt();
        }
        

        it++;
    }
}



void Simulation::processTriggerPktEvents(){
    
    routeScheduleInfo rsinfo;

    // List of iterators that we would delete in the end
    std::list<std::list<pktevent_p<triggerpkt_p>>::iterator> toDelete;

    toDelete.clear(); // Making sure that the list is empty

    auto it = this->TriggerPktEventList.begin();

    while (it != this->TriggerPktEventList.end())
    {
        pktevent_p<triggerpkt_p> event = *it;

        if(this->currTime >= event->pktForwardTime){

            // Pass the pkt to the next switch to handle
            event->nextSwitch->receiveTriggerPkt(event->pkt, event->pktForwardTime); // can parallelize switch's processing? 

            // Handling the case that the next hop is the pkt's dstSwitch
            if(event->nextSwitch->id == event->pkt->dstSwitchId){
                // The pkt requires no more network event processing
                // Add the event's iterator to the toDelete list
                toDelete.push_front(it); 
            }
            else // forward the event's packet
            {
                // Call routing on the next switch
                syndb_status_t status = event->nextSwitch->routeScheduleTriggerPkt(event->pkt, event->pktForwardTime, rsinfo);

                if(status != syndb_status_t::success){
                    std::string msg = fmt::format("Simulator failed to route trigger pkt of trigger event {}", event->pkt->triggerId);
                    throw std::logic_error(msg);
                }

                // event->doForwarding(rinfo);

                event->currSwitch = event->nextSwitch;
                event->nextSwitch = rsinfo.nextSwitch;
                event->pktForwardTime = rsinfo.pktNextForwardTime;
                
            }                
        }

        it++;  // iterating over TriggerPktEventList
    }

    // Now delete the enlisted TriggerPktEvents

    // debug_print(fmt::format("Deleting {} TriggerPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        TriggerPktEventList.erase(*it2);

        it2++;
    }

    
}


void Simulation::processNormalPktEvents(){

    routeScheduleInfo rsinfo;

    // List of iterators that we would delete in the end
    std::list<std::list<pktevent_p<normalpkt_p>>::iterator> toDelete;

    toDelete.clear(); // Making sure that the list is empty

    auto it = this->NormalPktEventList.begin();

    while (it != this->NormalPktEventList.end())
    {
        pktevent_p<normalpkt_p> event = *it;

        if(this->currTime >= event->pktForwardTime){

            
            // Handling the case that the next hop is the dst Host
            if(event->nextSwitch == NULL){
                // Mark the event for deletion
                toDelete.push_back(it);
                

                // Add end time INT data to the packet
                event->pkt->endTime = event->pktForwardTime;

                // Dump the pkt with INT data to the disk
                syndbSim.pktDumper.dumpPacket(event->pkt);

                #ifdef DEBUG
                /* debug_print_yellow("\nPkt ID {} dump:", event->pkt->id);
                debug_print("h{} --> h{}: {} ns (Start: {} ns | End: {} ns)", event->pkt->srcHost, event->pkt->dstHost, event->pkt->endTime - event->pkt->startTime, event->pkt->startTime, event->pkt->endTime);
                auto it1 = event->pkt->switchINTInfoList.begin();
                for(it1; it1 != event->pkt->switchINTInfoList.end(); it1++){
                    debug_print("Rx on s{} at {} ns", it1->swId, it1->rxTime);
                } */
                #endif
            }
            // Handling the case that the next hop is a switch (intermediate or dstTor)
            else{
                
                // Pass the pkt to the next switch to handle
                event->nextSwitch->receiveNormalPkt(event->pkt, event->pktForwardTime); // can parallelize switch's processing?

                // Call routing on the next switch
                syndb_status_t status = event->nextSwitch->routeScheduleNormalPkt(event->pkt, event->pktForwardTime, rsinfo);
                
                
                /* Update the event */
                event->currSwitch = event->nextSwitch;
                event->nextSwitch = rsinfo.nextSwitch; // will be NULL if next hop is a host
                event->pktForwardTime = rsinfo.pktNextForwardTime;               

            }

        } // end of the if(this->currTime >= event->pktForwardTime)

        it++;  // iterating over NormalPktEventList
    } 

    // Now delete the enlisted NormalPktEvents

    // debug_print(fmt::format("Deleting {} NormalPktEvents...", toDelete.size()));
    auto it2 = toDelete.begin();

    while (it2 != toDelete.end()){
        NormalPktEventList.erase(*it2);
        it2++;
    }


}


void Simulation::flushRemainingNormalPkts(){

    auto it = this->NormalPktEventList.begin();

    for(it; it != this->NormalPktEventList.end(); it++){
        pktevent_p<normalpkt_p> event = *it;

        // Dump the pkt with INT data to the disk
        syndbSim.pktDumper.dumpPacket(event->pkt);

        #ifdef DEBUG
        /* debug_print_yellow("\nPkt ID {} dump:", event->pkt->id);
        debug_print("h{} --> h{}: N/A ns (Start: {} ns | End: {} ns)", event->pkt->srcHost, event->pkt->dstHost, event->pkt->startTime, event->pkt->endTime);
        auto it1 = event->pkt->switchINTInfoList.begin();
        for(it1; it1 != event->pkt->switchINTInfoList.end(); it1++){
            debug_print("Rx on s{} at {} ns", it1->swId, it1->rxTime);
        } */
        #endif
    }

}

void Simulation::dumpTriggerInfoMap(){
    sim_time_t triggerOriginTime, rxTime; 
    trigger_id_t triggerId;
    switch_id_t originSwitch, rxSwitch;

    auto it1 = syndbSim.TriggerInfoMap.begin();

    ndebug_print_yellow("\nTrigger pkt latencies between switches");
    for(it1; it1 != syndbSim.TriggerInfoMap.end(); it1++){
        triggerId = it1->first;
        triggerOriginTime = it1->second.triggerOrigTime;
        originSwitch = it1->second.originSwitch;
        const SwitchType switchType = syndbSim.topo->getSwitchTypeById(originSwitch);

        ndebug_print_yellow("Trigger ID {} (origin switch: {} {})", triggerId, originSwitch, switchTypeToString(switchType));
        auto it2 = it1->second.rxSwitchTimes.begin();

        for(it2; it2 != it1->second.rxSwitchTimes.end(); it2++){
            rxSwitch = it2->first;
            rxTime = it2->second;

            ndebug_print("{} --> {}: {}ns", originSwitch, rxSwitch, rxTime - triggerOriginTime);
        } // end of iterating over rxSwitchTimes

    } // end of iterating over TriggerPktLatencyMap
}


void Simulation::cleanUp(){
    
    // Why this is needed? When std::list is destroyed, if its members are pointers, only the pointers are destroyed, not the objects pointed by the pointers.
    // BUT for shared_ptrs, when they are deleted, they do destruct the member objects.
    // SO this function is actually NOT really required.

    // Clean-up the members of the syndbSim

    // Clean-up the topo
}
