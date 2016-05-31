#!/bin/python

import sys
f = open(sys.argv[1])
lines = f.readlines()
f.close()

before_blank = 1
curr_set = []	#link with (small,large)
count_sentence = 0
count_token = 0
cross_sentence = 0

''' poorly implemented, but just for quick coding '''
for l in lines:
	fields = l.split()
	if(len(fields)==0):	#end of a sentence --- make sure the first line of the file is not empty
		before_blank = 1
		count_sentence += 1
		#deal with it
		tmp_flag = True
		for a1 in curr_set:
			for a2 in curr_set:
				if(tmp_flag and a1[1]>a2[0] and a1[1]<a2[1] and a1[0]<a2[0]):
					cross_sentence += 1
					tmp_flag = False
		curr_set = []
	else:
		before_blank = 0
		count_token += 1
		curr_set.append(sorted([int(fields[0]),int(fields[8])]))

print("All sentence %s non-proj %s token %s" % (count_sentence,cross_sentence,count_token))