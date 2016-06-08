//Simple class used to test the java api


//jmfapi needs to be in the classpath
import java.io.*;
import java.util.*;
import java.nio.file.*;
import edu.rutgers.winlab.jmfapi.*;
import edu.rutgers.winlab.jmfapi.GUID;

class Sender{
	private static void usage(){
		System.out.println("Usage:");
		System.out.println("sender <file> <dst_GUID> [<src_GUID>]");
	}
	public static void main(String []argv){
		if(argv.length < 2){
			usage();
			return;
		}
		String scheme = "basic";
		GUID srcGUID = null, dstGUID;
		dstGUID = new GUID(Integer.parseInt(argv[1]));
		if(argv.length == 3) srcGUID = new GUID(Integer.parseInt(argv[2]));
		Path file = FileSystems.getDefault().getPath(argv[0]);
		JMFAPI sender = new JMFAPI();
		try{
			if(srcGUID!=null) sender.jmfopen(scheme, srcGUID);
			else sender.jmfopen(scheme);
			byte[] fileArray;
			try {
				fileArray = Files.readAllBytes(file);
			} catch (IOException e){
				System.out.println("ERROR");
				return;
			}
			byte[] tempArray;
			int ret, read = 0;
			while(fileArray.length - read>=1000000){
				tempArray = Arrays.copyOfRange(fileArray, 0, 999999);
				sender.jmfsend(tempArray,1000000, dstGUID);
			}
			tempArray = Arrays.copyOfRange(fileArray, 0, fileArray.length - read - 1);
			sender.jmfsend(tempArray,fileArray.length - read, dstGUID);
			sender.jmfclose();
			System.out.println("Transfer completed");
		} catch (JMFException e){
			System.out.println(e.toString());
		}
	}
}
