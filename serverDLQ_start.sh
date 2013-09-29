#!/bin/bash

####################################################################
# This is a start up script for DLQ Multithreaded Concurrent Server
####################################################################

#
#
# USAGE: ./serverDLQ_start -p <port_no> -i <ip_address_of_the_current_server>
#
#

if [ $# -ne 4 ]
then 
    echo -e "INVALID number of arguments"
    echo -e "USAGE: $0 -p <port_no> -i <ip_address_of_the_current_server>"
    exit 1
fi

while getopts p:i: opt
do
    case $opt in
        p)
        # PORT NO
        PORT_NO=$OPTARG;;
        i)
        # IP ADDRESS
        IP_ADDRESS=$OPTARG;;
     esac
done

gcc serverDLQ.c -lpthread -o serverDLQ

./serverDLQ ${PORT_NO} ${IP_ADDRESS}

if [ $? -ne 0 ]
then
    exit
fi

# END OF SCRIPT
    

