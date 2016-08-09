
#from scipy.stats import zipf

def get_node_IPs_from_as_file():
    f = open(r'C:/Users/Colleen/Documents/thesis/GNRS-DOS/as-listing.txt')
    for line in f:
        splitLine = line.split(" ")
        print(splitLine[1]+":5000")
    f.close()

# parses per-message level server log, outputs csv for excel
def parse_server_log():
    f = open(r'C:/Users/Colleen/Documents/thesis/GNRS-DOS/4attackfreq_july29/gnrsd_target.log')
    out = open(r'C:/Users/Colleen/Documents/thesis/GNRS-DOS/4attackfreq_july29/server_log.csv', 'w')
    out.write(" , date,rtt(ns),GUID,type,seq#,success/fail\n ,") # first line is de-dented in excel csv
    lastLine = None
    for line in f:

        if "0.000 per second" in line or "Outstanding responses: 0" in line or "DEBUG" in line:
            lastLine = None
            continue #skip this line
        
        # date , time, seconds, log level, nothing,  NioDatagramAcceptor-1/TraceClient,
        # - , ns time, GUID, ->, INR, REQ#/[SUCCESS||FAILED]
        else:
            splitLine = line.split(" ")
            if lastLine != None:
                splitLine = [lastLine] + splitLine
                lastLine = None
            elif len(splitLine) == 1 and ("==Lookups==" in splitLine[0] or "==Inserts==" in splitLine[0]):
                lastLine = splitLine[0]
                continue
            for chunk in splitLine:
                if "," in chunk:
                    chunk = chunk.replace(",","")
        #       if chunk == "INFO" or chunk == "-" or chunk == "->" or chunk == "" or chunk == "NioDatagramAcceptor-1/TraceClient":
         #          continue
              #  if "[" in chunk:
               #     chunk = chunk.replace("[", "")
               #     chunk = chunk.replace("]","")
            #   if "ns]" in chunk:
             #      chunk = chunk.replace("ns]", "")
              #     chunk = chunk.replace(",", "")
               #if "," in chunk:
               #    continue # skip timestamp
               #if "/" in chunk:
               #    chunk = chunk.replace("#", "") #get rid of number sign
               #    splitChunk = chunk.split("/")
               #    chunk = splitChunk[0] + "," + splitChunk[1]
                out.write(chunk)
                out.write(",")
    f.close()


# parses per-message level client log, outputs csv for excel
def parse_client_log():
    f = open(r'C:/Users/Colleen/Documents/thesis/GNRS-DOS/results/client.log')
    out = open(r'C:/Users/Colleen/Documents/thesis/GNRS-DOS/results/client_log.csv', 'w')
    out.write(" , date,rtt(ns),GUID,type,seq#,success/fail\n ,") # first line is de-dented in excel csv
    for line in f:
        splitLine = line.split(" ")
        # date , time, seconds, log level, nothing,  NioDatagramAcceptor-1/TraceClient,
        # - , ns time, GUID, ->, INR, REQ#/[SUCCESS||FAILED]
        
        if len(splitLine) == 11: #formatted like a individual message log
           for chunk in splitLine:
               if chunk == "INFO" or chunk == "-" or chunk == "->" or chunk == "" or chunk == "NioDatagramAcceptor-1/TraceClient":
                   continue
               if "[" in chunk:
                   chunk = chunk.replace("[", "")
               if "ns]" in chunk:
                   chunk = chunk.replace("ns]", "")
                   chunk = chunk.replace(",", "")
               if "," in chunk:
                   continue # skip timestamp
               if "/" in chunk:
                   chunk = chunk.replace("#", "") #get rid of number sign
                   splitChunk = chunk.split("/")
                   chunk = splitChunk[0] + "," + splitChunk[1]
               out.write(chunk)
               out.write(",")
    f.close()
    
    
### ZIPF ###

    
def get_zipf(num_samples):
    N = 1000 # number of GUIDs
    k = 0 # rank of GUID

    a = 1.02 # skewness
    q = 100 # flatness

    r = zipf.rvs(a, size=num_samples)
    print(r)
