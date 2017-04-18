#!/bin/bash



#IN=/Volumes/WD_Elements/SPS_november/dut_data/tbc/*
#IN=/tmp/cosmic_000115/*
#IN=../tbconverter-Filter3Tcell/testbeamTJ/cosmic_000801/*
#IN=../tbconverter-Filter3Tcell/testbeamTJ/cosmic_000785/*
IN=overnight
IN=55fe
#OUT=/Users/Administrador/Documents/MAPS/XTB01/testbeams_data/SPSnovember/osc_converted/


for file in $IN
do


echo "procesing ${file%.*}"
#./tbConverter -c convertbintoroot -f configs/RCE_r5mods.cfg  -i $file -o ${file%.*}root
#./tbConverter -c convertbintoroot -f configs/RCE_6mod.cfg  -i $file -o ${file%.*}.root
./tbConverter -c convertlecroy3Tcelltoroot -f configs/diamondAmplCut.cfg  -i $file -o ${file%.*}.root #-n 100
#./tbConverter -c convertbintoroot -f configs/diamondAmplCut.cfg  -i $file -o ${file%.*}.root
done


#IN=/Volumes/WD_Elements/XFAB/all_november/2014-11-24_RUN000022
#OUT=/Volumes/WD_Elements/XFAB/all_november/2014-11-24_RUN000022c

#./tbConverter -c convertbintotext -f configs/diamond.cfg -i $IN -o $OUT
