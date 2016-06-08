// MF_StgRouter.click: cloned from MF_Router.click. In addition, this one has transport proxy component.
// In MF_Router.click, [0]intra_lkup[0]. Now:
// prx[0] --> [1]intra_lkup; prx[1] --> [2]intra_lkup
// intra_lkup[1] --> prx[0]; intra_lkup[2] --> prx[1].

// Parameters:
// my_GUID
// topo_file - GUID-based topology file 
// core_dev
// GNRS_server_port - listening port on server, assumes localhost gnrs
// GNRS_listen_ip - IP assoc w/ interface GNRS listens on
// GNRS_listen_port - response listening port for gnrs clients

// Maintains router-wide resource stats, etc. 
routerstats::MF_RouterStats;

//Control path elements
arp_tbl::MF_ARPTable();
nbr_tbl::MF_NeighborTable();
rtg_tbl::MF_RoutingTable(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl);
lp_hndlr::MF_LinkProbeHandler(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl, ROUTING_TABLE rtg_tbl);
lsa_hndlr::MF_LSAHandler(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl, ROUTING_TABLE rtg_tbl)

//Data path elements

chk_mngr::MF_ChunkManager();
resp_cache::GNRS_RespCache(); 
agg::MF_Aggregator(MY_GUID $my_GUID, ROUTER_STATS routerstats, CHUNK_MANAGER chk_mngr);
net_binder::MF_NetworkBinder(CHUNK_MANAGER chk_mngr);
intra_lkup::MF_IntraLookUp(MY_GUID $my_GUID, FORWARDING_TABLE rtg_tbl, CHUNK_MANAGER chk_mngr);
seg::MF_Segmentor(ROUTER_STATS routerstats, CHUNK_MANAGER chk_mngr);
// transport-related elements
stg_mgr::MF_StorageManager(CHUNK_MANAGER chk_mngr, STORAGE_CAPACITY 1073741824);
fctrl::MF_FlowController(STORAGE_MANAGER stg_mgr);
prx::MF_TransProxy(MY_GUID $my_GUID, STORAGE_MANAGER stg_mgr, FLOW_CTRL fctrl);


//enforces stated topology
topo_mngr::MF_TopologyManager(MY_GUID $my_GUID, TOPO_FILE $topo_file, ARP_TABLE arp_tbl);

//Counters and Statistics
//TODO: build a custom chunk counter to get proper data byte count
//incoming L2 pkt and byte count 
inCntr_pkt::Counter()
//incoming L3 chunk and byte count 
inCntr_chunk::Counter()
//outgoing L2 pkt and byte count 
outCntr_pkt::Counter

//Queues
inQ::ThreadSafeQueue(65535); //incoming L2 pkt Q
net_binderQ::ThreadSafeQueue(100); //chunk Q prior to NA resolution 
//rtgQ::ThreadSafeQueue(1000);          //Queue for intra_lookup; unused in this script
segQ::ThreadSafeQueue(100); //segmentor ready chunk Q
outQ_ctrl::ThreadSafeQueue(100); //L2 outgoing high priority control pkt queue
outQ_data::Queue(65535); //L2 outgoing lower priority data pkt queue
outQ_sched::PrioSched; //priority sched for L2 pkts
outQ_core::Queue(65535); //L2 outgoing pkt Q for 'core' port

//L2 packet sources
//core port
fd_core::FromDevice($core_dev, PROMISC false, SNIFFER true);

//L2 out i/f
td_core::ToDevice($core_dev);

//Core interface - port 0
fd_core 
        -> HostEtherFilter($core_dev, DROP_OWN true)
	//drop anything that isn't of MF eth type
        -> core_cla::Classifier(12/27C0, -)
        -> SetTimestamp 
        -> MF_Learn(IN_PORT 0, ARP_TABLE arp_tbl) // learn src eth, port
	-> Strip(14) // strip eth header; assumes no VLAN tag
        -> inQ;

core_cla[1] -> Discard;

//start incoming pkt processing
inQ -> Unqueue
	-> topo_mngr
	-> inCntr_pkt
	-> mf_cla::Classifier(
			00/00000003, // p0 Link probe
			00/00000004, // p1 Link probe response
			00/00000005, // p2 LSA
			00/00000000, // p3 Data
			00/00000001, // p4 CSYN
			00/00000002, // p5 CSYN-ACK
			00/00000006, // p6 Client association
			00/00000007, // p7 Client dis-association 
			-);          // p8 Unhandled type, discard

mf_cla[6] -> Discard; //No client access in this config
mf_cla[7] -> Discard; //No client access in this config
mf_cla[8] -> Discard;

// routing control pkts

mf_cla[0] -> [0]lp_hndlr; // link probe
mf_cla[1] -> [1]lp_hndlr; // link probe ack
mf_cla[2] -> [0]lsa_hndlr; // lsa

// data and Hop signalling pkts

mf_cla[3] -> [0]agg; // data 
mf_cla[4] -> [1]agg; // csyn 
mf_cla[5] -> [1]seg; // csyn-ack

// Net-level processing for aggregated data blocks (chunks)

agg[0] //incomplete chunks assembled post Hop transfer from upstream node
	-> aggregator_classifier_incomplete::Classifier(
    	08/00000000,    //upper protocol for normal data
    	-);
    	
aggregator_classifier_incomplete[0] -> inCntr_chunk 
			-> net_binderQ; //chunk queue prior to NA resolution
aggregator_classifier_incomplete[1] -> Discard


agg[2] //complete chunks assembled post Hop transfer from upstream node
	-> Discard;

net_binderQ 
	-> Unqueue 
	-> [0]net_binder;

net_binder[0] 
	-> svc_cla::Classifier(
			00/00000000, // p0 Default rtg, unicast
			-);          // p4 Unhandled type, discard

svc_cla[0] -> [0]intra_lkup; // default rtg
svc_cla[1] -> Print(WARN_UNHANDLED_SVC_TYPE) -> MF_ChunkSink(CHUNK_MANAGER chk_mngr);

//Forwarding decisions: default routing

intra_lkup[0] -> segQ; //send chunk to next hop
 
chk_mngr[0]->net_binderQ; 

segQ -> Unqueue
	-> [0]seg;

// transport
intra_lkup[1] -> [0]prx;
intra_lkup[2] -> [1]prx;
prx[0] -> [2]intra_lkup;
prx[1] -> [3]intra_lkup;

//Outgoing csyn/csyn-ack pkts - place in high priority queue 
agg[1] -> outQ_ctrl;

//Outgoing data frame
seg[0] -> outQ_data;

//Rebind chunks that failed transfer to specified downstream node
seg[1] -> net_binderQ;

//Outgoing control pkts
lp_hndlr[0] //outgoing link probe
	-> outQ_ctrl;

lp_hndlr[1] //outgoing link probe ack
	-> outQ_ctrl;

lsa_hndlr[0] //outgoing lsa
	-> outQ_ctrl;

//priority schedule data and control pkts
outQ_ctrl -> [0]outQ_sched; 
outQ_data -> [1]outQ_sched; 

//to switch outgoing L2 pkts to respective learnt ports
out_switch::PaintSwitch(ANNO 41);
outQ_sched 
	-> outCntr_pkt 
	-> Unqueue 
	-> paint::MF_Paint(arp_tbl) 
	-> out_switch;

//Send pkts switch to corresponding to-devices
//port 0 is core bound

//core pkts
out_switch[0] 
	-> MF_EtherEncap(0x27c0, $core_dev, ARP_TABLE arp_tbl) 
        -> outQ_core 
        -> td_core;


//GNRS insert/update/query handling
//requestor --> request Q --> gnrs client --> gnrs svc
//gnrs svc --> response Q --> gnrs client --> requestor
elementclass GNRS_Service {
	$my_GUID, $GNRS_server_ip, $GNRS_server_port, $GNRS_listen_ip, $GNRS_listen_port |
	
	gnrs_reqQ::ThreadSafeQueue(100); //gnrs request queue - thread safe because of multiple requestors
	gnrs_rrh::GNRS_ReqRespHandler(MY_GUID $my_GUID, NET_ID "NA", RESP_LISTEN_IP $GNRS_listen_ip, 
	                              RESP_LISTEN_PORT $GNRS_listen_port, RESP_CACHE resp_cache); //gnrs client to interact with service
	gnrs_svc_sndr_lstnr::Socket(UDP, $GNRS_server_ip, $GNRS_server_port, $GNRS_listen_ip, 
                                    $GNRS_listen_port, CLIENT true); //UDP request sender and response listener
	gnrs_respQ::Queue(100); //queue to hold responses from GNRS service

	//send requests to service
	input[0] -> gnrs_reqQ -> Unqueue -> [0]gnrs_rrh[0] -> [0]gnrs_svc_sndr_lstnr;  
	//recv & queue responses for processing & forwarding to requestors
	gnrs_svc_sndr_lstnr -> [1]gnrs_rrh[1] -> gnrs_respQ -> Unqueue -> [0]output;
}

//Paint incoming packets based on incoming port
// OFFSET 8 is the offset of GNRS_SERVICE_ANNO
in_port_painter::InPortPainter(OFFSET 8);
in_port_painter -> gnrs_svc;
//Switch that uses painted annotation to deliver to right requestor
out_port_switch::PaintSwitch(ANNO 8);
gnrs_svc -> out_port_switch;

gnrs_svc::GNRS_Service($my_GUID, $GNRS_server_ip, $GNRS_server_port, $GNRS_listen_ip, $GNRS_listen_port); 

//Requestor 1: Host association handler - successful host associations result in GNRS updates
assoc_hndlr -> [0]in_port_painter; 
//out_port_switch[0] -> [1]assoc_hndlr;
out_port_switch[0] -> Discard; //TODO: patch responses to updates back to assoc handler

//Requestor 2: Network binder 
net_binder[1] -> [1]in_port_painter;
out_port_switch[1] -> [1]net_binder; //TODO Patch GNRS lookup requests/response from/to GNRS service client


//Thread/Task Scheduling
//re-balance tasks to threads every 10ms
BalancedThreadSched(INTERVAL 10);
//StaticThreadSched(fd_core 1, td_core 2, rtg_tbl 3);
