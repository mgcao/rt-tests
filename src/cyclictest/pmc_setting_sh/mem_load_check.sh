#!/bin/bash

rt_cpu=1

#this is for Intel core serial CPU
############# PMC0 ################
# MEM_LOAD_RETIRED.L2_MISS
wrmsr -p $rt_cpu 0x186 0x4310d1

############# PMC1 ################
# MEM_LOAD_RETIRED.L3_HIT
wrmsr -p $rt_cpu 0x187 0x4304d1

############# PMC2 ################
#MEM_LOAD_L3_HIT_RETIRED.XSNP_NONE
wrmsr -p $rt_cpu 0x188 0x4308d2

############# PMC3 ################
# MEM_LOAD_L3_HIT_RETIRED.XSNP_HIT
wrmsr -p $rt_cpu 0x189 0x4302d2

############# PMC4 ################
# L2_RQSTS.MISS
wrmsr -p $rt_cpu 0x18A 0x433f24

############# PMC5 ################
# L3_LAT_CACHE.MISS
wrmsr -p $rt_cpu 0x18B 0x43412e

############# PMC6 ################
# L3_LAT_CACHE.REF
wrmsr -p $rt_cpu 0x18C 0x434F2e

############# PMC7 ################
# CYCLE_ACTIVITY.STALLS_L2_MISS
wrmsr -p $rt_cpu 0x18D 0x54305a3

#Instructions (Fixed 0)
#wrmsr -p 1 0x38f 0x7000000ff
wrmsr -p 1 0x38d 0xbbb

echo -e "Latency Times Inst Cycles Tsc Mem.L2.Miss Mem.L3.Hit XSNP_NONE XSNP_HIT L2-MISS L3-MISS L3-REF L2-Stalls"

