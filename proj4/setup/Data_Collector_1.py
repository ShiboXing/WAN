#!/usr/bin/env python
# coding: utf-8

# In[1]:


import pandas as pd
import time, os, json
import subprocess


# In[2]:


bw = 10 # mb
rtt = 40 # ms
reno_num = range(1, 34, 4)
bbr_num = range(1, 34, 4)

queues = [0.25, 0.5, 1, 4, 8, 16, 32]
times = [60, 60, 60, 60, 60, 100, 200]
q_ind = 0
len(reno_num) * len(bbr_num) * len(queues)


# ### total experiment time (hours)

# In[3]:


comb = len(reno_num) * len(bbr_num)
(comb * 5 * 60 + comb * 1 * 100 + comb * 1 * 200) / 3600


# ### EXP1, 2 data collection

# !source testbed_setup.sh
# !source exp1_kill.sh

# In[3]:


for r_num in reno_num:
    for b_num in bbr_num:
        output = subprocess.check_output(f"source exp1_run.sh {queues[q_ind]} {b_num} {r_num} {bw} {rtt} {times[q_ind]}", shell=True, executable="/bin/bash")
#                 os.environ.update(line.partition('=')[::2] for line in output.split('\0'))
#                 !source exp1_run.sh $q $b_num $r_num $bw $rtt
        print(output)
        time.sleep(times[q_ind]+5)
        print(f'exp: {queues[q_ind]} {b_num} {r_num} {bw} {rtt} {times[q_ind]} completed')

