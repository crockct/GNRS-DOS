package edu.rutgers.winlab.jgnrs;

public class TestAPI{
	public static void main(String []argv){
		if(argv.length<3){
			System.out.println("Usage: GNRS_IP:GNRS_Port local_IP:local:Port");
			return;
		}
		JGNRS gnrs = new JGNRS();
		gnrs.setGNRS(argv[1],argv[2]);
		int na[] = new int[2];
		na[0] = 2;
		na[1] = 3;
		gnrs.add(1, na);
		System.out.println("GNRS lookup: " + gnrs.lookup(1).toString());
		gnrs.deleteGNRS();
	}
}