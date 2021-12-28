#!/bin/bash

rt_cpu=1

#off perf-ctrl first
wrmsr -p $rt_cpu 0x38f 0x0

# L2_RQSTS.REF
#wrmsr -p 1 0x186 0x43ff24

# ------- for offcore-rsp------------------
#choose 186 and rsp-0
wrmsr -p $rt_cpu 0x186 0x4301B7
#OFFCORE_RESPONSE.OTHER.L3_HIT.ANY_SNOOP  0x3FC01C8000
wrmsr -p $rt_cpu 0x1a6 0x3FC01C8000

wrmsr -p $rt_cpu 0x187 0x4301BB
#OFFCORE_RESPONSE.DEMAND_DATA_RD.ANY_RESPONSE   0x0000010001
wrmsr -p $rt_cpu 0x1a7 0x0000010001

#--------------above for RSP---------------

# L2_RQSTS.MISS, CFL/Sky-L, Kaby-L (whl same)
wrmsr -p $rt_cpu 0x188 0x433f24

# L3_LAT_CACHE.MISS
wrmsr -p $rt_cpu 0x189 0x43412e

# CYCLE_ACTIVITY.STALLS_L2_MISS
wrmsr -p $rt_cpu 0x18A 0x54305a3

# OFFCORE_REQUESTS.ALL_REQUESTS 
wrmsr -p $rt_cpu 0x18B 0x4380b0

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DATA_RD
wrmsr -p $rt_cpu 0x18C 0x1430860

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DEMAND_DATA_RD
wrmsr -p $rt_cpu 0x18D 0x1430160

#CYCLE_ACTIVITY.STALLS_MEM_ANY
#wrmsr -p 1 0x189 0x144314a3

#Instructions (Fixed 0/1/2)
wrmsr -p 1 0x38d 0xbbb

echo -e "Latency Times Inst Cycles Tsc Off-rsp0 Off-rsp1 L2-Miss L3-Miss L2-stalls Offcore-AllReq Offcore-allDataRD Offcore-RDcycles Offcore-DmndRD"

#echo -e "Latency Times Inst Cycles Tsc L2-Ref L2-Miss L3-Miss Offcore-All Offcore-allDataRD Offcore-RFO Stalls-L2 Stalls-L3"
