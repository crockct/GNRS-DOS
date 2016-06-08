<?php
const GUID_BINARY_SIZE = 20;
class Guid{

	
	public $guidstr;

	

	public function pack_in_bytes($guidstr){//input guid is a int, need to be packed into 20 byte long format
	
			$buf = array();
			$i=0;

		$guidhex = dechex($guidstr);
		$input_len = strlen($guidhex);
		if($input_len<=40&&($input_len%2==0)){
			for($j=0;$j<(40-$input_len)/2;$j++){
				
				$buf[$i]=pack("H",0);
				$i++;			
			
			}
                           
			
			$buf[$i] = pack("H*",$guidhex);
			$i++;
		} else if($input_len<=40&&($input_len%2==1)){
			for($j=0;$j<(40-$input_len-1)/2;$j++){
			
				$buf[$i]=pack("H",0);
				$i++;	
				
			}
			
			$guidhex = "0".$guidhex;			
			$buf[$i] = pack("H*",$guidhex);
			$i++;	
		}

		
			return $buf = implode("",$buf);
}

}
?>
