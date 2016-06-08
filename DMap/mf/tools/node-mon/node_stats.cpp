#include "node_stats.h"

void node_stats::inject_into_oml(unsigned mp_index){

    OmlValueU v[4];

    omlc_zero_array(v, 4);

    omlc_set_uint32(v[0], mp_index);
    omlc_set_string(v[1], node_id.c_str());
    omlc_set_double(v[2], cpu_usage);
    omlc_set_double(v[3], mem_usage);

    omlc_inject(oml_mp, v);

    omlc_reset_string(v[1]);

#ifdef DEBUG
    char msg[500];
    sprintf(msg, "NODE(%s): ts cpu%% mem%%\n %lld %f %f", 
        node_id.c_str(),
        t_stamp,
        cpu_usage, mem_usage
        );
    cout << "DEBUG: " << msg << endl;
#endif //DEBUG
}

void node_stats::read(){

    timeval nowtime;
    gettimeofday(&nowtime, NULL);
    t_stamp = nowtime.tv_sec; //+(nowtime.tv_usec/1000000.0);

    /* read cpu stats 
    * top -b -n2 -d 00.2 |grep Cpu|tail -1 | awk -F ":" '{ print $2 }'
    * Requires 2 iterations since first reported set is since boot time
    * TODO: this increases the sampling time, and causes about 1sec drift on
    * each iteration!
    * Sample result:
    *10.8%us,  0.0%sy,  0.0%ni, 89.2%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
    */
    FILE *fp;
    int status;
    char result[TOP_RESULT_MAX_CHARS];

    //char cpu_cmd[] = "top -b -n2 -d 00.2 |grep Cpu|tail -1 | awk -F ':' '{ print $2 }' | cut -d, -f4 | cut -d'%' -f1";
    char cpu_cmd[] = "top -b -n2 -d 00.2 |grep Cpu|tail -1 | awk -F ':' '{ print $2 }'";

    if(!(fp = popen(cpu_cmd, "r"))) {
        printf("Error running top cmd\n");
        return;
    } else {
        if(fgets(result, TOP_RESULT_MAX_CHARS, fp) != NULL) {
            //printf("DEBUG: top cpu result:\n%s\n", result);
            sscanf(result, " %f%%us,  %f%%sy,  %f%%ni, %f%%id,  %f%%wa,"
                "  %f%%hi,  %f%%si,  %f%%st", 
                &cpu_user, &cpu_sys, &cpu_nice, &cpu_idle, &cpu_wait, 
                &cpu_hirq, &cpu_sirq, &cpu_steal);
            cpu_usage = 100 - cpu_idle;
        }else{
                printf("Error getting result from top cmd\n");
                return;
        }
    }
    if((status = pclose(fp)) == -1){
        printf("Error running top cmd\n");
        return;
    }

    /* memory stats
    * top -b -n2 -d 00.2 |grep Mem|tail -1 | awk -F ':' '{ print $2 }'
    * Sample output:
    * '8184396k total,  1748140k used,  6436256k free,   348220k buffers'
    *
    * free |grep Mem| awk -F ':' '{ print $2 }'
    *              total       used       free     shared    buffers     cached
    * Mem:       3087472     440896    2646576          0      48508     279200
    * -/+ buffers/cache:     113188    2974284
    * Swap:            0          0          0
    */
    //char mem_cmd[] = "top -b -n2 -d 00.2 |grep Mem|tail -1 | awk -F ':' '{ print $2 }'";
    char mem_cmd[] = "free |grep Mem| awk -F ':' '{ print $2 }'";

    if(!(fp = popen(mem_cmd, "r"))){
        printf("Error running top cmd\n");
        return;
    }else{
        if(fgets(result, TOP_RESULT_MAX_CHARS, fp) != NULL){
            //printf("DEBUG: top mem result\n:%s\n", result);
            //result from 'top'
            //sscanf(result, " %lluk total, %lluk used, %lluk free, %lluk buffers", &mem_total, &mem_used, &mem_free, &mem_buffers);
            //result from 'free'
            sscanf(result, " %llu %llu %llu %llu %llu %llu", &mem_total, &mem_used, &mem_free, &mem_shared, &mem_buffers, &mem_cached);
            //printf("DEBUG: mem used %lluk\n", mem_used);
            mem_usage = (float) (100.0*(double)(mem_total - (mem_free + mem_buffers + mem_cached))/(double)mem_total);
        }else{
            printf("Error getting mem result from top cmd\n");
            return;
        }
    }
    if((status = pclose(fp)) == -1){
        printf("Error running top cmd\n");
        return;
    }
}
