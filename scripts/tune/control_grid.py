#control center for the tuning process
#tuning the parameters using grid in random order
from threading import Thread,Lock
import os
import time
import random

#files 
file_template="ttemplate.txt"	#tuning template
file_range="trange.txt"		#the range of tuning
file_process="tprocess.txt"	#record the process

 
#center info
ct_num_all = 1
ct_num_done = 0

file_maxfile="tmax.txt"
ct_running_max = 4
ct_running_now = 0
ct_lock = Lock()

#center tables
ct_sorted_tasks=[]
ct_tasks_table=dict()	#frozenset->one_task
ct_specific_task=None

#just collections of functions
class specific_task:
    def execute(self,iden):
        pass
    def high_order(self,s1,s2):
        pass
 
class one_task(Thread):
    def __init__(self,iden):
        Thread.__init__(self)
        self.iden = iden	#frozenset of configures
        self.result = [ -1 for i in range(20) ] 	#if result > 0, then it is done
    def set_result(self,s):
        self.result = s
    def __lt__(self,v):
	global ct_specific_task
        return ct_specific_task.high_order(self.result,v.result)   #high first
    #def __str___(self):
    #    return "%s-%s" % (self.iden,self.result)
    def run(self):
        global ct_specific_task
        global ct_lock
        global ct_sorted_tasks
        global file_process
        global ct_lock
        global ct_running_now
        
        print("Running one of %s" % self.iden)
        one_result = ct_specific_task.execute(self.iden)
        self.set_result(one_result)
        #deal with the cleaning up
        ct_lock.acquire()		#####LOCK_ACQUIRE#####
        ct_running_now -= 1
        ct_sorted_tasks=sorted(ct_sorted_tasks)
        f = open(file_process,"w")
        for x in ct_sorted_tasks:
            if(x.result[0] < 0):
                break;
            for i in sorted(x.iden):
                f.write("%s " % str(i))
            f.write(" -- ")
            for i in x.result:
                f.write("%s " % str(i))
            f.write("\n")
        f.close()
        ct_lock.release()		#####LOCK_RELEASE#####
    
def ct_init():
    global file_template
    global file_range
    global file_process
    global ct_num_all
    global ct_num_done
    global ct_specific_task
    global file_maxfile
    global ct_sorted_tasks
    global ct_tasks_table
    global ct_running_max
    global ct_running_now
    global ct_lock
    
    confs = dict()
    num = 1
    #data init
    f = open(file_range,"r")
    for line in f:
        tmp_l = line.split()
        if(len(tmp_l) >= 2):
            confs[tmp_l[0]] = tmp_l[1:]
            num = num * (len(tmp_l)-1)
    ct_num_all = num
    f.close()
    #init all task (special format for the identification)
    ##############recursive function#######################
    def ct_init_recuradd(l,s,n):
        try:
            conf_item = l[n]
            conf_list = confs[conf_item]
            for conf_one in conf_list:
                str_add = "%s:%s" % (conf_item,conf_one)
                s.add(str_add)
                ct_init_recuradd(l,s,n+1)
                s.remove(str_add)
        except:
            #at the end
            iden = frozenset(s)
            one = one_task(iden)
            ct_sorted_tasks.append(one)
            ct_tasks_table[iden] = one
    ##############recursive function#######################
    tmp_s = set()
    ct_init_recuradd(list(confs.keys()),tmp_s,0)
    #read process file
    try:
        fp = open(file_process,"r")
        for line in fp:
	    two_part = line.split("--")
            tmp_l = two_part[0].split()
            tmp_s = two_part[1].split()
            if(len(tmp_l) > 0):
                tmp_iden = frozenset(tmp_l)
                tmp_score = [float(i) for i in tmp_s]
                ct_tasks_table[tmp_iden].set_result(tmp_score)
                ct_num_done += 1
        fp.close()
    except:
        pass
    #ok, init finish
    print("Init ok, all %d, done %d." % (ct_num_all,ct_num_done))
    
def ct_main(ttt):
    global file_template
    global file_range
    global file_process
    global ct_num_all
    global ct_num_done
    global ct_specific_task
    global file_maxfile
    global ct_sorted_tasks
    global ct_tasks_table
    global ct_running_max
    global ct_running_now
    global ct_lock
    
    ct_specific_task=ttt
    os.system("touch %s"%file_maxfile)
    keys_l = list(ct_tasks_table.keys())
    random.shuffle(keys_l)
    for one in keys_l:
        if(ct_tasks_table[one].result[0] > 0):
            continue	#done
        #can we add one task
        stay_here = True
        while(stay_here):
            #check max-file
            mf = open(file_maxfile,"r")
            try:
                cur_max = int(mf.read().split()[0])
                if(cur_max > 0):
                    ct_running_max = cur_max
            except:
                pass
            mf.close()
            #check current running num
            ct_lock.acquire()		#####LOCK_ACQUIRE#####
            if(ct_running_now < ct_running_max):
                ct_running_now += 1
                ct_tasks_table[one].start()
                ct_lock.release()		#####LOCK_RELEASE#####
                stay_here = False
                continue
            else:
                ct_lock.release()		#####LOCK_RELEASE#####
                time.sleep(10)
                continue
    while(1):
        ct_lock.acquire()		#####LOCK_ACQUIRE#####
        if(ct_running_now==0):
            ct_lock.release()		#####LOCK_RELEASE#####
            break
        ct_lock.release()		#####LOCK_RELEASE#####
        time.sleep(10)
    print("All search done.")
