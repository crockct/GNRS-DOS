// Parameters:
// my_GUID
// topo_file - GUID-based topology file 
// core_dev
// GNRS_server_port - listening port on server, assumes localhost gnrs
// GNRS_listen_ip - IP assoc w/ interface GNRS listens on
// GNRS_listen_port - response listening port for gnrs clients
// edge_dev - name of wireless interface on which client connects
// edge_dev_ip - IP of edge device on this bridge that the client connects on

// Maintains router-wide resource stats, etc. 
routerstats::MF_RouterStats;

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
anycast_rtg::MF_AnycastRouting(MY_GUID $my_GUID, FORWARDING_TABLE rtg_tbl, CHUNK_MANAGER chk_mngr);
mcast_rtg::MF_MultiUnicastRouting(MY_GUID $my_GUID, CHUNK_MANAGER chk_mngr);
multihome_rtg::MF_MultiHomeRouting(MY_GUID $my_GUID, FORWARDING_TABLE rtg_tbl, BITRATE_CACHE bitrate_cache, CHUNK_MANAGER chk_mngr);
// transport-related elements
stg_mgr::MF_StorageManager(CHUNK_MANAGER chk_mngr, STORAGE_CAPACITY 1073741824);
fctrl::MF_FlowController(STORAGE_MANAGER stg_mgr);
prx::MF_TransProxy(MY_GUID $my_GUID, STORAGE_MANAGER stg_mgr, FLOW_CTRL fctrl, CHUNK_MANAGER chk_mngr);

bitrate_handler::MF_BitrateHandler(MY_GUID $my_GUID, ARP_TABLE arp_tbl, BITRATE_CACHE bitrate_cache, CHUNK_MANAGER chk_mngr); 


//enforces stated topology
topo_mngr::MF_TopologyManager(MY_GUID $my_GUID, TOPO_FILE $topo_file, ARP_TABLE arp_tbl);

//Counters and Statistics
//TODO: build a custom chunk counter to get proper data byte count
//incoming L2 pkt and byte count 
inCntr_pkt::Counter()
//incoming L3 chunk and byte count 
inCntr_chunk::Counter()
//outgoing L2 pkt and byte count 
//outCntr_pkt::Counter

//Queues
inQ::ThreadSafeQueue(65535); //incoming L2 pkt Q
net_binderQ::ThreadSafeQueue(1000); //chunk Q prior to NA resolution 
segQ::ThreadSafeQueue(1000); //segmentor ready chunk Q
rtgQ::ThreadSafeQueue(1000);   //Queue for intra look up
outQ_ctrl::ThreadSafeQueue(1000); //L2 outgoing high priority control pkt queue
//outQ_data::Queue(65535); //L2 outgoing lower priority data pkt queue
core_outQ_sched::PrioSched; //priority sched for L2 pkts
edge_outQ_sched::PrioSched; 
//outQ_core::Queue(65535); //L2 outgoing pkt Q for 'core' port
//outQ_edge::Queue(65535); //L2 outgoing pkt Q for 'edge' port

//L2 packet sources
//core port
fd_core::FromDevice($core_dev, PROMISC false, SNIFFER true);
//edge port
fd_edge::FromDevice($edge_dev, PROMISC false, SNIFFER true);

//L2 out i/f
td_core::ToDevice($core_dev);
td_edge::ToDevice($edge_dev);

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

//Edge interface - port 1
fd_edge 
        -> HostEtherFilter($edge_dev, DROP_OWN true)
        //only IP-encap MF pkts, i.e.,  with IP Protocol ID = 0x05
        -> edge_cla::Classifier(12/0800 23/05, -)
        -> SetTimestamp 
	-> MF_Learn(IN_PORT 1, ARP_TABLE arp_tbl) // learn src eth, IP, port
        -> Strip(34) // strip eth, IP hdrs; assumes no VLAN tag
	-> inQ;

edge_cla[1] -> Discard;


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
			00/00000001, // p1 multicast
			00/00000002, // p2 anycast
			00/00000004, // p3 multihome
      00/00000200, // p4 contest req
      00/00000400, // p5 contest resp
			-);          // p6 Unhandled type, discard

svc_cla[0] -> [0]intra_lkup; // default rtg
svc_cla[1] -> mcast_rtg[0] -> rtgQ; // multicast
svc_cla[2] -> anycast_rtg[0] -> rtgQ; // anycast
svc_cla[3] -> multihome_rtg[0] -> rtgQ; // multihome
svc_cla[4] -> [0]intra_lkup;
svc_cla[5] -> [0]intra_lkup;
svc_cla[6] -> Print(WARN_UNHANDLED_SVC_TYPE) -> chnk_snk::MF_ChunkSink(CHUNK_MANAGER chk_mngr);

//Forwarding decisions: default routing

intra_lkup[0] -> segQ; //send chunk to next hop


chk_mngr[0]->net_binderQ; 
chk_mngr[1]->segQ;

rtgQ -> rtqUnQ::Unqueue -> [1]intra_lkup

segQ -> segUnq::Unqueue
	-> [0]seg;

// transport
intra_lkup[1] -> [0]prx;
intra_lkup[2] -> [1]prx;
prx[0] -> [2]intra_lkup;
prx[1] -> [3]intra_lkup;

rtgQ -> rtgUnQ::Unqueue -> [1]intra_lkup;


//Outgoing csyn/csyn-ack pkts - place in high priority queue 
agg[1] -> outQ_ctrl;

//Outgoing data frame
//seg[0] -> outQ_data; 
//seg[2] -> outQ_rsnd_data::Queue(65535); 

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
//outQ_ctrl -> [0]outQ_sched;
//outQ_rsnd_data -> [1]outQ_sched; 
//outQ_data -> [2]outQ_sched; 

elementclass OutPacket_Process {
  $core_dev, $edge_dev, $edge_dev_ip | 
  //to switch outgoing L2 pkts to respective learnt ports
  out_switch::PaintSwitch(ANNO 41); 
  input[0] -> MF_Paint(arp_tbl) -> out_switch;
  out_switch[0] -> MF_EtherEncap(0x27c0, $core_dev, ARP_TABLE arp_tbl) -> Queue(65535) ->[0] output;
  out_switch[1] 
         //Use IP protocol ID=5 for encapsulating MF pkts
         -> MF_IPEncap(PROTO 5, SRC $edge_dev_ip, DST NXTHOP_ANNO, ARP_TABLE arp_tbl)
         -> MF_EtherEncap(0x0800, $edge_dev, ARP_TABLE arp_tbl)
         -> Queue(65535) 
         -> [1]output; 
}

ctrl_process::OutPacket_Process($core_dev, $edge_dev, $edge_dev_ip);
outQ_ctrl -> ctrlUnq::Unqueue -> ctrl_process;
ctrl_process[0] -> [0]core_outQ_sched;
ctrl_process[1] -> [0]edge_outQ_sched;

resend_data_process::OutPacket_Process($core_dev, $edge_dev, $edge_dev_ip);
seg[2] -> resend_data_process;
resend_data_process[0] -> [1]core_outQ_sched;
resend_data_process[1] -> [1]edge_outQ_sched; 

data_process::OutPacket_Process($core_dev, $edge_dev, $edge_dev_ip);
seg[0] -> data_process;
data_process[0] -> [2]core_outQ_sched;
data_process[1] -> [2]edge_outQ_sched;
 
core_outQ_sched -> td_core; 
edge_outQ_sched -> td_edge; 

//outQ_sched 
//	-> outCntr_pkt 
//	-> swUnq::Unqueue 
//	-> paint::MF_Paint(arp_tbl) 
//	-> out_switch;

//Send pkts switch to corresponding to-devices
//port 0 is core bound
//port 1 is edge bound

//core pkts
//out_switch[0] 
//	-> MF_EtherEncap(0x27c0, $core_dev, ARP_TABLE arp_tbl) 
//        -> outQ_core 
//        -> td_core;

//edge pkts - IP encap is required
//out_switch[1] 
//	->MF_PacketPrint()
//EUse IP protocol ID=5 for encapsulating MF pkts  
//	-> MF_IPEncap(PROTO 5, SRC $edge_dev_ip, DST NXTHOP_ANNO, ARP_TABLE arp_tbl)
//set ethertype to IP
//	-> MF_EtherEncap(0x0800, $edge_dev, ARP_TABLE arp_tbl)
//	-> outQ_edge 
//        -> td_edge;

//upper protocol handling
upper_protocol_sock::Socket(UDP, 127.0.0.1, 6001, CLIENT true); 
upper_protocol_sndrQ::ThreadSafeQueue(100); 
upper_protocol_lstnrQ::Queue(100); 
bitrate_handler[1] -> upper_protocol_sndrQ -> Unqueue -> upper_protocol_sock;
upper_protocol_sock -> upper_protocol_lstnrQ -> Unqueue -> Print(UPPER_PROTOCOL) -> [1]bitrate_handler;


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
//StaticThreadSched(fd_core 1, td_core 1, fd_edge 2, td_edge 2, rtg_tbl 3);
//StaticThreadSched(fd_core 0, inUnq 1, nbUnq 1, segUnq 2, ctrlUnq 2, td_core 3, seg 4, fd_edge 5, td_edge 6, lp_hndlr 7, lsa_hndlr 7, rtg_tbl 7);
StaticThreadSched(fd_core 0, inUnq 1, nbUnq 1, prx 1, segUnq 2, ctrlUnq 2, td_core 3, seg 4, fd_edge 5, td_edge 6, lp_hndlr 7, lsa_hndlr 7, rtg_tbl 7);
