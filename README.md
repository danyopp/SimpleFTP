Server.py and FTclient.c user instructions:

1. Create a client directory and place client.c into the directory 
   Ensure client.c is compiled by entering the client directoryvia the command line and running the line of code:

	gcc client.c -o ftclient

2. You may now navigate to a directory that contains the ftserver.py

3. Any files you would like the server to broadcast must be in the same directory as ftserver.py

4. Run the server by using the following line including a portnumber between 100 and 65535

	python ftserver.py 14556

5. In a different window, you may navigate to the ftclient executable and run in any of the following formats
(Note: 14556 is server’s port and 17884 is client’s receiving port)

	./ftclient flip2 14556 -l 17884	
	./ftclient localhost 14556 -g filename.txt  17885

	-i = lists the files available to transfer
	-g = gets the named file (does not currently support wildcards)

6. Be sure to change the receiving ports (i.e. 17884/17885) with each request as the OS may not have released the port in time for the second request.

