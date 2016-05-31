#!/bin/python
#calculating the f1 scores on dependency length

## usage: python length_f1.py <gold-file> <predict-file>

import sys

#1. readings
fgold = sys.argv[1]
ftest = sys.argv[2]

lgold = open(fgold).readlines()
ltest = open(ftest).readlines()

def Error(x):
    print(x)
    exit(1)

# if(len(lgold) != len(ltest)):
#     Error("not equal length all.")
    
#2.processing
MAX_LEN = 150
l_gold = [0.] * MAX_LEN
l_predict_right = [0.] * MAX_LEN
l_predict_wrong = [0.] * MAX_LEN
for g,t in zip(lgold,ltest):
    lg = g.split()
    lt = t.split()
    if(lt and lg):
        lg_1,lt_1 = int(lg[0]),int(lt[0])
        lg_h,lt_h = int(lg[8]),int(lt[8])
        if(lg_1 != lt_1):
            Error("not equal index %d and %d." % (lg_1,lt_1))
        d_gold = abs(lg_1-lg_h)
        d_test = abs(lt_1-lt_h)
        l_gold[d_gold] += 1
        if(d_gold == d_test):
            l_predict_right[d_test] += 1
        else:
            l_predict_wrong[d_test] += 1

#3.after ...
for i in range(MAX_LEN):
    pr = re = f1 = 0
    if(l_predict_right[i]+l_predict_wrong[i]):
        pr = l_predict_right[i] / (l_predict_right[i]+l_predict_wrong[i])
    else:
        pr = 0
    if(l_gold[i]):
        re = l_predict_right[i] / l_gold[i]
    else:
        re = 0
    if(pr and re):
        f1 = (2*pr*re)/(pr+re)
    print("%d\t%f" % (i,f1))
    # print("%d\t(%d,%d,%d)\t%f\t%f\t%f" % (i,l_gold[i],l_predict_right[i],l_predict_wrong[i],pr,re,f1))
    # print("%d\t%f\t%f\t%f" % (i,pr,re,f1))



