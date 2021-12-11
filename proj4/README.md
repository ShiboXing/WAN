# BBR vs loss-based congestion

## Testbed set up
### Add Resource on GENI 
Create three VMs as sender, router and server respectively. The site we were using is University of Chicago

### Enable BBR on three nodes

BBR is not available congestion control on GENI node by default.
To enable it, some commands need to be issued.

If you type this commands
```console
$ sysctl net.ipv4.tcp_available_congestion_control
```
It should report
```console
$ net.ipv4.tcp_available_congestion_control = reno cubic
```
First, issue the following command
```console
$ sudo nano /etc/sysctl.conf
```
At the bottom of this file, add the following two lines
```console
net.core.default_qdisc=fq
net.ipv4.tcp_congestion_control=bbr
```
Save and close that file, reload sysctl with this command
```console
$ sudo sysctl -p
```
Follow the above instructions on three nodes to enable the BBR.<br />
Reference: [How to enable TCP BBR to improve network speed on Linux](https://www.techrepublic.com/article/how-to-enable-tcp-bbr-to-improve-network-speed-on-linux/)


### Install iperf3

First, into iperf directory
```console
$ cd iperf-3.10.1
```
Execute the following commands
```console
$ ./configure; make; make install
```
(Note: If configure fails, try running ./bootstrap.sh first)<br/>
To test if iperf3 install successfully,run
```console
$ src/iperf3 -v
```
and will shows iperf3 3.10.1 (cJSON 1.7.13) <br/>

Reference: [iperf github](https://github.com/esnet/iperf)
###

### Install Anaconda and use jupyternote book
Download Anaconda Linux version
```console
$ wget https://repo.anaconda.com/archive/Anaconda3-2021.11-Linux-x86_64.sh 
```
Ater success install conda,type the following command to activate the conda environment (or following the instruction on anaconda)
```console
$ conda activate
```
Install jupyter
```console
$ conda install jupyter
```
Ater successful installation, we can run the script on sender node to run the experiments

## Run experiments
Change directory to *setup* <br/>
### Server node
(The server can remain open during the all 3 experiments. To prevent accidently shut down the iperf3 server, recommand to run the server in *screen* session)<br/>

Run the source command on the following two .sh files as order
```console
$ source exp1_kill.sh #reset tc qdisc and kill any program that occpied the port 5001 and 5002,we will use these two ports for listening sender

$ source testbed_setup.sh #alias iperf3 and jupyternotebook, hostname

$ source exp1_run.sh 1 1 1 1 1 1 # usage: source exp1_run.sh [queue(# of BDPs)] [# of BBRs] [# of Renos] [bandwidth(mbps)] [RTT(ms)] [duration(s)]
Server don't need those parameters to start, you can put any number other than 1 as long as there are 6 parameters
```

After servers started, detach screen session or simply just leave it there, we won't need to do anything with server node until the experiements are finished 

### Router node

Run the source command on exp1_kill.sh to reset tc qdisc
```console
$ source exp1_kill.sh 
```

## Validation Experiment and extend Experinment 1
We need to change the queue size for each experiment on router

For example, our first experiment is to compare 1-33 BBR vs 1-33 Cubic in 0.25 BDP queue size, 10 mbits bandwidth and 40ms one way delay from router to sender to hold all ACKs <br/>

To set router for first experiment,run the following command
```console
$ source exp1_setup.sh 1 10 40 4 0 #usage: source exp1_setup [# of BDP] [bandwidth(mbits)] [RTT(ms)] [divisor (for fraction BDP)] [loss_rate(%)]
```
10 represents bandwidth, 40 is one way delay<br/>
1 is number of BDP, 4 is divisor <br/>
Because bash don't accept demical number, divisor is added to divide BDP number, as example, we want to run our experiment in 0.25 BDP queue size, we have to input 1 as # of BDP and 4 as divisor, so it represents 0.25. Respectively, 0.5 is 1 BDP and 2 divisor, and 4 is 4 BDP and 1 divisor etc.. 
And loss rate is for extend experiment 2,it is set to 0 for validation experiment and extend experiment 1

Basically, for validation experiment and extend experiment 1, the parameters only need to change is number of BDP and divisor, the remain parameters can remain unchange.

## Extend Experinment 2
Run the source exp1_kill.sh to reset tc qdisc, and run the following command
```console
$ source exp1_setup.sh 4 10 40 1 1
```
For extend experinment 2, we are interested in see how mutiple bbr and cubic behave in different loss rate, so we set parameters to 4 bdp, 10 mbits,40 one way delay, 1 divisor. The only parameters need to change is loss rate, for first experiement the loss rate is 1%

### Sender node

**Start JupyterNotebook** <br/>

We have provide the ipynb scripts for convenience. Basically all we need to do is after each run, use exp1_setup.sh to change the configuration on router, and change the q_ind in scripts and run the scripts again.

To use jupyter notebook to run the script,first to forward port on localhost before ssh to the sender node.
```console
$ ssh -i ~/.ssh/id_geni_ssh_rsa -p port_num id@server_address -L 8008:localhost:8008
```
Change directory to setup and run testbed_setup.sh and exp1_kill.sh
```console
$ source testbed_set.sh #alias iperf3 and jupyternotebook
$ source exp1_kill.sh
```
And then run init_exp to start jupyternotebook

```console
$ init_exp
```
Then the notebook is started and operate in background

```console
(base) qil77@sender:~/WAN/proj4/setup$ init_exp
[1] 618
(base) qil77@sender:~/WAN/proj4/setup$ [W 2021-12-11 17:03:53.729 LabApp] 'port' has moved from NotebookApp to ServerApp. This config will be passed to ServerApp. Be sure to update your config before our next release.
[W 2021-12-11 17:03:53.734 LabApp] 'port' has moved from NotebookApp to ServerApp. This config will be passed to ServerApp. Be sure to update your config before our next release.
[W 2021-12-11 17:03:53.734 LabApp] 'port' has moved from NotebookApp to ServerApp. This config will be passed to ServerApp. Be sure to update your config before our next release.
[I 2021-12-11 17:03:53.756 LabApp] JupyterLab extension loaded from /users/qil77/anaconda3/lib/python3.9/site-packages/jupyterlab
[I 2021-12-11 17:03:53.758 LabApp] JupyterLab application directory is /users/qil77/anaconda3/share/jupyter/lab
[I 17:03:53.769 NotebookApp] The port 8008 is already in use, trying another port.
[I 17:03:53.771 NotebookApp] Serving notebooks from local directory: /users/qil77/WAN/proj4/setup
[I 17:03:53.773 NotebookApp] Jupyter Notebook 6.4.5 is running at:
[I 17:03:53.776 NotebookApp] http://localhost:8009/?token=dda8c330b9e2621b820c3198a961e0b83ba1716aac4047d9
[I 17:03:53.776 NotebookApp]  or http://127.0.0.1:8009/?token=dda8c330b9e2621b820c3198a961e0b83ba1716aac4047d9
[I 17:03:53.776 NotebookApp] Use Control-C to stop this server and shut down all kernels (twice to skip confirmation).
[C 17:03:53.785 NotebookApp] 
    
    To access the notebook, open this file in a browser:
        file:///users/qil77/.local/share/jupyter/runtime/nbserver-618-open.html
    Or copy and paste one of these URLs:
        http://localhost:8009/?token=dda8c330b9e2621b820c3198a961e0b83ba1716aac4047d9
     or http://127.0.0.1:8009/?token=dda8c330b9e2621b820c3198a961e0b83ba1716aac4047d9
```
Copy and paste URL in browser to access the notebook

**Run the script**

Open Data_Collector.ipynb

For validation experiment and extend experiment 1

1. Change q_ind start from 0 in block 2, and then Run block 1~4
This will start the experiment for 1-33 BBR vs 1-33 Cubic respectively in 0.25 BDP, 10 mbits, 40ms, each competition run 60 seconds and then sleep 5 seconds to next competition. each experiment generate 81 competitions and 162 json files as results.

2. After experiment finished,run 
``` console
$ cd iperf3_results
$ mkdir all_{current BDP number} #all_0.25 for first experiment
$ mv *.json all_{current BDP number}
```

3. After first experiment finish, change q_ind = 1, and set router as instruction in router secion, change number of BDP to queues[q_ind] ,queues array is in block 2 in Data_collector.ipynb file, when q_ind = 1, queues[q_ind] = 0.5, means to change the number of BDP in router to 1 and divisor is 2.

4. After change the q_ind and router, run the block 1~4 again to run the next experiment.

5. Keep doing procedure 2 and 3 until finish the experiment when q_ind = 6, which is the last element in list *queues* and list *times*

For extend experiment 2

1. Set router as instruction in router section for experiment 2

2. Run block 5~6 in EXP3 data collection, it generate 4 competitions and 8 json files as results.

3. After experiment finished,run 
``` console
$ cd iperf3_results
$ mkdir all_loss_{current BDP number} #all_loss_1 for first experiment
$ mv *.json all_{current BDP number}
```
4. Reset the loss rate parameter in router to the next number in list *loss_rate*, the second number is 2. 

5. Repeat procedure 2 and 3 until finish all the loss rate in list *loss_rate*



