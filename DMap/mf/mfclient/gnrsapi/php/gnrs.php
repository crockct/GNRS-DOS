<?php
include 'message.php';
include 'udpipv4_endpoint.php';



class Gnrs{
	public $requestid;
	
	public function get_request_id(){
		$this->requestid++;
		return $this->requestid;
	}

	public function lookup($guid){

		
		//set request message
		$req = new req_t();
		$gmh = new GnrsMessageHelper();

		$req->version = 0;//0 for development
		$req->type = LOOKUP_REQUEST;//
		$req->id = $this->get_request_id();
		$req->src_addr = new  addr_tlv_t();
		$req->src_addr->type = 0;//0 for IP,1 for guid, 2 for NA
		$ipinbyte = ip2long($_SERVER['SERVER_ADDR']);
		$udp = new Udpipv4_endpoint('127.0.0.1',5001);//GNRS server is listening at the udp port
		$req->src_addr->value = pack("Nn",$ipinbyte,$udp->clientport);//ipv4, 4 byte ip, 2 byte udp 
		echo "local host:".$_SERVER['SERVER_ADDR'];
		$req->src_addr->len = strlen($req->src_addr->value);

		$lkup = new lookup_t();
		$lkup->guid = $guid;
		$req->data = $lkup;
		
		$data_len = GUID_BINARY_SIZE;
		$req->data_len = $data_len;

		$req->opts_len = 0;
		$req->num_opts = 0;

		$redirect_opt = new opt_tlv_t();
		$redirect_opt->type = OPTION_REQUEST_REDIRECT;
		$redirect_opt->len = 2;
		$redirect_opt->value = 0;//$redirect_val;
			$req->opts_len+= 2 + $redirect_opt->len;
			$req->num_opts++; 
	
		$req->opts = $redirect_opt;

		$sendbuf = $gmh->build_request_msg($req,2048);
		$req->len = strlen($sendbuf);

	/****** send request ******/
		if($udp->udp_send($sendbuf)!=$req->len){
			echo "Error:gnrs_api:Error in sending lookup msg\n";	
			echo "req->len: ".$req->len." bytes";
			return;
			}



		$rsp = new resp_t();
		$recvbuf;
		$len;
                // $udpr = new Udpipv4_endpoint('127.0.0.1',4001);
	        $recvbuf=$udp->udp_recv();
                
		$len = strlen($recvbuf);
		if($len==0){
			echo "ERROR: gnrs recv:ERROR in recv of insert response\n";
			return;
		}
		echo "recvbuf:".$len." bytes\n";
		$rsp_result=array();
  		$rsp_result=$gmh->parse_reponse_msg($recvbuf,$len);
		
		return $rsp_result;

	}

	public function add($srcguid, $dstguid){
		
		$req = new req_t();
		$gmh = new GnrsMessageHelper($req);
	
		$req->version = 0;//0 for development
		$req->type = 0;//0 for insertion 
		$req->id = $this->get_request_id();// should be get request id
		$req->src_addr = new addr_tlv_t();// local server address
		$req->src_addr->type = 0;//0 for IP,1 for GUID,2 for NA
		$ipinbyte = ip2long($_SERVER['SERVER_ADDR']);
		$udp = new Udpipv4_endpoint('127.0.0.1',5001);//GNRS server is listening at the udp port
		$req->src_addr->value = pack("Nn",$ipinbyte,$udp->clientport);//ipv4, 4 byte ip, 2 byte udp 
		echo "local host:".$_SERVER['SERVER_ADDR'];
		$req->src_addr->len = strlen($req->src_addr->value);


		$ups = new upsert_t();
		$data_len = GUID_BINARY_SIZE + 4;
		$data_len = $data_len+2+2+GUID_BINARY_SIZE;//since we insert the GUID HERE
		//addr list should be implemented here
		/*while(is not the end of the list){
		$data_len + offset of each address

		}*/
		
		$ups->guid = $srcguid;//should convert this value to bits wtih 20 bytes length

		$ups->addrs = new addr_tlv_t(); //this should be a list of address
		$ups->addrs->type = 1;//1 for guid
		$ups->addrs->value = $dstguid; 
		$ups->addrs->len = 20;//the length of  guid is 20 bytes
		$ups->size = 1;//hard-coded, should be the number of element in this list
	
		//$data_len += 4+lengthofeveryaddressTLV;//should be the length of an GUID TLV.
		$req->data = $ups;
		$req->data_len = $data_len;

		//set up options

		
		$req->opts_len = 0;
		$req->num_opts = 0;
		//redirect options: allow request to be redirected to other GNRS SERVER
		$redirect_val = array(0,1);

		$redirect_opt = new opt_tlv_t();
		$redirect_opt->type = OPTION_REQUEST_REDIRECT;
		$redirect_opt->len = 2;
		$redirect_opt->value = $redirect_val;
			$req->opts_len+= 2 + $redirect_opt->len;
			$req->num_opts++; 
	
		$req->opts = $redirect_opt;
		//$req->src_addr->len
	       
		$sendbuf = $gmh->build_request_msg($req,2048);
		$req->len = strlen($sendbuf);
		

		if($udp->udp_send($sendbuf)!=$req->len){
			echo "Error:gnrs_api:Error in sending insert msg\n";	
			return;
			}
                echo "send done!";
		//attemp receive 
		$rsp = new resp_t();
		$recvbuf;
		$len;
	        $recvbuf=$udp->udp_recv();
		$len = strlen($recvbuf);
		if($len==0){
			echo "ERROR: gnrs recv:ERROR in recv of insert response\n";
			return;
		}
		echo "recvbuf:".$len." bytes\n";
		
		$rsp = new resp_t();
		$rsp = $gmh->parse_reponse_msg($recvbuf,$len);
		
	
		

	     return $rsp;
		

}

}
?>
