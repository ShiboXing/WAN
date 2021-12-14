#!/usr/bin/env python
# coding: utf-8

# In[1]:


import pandas as pd
import time, os, json
import subprocess


# In[2]:




# ### EXP 3 data collection

# In[3]:


bw = 10 # mb
rtt = 40 # ms
reno_num = (9, 13)
bbr_num = (9, 13)
bdp = 4
loss_rate = [1, 2, 5, 10, 20, 50]


# In[4]:


for r_num in reno_num:
    for b_num in bbr_num:
        output = subprocess.check_output(f"source exp1_run.sh {bdp} {b_num} {r_num} {bw} {rtt} 60", shell=True, executable="/bin/bash")
#                 os.environ.update(line.partition('=')[::2] for line in output.split('\0'))
#                 !source exp1_run.sh $q $b_num $r_num $bw $rtt
        print(output)
        time.sleep(60)
        print(f'exp: {queues[q_ind]} {b_num} {r_num} {bw} {rtt} {times[q_ind]} completed')


# In[ ]:




