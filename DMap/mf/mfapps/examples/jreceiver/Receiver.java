//Simple class used to test the java api

import java.io.*;
import java.util.*;
import java.nio.file.*;
import edu.rutgers.winlab.jmfapi.*;

class Receiver{
	private static void usage(){
		System.out.println("Usage:");
		System.out.println("receiver [<src_GUID>]");
	}
	public static void main(String []argv){
		String scheme = "basic";
		GUID srcGUID = null;
		int i = 0;
		if(argv.length == 1) srcGUID = new GUID(Integer.parseInt(argv[0]));
		Path file = FileSystems.getDefault().getPath("temp.txt");
		try{
			Files.createFile(file);
		} catch(IOException e){
			try{
				Files.delete(file);
				Files.createFile(file);
			} catch(IOException e2){
				return;
			}
		}
		byte[] buf = new byte[1000000];
		int ret;
		JMFAPI receiver = new JMFAPI();
		try{
			if(srcGUID!=null) receiver.jmfopen(scheme, srcGUID);
			else receiver.jmfopen(scheme);
			while(i < 24954287){
				ret = receiver.jmfrecv_blk(null, buf, 1000000);
				try{
					Files.write(file, buf, StandardOpenOption.APPEND);
				} catch (IOException e){
					System.out.println(e.toString());
				}
				i += ret;

			}
			receiver.jmfclose();
		} catch (JMFException e){
			System.out.println(e.toString());
		}
		System.out.println("Transfer completed");
	}
}
