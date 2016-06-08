// Parameters:// Parameters:
// my_GUID
// topo_file - GUID-based topology file 
// core_dev
// GNRS_server_ip - listening ip on server
// GNRS_server_port - listening port on server
// GNRS_listen_ip - IP assoc w/ interface GNRS listens on
// GNRS_listen_port - response listening port for gnrs clients

//External libraries
require(library /usr/local/src/mobilityfirst/router/click/conf/libraries/MF_GNRSService.click);
require(library /usr/local/src/mobilityfirst/router/click/conf/libraries/MF_Interface.click);
require(library /usr/local/src/mobilityfirst/router/click/conf/libraries/MF_IPInterface.click);


// Maintains router-wide resource stats, etc. 
routerstats::MF_RouterStats();
inCntr_pkt::Counter(); //incoming L2 pkt and byte count 
inCntr_chunk::Counter(); //incoming L3 chunk and byte count 
outCntr_pkt::Counter(); //outgoing L2 pkt and byte count 


//Control path elements
arp_tbl::MF_ARPTable();
assoc_tbl::MF_AssocTable();
nbr_tbl::MF_NeighborTable();
rtg_tbl::MF_RoutingTable(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl);
lp_hndlr::MF_LinkProbeHandler(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl, ROUTING_TABLE rtg_tbl);
lsa_hndlr::MF_LSAHandler(MY_GUID $my_GUID, NEIGHBOR_TABLE nbr_tbl, ROUTING_TABLE rtg_tbl)
assoc_hndlr::MF_AssocHandler(ASSOC_TABLE assoc_tbl);


//Data path elements
chk_mngr::MF_ChunkManager();
resp_cache::GNRS_RespCache();
bitrate_cache::MF_BitrateCache();
seg::MF_Segmentor(routerstats, CHUNK_MANAGER chk_mngr);
agg::MF_Aggregator(MY_GUID $my_GUID, ROUTER_STATS routerstats, CHUNK_MANAGER chk_mngr, SEGMENTOR seg);
net_binder::MF_NetworkBinder(CHUNK_MANAGER chk_mngr);
intra_lkup::MF_IntraLookUp(MY_GUID $my_GUID, FORWARDING_TABLE rtg_tbl, CHUNK_MANAGER chk_mngr, ASSOC_TABLE assoc_tbl);


//Bitrate service
bitrate_handler::MF_BitrateHandler(MY_GUID $my_GUID, ARP_TABLE arp_tbl, BITRATE_CACHE bitrate_cache, CHUNK_MANAGER chk_mngr); 


//enforces stated topology
topo_mngr::MF_TopologyManager(MY_GUID $my_GUID, TOPO_FILE $topo_file, ARP_TABLE arp_tbl);


//Queues
inQ::ThreadSafeQueue(65535); //incoming L2 pkt Q
net_binderQ::ThreadSafeQueue(100); //chunk Q prior to NA resolution 
segQ::ThreadSafeQueue(100); //segmentor ready chunk Q
rtgQ::ThreadSafeQueue(100);   //Queue for intra look up
outQ_ctrl::ThreadSafeQueue(100); //L2 outgoing high priority control pkt queue
outQ_data::Queue(65535); //L2 outgoing lower priority data pkt queue
outQ_sched::PrioSched; //priority sched for L2 pkts


//Core interface
port_core::MF_Interface($core_dev, 0, arp_tbl); //First core interface
port_core -> inQ;

//start incoming pkt processing
inQ -> inUnq::Unqueue
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

// discarded packets
mf_cla[7] -> Discard; // TODO process client dis-assoc
mf_cla[8] -> Discard;

// routing control pkts
mf_cla[0] -> [0]lp_hndlr; // link probe
mf_cla[1] -> [1]lp_hndlr; // link probe ack
mf_cla[2] -> [0]lsa_hndlr; // lsa

// data and Hop signalling pkts
mf_cla[3] -> [0]agg; // data 
mf_cla[4] -> [1]agg; // csyn 
mf_cla[5] -> [1]seg; // csyn-ack
mf_cla[6] -> assoc_hndlr; // host association request

// Net-level processing for aggregated data blocks (chunks)

agg[0] //incomplete chunks assembled post Hop transfer from upstream node
	-> aggregator_classifier_incomplete::Classifier(
    	08/00000000,    //upper protocol for normal data
    	-);
    	
aggregator_classifier_incomplete[0] -> inCntr_chunk 
			-> net_binderQ; //chunk queue prior to NA resolution
aggregator_classifier_incomplete[1] -> Discard


agg[2] //complete chunks assembled post Hop transfer from upstream node
	-> aggregator_classifier_complete::Classifier(
		08/00000001,    //upper protocol for bitrate req and resp
		-);
		
bitrate_queue::ThreadSafeQueue(100);

aggregator_classifier_complete[0]
	-> bitrate_queue
	-> Unqueue
	-> [0]bitrate_handler[0]
	->net_binderQ;
	
aggregator_classifier_complete[1] -> Discard


net_binderQ 
	-> nbUnq::Unqueue 
	-> [0]net_binder;

net_binder[0] 
	-> svc_cla::Classifier(
			00/00000000, // p0 Default rtg, unicast
			-);          // p6 Unhandled type, discard

svc_cla[0] -> [0]intra_lkup; // default rtg
svc_cla[1] -> Print(WARN_UNHANDLED_SVC_TYPE) -> chnk_snk::MF_ChunkSink(CHUNK_MANAGER chk_mngr);


//Forwarding decisions: default routing
intra_lkup[0] -> segQ; //send chunk to next hop


//For timed push
chk_mngr[0]->net_binderQ; 
chk_mngr[1]->segQ;


rtgQ -> rtqUnQ::Unqueue -> [1]intra_lkup;
segQ -> segUnq::Unqueue -> [0]seg;


//Outgoing csyn/csyn-ack pkts - place in high priority queue 
agg[1] -> outQ_ctrl;


//Outgoing data frame
seg[0] -> outQ_data; 
seg[2] -> outQ_rsnd_data::Queue(65535); 


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
outQ_rsnd_data -> [1]outQ_sched; 
outQ_data -> [2]outQ_sched; 


//to switch outgoing L2 pkts to respective learnt ports
out_switch::PaintSwitch(ANNO 41);
outQ_sched 
	-> outCntr_pkt 
	-> swUnq::Unqueue 
	-> paint::MF_Paint(arp_tbl) 
	-> out_switch;

//Send pkts switch to corresponding to-devices
out_switch[0] //core bound
	-> port_core
	
	
//upper protocol handling
upper_protocol_sock::Socket(UDP, 127.0.0.1, 6001, CLIENT true); 
upper_protocol_sndrQ::ThreadSafeQueue(100); 
upper_protocol_lstnrQ::Queue(100); 
bitrate_handler[1] -> upper_protocol_sndrQ -> Unqueue -> upper_protocol_sock;
upper_protocol_sock -> upper_protocol_lstnrQ -> Unqueue -> Print(UPPER_PROTOCOL) -> [1]bitrate_handler;


//GNRS Service
gnrs_svc::GNRS_Service($my_GUID, $GNRS_server_ip, $GNRS_server_port, $GNRS_listen_ip, $GNRS_listen_port); 

//Paint incoming packets based on incoming port
// OFFSET 8 is the offset of GNRS_SERVICE_ANNO
in_port_painter::InPortPainter(OFFSET 8);
in_port_painter -> gnrs_svc;
//Switch that uses painted annotation to deliver to right requestor
out_port_switch::PaintSwitch(ANNO 8);
gnrs_svc -> out_port_switch;

//Requestor 1: Host association handler - successful host associations result in GNRS updates
assoc_hndlr -> [0]in_port_painter; 
//out_port_switch[0] -> [1]assoc_hndlr;
out_port_switch[0] -> Discard; //TODO: patch responses to updates back to assoc handler

//Requestor 2: Network binder 
net_binder[1] -> [1]in_port_painter;
out_port_switch[1] -> [1]net_binder; //TODO Patch GNRS lookup requests/response from/to GNRS service client


//Thread/Task Scheduling (MIN 4 threads)
StaticThreadSched(port_core 1, gnrs_svc 2, rtg_tbl 3);