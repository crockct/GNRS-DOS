#ifndef NODE_STATS_H
#define NODE_STATS_H

#include <iostream>

using namespace std;

/* buffer size to receive pruned results from 'top' - should normally be ~80 chars */
#define TOP_RESULT_MAX_CHARS 512

#ifdef __cplusplus
extern "C" {
#endif

/* Get size_t and NULL from <stddef.h>.  */
#include <stddef.h>

#include <oml2/omlc.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static OmlMPDef node_stats_mpd[] = {
  {"mp_index", OML_UINT32_VALUE},
  {"node_id", OML_STRING_VALUE},
  {"cpu_usage", OML_DOUBLE_VALUE},
  {"mem_usage", OML_DOUBLE_VALUE},
  {NULL, (OmlValueT)0}
};

#ifdef __cplusplus
}
#endif

class node_stats{

private:
	OmlMP* oml_mp;
	int64_t t_stamp;
	string node_id;
	float cpu_usage;//percentage
	float mem_usage;//percentage
	uint64_t mem_total, mem_used, mem_free, mem_shared, mem_buffers, mem_cached;
	float cpu_user, cpu_sys, cpu_nice, cpu_idle, cpu_wait, cpu_hirq, cpu_sirq, cpu_steal;

public:

    node_stats(const char* id){

        node_id = id;
        if((oml_mp = omlc_add_mp("node_stats", node_stats_mpd)) == NULL){
            cout << "FATAL: Unable to add node_stats OML measurement pt." 
                    << endl;
            exit(1);
        }
        cpu_usage = mem_usage = 0;
    }	

    void inject_into_oml(unsigned mp_index);
    void read();
};

#endif //NODE_STATS_H
