/*
 * Adreno memory bandwidth sampler.
 *
 * Splits counters into batches of 4 to stay within KGSL perfcounter slot
 * limits and avoid the library's wraparound bug across many counters.
 *
 * Adreno 6xx "beat" = 32 bytes; on 5xx it's 16 bytes. Override with
 * BEAT_BYTES env var if needed.
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hpc/gpu/adreno/common.h"
#include "hpc/gpu/base_utilities.h"

static void *allocate(void *user_data, size_t size) { return malloc(size); }
static void deallocate(void *user_data, void *memory) { return free(memory); }

static int print_error(int status, char *message) {
  if (-status >= HPC_GPU_FIRST_ERROR_CODE) {
    printf("error %d: %s\n", status, message);
  } else {
    perror(message);
  }
  return status;
}

typedef struct {
  hpc_gpu_adreno_common_counter_t id;
  const char *short_name;
  const char *desc;
  int unit; /* 0=beat(32B), 1=byte, 2=transaction(16B) */
} counter_spec_t;

#define B  0
#define Y  1
#define T  2

static const counter_spec_t batch_a[] = {
    {HPC_GPU_ADRENO_COMMON_UCHE_GMEM_READ_BEATS,
     "dram_rd",  "L2 miss → DRAM read  (UCHE_GMEM_READ_BEATS)",        B},
    {HPC_GPU_ADRENO_COMMON_CCU_GMEM_READ,
     "ccu_rd",   "color/depth DRAM read  (CCU_GMEM_READ)",            Y},
    {HPC_GPU_ADRENO_COMMON_CCU_GMEM_WRITE,
     "ccu_wr",   "color/depth DRAM write (CCU_GMEM_WRITE)",           Y},
    {HPC_GPU_ADRENO_COMMON_SP_UCHE_READ_TRANS,
     "sp_l2_rd", "shader L1→L2 read trans (SP_UCHE_READ_TRANS)",      T},
};
static const counter_spec_t batch_b[] = {
    {HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_TP,
     "tp",  "texture L2 reads (UCHE_VBIF_READ_BEATS_TP)",             B},
    {HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_VFD,
     "vfd", "vertex fetch L2 reads (UCHE_VBIF_READ_BEATS_VFD)",       B},
    {HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_SP,
     "sp",  "shader L2 reads (UCHE_VBIF_READ_BEATS_SP)",              B},
    {HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_LRZ,
     "lrz", "LRZ L2 reads (UCHE_VBIF_READ_BEATS_LRZ)",                 B},
};
static const counter_spec_t batch_c[] = {
    {HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_HLSQ,
     "hlsq",   "HLSQ L2 reads (UCHE_VBIF_READ_BEATS_HLSQ)",            B},
    {HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_CH0,
     "ch0",    "total L2 reads channel 0",                            B},
    {HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_CH1,
     "ch1",    "total L2 reads channel 1",                            B},
    {HPC_GPU_ADRENO_COMMON_SP_UCHE_WRITE_TRANS,
     "sp_l2_wr", "shader L1→L2 write trans (SP_UCHE_WRITE_TRANS)",    T},
};
static const counter_spec_t batch_d[] = {
    {HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_READ_DATA,
     "cmp_rd_byte", "CMP VBIF read bytes (CMP_CMPDECMP_VBIF_READ_DATA)",  Y},
    {HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_WRITE_DATA,
     "cmp_wr_byte", "CMP VBIF write bytes (CMP_CMPDECMP_VBIF_WRITE_DATA)", Y},
    {HPC_GPU_ADRENO_COMMON_UCHE_GMEM_READ_BEATS,
     "dram_beats",  "UCHE_GMEM_READ_BEATS (for calibration)",            B},
    {HPC_GPU_ADRENO_COMMON_CCU_GMEM_WRITE,
     "ccu_wr",      "CCU_GMEM_WRITE",                                   Y},
};

static double to_mb_s(uint64_t delta, double secs, int unit, size_t beat_b,
                      size_t trans_b) {
  size_t bpb;
  switch (unit) {
    case B: bpb = beat_b; break;
    case Y: bpb = 1; break;
    case T: bpb = trans_b; break;
    default: bpb = beat_b; break;
  }
  return (double)delta * bpb / secs / (1024.0 * 1024.0);
}

static void run_batch(const counter_spec_t *batch, size_t n,
                      size_t beat_b, size_t trans_b, int iters) {
  hpc_gpu_adreno_common_counter_t ids[8];
  for (size_t i = 0; i < n; ++i) ids[i] = batch[i].id;

  hpc_gpu_adreno_context_t *context = NULL;
  hpc_gpu_host_allocation_callbacks_t allocator = {NULL, &allocate,
                                                   &deallocate};
  int status = hpc_gpu_adreno_common_create_context((uint32_t)n, ids,
                                                    &allocator, &context);
  if (status < 0) {
    print_error(status, "create context");
    return;
  }
  status = hpc_gpu_adreno_common_start_counters(context);
  if (status < 0) {
    print_error(status, "start");
    return;
  }

  uint64_t cur[8];
  struct timespec ts = {0, 100000000}, rem;

  for (int i = 0; i < iters; ++i) {
    status = hpc_gpu_adreno_common_query_counters(context, cur);
    if (status < 0) {
      printf("  t+%-3d  ERROR st=%d  reset and retry\n", i + 1, status);
      hpc_gpu_adreno_common_stop_counters(context);
      hpc_gpu_adreno_common_start_counters(context);
      nanosleep(&ts, &rem);
      continue;
    }
    printf("  t+%-3ds ", i + 1);
    for (size_t k = 0; k < n; ++k) {
      printf("%-7s=%7.0fMB/s  ", batch[k].short_name,
             to_mb_s(cur[k], 0.1, batch[k].unit, beat_b, trans_b));
    }
    printf("\n");
    nanosleep(&ts, &rem);
  }

  hpc_gpu_adreno_common_stop_counters(context);
  hpc_gpu_adreno_common_destroy_context(context, &allocator);
}

int main(void) {
  const char *beat_env = getenv("BEAT_BYTES");
  size_t beat_bytes = beat_env ? (size_t)atoi(beat_env) : 32;
  size_t trans_bytes = 16;
  const char *trans_env = getenv("TRANS_BYTES");
  if (trans_env) trans_bytes = (size_t)atoi(trans_env);
  int iters = 30;

  const char *iters_env = getenv("ITERS");
  if (iters_env) iters = atoi(iters_env);

  printf("# Adreno bandwidth sampler  beat=%zuB  trans=%zuB  iters=%d\n",
         beat_bytes, trans_bytes, iters);
  printf("# batch A (DRAM + CCU + shader L1→L2):\n");
  run_batch(batch_a, sizeof(batch_a) / sizeof(batch_a[0]), beat_bytes,
            trans_bytes, iters);
  printf("\n# batch B (per-client L2 reads):\n");
  run_batch(batch_b, sizeof(batch_b) / sizeof(batch_b[0]), beat_bytes,
            trans_bytes, iters);
  printf("\n# batch C (HLSQ + totals + shader L1→L2 writes):\n");
  run_batch(batch_c, sizeof(batch_c) / sizeof(batch_c[0]), beat_bytes,
            trans_bytes, iters);
  printf("\n# batch D (CMP byte counters for calibration):\n");
  run_batch(batch_d, sizeof(batch_d) / sizeof(batch_d[0]), beat_bytes,
            trans_bytes, iters);
  return 0;
}
