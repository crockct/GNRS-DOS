/*
 * This file was automatically generated by oml2-scaffold V2.10.1
 * for node_mon version 0.9.1.
 * Please do not edit.
 */

#ifndef NODE_MON_OML_H
#define NODE_MON_OML_H

#ifdef __cplusplus
extern "C" {
#endif

/* Get size_t and NULL from <stddef.h>.  */
#include <stddef.h>

#include <oml2/omlc.h>

typedef struct {
  OmlMP* node_stats;

} oml_mps_t;


#ifdef OML_FROM_MAIN
/*
 * Only declare storage once, usually from the main
 * source, where OML_FROM_MAIN is defined
 */

static OmlMPDef oml_node_stats_def[] = {
  {"mp_index", OML_UINT32_VALUE},
  {"node_id", OML_STRING_VALUE},
  {"cpu_usage", OML_DOUBLE_VALUE},
  {"mem_usage", OML_DOUBLE_VALUE},
  {NULL, (OmlValueT)0}
};

static oml_mps_t g_oml_mps_storage;
oml_mps_t* g_oml_mps_node_mon = &g_oml_mps_storage;

static inline void
oml_register_mps()
{
  g_oml_mps_node_mon->node_stats = omlc_add_mp("node_stats", oml_node_stats_def);

}

#else
/*
 * Not included from the main source, only declare the global pointer
 * to the MPs and injection helpers.
 */

extern oml_mps_t* g_oml_mps_node_mon;

#endif /* OML_FROM_MAIN */

static inline void
oml_inject_node_stats(OmlMP* mp, uint32_t mp_index, const char* node_id, double cpu_usage, double mem_usage)
{
  OmlValueU v[4];

  omlc_zero_array(v, 4);

  omlc_set_uint32(v[0], mp_index);
  omlc_set_string(v[1], node_id);
  omlc_set_double(v[2], cpu_usage);
  omlc_set_double(v[3], mem_usage);

  omlc_inject(mp, v);

  omlc_reset_string(v[1]);
}


/* Compatibility with old applications */
#ifndef g_oml_mps
# define g_oml_mps	g_oml_mps_node_mon
#endif

#ifdef __cplusplus
}
#endif

#endif /* NODE_MON_OML_H */
