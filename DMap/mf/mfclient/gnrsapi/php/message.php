<?php
include 'guid.php';
include 'common.php';

const RESPONSE_INCOMPLETE = 0;
const LOOKUP_REQUEST = 1;
const INSERT_REQUEST = 0;
const UPDATE_REQUEST = 2;
const OPTION_REQUEST_REDIRECT = 0; 
const FINAL_OPTION_FLAG = 0x80;

class req_t{
	public $version;
	public $type;
	public $len;
	public $id;
	public $src_addr;
	public $data_len;
	public $data;
	public $num_opts;
	public $opts;
	public $opts_len;
	
	
}

class lookup_t{
	public $guid;
}

class  upsert_t{
	public $guid;
	public $size;
	public $addrs;
	

}
class lookup_resp_t{
	public $size;
	public $addr;
	function __construct(){
	$this->addr = new addr_tlv_t();
	}
}
class resp_t{
	public $status;
	public $version;
	public $type;
	public $len;
	public $req_id;
	public $src_addr;
	public $code;
	public $data_len;
	public $lkup_data;
	public $num_opts;
	
	function __construct(){
	$this->lkup_data =new  lookup_resp_t();
	}
	
}

class GnrsMessageHelper{
	

        
	
	public function build_request_msg($req,$max_len){
			$i = 0;
			$bytecount = 0;
		if($max_len < (16 + $req->src_addr->len + $req->data_len + $req->opts_len)){
			return -1;
			}
		$buf = array();
		
		//version 
		 $buf[$i]=pack("C",$req->version);
		//echo "version :".$buf[$i]."\n";
		$i++;
		$bytecount++;
		//type
		 $buf[$i]=pack("C",$req->type);//byte array implemantation
		
		$i++;
		$bytecount++;
		//$len fill after counting
		$buf[$i] = pack("n",$bytecount);//occupy this slot; refill later
		$req_len_offset = $i;
		$i++;//come back to fill this slot later
		$bytecount+=2;
		
		
		//request id
		$buf[$i]=pack("N",$req->id);
		
		$i++;
		$bytecount+=4;
		//options offset
		$options_offset = 12+4+$req->src_addr->len+$req->data_len;//change here
		$buf[$i]=pack("n",$options_offset);
		
		$i++;
		$bytecount+=2;	
		//data offsets
		$data_offsets = 12+4+$req->src_addr->len;
		$buf[$i]=pack("n",$data_offsets);
		//echo "data offsets :".$data_offsets."\n";
		$i++;
		$bytecount+=2;
		//requestor address
		$buf[$i]=pack("n",$req->src_addr->type);
		//echo "requestor addr type :".$buf[$i]."\n";
		$i++;
		$buf[$i]=pack("n",$req->src_addr->len);
		//echo "request addr len :".$req->src_addr->len."\n";
		$i++;
		$buf[$i]=$req->src_addr->value;//pack in add funstion
		//echo "request addr value :".$req->src_addr->value."\n";
		$i++;
		$bytecount=$bytecount+2+2+$req->src_addr->len;
		
		
		//request payload
		if($req->type==LOOKUP_REQUEST){
			
			if($req->data_len<GUID_BINARY_SIZE){
			echo "lookup data len mismatches!";
			return -1;			
			}
		
		$lkup = $req->data;

		$guid  = new Guid();
		$packedguid = $guid->pack_in_bytes($lkup->guid);
		$buf[$i] =$packedguid;//pack("N*",$packedguid);//guid for lookup
		$i++;
		//echo "lookup guid length".strlen($packedguid);
		$bytecount+=20;

		/*options*/

		$opts = $req->opts;//only one option here
		$opts_len = 0;
		/*for($j=0;$j<$req->num_opts;$j++){
			if($req->$opts_len+2+$opts->len){
				echo "opt len mismatches!";
				return -1;
				}
			if(j==(req))
			}*/
		$buf[$i] = pack("C",$opts->type|FINAL_OPTION_FLAG);
		$i++;
		$bytecount++;
		$buf[$i] = pack("C",$opts->len);
		$i++;
		$bytecount++;
		$buf[$i] = pack("n",$opts->value);
		$i++;
		$bytecount+=2;

		$buf[$req_len_offset]=pack("n",$bytecount);
		$string  = implode('',$buf);
		return $string;
		
			
		
		}else if($req->type==INSERT_REQUEST||$req->type==UPDATE_REQUEST){
		$data_len = 0;
		/*put the guid into the buffer at the right position*/
			if($req->data_len<(GUID_BINARY_SIZE + 4)) {
			echo "begin data len mismatches!";
			return -1;
			}

		$ups = new upsert_t();
		$ups = $req->data;//point to the data from add();
		$guid = new Guid();
		

		/***pack input GUID into 20 byte-long array, need to be done in GUID CLASS***/
                
		$buf[$i]=$guid->pack_in_bytes($ups->guid);
		$i++;
		
		$bytecount+=20;
		$data_len+=GUID_BINARY_SIZE;
		$buf[$i]=pack("N",$ups->size);
		$i++;
		$bytecount+=4;
		$data_len+=4;
	
		/*address entries*/
		//go with 1 entry first
	
		//$ups->addrs= new addr_tlv_t();//ups->addrs here might be a list of NA, now it 
					//is a singel NA
		for($j=0;$j<1;$j++){//modified if support list
		if($req->data_len<($data_len+4+$ups->addrs->len)){
			
		return;		

			}
		
		$buf[$i] = pack("n",$ups->addrs->type);
		$i++;
		$bytecount+=2;
		$buf[$i] = pack("n",$ups->addrs->len);
		$i++;
		$bytecount+=2;
		$guid = new Guid();
			
		$buf[$i] =$guid->pack_in_bytes($ups->addrs->value); //
		$i++;
		$bytecount+=GUID_BINARY_SIZE;
		$data_len+=4 + ($ups->addrs->len);
		}
                
		/*options*/

		$opts = $req->opts;//only one option here
		$opts_len = 0;
		
		$buf[$i] = pack("C",$opts->type|FINAL_OPTION_FLAG);
		$i++;
		$bytecount++;
		$buf[$i] = pack("C",$opt->len);
		$i++;
		$bytecount++;
		$buf[$i] = pack("n",$opt->value);
		$i++;
		$bytecount+=2;
		
		$buf[$req_len_offset]=pack("n",$bytecount);
		echo "len offset index :".$req_len_offset."\n";
		echo "bytecount :".$bytecount; 
		$string  = implode('',$buf);
		return $string;
		
		}
		
	}

		


		
		

	public	function parse_reponse_msg($buf,$len){
		echo "parse_response_msg: ";
		$rsp = new resp_t();
		$rsp->status = RESPONSE_INCOMPLETE;
		$rsp->lkup_data = new lookup_resp_t();
		$rsp->lkup_data->addr = new addr_tlv_t();
		$rsp->lkup_data->addr->type = array();
		$rsp->lkup_data->addr->len = array();
		$rsp->lkup_data->addr->value = array();
		$resultbuf = array();
		print_r(unpack("H*",$buf));
		$response_msg = unpack("H*",$buf);	
		$i = 0;// index
		$j=2;
		
		$rsp->version = hexdec(substr($response_msg[1],$i,$j));
		echo "version :".$rsp->version;

		
		$i+=$j;
		$j=2;

		
		$rsp->type = substr($response_msg[1],$i,$j);
		echo "type :".$rsp->type;

		

		$i+=$j;
		$j=4;
		
		$rsp->len = hexdec(substr($response_msg[1],$i,$j));
		echo "total length :".$rsp->len;

		$i+=$j;
		$j=8;
		if($i<=$rsp->len*2){
		$rsp->req_id = hexdec(substr($response_msg[1],$i,$j));
		echo "requestor id :".$rsp->req_id;
		}else{
			return;
		}
		$i+=$j;
		$j=4;
		if($i<=$rsp->len*2){
		$opts_offset = hexdec(substr($response_msg[1],$i,$j));
		echo "option offset :".$opts_offset;

		}else{
			return;
		}
	
		$i+=$j;
		$j=4;
		if($i<=$rsp->len*2){
		$data_offset = hexdec(substr($response_msg[1],$i,$j));
		echo "data offset :".$data_offset;

		}else{
			return;
		}
		
		$i+=$j;
		$j=4;
		if($i<=$rsp->len*2){
		$rsp->src_addr = new addr_tlv_t();
		$rsp->src_addr->type = substr($response_msg[1],$i,$j);
		echo "src_addr type :".$rsp->src_addr->type;
		}else{
			return;
		}
		$i+=$j;
		$j=4;
		if($i<=$rsp->len*2){
		$rsp->src_addr->len = substr($response_msg[1],$i,$j);
		echo "src_addr len :".$rsp->src_addr->len;
		}else{
			return;
		}
		
		$i+=$j;
		$j=(int) $rsp->src_addr->len*2;
		
		if($i<=$rsp->len*2){
		$rsp->src_addr->value = substr($response_msg[1],$i,$j);
		echo "src_addr value :".$rsp->src_addr->value;
		}else{
			return;
		}

		$i+=$j;
		$j=8;//2 byte value and 2 byte pad
		if($i<=$rsp->len*2){
		$rsp->code = substr($response_msg[1],$i,$j);
		echo "response code :".$rsp->code;
		}else{
			return;
		}
		
		
		
		
		
		$i+=$j;
		
		$j=8;	//
		
		if(hexdec($rsp->type)-128==LOOKUP_REQUEST){//lookup :0x81 insert:0x80 update:0x82
			//$rsp->lkup_data = new lookup_resp_t();
			$rsp->lkup_data->size = hexdec(substr($response_msg[1],$i,$j));
			echo "rsp->lkup_data->size: ".$rsp->lkup_data->size;

			
			$index=0;
			for($index=0;$index<$rsp->lkup_data->size;$index++)
			{
			$i+=$j;
			$j=4;
			
			
			$rsp->lkup_data->addr->type[$index] = hexdec(substr($response_msg[1],$i,$j));	//rsp->lkup_data->addr->
			//echo "rsp->lkup_data->addr->type :".$rsp->lkup_data->addr->type[$index];
		
			$i+=$j;
			$j=4;
			$rsp->lkup_data->addr->len[$index] = hexdec(substr($response_msg[1],$i,$j));	
			
			//echo "rsp->lkup_data->addr->len :".$rsp->lkup_data->addr->len[$index];
			
			$i+=$j;
			$j=$rsp->lkup_data->addr->len[$index]*2;
			$rsp->lkup_data->addr->value[$index] = hexdec(substr($response_msg[1],$i,$j));
			$resultbuf[$index]=$rsp->lkup_data->addr->value[$index];
			//echo "rsp->lkup_data->addr->value :".$rsp->lkup_data->addr->value[$index];
			}
		
			
		}else if(hexdec($rsp->type)-128==INSERT_REQUEST){
				echo "This is a insert, do nothing...";	
				
			}else if(hexdec($rsp->type)-128==UPDATE_REQUEST){
				echo "This is a update, do nothing...";	
				
			}
			$opts_done = false;
			$rsp->opts = new opt_tlv_t();
			$rsp->opts->type = array();
			$rsp->opts->len = array();
			$rsp->opts->value = array();
			$a=0;
			while(!$opts_done){
				$i+=$j;
				$j=2;
				if($i<=$rsp->len*2){
				$rsp->opts->type[$a] = hexdec(substr($response_msg[1],$i,$j));
				 }else{
				       return $rsp->lkup_data->addr->value;
					}
				if($rsp->opts->type[$a]&0x80){
				$opts_done =true;
				}
				
				$i+=$j;
				$j=2;
                                if($i<=$rsp->len*2){
				$rsp->opts->len[$a] = hexdec(substr($response_msg[1],$i,$j));
				 }else{
					return $rsp->lkup_data->addr->value;
					}
				$i+=$j;
				$j=16;
				if($i<=$rsp->len*2){
				$rsp->opts->value[$a] = hexdec(substr($response_msg[1],$i,$j));
				 }else{
					return $rsp->lkup_data->addr->value;
					}
				
				
				
				
				
				$a++;
				
				
			}
                              
			$rsp->status = RESPONSE_COMPLETE;
		
		
		
		return $rsp->lkup_data->addr->value;
		
		}


}




?>
