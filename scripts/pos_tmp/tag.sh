#!/bin/bash

#$1,$2 input,output

origin_f=$1
plain_f="$origin_f.plain"
tag_f="$origin_f.tag"
final_f=$2

#1.first dp to plain
echo "awk $origin_f > $plain_f"
awk '
/^$/{printf("\n");}
NF==10{printf("%s ",$2);}
' $origin_f > $plain_f

#2.tag
echo "java $plain_f > $tag_f"
java -mx300m -classpath stanford-corenlp-3.5.0.jar edu.stanford.nlp.tagger.maxent.MaxentTagger -model wsj-0-18-bidirectional-distsim.tagger  -tokenize false -textFile $plain_f > $tag_f

#2.5 need some minor changes --- in_place
#well, some strange character
##dev
#./exclude_strange < $tag_f > "$tag_f.tmp"
#mv "$tag_f.tmp" $tag_f
#sed -i 's/\([0-9][0-9]*\) /\1_CD /g' $tag_f
#sed -i 's/low_JJ -_:/low-_JJ/' $tag_f
#sed -i 's/47_CD %_NN -_: controlled_VBN/47%-controlled_VBN/' $tag_f
#sed -i 's/19-to_CD -_: $_$ 21_CD/19-to-$21_JJ/' $tag_f


#3. bind tag
echo "./tag_bind $origin_f $tag_f > $final_f"
./tag_bind $origin_f $tag_f > $final_f
