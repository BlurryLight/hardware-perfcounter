/*
 * Freedreno Perfetto Counter Formulas — Reference
 *
 * Source: mesa/src/freedreno/ds/fd_pps_{a6xx,a7xx}.cc
 *
 * These are the GPU performance counter formulas used by mesa's freedreno
 * Perfetto producer (gfx-pps). They derive high-level metrics from raw
 * hardware countables.
 *
 * Notation:
 *   BEAT = 32 bytes on a6xx/a7xx (VBIF/GBIF data bus = 256 bit)
 *   time = seconds since last sample
 *   max_freq = max GPU frequency in Hz
 *   info->num_sp_cores = number of SP cores (2 on a630/a6xx)
 *   number_of_usptp = info->num_sp_cores * 2
 *   number_of_alus_per_usptp = 128
 *
 * Counter encoding: (group_id << 8) | counter_id
 *   group_id = enum upper 24 bits, counter_id = lower 8 bits
 */

/*
 * ============================================================================
 * Helper functions (from fd_pps_driver.h)
 * ============================================================================
 */

/* safe division: returns 0 if divisor is 0 */
static inline double safe_div(uint64_t a, uint64_t b)
{
    if (b == 0) return 0;
    return a / (double)b;
}

/* percentage: returns 0 if b == 0 or a > b (clamps bogus values) */
static inline float percent(uint64_t a, uint64_t b)
{
    if (b == 0 || a > b) return 0;
    return 100.f * (a / (double)b);
}

/*
 * ============================================================================
 * A6XX Group IDs (from hardware-perfcounter encoding)
 * ============================================================================
 *
 *   Group         ID    Shifted (ID << 8)
 *   ---------     ---   ----------------
 *   CP            0     0x0000
 *   RBBM          1     0x0100
 *   PC            2     0x0200
 *   VFD           3     0x0300
 *   HLSQ          4     0x0400
 *   VPC           5     0x0500
 *   TSE           6     0x0600
 *   RAS           7     0x0700
 *   UCHE          8     0x0800
 *   TP            9     0x0900
 *   SP            10    0x0A00
 *   RB            11    0x0B00
 *   VSC           12    0x0C00
 *   CCU           13    0x0D00
 *   LRZ           14    0x0E00
 *   CMP           26    0x1A00
 *
 *   NOT EXPOSED on a6xx KGSL:
 *     GBIF (AXI bus counters) — hardware exists at 0x3cc2
 *     VBIF (a5xx block) — returns 0 on a6xx
 */

/*
 * ============================================================================
 * A6XX GPU General
 * ============================================================================
 */

/*
 * GPU Clocks / Second (GPU Frequency)
 *
 * Countable: CP, PERF_CP_ALWAYS_COUNT (group=0, counter=0)
 *
 * Counter IDs:
 *   a6xx: HPC_GPU_ADRENO_A6XX_CP_ALWAYS_COUNT = 0x0000
 *   common: HPC_GPU_ADRENO_COMMON_CP_ALWAYS_COUNT = 0x0000
 */
/* return PERF_CP_ALWAYS_COUNT / time; */

/*
 * GPU % Utilization
 *
 * Countable: CP, PERF_CP_BUSY_CYCLES (group=0, counter=2)
 *
 * Counter IDs:
 *   a6xx: HPC_GPU_ADRENO_A6XX_CP_BUSY_CYCLES = 0x0002
 *   common: HPC_GPU_ADRENO_COMMON_CP_BUSY_CYCLES = 0x0002
 */
/* return percent(PERF_CP_BUSY_CYCLES / time, max_freq); */

/*
 * Shader Core Utilization
 *
 * Countable: SP, PERF_SP_BUSY_CYCLES (group=10, counter=0)
 *
 * Counter IDs:
 *   a6xx: HPC_GPU_ADRENO_A6XX_SP_BUSY_CYCLES = 0x0A00
 *   common: HPC_GPU_ADRENO_COMMON_SP_BUSY_CYCLES = 0x0A00
 */
/* return percent(PERF_SP_BUSY_CYCLES / time, max_freq * info->num_sp_cores); */

/*
 * Preemptions / second
 *
 * Countables: CP, PERF_CP_NUM_PREEMPTIONS (group=0, counter=3)
 *
 * Counter IDs:
 *   a6xx: HPC_GPU_ADRENO_A6XX_CP_NUM_PREEMPTIONS = 0x0003
 *   common: HPC_GPU_ADRENO_COMMON_CP_NUM_PREEMPTIONS = 0x0003
 */
/* return PERF_CP_NUM_PREEMPTIONS * (1.f / time); */

/*
 * Avg Preemption Delay
 *
 * Countables: CP, PERF_CP_PREEMPTION_REACTION_DELAY (group=0, counter=4)
 *   a6xx: HPC_GPU_ADRENO_A6XX_CP_PREEMPTION_REACTION_DELAY = 0x0004
 */
/* return PERF_CP_PREEMPTION_REACTION_DELAY * (1.f / time); */

/*
 * ============================================================================
 * A6XX GPU Primitive Processing
 * ============================================================================
 */

/*
 * Pre-clipped Polygons / Second
 *
 * Countable: TSE, PERF_TSE_INPUT_PRIM (group=6, counter=6)
 *   a6xx: HPC_GPU_ADRENO_A6XX_TSE_INPUT_PRIM = 0x0606
 */
/* return PERF_TSE_INPUT_PRIM * (1.f / time); */

/*
 * % Prims Trivially Rejected
 *
 * Countables: TSE, PERF_TSE_TRIVAL_REJ_PRIM / PERF_TSE_INPUT_PRIM
 *   a6xx: HPC_GPU_ADRENO_A6XX_TSE_TRIVAL_REJ_PRIM = 0x0608
 */
/* return percent(PERF_TSE_TRIVAL_REJ_PRIM, PERF_TSE_INPUT_PRIM); */

/*
 * % Prims Clipped
 *
 * Countables: TSE, PERF_TSE_CLIPPED_PRIM / PERF_TSE_INPUT_PRIM
 *   a6xx: HPC_GPU_ADRENO_A6XX_TSE_CLIPPED_PRIM = 0x0609
 */
/* return percent(PERF_TSE_CLIPPED_PRIM, PERF_TSE_INPUT_PRIM); */

/*
 * Average Vertices / Polygon
 *
 * Countables: PC, PERF_PC_VS_INVOCATIONS (0x021A) / TSE_INPUT_PRIM (0x0606)
 */
/* return PERF_PC_VS_INVOCATIONS / PERF_TSE_INPUT_PRIM; */

/*
 * Reused Vertices / Second (post-transform vertex cache hits)
 *
 * Countable: PC, PERF_PC_VERTEX_HITS (group=2, counter=19)
 *   a6xx: HPC_GPU_ADRENO_A6XX_PC_VERTEX_HITS = 0x0214 (532)
 * Wait: 0x0214 = 532, but counter is 19? Let's check:
 *   0x0200 + 19 = 0x0213, but enum says 0x0214.
 *   Actually: HPC_GPU_ADRENO_A6XX_PC_VERTEX_HITS = 532 = 0x0214
 *   group=2, counter=(532 & 0xFF) = 20. Yes, counter_id=20.
 * Wait no: 532 >> 8 = 2, 532 & 0xFF = 20. counter_id = 20.
 */
/* return PERF_PC_VERTEX_HITS * (1.f / time); */

/*
 * Average Polygon Area (pixels / visible prim)
 *
 * Countables: HLSQ, PERF_HLSQ_QUADS (0x0410) / TSE_OUTPUT_VISIBLE_PRIM (0x060E)
 *   quads * 4 = fragments (pixels)
 */
/* return safe_div(PERF_HLSQ_QUADS * 4, PERF_TSE_OUTPUT_VISIBLE_PRIM); */

/*
 * ============================================================================
 * A6XX GPU Shader Processing
 * ============================================================================
 */

/*
 * Vertices Shaded / Second
 *
 * Countable: PC, PERF_PC_VS_INVOCATIONS (0x021A)
 */
/* return PERF_PC_VS_INVOCATIONS * (1.f / time); */

/*
 * Fragments Shaded / Second
 *
 * Countable: HLSQ, PERF_HLSQ_QUADS (0x0410)
 *   quads * 4 = fragments (pixels)
 */
/* return PERF_HLSQ_QUADS * 4 * (1.f / time); */

/*
 * Vertex Instructions / Second
 *
 * Countables: SP, VS_STAGE_FULL_ALU (0x0A23) + VS_STAGE_EFU (0x0A22)
 *   a6xx: HPC_GPU_ADRENO_A6XX_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS = 2595 = 0x0A23
 *         HPC_GPU_ADRENO_A6XX_SP_VS_STAGE_EFU_INSTRUCTIONS = 2594 = 0x0A22
 */
/* return (PERF_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS +
           PERF_SP_VS_STAGE_EFU_INSTRUCTIONS) * (1.f / time); */

/*
 * Fragment Instructions / Second
 *
 * Countables: SP, FS_STAGE_FULL_ALU (0x0A28) + FS_STAGE_HALF_ALU/2 (0x0A29) + FS_STAGE_EFU (0x0A27)
 *   a6xx: HPC_GPU_ADRENO_A6XX_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS = 2600 = 0x0A28
 *         HPC_GPU_ADRENO_A6XX_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS = 2601 = 0x0A29
 *         HPC_GPU_ADRENO_A6XX_SP_FS_STAGE_EFU_INSTRUCTIONS = 2599 = 0x0A27
 */
/* return (PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS +
           PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS / 2 +
           PERF_SP_FS_STAGE_EFU_INSTRUCTIONS) * (1.f / time); */

/*
 * Fragment ALU Instructions / Sec (Full)
 *
 * Countable: SP, FS_STAGE_FULL_ALU (0x0A28)
 */
/* return PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS * (1.f / time); */

/*
 * Fragment ALU Instructions / Sec (Half)
 *
 * Countable: SP, FS_STAGE_HALF_ALU (0x0A29)
 */
/* return PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS * (1.f / time); */

/*
 * Fragment EFU Instructions / Second
 *
 * Countable: SP, FS_STAGE_EFU (0x0A27)
 */
/* return PERF_SP_FS_STAGE_EFU_INSTRUCTIONS * (1.f / time); */

/*
 * Textures / Vertex
 *
 * Countables: SP, VS_STAGE_TEX (0x0A21) / PC, VS_INVOCATIONS (0x021A)
 *   a6xx: HPC_GPU_ADRENO_A6XX_SP_VS_STAGE_TEX_INSTRUCTIONS = 2593 = 0x0A21
 */
/* return safe_div(PERF_SP_VS_STAGE_TEX_INSTRUCTIONS, PERF_PC_VS_INVOCATIONS); */

/*
 * Textures / Fragment
 *
 * Countables: TP, OUTPUT_PIXELS (0x090A) / HLSQ_QUADS*4 (0x0410*4)
 *   a6xx: HPC_GPU_ADRENO_A6XX_TP_OUTPUT_PIXELS = 2314 = 0x090A
 */
/* return safe_div(PERF_TP_OUTPUT_PIXELS, PERF_HLSQ_QUADS * 4); */

/*
 * ALU / Vertex
 *
 * Countables: SP, VS_STAGE_FULL_ALU (0x0A23) / PC, VS_INVOCATIONS (0x021A)
 */
/* return safe_div(PERF_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS, PERF_PC_VS_INVOCATIONS); */

/*
 * EFU / Vertex
 *
 * Countables: SP, VS_STAGE_EFU (0x0A22) / PC, VS_INVOCATIONS (0x021A)
 */
/* return safe_div(PERF_SP_VS_STAGE_EFU_INSTRUCTIONS, PERF_PC_VS_INVOCATIONS); */

/*
 * ALU / Fragment
 *
 * Countables: (FS_STAGE_FULL_ALU + FS_STAGE_HALF_ALU/2) / HLSQ_QUADS
 */
/* return safe_div(PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS +
                   PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS / 2,
                   PERF_HLSQ_QUADS); */

/*
 * EFU / Fragment
 *
 * Countables: SP, FS_STAGE_EFU (0x0A27) / HLSQ_QUADS (0x0410)
 */
/* return safe_div(PERF_SP_FS_STAGE_EFU_INSTRUCTIONS, PERF_HLSQ_QUADS); */

/*
 * % Time Shading Vertices / Fragments / Compute
 *
 * Countables: SP, ANY_EU_WORKING_{VS,FS,CS}_STAGE
 *   a6xx: HPC_GPU_ADRENO_A6XX_SP_ANY_EU_WORKING_VS_STAGE = 2636 = 0x0A4C
 *         HPC_GPU_ADRENO_A6XX_SP_ANY_EU_WORKING_FS_STAGE = 2634 = 0x0A4A
 *         HPC_GPU_ADRENO_A6XX_SP_ANY_EU_WORKING_CS_STAGE = 2638 = 0x0A4E
 *
 *   total = ANY_EU_WORKING_VS + ANY_EU_WORKING_FS + ANY_EU_WORKING_CS
 *   % VS = ANY_EU_WORKING_VS / total
 *   % FS = (ANY_EU_WORKING_FS - ANY_EU_WORKING_CS) / total   (CS is subset of FS)
 *   % CS = ANY_EU_WORKING_CS / total
 */
/*
   int64_t total = PERF_SP_ANY_EU_WORKING_VS_STAGE +
                   PERF_SP_ANY_EU_WORKING_FS_STAGE;
   percent(PERF_SP_ANY_EU_WORKING_VS_STAGE, total);   // VS
   percent(PERF_SP_ANY_EU_WORKING_FS_STAGE - PERF_SP_ANY_EU_WORKING_CS_STAGE, total);  // FS
   percent(PERF_SP_ANY_EU_WORKING_CS_STAGE, total);   // CS
*/

/*
 * % Shader ALU Capacity Utilized
 *
 * Countables: (VS_FULL_ALU + FS_FULL_ALU + FS_HALF_ALU/2) / 64 / SP_BUSY_CYCLES
 *   Denominator: SP_BUSY_CYCLES (the number of uSPTP clock cycles)
 *   Numerator / 64 = ALU instructions per uSPTP (64 ALU lanes per uSPTP)
 */
/*
   percent((PERF_SP_VS_STAGE_FULL_ALU_INSTRUCTIONS +
            PERF_SP_FS_STAGE_FULL_ALU_INSTRUCTIONS +
            PERF_SP_FS_STAGE_HALF_ALU_INSTRUCTIONS / 2) / 64,
           PERF_SP_BUSY_CYCLES);
*/

/*
 * % Time ALUs Working
 *
 * Note: ALU_WORKING_CYCLES halved because each uSPTP has 2 ALU pipelines
 */
/* return percent(PERF_SP_ALU_WORKING_CYCLES / 2, PERF_SP_BUSY_CYCLES); */

/*
 * % Time EFUs Working
 */
/* return percent(PERF_SP_EFU_WORKING_CYCLES / 2, PERF_SP_BUSY_CYCLES); */

/*
 * ============================================================================
 * A6XX GPU Memory & Bandwidth
 * ============================================================================
 */

/*
 * Vertex Memory Read (Bytes/sec)
 *
 * Countable: UCHE, PERF_UCHE_VBIF_READ_BEATS_VFD (group=8, counter=5)
 *   a6xx: HPC_GPU_ADRENO_A6XX_UCHE_VBIF_READ_BEATS_VFD = 2053 = 0x0805
 *   common: HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_VFD = 0x0805
 *
 *   BEATS * 32 bytes/beat / time
 */
/* return PERF_UCHE_VBIF_READ_BEATS_VFD * 32 * (1.f / time); */

/*
 * SP Memory Read (Bytes/sec)
 *
 * Countable: UCHE, PERF_UCHE_VBIF_READ_BEATS_SP (group=8, counter=8)
 *   a6xx: HPC_GPU_ADRENO_A6XX_UCHE_VBIF_READ_BEATS_SP = 2056 = 0x0808
 *   common: HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_SP = 0x0808
 */
/* return PERF_UCHE_VBIF_READ_BEATS_SP * 32 * (1.f / time); */

/*
 * Texture Memory Read BW (Bytes/sec) — a7xx formula (DISABLED on a6xx)
 *
 * Countables:
 *   UCHE, PERF_UCHE_VBIF_READ_BEATS_TP (0x0804)
 *   CMP,  PERF_CMPDECMP_VBIF_READ_DATA (0x1A07)
 *
 * Note: a6xx Perfetto producer found PERF_CMPDECMP_VBIF_READ_DATA
 *       "cannot get countable" and commented this out.
 */
/* return ((PERF_UCHE_VBIF_READ_BEATS_TP + PERF_CMPDECMP_VBIF_READ_DATA) * 32) / time; */

/*
 * Avg Bytes / Fragment
 *
 * Countables: UCHE, PERF_UCHE_VBIF_READ_BEATS_TP (0x0804)
 *             HLSQ, PERF_HLSQ_QUADS (0x0410)
 */
/* return safe_div(PERF_UCHE_VBIF_READ_BEATS_TP * 32, PERF_HLSQ_QUADS * 4); */

/*
 * Avg Bytes / Vertex
 *
 * Countables: UCHE, PERF_UCHE_VBIF_READ_BEATS_VFD (0x0805)
 *             PC, PERF_PC_VS_INVOCATIONS (0x021A)
 */
/* return safe_div(PERF_UCHE_VBIF_READ_BEATS_VFD * 32, PERF_PC_VS_INVOCATIONS); */

/*
 * ============================================================================
 * A7XX Bandwidth Formulas (NOT available on a6xx — requires GBIF/VBIF)
 * ============================================================================
 *
 * These formulas reference counters that are NOT exposed through
 * the KGSL perfcounter API on a6xx. They are documented here as a
 * reference for future hardware or for understanding what SP measures.
 */

/*
 * DISABLED: Read Total (Bytes/sec)
 *
 * REQUIRES: KGSL_PERFCOUNTER_GROUP_VBIF (or GBIF)
 *   PERF_GBIF_AXI0_READ_DATA_BEATS_TOTAL = counter_id 34
 *   PERF_GBIF_AXI1_READ_DATA_BEATS_TOTAL = counter_id 35
 *
 * SP formula:
 *   GPU Read Total = (AXI0_READ_DATA_BEATS + AXI1_READ_DATA_BEATS) * 32 / time
 *
 * mesa comment:
 *   "requires VBIF perfcounter group exposure which isn't trivial because of
 *    more complex way that those counters are enabled"
 */
/* DISABLED: return (GBIF_AXI0_READ_DATA_BEATS + GBIF_AXI1_READ_DATA_BEATS) * 32 / time; */

/*
 * DISABLED: Write Total (Bytes/sec)
 *
 * REQUIRES: KGSL_PERFCOUNTER_GROUP_VBIF (or GBIF)
 *   PERF_GBIF_AXI0_WRITE_DATA_BEATS_TOTAL = counter_id 46
 *   PERF_GBIF_AXI1_WRITE_DATA_BEATS_TOTAL = counter_id 47
 *
 * SP formula:
 *   GPU Write Total = (AXI0_WRITE_DATA_BEATS + AXI1_WRITE_DATA_BEATS) * 32 / time
 */
/* DISABLED: return (GBIF_AXI0_WRITE_DATA_BEATS + GBIF_AXI1_WRITE_DATA_BEATS) * 32 / time; */

/*
 * DISABLED: GPU % Bus Busy
 *
 * REQUIRES: KGSL_PERFCOUNTER_GROUP_VBIF + RBBM + UCHE
 *   PERF_UCHE_STALL_CYCLES_ARBITER + sum(AXI{0,1}_{RD,WR}_DATA_BEATS)
 *   divided by (4 * PERF_RBBM_STATUS_MASKED)
 *
 * mesa comment:
 *   "requires VBIF perfcounter group exposure which isn't trivial because of
 *    more complex way that those counters are enabled"
 */

/*
 * ============================================================================
 * A7XX Compute Bandwidth Formulas (UCHE counters, usable on a6xx via common.h)
 * ============================================================================
 */

/*
 * Bytes Data Actually Written (L2 eviction bytes)
 *
 * Countable: UCHE, PERF_UCHE_EVICTS (group=8, counter=18)
 *   a6xx: HPC_GPU_ADRENO_A6XX_UCHE_EVICTS = 2066 = 0x0812
 *   common: HPC_GPU_ADRENO_COMMON_UCHE_EVICTS = 0x0812
 *
 *   Multiply by 64 (cacheline size)
 */
/* return PERF_UCHE_EVICTS * 64; */

/*
 * Bytes Data Write Requested
 *
 * Countable: UCHE, PERF_UCHE_WRITE_REQUESTS_SP (group=8, counter=15)
 *   a6xx: HPC_GPU_ADRENO_A6XX_UCHE_WRITE_REQUESTS_SP = 2063 = 0x080F
 *   common: HPC_GPU_ADRENO_COMMON_UCHE_WRITE_REQUESTS_SP = 0x080F
 *
 *   Multiply by 16 (transaction size for write requests)
 */
/* return PERF_UCHE_WRITE_REQUESTS_SP * 16; */

/*
 * Global Buffer Data Read BW (Bytes/sec) — SP L2 reads to DRAM
 *
 * Countable: UCHE, PERF_UCHE_VBIF_READ_BEATS_SP (0x0808)
 */
/* return (PERF_UCHE_VBIF_READ_BEATS_SP * 32) / time; */

/*
 * Global Buffer Data Read Request BW (Bytes/sec) — SP L2 read requests
 *
 * Countable: UCHE, PERF_UCHE_READ_REQUESTS_SP (group=8, counter=13)
 *   a6xx: HPC_GPU_ADRENO_A6XX_UCHE_READ_REQUESTS_SP = 2061 = 0x080D
 *   common: HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_SP = 0x080D
 *
 *   Multiply by 16 (transaction size for read requests)
 */
/* return (PERF_UCHE_READ_REQUESTS_SP * 16) / time; */

/*
 * % Global Buffer Read L2 Hit
 *
 *   Hit = total requests - (VBIF beats / 2)
 *   (beats represent 32-byte transactions; requests are 16-byte)
 *
 * Countables: UCHE_READ_REQUESTS_SP (0x080D), UCHE_VBIF_READ_BEATS_SP (0x0808)
 */
/* return percent(PERF_UCHE_READ_REQUESTS_SP - (PERF_UCHE_VBIF_READ_BEATS_SP / 2),
                  PERF_UCHE_READ_REQUESTS_SP); */

/*
 * % Global Buffer Write L2 Hit
 *
 *   Hit = write requests - evictions
 *
 * Countables: UCHE_WRITE_REQUESTS_SP (0x080F), UCHE_EVICTS (0x0812)
 */
/* return percent(PERF_UCHE_WRITE_REQUESTS_SP - PERF_UCHE_EVICTS,
                  PERF_UCHE_WRITE_REQUESTS_SP); */

/*
 * Global Image Compressed Data Read BW (Bytes/sec)
 *
 * Countable: CMP, PERF_CMPDECMP_VBIF_READ_DATA (group=26, counter=7)
 *   a6xx: HPC_GPU_ADRENO_A6XX_CMP_CMPDECMP_VBIF_READ_DATA = 6663 = 0x1A07
 *   common: HPC_GPU_ADRENO_COMMON_CMP_CMPDECMP_VBIF_READ_DATA = 0x1A07
 */
/* return (PERF_CMPDECMP_VBIF_READ_DATA * 32) / time; */

/*
 * Global Image Data Read Request BW (Bytes/sec)
 *
 * Countable: UCHE, PERF_UCHE_READ_REQUESTS_TP (group=8, counter=9)
 *   a6xx: HPC_GPU_ADRENO_A6XX_UCHE_READ_REQUESTS_TP = 2057 = 0x0809
 *   common: HPC_GPU_ADRENO_COMMON_UCHE_READ_REQUESTS_TP = 0x0809
 *
 *   Multiply by 16 (transaction size)
 */
/* return (PERF_UCHE_READ_REQUESTS_TP * 16) / time; */

/*
 * Global Image Uncompressed Data Read BW (Bytes/sec)
 *
 * Countable: UCHE, PERF_UCHE_VBIF_READ_BEATS_TP (group=8, counter=4)
 *   a6xx: HPC_GPU_ADRENO_A6XX_UCHE_VBIF_READ_BEATS_TP = 2052 = 0x0804
 *   common: HPC_GPU_ADRENO_COMMON_UCHE_VBIF_READ_BEATS_TP = 0x0804
 */
/* return (PERF_UCHE_VBIF_READ_BEATS_TP * 32) / time; */

/*
 * % Image Read L2 Hit
 *
 * Countables: UCHE_READ_REQUESTS_TP (0x0809), UCHE_VBIF_READ_BEATS_TP (0x0804)
 */
/* return percent(PERF_UCHE_READ_REQUESTS_TP - (PERF_UCHE_VBIF_READ_BEATS_TP / 2),
                  PERF_UCHE_READ_REQUESTS_TP); */

/*
 * ============================================================================
 * A6XX GPU Stalls
 * ============================================================================
 */

/*
 * % Stalled on System Memory (L2 cache stalled waiting for DRAM)
 *
 * Countable: UCHE, PERF_UCHE_STALL_CYCLES_ARBITER (group=8, counter=1)
 *   a6xx: HPC_GPU_ADRENO_A6XX_UCHE_STALL_CYCLES_ARBITER = 2049 = 0x0801
 */
/*
   return percent(PERF_UCHE_STALL_CYCLES_ARBITER / time,
                  max_freq * info->num_sp_cores);
*/

/*
 * % Texture Fetch Stall (shader stalled waiting for texture data)
 *
 * Countable: SP, PERF_SP_STALL_CYCLES_TP (group=10, counter=4)
 *   a6xx: HPC_GPU_ADRENO_A6XX_SP_STALL_CYCLES_TP = 2564 = 0x0A04
 *   common: HPC_GPU_ADRENO_COMMON_SP_STALL_CYCLES_TP = 0x0A04
 */
/*
   return percent(PERF_SP_STALL_CYCLES_TP / time,
                  max_freq * info->num_sp_cores);
*/

/*
 * % Vertex Fetch Stall (PC stalled waiting for VFD)
 *
 * Countable: PC, PERF_PC_STALL_CYCLES_VFD (group=2, counter=2)
 *   a6xx: HPC_GPU_ADRENO_A6XX_PC_STALL_CYCLES_VFD = 514 = 0x0202
 *   common: HPC_GPU_ADRENO_COMMON_PC_STALL_CYCLES_VFD = 0x0202
 */
/*
   return percent(PERF_PC_STALL_CYCLES_VFD / time,
                  max_freq * info->num_sp_cores);
*/

/*
 * ============================================================================
 * A6XX GPU Texturing
 * ============================================================================
 */

/*
 * TP L1 Cache Misses / sec
 *
 * Countable: TP, PERF_TP_L1_CACHELINE_MISSES (group=9, counter=7)
 *   a6xx: HPC_GPU_ADRENO_A6XX_TP_L1_CACHELINE_MISSES = 2311 = 0x0907
 *   common: HPC_GPU_ADRENO_COMMON_TP_L1_CACHELINE_MISSES = 0x0907
 */
/* return PERF_TP_L1_CACHELINE_MISSES / time; */

/*
 * L1 Texture Cache Miss Per Pixel
 *
 * Countables: TP, L1_CACHELINE_MISSES (0x0907) / (HLSQ_QUADS*4) (0x0410*4)
 */
/* return safe_div(PERF_TP_L1_CACHELINE_MISSES, PERF_HLSQ_QUADS * 4); */

/*
 * % Texture L1 Miss
 *
 * Countables: TP, L1_CACHELINE_MISSES (0x0907) / L1_CACHELINE_REQUESTS (0x0906)
 *   a6xx: HPC_GPU_ADRENO_A6XX_TP_L1_CACHELINE_REQUESTS = 2310 = 0x0906
 */
/* return percent(PERF_TP_L1_CACHELINE_MISSES, PERF_TP_L1_CACHELINE_REQUESTS); */

/*
 * % Texture L2 Miss
 *
 * Countables: UCHE, VBIF_READ_BEATS_TP (0x0804) * 2 / READ_REQUESTS_TP (0x0809)
 *   NOTE: beats/2 mystery constant — each beat=32B, request=16B
 */
/* return percent(PERF_UCHE_VBIF_READ_BEATS_TP / 2, PERF_UCHE_READ_REQUESTS_TP); */

/*
 * Texture Filter method percentages:
 *
 *   % Anisotropic    = PERF_TP_OUTPUT_PIXELS_ANISO  (0x0924) / OUTPUT_PIXELS (0x090A)
 *   % Linear         = PERF_TP_OUTPUT_PIXELS_BILINEAR (0x0922) / OUTPUT_PIXELS
 *   % Nearest/Point  = PERF_TP_OUTPUT_PIXELS_POINT  (0x0921) / OUTPUT_PIXELS
 *   % Non-Base Level = 100% - PERF_TP_OUTPUT_PIXELS_ZERO_LOD (0x0925) / OUTPUT_PIXELS
 */

/*
 * ============================================================================
 * A6XX LRZ (Low Resolution Z)
 * ============================================================================
 */

/*
 * % LRZ Pixel Killed
 *
 * Countables: LRZ, TOTAL_PIXEL (0x0E11), VISIBLE_PIXEL_AFTER_LRZ (0x0E12)
 *   LRZ_TOTAL_PIXEL (6400 + 17 = 0x0E11): HPC_GPU_ADRENO_A6XX_LRZ_TOTAL_PIXEL = 6417
 *   LRZ_VISIBLE_PIXEL_AFTER_LRZ (6400 + 18 = 0x0E12): HPC_GPU_ADRENO_A6XX_LRZ_VISIBLE_PIXEL_AFTER_LRZ = 6418
 */
/* return percent(PERF_LRZ_TOTAL_PIXEL - PERF_LRZ_VISIBLE_PIXEL_AFTER_LRZ,
                  PERF_LRZ_TOTAL_PIXEL); */

/*
 * LRZ Primitives Killed
 *
 * Countable: LRZ, PRIM_KILLED_BY_LRZ (group=14, counter=12)
 *   a6xx: HPC_GPU_ADRENO_A6XX_LRZ_PRIM_KILLED_BY_LRZ = 6412 = 0x0E0C
 */
/* return PERF_LRZ_PRIM_KILLED_BY_LRZ; */

/*
 * LRZ Tiles Killed
 *
 * Countable: LRZ, TILE_KILLED (group=14, counter=16)
 *   a6xx: HPC_GPU_ADRENO_A6XX_LRZ_TILE_KILLED = 6416 = 0x0E10
 */
/* return PERF_LRZ_TILE_KILLED; */

/*
 * ============================================================================
 * Known Limitations (a6xx KGSL perfcounter API)
 * ============================================================================
 *
 * 1. MAX 4 counters per ioctl batch (ADRENO_IOCTL_COUNTER_GET limit)
 *
 * 2. NO GBIF group exposed. The AXI bus-level counters that Snapdragon
 *    Profiler uses for "Read Total" / "Write Total" (GBIF_AXI{0,1}_{READ,WRITE}_DATA_BEATS_TOTAL)
 *    are not accessible via /dev/kgsl-3d0 on a6xx.
 *
 *    Hardware registers exist: GBIF_PERF_CNT_SEL @ 0x3cc2, counters @ 0x3cc4-0x3ccb
 *    Counter IDs: 34=AXI0_READ, 35=AXI1_READ, 46=AXI0_WRITE, 47=AXI1_WRITE
 *    (confirmed from mesa a7xx_perfcntrs.xml/a8xx_perfcntrs.xml)
 *
 *    mesa's fd_pps_a7xx.cc also has Read/Write Total DISABLED with comment:
 *    "TODO: requires VBIF perfcounter group exposure which isn't trivial
 *     because of more complex way that those counters are enabled"
 *
 * 3. Snapdragon Profiler accesses these via sdpservice (root) → pluginGPU-GGPM →
 *    libSDPCore.so (private RPC), likely through direct register read or
 *    a kernel interface not available to userspace.
 *
 * 4. Coverage: UCHE/CCU/CMP counters provide ~52% of read bandwidth and
 *    ~21% of write bandwidth compared to SP's AXI-bus totals.
 *    Gap = L2 writeback + prefetch + CP fetches + bypass writes.
 */
