/******************************************************8
 * Author: Daniel Yopp
 * Date: 8/8/19
 * Program Name: ftclient
 * Description: client that can request and recieve file transfers from ftserver
 * ****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

//Function Prototypes
void checkArgs(int argc, char *argv[]);
void sendStartData(int socketFD, char *argv[]);
void recFileList(int portNumber);
void recFile(int portNumber, char* fileName); 

int main(int argc, char *argv[])
{
	//Check command line arguments
	checkArgs(argc, argv); 

	//Set up server connection
	int socketFD, portNumber; 
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //clear server address struct
	portNumber = atoi(argv[2]); //get port number for server from command line
	serverAddress.sin_family = AF_INET; //TCP
	serverAddress.sin_port = htons(portNumber); //Put port number in address struct
	serverHostInfo = gethostbyname(argv[1]); //Find out the host IP
	if (serverHostInfo == NULL){fprintf(stderr, "ERROR: no such server host!\n"); exit(6);}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

	//create socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0){fprintf(stderr, "ERROR: failed to open socket\n"); exit(7);}	

	//connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{fprintf(stderr, "ERROR: failed to connect!\n"); exit(8);};

	//send start data: request type, client port, filename
	sendStartData( socketFD, argv);

	//Determine what type of request the client wanted
	if(strcmp(argv[3],"-l")==0) //dir list requested
	{
		recFileList(atoi(argv[4]));
	}
	else  //file requested
	{
		recFile(atoi(argv[5]), argv[4]);
	}

	return 0;
}

//////////////////////////////////////////////
// Name: Check Arguments
// Description: Validation of command line arguments
// Parameters: command line arguments
/////////////////////////////////////////////
void checkArgs(int argc, char *argv[])
{
	//Check for correct number of commandline args
	if(argc < 5 || argc > 6)
	{
		fprintf(stderr, "ERROR: Incorrect Command Line Arguments!\n");
		fprintf(stderr, "Entry Format: ./ftclient [SERVERADDR] [SERVERPORT] [-l or -g] [if-g FileName] [CLIENTRCVPORT]\n");
		exit(1);
	}
	
	//Check that server port is within range
	if(atoi(argv[2]) < 100 || atoi(argv[2]) > 65535)
	{
		printf("Arg 3: %s\n",argv[3]);
		fprintf(stderr, "ERROR: Invalid Server Port: %d\n", atoi(argv[2])); 
		fprintf(stderr, "Port number must be between 100 and 65535\n");
		exit(2);
	} 

	//Check that flag is either -l or -g 
	if(strcmp(argv[3],"-l")!=0 && strcmp(argv[3],"-g")!=0)
	{
		printf("|%s|\n", argv[3]);
		fprintf(stderr, "ERROR: Invalid flag: %s\n", argv[3]);
		exit(3);
	}

	//check -l parameters 
	if(strcmp(argv[3],"-l")==0) 
	{
		//check client return port number for -l request
		if(atoi(argv[4]) < 100 || atoi(argv[4]) > 65535)
		{
			fprintf(stderr, "ERROR: invalid client port: %d\n", atoi(argv[4]));
			fprintf(stderr, "Port number must be between 100 and 65535\n");
			exit(4);
		}
	}
	//check -g parameters 
	if(strcmp(argv[3], "-g") == 0)
	{
		//check client return port number of -g request
		if(atoi(argv[5]) < 100 || atoi(argv[5]) > 65535)
		{
			fprintf(stderr, "ERROR: invalid client port: %d\n", atoi(argv[5]));
			fprintf(stderr, "Port number must be between 100 and 65535\n");
			exit(4);
	 	}
		//check correct number of arguments for a -g request	
		if(argc != 6)
		{
			fprintf(stderr, "ERROR: not enough arguments for -g flag\n");
			exit(5);
		}
	}

}


/////////////////////////////////////////////////////////////
// Name: Send Start Data
// Description: sends start request information to server 
// Parameters: control connection socket, command line arguments
/////////////////////////////////////////////////////////////
void sendStartData(int socketFD, char *argv[])
{
	int charsWritten;

	//send client connection type(-l or -g) wrapped with '$'
	char typeFile[5];
	memset (typeFile, '\0', 5);
	strcat(typeFile, "$");
	strcat(typeFile, argv[3]);
	strcat(typeFile, "$");
	charsWritten = send(socketFD, typeFile, strlen(typeFile),0);
	if (charsWritten < 0) {fprintf(stderr, "ERROR: failed to send connection details\n"); exit(9);};
	if (charsWritten < strlen(typeFile)) {printf("WARNING: connection data fragmented!\n"); exit(10);};

	//get correct parameter argument depending on if its a list or get requests.  
	int argNum = 5;
	if(strcmp(argv[3],"-l")==0)
		{argNum = 4;}
	//Send recieving port number wrapped with '@'
	char recievePort[12];
	memset(recievePort, '\0', 12);
	strcat(recievePort, "@");
	strcat(recievePort, argv[argNum]);
   	strcat(recievePort, "@");
	charsWritten = send(socketFD, recievePort, strlen(recievePort),0);
	if (charsWritten < 0) {fprintf(stderr, "ERROR: failed to send connection details\n"); exit(9);}
	if (charsWritten < strlen(recievePort)) {printf("WARNING: connection data fragmented!\n"); exit(10);}

	//Send file name or placeholder wrapped in #%FILENAME%###
	if(strcmp(argv[3],"-g")==0)
	{
		char fileName[513];
		memset(fileName, '\0', 513);
		strcat(fileName, "#%");
		strcat(fileName, argv[4]);
		strcat(fileName, "%###");
		charsWritten = send(socketFD, fileName, strlen(fileName),0);
		if (charsWritten < 0) {fprintf(stderr, "ERROR: failed to send connection details\n"); exit(9);}
		if (charsWritten < strlen(fileName)) {printf("WARNING: connection data fragmented!\n"); exit(10);}
	}
	else //Because -l doesn't need file name send "###%###" as place holder to server
	{
		charsWritten = send(socketFD, "###%###", 7, 0);
		if (charsWritten < 0) {fprintf(stderr, "ERROR: failed to send connection details\n"); exit(9);}
		if (charsWritten < 7) {printf("WARNING: connection data fragmented!\n"); exit(10);}
	}
	printf("Request made to server!\n");
}


///////////////////////////////////////////////////////////////
// Name: Receive File List
// Description: Recieves a list of files avalible to download from server
// Parameters: port number client will listen for data on
///////////////////////////////////////////////////////////////
void recFileList(int portNumber)
{
	//Set up connection to recieve directory list
	int listenSocketFD, establishedConnectionFD, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr.s_addr = INADDR_ANY;	
	//Socket setup
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0)
		 {fprintf(stderr, "ERROR: Could not open receiving socket\n"); exit(11);}
	//Listen on socket for server data
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		 {fprintf (stderr, "ERROR: Could not bind receiving socket\n"); exit(12);}
	listen(listenSocketFD, 1);
	sizeOfClientInfo = sizeof(clientAddress);
	//accept connection from server
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
	if (establishedConnectionFD < 0)
		 {fprintf(stderr, "ERROR: Recieving connection rejected\n"); exit(13);}
	else
	{
		//Print connection info	
		char incomingIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientAddress.sin_addr), incomingIP, INET_ADDRSTRLEN);
		printf("Server Data Connection Established from - %s \nOn local Port: %d\n", incomingIP , portNumber );
		printf("Receiving File List!\n");

		//create a receiving buffer for socket
		char recBuffer[256];
		int endingChar= 1; //used to flag end of msg
		int rolloverEnding = 0;

		while(endingChar) //while ending flag has not been detected
		{
			memset(recBuffer,'\0', 256); //clear buffer
			charsRead = recv(establishedConnectionFD, recBuffer, 255, 0); //receive from socket
				if (charsRead < 0){fprintf(stderr,"ERROR: Failure reading from socket"); exit(14);}			
	
			//scan for ending flag "@@@"
			int j = 0;
				//Test if last transmission was a rollover to see if current part is end of tail	
			if (rolloverEnding == 1 && recBuffer[0] == '@' )
				//this is an actual rollover ending where half the tail was recieved at the end of the transmission
				//and the rest at the beggining of the next		
				{endingChar = 0;}
			else if (rolloverEnding == 1)
				//Just an @ symbol in data, not a rollover
				{printf("@");} //have to reprint symbol because it was not ending tag 
	
			//test if current transmission is start of roll over
			if (recBuffer[254] =='@')
				{rolloverEnding = 1;
				 recBuffer[254] = '\0';	
				}

			//Test for normal cases of tail occuring in middle of buffer
			for (j=0; j < 255; j++)
			{ 
				if (recBuffer[j] == '@')
				{   if (j<=253 && recBuffer[j+1] == '@')
				     { endingChar = 0;
					recBuffer[j] = '\0';
					break;		
			 	     }		
				    		

				}
			}
			
			printf("%s", recBuffer); 
			
		}
		printf("\nTransfer Complete!\n");
		close(establishedConnectionFD);
	}




}

////////////////////////////////////////////////////////////////////////
// Name: Receive Actual File
// Description: Recieves file data from server and stores in file
// Parameters: port number for client to listen for daa on, filename of requested file
////////////////////////////////////////////////////////////////////////
void recFile(int portNumber ,char* fileName)
{
	//create file to put incoming data in
	FILE *recFile;
	recFile = fopen(fileName, "w");

	//open data connection with server
	int listenSocketFD, establishedConnectionFD, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	//struct address set up - listening port number
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	serverAddress.sin_addr.s_addr = INADDR_ANY;	
	//socket setup
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0)
		 {fprintf(stderr, "ERROR: Could not open receiving socket\n"); exit(11);}
	//Listen on socket
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		 {fprintf (stderr, "ERROR: Could not bind receiving socket\n"); exit(12);}
	listen(listenSocketFD, 1);
	sizeOfClientInfo = sizeof(clientAddress);
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
	if (establishedConnectionFD < 0)
		 {fprintf(stderr, "ERROR: Recieving connection rejected\n"); exit(13);}
	else
	{
		//print connection information
		char incomingIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientAddress.sin_addr), incomingIP, INET_ADDRSTRLEN);
		printf("Server Data Connection Established from - %s  \nOn local Port: %d\n", incomingIP , portNumber );
		printf("Receiving Data!\n");
		char recBuffer[256];
		int endingChar= 1; //used to flag end of msg
		int rolloverEnding = 0;

		while(endingChar)
		{
			memset(recBuffer,'\0', 256);
			charsRead = recv(establishedConnectionFD, recBuffer, 255, 0);
			if (charsRead < 0){fprintf(stderr,"ERROR: Failure reading from socket"); exit(14);}			
			//scan for ending flag "@@@"
			int j = 0;
	
			//Test if last transmission was a rollover to see if current part is end of tail	
			if (rolloverEnding == 1 && recBuffer[0] == '@' )
				//this is an actual roll over ending where half the tail was recieved at the end of the transmission
				//and the rest at the beggining of the next		
				{endingChar = 0;}
			else if (rolloverEnding == 1)
				//Just an @ symbol, not a rollover
				{printf("@");} //have to reprint symbol because it was not ending tag 
	
			//test if current transmission is start of roll over
			if (recBuffer[254] =='@')
				{rolloverEnding = 1;
				 recBuffer[254] = '\0';	
				}

			//Test for normal cases of tail occuring in middle of buffer
			for (j=0; j < 255; j++)
			{ 
				if (recBuffer[j] == '@')
				{   if (j<=253 && recBuffer[j+1] == '@')
				     { endingChar = 0;
					recBuffer[j] = '\0';
					break;		
			 	     }		
				    		

				}
			}
			
			//save to file
			fprintf(recFile, "%s", recBuffer);
			
			
		}
		printf("\n");
		fclose(recFile);
		close(establishedConnectionFD);

		//check file to ensure error message wasnt sent
		char *firstLine = NULL;
		size_t inputSize = 0;
		memset(&firstLine, '\0', sizeof(firstLine));
		FILE *checkFile;
		checkFile = fopen(fileName, "r");
		getline(&firstLine, &inputSize, checkFile);

		//if either error, delete file and print error message
		if(strcmp(firstLine, "ERROR: Failure to access file on server!")==0)
		{
			printf("ERROR: Failure to access file on server!\n");
			fclose(checkFile);
			remove(fileName);
		}
		else if(strcmp(firstLine, "ERROR: No such file in directory!") == 0)
		{
			printf("ERROR: No such file in directory!\n");
			fclose(checkFile);
			remove(fileName);
		}	
		else
		{
			fclose(checkFile);
			printf("File transfer complete!\n");
		}

		free(firstLine); // free getline malloc

	}//else - connection established, data recieved 

}	


	
