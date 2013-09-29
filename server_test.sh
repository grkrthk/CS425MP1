#!/bin/bash

#
#
# Server UT
#
#

#
#
# USAGE: ./server_test -p <port_no> -i <ip_address>
#
#

if [ $# -ne 4 ]
then
    echo -e "\nINVALID number of arguments\n"
    echo -e "\nUSAGE: $0 -i <ip_address>\n "
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

#
# test case 1
#
echo -e "\nCalling server with invalid arguments\n"
echo -e "\nExpected behaviour: Server fails and displays usage information\n"

# calling server with invalid arguments
./serverDLQ_start.sh
if [ $? -ne 0 ]
then
    echo -e "\nTest case passed\n"
fi

#
# test case 2
# 
echo -e "\nBinding server with invalid IP\n"
echo -e "\nExpected behaviour: Server fails with bind error\n"

# calling server with invalid ip
./serverDLQ_start.sh -p ${PORT_NO} -i 0.0.0.0
if [ $? -ne 0 ]
then
    echo -e "\nTest case passed\n"
fi

#
# test case 3
#
echo -e "\nCalling server with CORRECT parameters\n"
echo -e "\nExpected Behaviour: Server starts SUCCESSFULLY\n"

# calling server with correct parameters
./serverDLQ_start.sh -p ${PORT_NO} -i ${IP_ADDRESS}

#
# End of server UT
#
