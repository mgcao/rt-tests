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

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DATA_RD
wrmsr -p $rt_cpu 0x189 0x1430860

#OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DEMAND_DATA_RD
#wrmsr -p $rt_cpu 0x18D 0x1430160

#Instructions (Fixed 0/1/2)
wrmsr -p $rt_cpu 0x38d 0xbbb

#-------------for uncore setting---------------------
#stop first
wrmsr -p $rt_cpu  0xE01  0

#CBO 0-3 preset-0/1: bit22: EN, bit18: cycles/occurs, bit0-15: event/mask
#for all counters 0:
cbo_evt_0=0x00444122
cbo_evt_1=0x00448122

wrmsr -p $rt_cpu  0x700 $cbo_evt_0   #CBO-0  
wrmsr -p $rt_cpu  0x710 $cbo_evt_0   #CBO-1
wrmsr -p $rt_cpu  0x720 $cbo_evt_0   #CBO-2
wrmsr -p $rt_cpu  0x730 $cbo_evt_0   #CBO-3

#for all counters 1:
wrmsr -p $rt_cpu  0x701 $cbo_evt_1   #CBO-0
wrmsr -p $rt_cpu  0x711 $cbo_evt_1   #CBO-1
wrmsr -p $rt_cpu  0x721 $cbo_evt_1   #CBO-2
wrmsr -p $rt_cpu  0x731 $cbo_evt_1   #CBO-3

#for arb-0:
wrmsr -p $rt_cpu  0x3B2  0x00440181 #UNC_ARB_TRK_REQUESTS.ALL

#for arb-1:
wrmsr -p $rt_cpu  0x3B3 0x00440184 #UNC_ARB_COH_TRK_REQUESTS.ALL

#fixed ctrl - local, bit22
wrmsr -p $rt_cpu  0x394  0x00400000

#--------------uncore above-------------------------

#echo -e "Latency Times Inst Cycles Tsc L2-Miss L3-Miss Stalls-L2 Stalls-L3 Offcore-AllReq Offcore-allDataRD Offcore-RD-Cycles Offcore-RFO"
echo -e "Latency Times Inst Cycles Tsc L2-Miss L3-Miss Stalls-L2 Offcore-allDataRD unc-0 unc-1 arb-0 arb-1 u-clk"
