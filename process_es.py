import os
os.environ['OPENBLAS_NUM_THREADS'] = '32' # cannot create too many threads an error on Saturn
import math
import numpy
import sys

if len(sys.argv) != 3:
    sys.exit()

folder = str(sys.argv[1])
config_file = str(sys.argv[2])

# read config file and set up dictionaries
d_nodes = {}
l_nodes = []
d_ppns = {}
l_ppns = []
d_umods = {}
l_umods = []
d_lmods = {}
l_lmods = []
with open(config_file) as f:
    for line in f:
        line = line.replace('=', ' ')
        line = line.replace('(', '')
        line = line.replace(')', '')
        line = line.replace('"', '')
        l = line.split()
        print(l)
        if l[0] == "nodes":
            for i in range(1, len(l)):
                d_nodes[int(l[i])]=i-1
                l_nodes.append(int(l[i]))
            max_nodes = int(l[-1])
        if l[0] == "ppns":
            for i in range(1, len(l)):
                d_ppns[int(l[i])]=i-1
                l_ppns.append(int(l[i]))
        if l[0] == "umods":
            for i in range(1, len(l)):
                d_umods[l[i]]=i-1
                l_umods.append(l[i])
        if l[0] == "lmods":
            for i in range(1, len(l)):
                d_lmods[l[i]]=i-1
                l_lmods.append(l[i])
        if l[0] == "min_msg":
            min_msg = int(l[1])
        if l[0] == "max_msg":
            max_msg = int(l[1])
        if l[0] == "min_seg":
            min_seg = int(l[1])
        if l[0] == "max_seg":
            max_seg = int(l[1])
            
d_msgs = {}
cur = min_msg
id = 0;
while cur <= max_msg:
    d_msgs[cur] = id
    id = id + 1
    cur *= 2
    
d_segs = {}
cur = min_seg
id = 0;
while cur <= max_seg:
    d_segs[cur] = id
    id = id + 1
    cur *= 2

# read the costs of bcasts
cost_bcast = numpy.full((len(d_nodes), len(d_ppns), len(d_umods), len(d_lmods), len(d_segs), len(d_msgs)), float('inf'))
for filename in os.listdir(folder):
    if filename.startswith("bench_bcast."):
        filename_list = filename.split(".")
        argu_list = filename_list[1].split("_")
        print(argu_list)
        node_id = d_nodes[int(argu_list[0])]
        ppn_id = d_ppns[int(argu_list[1])]
        umod_id = d_umods[argu_list[2]]
        lmod_id = d_lmods[argu_list[3]]
        seg_id = d_segs[int(argu_list[4])]
        with open(folder+'/'+filename) as f:
            for line in f:
                l = line.split()
                msg_id = d_msgs[int(l[0])]
                cost_bcast[node_id][ppn_id][umod_id][lmod_id][seg_id][msg_id] = float(l[1])

def bcast(msg, node, ppn, umod, lmod, seg):
    if msg <= min_seg:
        return cost_bcast[d_nodes[node]][d_ppns[ppn]][d_umods[umod]][d_lmods[lmod]][d_segs[min_seg]][d_msgs[msg]]
    return cost_bcast[d_nodes[node]][d_ppns[ppn]][d_umods[umod]][d_lmods[lmod]][d_segs[seg]][d_msgs[msg]]

def print_strategies():
    for node,node_id in d_nodes.items():
        for ppn,ppn_id in d_ppns.items():
            for msg,msg_id in d_msgs.items():
                if msg <= min_seg:
                    tmp_min = float("inf")
                    tmp_min_args = [] 
                    for umod,umod_id in d_umods.items():
                        for lmod,lmod_id in d_lmods.items():
                            tmp = bcast(msg, node, ppn, umod, lmod, min_seg)
                            if tmp < tmp_min:
                                tmp_min = tmp
                                tmp_min_args = [umod, lmod, min_seg]
                    print(str(node) + " nodes " + str(ppn) + " PPN " + str(msg) + "B message" + " -> " + str(tmp_min_args))
                else:
                    tmp_min = float("inf")
                    tmp_min_args = [] 
                    for umod,umod_id in d_umods.items():
                        for lmod,lmod_id in d_lmods.items():
                            for seg,seg_id in d_segs.items():
                                tmp = bcast(msg, node, ppn, umod, lmod, seg)
                                if tmp < tmp_min:
                                    tmp_min = tmp
                                    tmp_min_args = [umod, lmod, seg]
                    print(str(node) + " nodes " + str(ppn) + " PPN " + str(msg) + "B message" + " -> " + str(tmp_min_args))

print_strategies()