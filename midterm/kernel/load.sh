#!/bin/sh
module="Key-Generator"
device="keygen"

#Question 32: What is the following line doing?
#answer: insert kernel module Key-Generator.ko to kernel
/sbin/insmod ./$module.ko

rm -f /dev/${device}

#Question 33: From what file is the major number being extracted from in the following line?
#answer: /proc/devices
major=`awk "\\$2==\"$module\" {print \\$1}" /proc/devices`

#Question 34: What is the following line doing?
#answer: create /dev/keygen as character device with major number $major and minor number 0
mknod /dev/${device} c $major 0

# permit the file to be accessible from user space
chmod 666 /dev/${device}
