#!/bin/bash



#PATH=/Users/Administrador/Desktop/dut_october/method1_m100_testbeam/files/*


#for file in $PATH
#do
#echo "procesing ${file%.*}.root"
#   ./tbConverter -c convertrcetoroot -f configs/RCE.cfg -i $file -o ${file%.*}.root
#done


#IN=/Volumes/Elements_1/SPS_october/cosmicData/2014-10-30/cosmic_000003/cosmic_000003_000000.dat
#OUT=/Users/Administrador/Documents/MAPS/XTB01/testbeams_data/SPSoctober/cosmic_data/lasttbconverter/2014-10-30_run3
#IN=../tbconverter-Filter3Tcell/testbeamTJ/cosmic_000785/cosmic_000785_000000.dat
#OUT=../tbconverter-Filter3Tcell/testbeamTJ/cosmic_000785/cosmic_000785_000000
#IN=/Users/schae/testarea/Judith_CERN/TowerJazz/cosmic_000804_000000.dat
#OUT=/Users/schae/testarea/Judith_CERN/TowerJazz/cosmic_000804_000000
#IN=/Users/schae/testarea/Judith_CERN/TowerJazz/cosmic_000925_000000.dat
#OUT=/Users/schae/testarea/Judith_CERN/TowerJazz/cosmic_000925_000000
#IN=/Users/schae/testarea/Judith_CERN/TowerJazz/cosmic_000946_000000.dat 
#OUT=/Users/schae/testarea/Judith_CERN/TowerJazz/cosmic_000946_000000
IN=/Users/schae/testarea/Judith_CERN/TowerJazz/cosmic_033037_000000.dat
OUT=/Users/schae/testarea/Judith_CERN/TowerJazz/cosmic_033037_000000
IN=/Volumes/TestBeam/$1.dat
OUT=/Volumes/TestBeam/$1
#IN=/tmp/$1.dat
#OUT=/tmp/$1
#IN=/Volumes/CORSAIR/TestBeam/cosmic_033017_000000.dat
#OUT=/Volumes/CORSAIR/TestBeam/cosmic_033017_000000
#IN=/Users/schae/testarea/Judith_CERN/TestData/cosmic_032873/cosmic_032873_000000.dat 
#OUT=/Users/schae/testarea/Judith_CERN/TestData/cosmic_032873/cosmic_032873_000000
#IN=../tbconverter-Filter3Tcell/testbeamTJ/cosmic_000509_000000.dat
#OUT=../tbconverter-Filter3Tcell/testbeamTJ/cosmic_000509_000000

#./tbConverter -c convertrcetoroot -f configs/RCE.cfg -i $IN -o ${OUT%.*}.root
#./tbConverter -c convertrcetoroot -f configs/RCE.cfg -i $IN -o ${OUT}.root
#./tbConverter -c convertrcetoroot -f configs/RCE_6mod.cfg -i $IN -o ${OUT}.root
./tbConverter -c convertrcetoroot -f configs/RCE_5mod_HSIO2.cfg -i $IN -o ${OUT}.root
#./tbConverter -c convertrcetoroot -f configs/RCE_r5mods.cfg -i $IN -o ${OUT}.root
