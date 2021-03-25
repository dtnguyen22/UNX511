#!/bin/sh
module="Key-Generator"
device="keygen"

#Question 35: What is the following line doing?
#answer: unload/remove kernel module Key-Generator.ko
/sbin/rmmod ./$module.ko

# remove stale node
rm -f /dev/${device}
