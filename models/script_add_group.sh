#!/bin/bash

for file in $(ls *.h);
do
 if [[ $(grep 'CommonSynapseProperties' $file) ]]
 then
   echo "Working on $file"
   sed -i -e '/@BeginDocumentaion/a @ingroup Synapses\n' $file
 fi
done
