#ifndef SWITCH_H
#define SWITCH_H

#include <memory>
#include <unordered_map>
#include "utils/types.hpp"
#include "topology/link.hpp"
#include "traffic/packet.hpp"


typedef std::unordered_map<switch_id_t, switch_id_t> routing_table_t;
typedef std::pair<switch_id_t, switch_id_t> routing_table_pair;
typedef std::unordered_map<switch_id_t, link_p> neighbor_switch_table_t;
typedef std::pair<switch_id_t, link_p> neighbor_switch_table_pair; 
typedef std::unordered_map<host_id_t, link_p> neighbor_host_table_t;
typedef std::pair<host_id_t, link_p> neighbor_host_table_pair;


typedef struct routeInfo
{
    next_hop_id nextHopId;
    NextNodeType nextHopType;
    link_p nextLink;
} routeInfo;



/* Switch struct */
typedef struct Switch
{
    switch_id_t id;
    sim_time_t hop_delay;
    neighbor_switch_table_t neighborSwitchTable;
    neighbor_host_table_t neighborHostTable;
    routing_table_t routingTable;

    /* Switch's processing on receiving a pkt: logging, SyNDB, etc. */
    void receiveNormalPkt(normalpkt_p pkt);
    void receiveTriggerPkt(triggerpkt_p pkt);

    /* Fills rinfo. Returns Success or Failure */
    status_t routeNormalPkt(normalpkt_p pkt, routeInfo &rinfo);
    status_t routeTriggerPkt(triggerpkt_p pkt, routeInfo &rinfo);
    status_t routeToDstSwitch(switch_id_t dstSwitchId, routeInfo &rinfo);

    Switch() = default; 
    Switch(switch_id_t id);

} Switch;

typedef std::shared_ptr<Switch> switch_p;


/* Routing Table struct */


#endif
