#!/bin/bash
echo "CPU Model:"
lscpu | grep "Model name" | sed 's/Model name: *//'

echo "CPU Base Frequency:"
lscpu | grep "CPU MHz" | head -1 | sed 's/CPU MHz: *//' | awk '{printf "%.1f GHz\n", $1/1000}'

echo "Cores and Threads:"
CORES=$(lscpu | grep "^Core(s) per socket:" | awk '{print $4}')
THREADS=$(lscpu | grep "^Thread(s) per core:" | awk '{print $4}')
SOCKETS=$(lscpu | grep "^Socket(s):" | awk '{print $2}')
echo "$((CORES * SOCKETS)) cores, $((THREADS * CORES * SOCKETS)) threads"

echo "RAM:"
free -h | grep "^Mem:" | awk '{print $2}'

echo "Cache:"
lscpu | grep -E "L1d|L1i|L2|L3"

echo -e "\nOperating System:"
lsb_release -ds 2>/dev/null || cat /etc/issue | head -1

echo "Kernel:"
uname -r

echo "Compiler:"
g++ --version | head -1
