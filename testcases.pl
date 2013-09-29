#/usr/bin/perl

my $ip =$ARGV[0];
print "\n---------------------$ip--------\n";
my $port =$ARGV[1];
print "\n--------------$port-------------\n";

## FIRST TEST CASE : INPUT A WRONG IP AND PORT 0

print "\n-----------------------------------------------------------------------------\n";
print "\nFIRST TEST CASE - INPUT A WRONG IP AND PORT\n";
system("./client 0.0.0.0 0");
print "\nDisplays appropriate error message : connect will fail\n";
print "\n-----------------------------------------------------------------------------\n";


## SECOND TEST CASE : ENTER WRONG INPUT 

print "\n-----------------------------------------------------------------------------\n";
print "\nSECOND TEST CASE\n";
print "\nOn display of options press something beyond 0-11\n";
system("./client $ip $port");
print "\nOn doing the above, the program should display appropriate message and EXIT\n";
print "\n-----------------------------------------------------------------------------\n";

## THIRD TEST CASE : GREP FAILS 

print "\n-----------------------------------------------------------------------------\n";
print "\nTHIRD TEST CASE\n";
print "\npass non existant pattern to get unscuccesful results from grep command, use option 5\n";
system("./client $ip $port");
print "\nSelect option 5 and enter random pattern : <gooli2>\n";
print "\nProgram displays appropriate error message and exits\n";
print "\n-----------------------------------------------------------------------------\n";

## FOURTH TEST CASE : LISTEN TO THE WRONG SERVER PORT

print "\n------------------------------------------------------------------------------\n";
print "\nFOURTH TEST CASE\n";
print "\nStart the server and make it listen to the wrong port\n";
system("./client $ip 100");
print "\nProgram displays error message and exits\n";
print "\n-------------------------------------------------------------------------------\n";

## FIFTH TEST CASE : PASS NO ARGUMENTS TO THE BINARY FILE OF THE CLIENT 

print "\n--------------------------------------------------------------------------------\n";
print "\nFIFTH TEST CASE\n";
print "\nStart the client and pass no arguments\n";
system("./client");
print "\nDisplays an error message and exits\n";
print "\n---------------------------------------------------------------------------------\n";

## SIXTH TEST CASE : GIVE THE FIRST ARGUMENT AS THE PORT FOLLOWED BY IP 

print "\n---------------------------------------------------------------------------------\n";
print "\nSIXTH TEST CASE\n";
print "\nInterchange the IP and port no\n";
system("./client $port $ip");
print "\nDisplay error message and exit\n";
print "\n---------------------------------------------------------------------------------\n";

## SEVENTH TEST CASE : GIVE ODD  NUMBER OF ARGUMENTS WITHOUT THE PORT FOR THE LAST IP

print "\n----------------------------------------------------------------------------------\n";
print "\nSEVENTH TEST CASE\n";
print "\nPASSING ODD NUMBER OF ARGUMENTS\n";
system("./client 192.168.23.12 1234 123.56.32.43");
print "Display appropriate error message and exit\n";
print "-----------------------------------------------------------------------------------\n";

# EIGHT TEST CASE : RUN THE SERVER AND THE CLIENT ON THE SAME MACHINE AND COMPARE THE GREP OUTPUT FILE SIZE
#                   AND RESULT FILE

print "------------------------------------------------------------------------------------\n";
print "\nEIGHTH TEST CASE\n";
print "\nRUN THE SERVER ON THE LOCAL MACHINE TO VERIFY GREP RESULTS\n";
print "\nServer creates a local result file\n";
system("./client $ip $port");
print "\nuse option 5\n";
print "\nCOMPARE MACHINE_LOG AS RECIEVED BY CLIENT AND RESULT FILE FROM SERVER\n";
system("ls -l ~/");
system("ls -l");

# NINTH TEST CASE : SEARCH FOR THE NON EXISTANT KEY IN THE LOG-FILE

print "-------------------------------------------------------------------------------------\n";
print "NINTH TEST CASE\n";
print "go to option 10 and provide an non-existant key\n";
system("./client $ip $port");
print ("Enter non-existant key\n");
print ("GREP FAILS AND PROVIDES AN ERROR MESSAGE\n");

#TENTH TEST CASE : SEARCH FOR THE NON EXISTANT VALUE IN THE LOG-FILE

print "-------------------------------------------------------------------------------------\n";
print "NINTH TEST CASE\n";
print "go to option 11 and provide an non-existant value\n";
system("./client $ip $port");
print ("Enter non-existant VALUE\n");
print ("GREP FAILS AND PROVIDES AN ERROR MESSAGE\n");


