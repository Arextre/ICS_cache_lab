#!/bin/bash
rm ./task1/* > /dev/null 2<&1
rm ./src/csim  > /dev/null 2<&1
rm ./src/test-csim > /dev/null 2<&1
cd ./src  > /dev/null 2<&1
gcc -g -Wall -Werror -std=c99 -m64 -o csim csim.c cachelab.c -lm  >/dev/null 2>&1
gcc -g -Wall -Werror -std=c99 -m64 -o test-csim test-csim.c >/dev/null 2>&1
python driver.py > info 2>&1
cat info | grep "程序无效" >result 2>&1
if [ $? -eq 0 ]; then
    cat info
else
#    echo "程序通过"
    ./csim -s 1 -E 1 -b 1 -t traces/yi2.trace
    ./csim -s 4 -E 2 -b 4 -t traces/yi.trace
    ./csim -s 2 -E 1 -b 4 -t traces/dave.trace
    ./csim -s 2 -E 1 -b 3 -t traces/trans.trace
    ./csim -s 2 -E 2 -b 3 -t traces/trans.trace
    ./csim -s 2 -E 4 -b 3 -t traces/trans.trace
    ./csim -s 5 -E 1 -b 5 -t traces/trans.trace
    ./csim -s 5 -E 1 -b 5 -t traces/long.trace
fi
cat info > ../task1/step1.txt   2>&1