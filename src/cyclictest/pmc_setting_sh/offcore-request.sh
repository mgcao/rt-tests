#!/bin/bash

rt_cpu=1

#off perf-ctrl first
wrmsr -p $rt_cpu 0x38f 0x0

# L2_RQSTS.REF
#wrmsr -p 1 0x186 0x43ff24

# L2_RQSTS.MISS, CFL/Sky-L, Kaby-L (whl same)
wrmsr -p $rt_cpu 0x186 0x433f24

# L3_LAT_CACHE.MISS
wrmsr -p $rt_cpu 0x187 0x43412e

# CYCLE_ACTIVITY.STALLS_L2_MISS
wrmsr -p $rt_cpu 0x188 0x54305a3

#CYCLE_ACTIVITY.STALLS_L3_MISS 
wrmsr -p $rt_cpu 0x189 0x64306a3

# OFFCORE_REQUESTS.ALL_REQUESTS 
wrmsr -p $rt_cpu 0x18A 0x4380b0

#OFFCORE_REQUESTS_OUTSTANDING.ALL_DATA_RD
wrmsr -p $rt_cpu 0x18B 0x430860

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DATA_RD
wrmsr -p $rt_cpu 0x18C 0x1430860

#OFFCORE_REQUESTS_OUTSTANDING.DEMAND_RFO
#wrmsr -p $rt_cpu 0x18D 0x430460

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DEMAND_DATA_RD
wrmsr -p $rt_cpu 0x18D 0x1430160

#CYCLE_ACTIVITY.STALLS_MEM_ANY
#wrmsr -p $rt_cpu 0x189 0x144314a3

#Instructions (Fixed 0/1/2)
wrmsr -p $rt_cpu 0x38d 0xbbb

echo -e "Latency Times Inst Cycles Tsc L2-Miss L3-Miss Stalls-L2 Stalls-L3 Offcore-AllReq Offcore-allDataRD Offcore-RD-Cycles Offcore-RFO"
