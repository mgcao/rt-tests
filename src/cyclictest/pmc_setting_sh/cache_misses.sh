#!/bin/bash

#Instructions (Fixed 0)
wrmsr -p 1 0x38d 0xb

# L2_RQSTS.MISS
wrmsr -p 1 0x186 0x433f24

# L3_LAT_CACHE.MISS
wrmsr -p 1 0x187 0x43412e

#CYCLE_ACTIVITY.CYCLES_L2_MISS  
wrmsr -p 1 0x188 0x14301a3

# CYCLE_ACTIVITY.STALLS_L2_MISS
wrmsr -p 1 0x189 0x54305a3

#CYCLE_ACTIVITY.CYCLES_L3_MISS 
wrmsr -p 1 0x18a 0x24302a3

#CYCLE_ACTIVITY.STALLS_L3_MISS 
wrmsr -p 1 0x18b 0x64306a3

#CYCLE_ACTIVITY.CYCLES_MEM_ANY
wrmsr -p 1 0x18c 0x104310a3

# CYCLE_ACTIVITY.STALLS_MEM_ANY 
wrmsr -p 1 0x18d 0x144314a3

#echo -e "\t\t\t\t  total-Inst  L2-Miss      L3-Miss      Cycles-L2    Cycles-L3    Cycles-Mem   Stalls-L2    Stalls-L3    Stalls-Mem"
echo -e "Latency\tTimes\tInst\tL2-Miss\tL3-Miss\tCycles-L2\tStalls-L2\tCycles-L3\tStalls-L3Cycles-Mem\tStalls-Mem"
