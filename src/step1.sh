#!/bin/bash
rm ./task1/* > /dev/null 2<&1
rm ./src/csim  > /dev/null 2<&1
cd ./src  > /dev/null 2<&1
gcc -g -Wall -Werror -std=c99 -m64 -o csim csim.c cachelab.c -lm  >/dev/null 2>&1
gcc -g -Wall -Werror -std=c99 -m64 -o csim-ref csim-ref.c cachelab.c -lm >/dev/null 2>&1
gcc -g -Wall -Werror -std=c99 -m64 -o test-csim test-csim.c >/dev/null 2>&1
dir
python driver.py > info 2>&1
cat info | grep "程序没有改进" >result 2>&1
if [ $? -eq 0 ]; then
    echo "程序通过"
else
    cat info
fi
cd ..  >/dev/null 2>&1
cat ./src/info > ./task1/step1.txt  >/dev/null 2>&1