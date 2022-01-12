#!/bin/bash

rt_cpu=1

#off perf-ctrl first
wrmsr -p $rt_cpu 0x38f 0x0

# L2_RQSTS.MISS, CFL/Sky-L, Kaby-L (whl same)
wrmsr -p $rt_cpu 0x186 0x433f24

# L3_LAT_CACHE.MISS
wrmsr -p $rt_cpu 0x187 0x43412e

# CYCLE_ACTIVITY.STALLS_L2_MISS
wrmsr -p $rt_cpu 0x188 0x54305a3

# OFFCORE_REQUESTS.ALL_REQUESTS 
wrmsr -p $rt_cpu 0x189 0x4380b0
#wrmsr -p $rt_cpu 0x18a 0x4301b0
#wrmsr -p $rt_cpu 0x18b 0x4302b0
#wrmsr -p $rt_cpu 0x18c 0x4308b0

#OFFCORE_REQUESTS_OUTSTANDING.ALL_DATA_RD
#wrmsr -p 1 0x189 0x430860

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DATA_RD
wrmsr -p $rt_cpu 0x18a 0x1430860

#OFFCORE_REQUESTS_OUTSTANDING.DEMAND_DATA_RD
#wrmsr -p $rt_cpu 0x18A 0x430160

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DEMAND_CODE_RD
wrmsr -p $rt_cpu 0x18b 0x430260

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DEMAND_DATA_RD
wrmsr -p $rt_cpu 0x18c 0x1430160

#OFFCORE_REQUESTS_OUTSTANDING.DEMAND_DATA_RD_GE_6
wrmsr -p $rt_cpu 0x18d 0x6430160

#OFFCORE_REQUESTS_BUFFER.SQ_FULL
#wrmsr -p $rt_cpu 0x18D 0x4301B2

#OFFCORE_REQUESTS_OUTSTANDING.DEMAND_RFO
#wrmsr -p 1 0x18d 0x1430460


#CYCLE_ACTIVITY.STALLS_MEM_ANY
#wrmsr -p 1 0x189 0x144314a3

#Instructions (Fixed 0/1/2)
wrmsr -p 1 0x38d 0xbbb

echo -e "Latency Times Inst Cycles Tsc L2-Miss L3-Miss L2-stalls Offcore-RDcycles Offcore-dmndRD Offcore-dmndCycles Offcore-DmndGe6 Offcore-SQfull"

#echo -e "Latency Times Inst Cycles Tsc L2-Ref L2-Miss L3-Miss Offcore-All Offcore-allDataRD Offcore-RFO Stalls-L2 Stalls-L3"
