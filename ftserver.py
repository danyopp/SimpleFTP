#!/usr/bin/python
##########################################################
#Author: Daniel Yopp
#Date: 8/8/19
#Assignment: CS372 - Project 2
#Description: FTP server program
##########################################################

import socket
import sys
import time
import os


#########################################################
## Name: Check Parameter Input
## Description: Checks command line parameters to make sure they are correct
#########################################################
def paramCheck():
	if len(sys.argv) != 2: #wrong number of args
		print "ERROR: Invalid parameter input (./ftserver [PORTNUMBER])"	
		exit(1)
	elif (int(sys.argv[1]) > 65535) or (int(sys.argv[1]) < 100): #second arg is not a port number
		print "Port entered: ", sys.argv[1]
		print "Error: Select port between 100 and 65535"  
		exit(2)

########################################################
## Name: Connection Type Check
## Description: Checks to see what data connecting client needs(-l or -g)
## Returns (-l/-g) Portnumber and filename in list 	
########################################################
def ConnectionTypeCheck(connectionSocket):
	FinalList = [] 

	clientRequest = "" 
	clientRequest = connectionSocket.recv(512)
	
	#continue recieving until ending tag of%### comes through 
	while not (clientRequest.endswith("%###")):
		clientRequest += connectionSocket.recv(512)

	
#	print "Client Request: ", clientRequest

	#Add -l or -g to list (request type is recieved with '$' on both ends)  
	splitResults = clientRequest.split('$',3 )	
	FinalList.append(splitResults[1])

	#Add portnumber to list (portnumber is recieved with '@' on bothh ends)
	splitResults = clientRequest.split('@',3 )	
	FinalList.append(splitResults[1])

	#Add filename to list (filename is is recieved with '%' on both ends) 
	splitResults = clientRequest.split('%',3 )	
	FinalList.append(splitResults[1])

	return FinalList

########################################################
## Directory List Transmission
## Description: Sends file list in directory to clients requested port
## Parameters: ConnectionInfo: list of -l, portnumber, and unused filename
##	       addr: address of client		    
########################################################

def SendDir(connectionInfo, addr ):
	#get list of current files in directory  
	currentFiles = []
	files = [f for f in os.listdir('.') if os.path.isfile(f)]
	for f in files:
		if f[0] != '.' and f != "server.py" and f != "ftserver.py" and f != "client.c" and f != "ftclient.exe"  :
			#Add files to list that do not match the file names of the programs			
			currentFiles.append(f)
	#print currentFiles	
	currentFiles.sort()
	
	#create string from list with newline seperators
	stringResult = "\n".join(str(e) for e in currentFiles)
	#print stringResult


	#Create New Connection to clients recieving port
	clientPort = connectionInfo[1]
	clientIP = addr[0]
	socket2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server_address = (addr[0], clientPort)
	print "Connecting to :", server_address[0], "   Port: ", server_address[1]
	socket2.connect((server_address[0], int(server_address[1])))

	#Add @@@ tail to string and send to client recieving port
	stringResult+="@@@"
	socket2.sendall (stringResult)
	print "Directory Information Sent To Client"
	
#########################################################
## File Transmission
## Description: Checks for and Transmits a requested file to the client
## Parameters: ConnectionInfo: list of -g, portnumber, and filename
##	       addr: address of client		    
#########################################################

def SendFile(connectionInfo, addr):

	#get current files avaliable for transfer 
	currentFiles = []
	files = [f for f in os.listdir('.') if os.path.isfile(f)]
	for f in files:
		if f[0] != '.' and f != "server.py" and f != "ftserver.py" and f != "client.c" and f != "ftclient.exe"  :
			currentFiles.append(f)
	currentFiles.sort()

	#determine if request file matches an avalible file
	fileContents = ""
	if connectionInfo[2] in currentFiles:
		#File was found in the list of current files avalible
		print "Matching file found for requested file named: ", connectionInfo[2]
		print "Preparing to transmit file."

		#Open file handle and read file into string
		fHandle = open(connectionInfo[2], "r")
		if fHandle.mode == 'r':
			#file read successfully
			fileContents = fHandle.read()
			
		else:
			#file was unable to be opened. Send error to user
			print "Error: failed to open file!"
			print "Sending error message to client!"
			fileContents = "ERROR: Failure to access file on server!"
		fHandle.close()
	#if no file match, send error to user
	else:
		print "Error! No file match found for requested file named: ", connectionInfo[2]
		print "Sending error message to client!"  
		fileContents = "ERROR: No such file in directory!"

	#else send file 
	#Create New Connection to clients recieving port
	clientPort = connectionInfo[1]
	clientIP = addr[0]
	socket2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server_address = (addr[0], clientPort)
	print "Connecting to :", server_address[0], "   Port: ", server_address[1]
	socket2.connect((server_address[0], int(server_address[1])))

	#add @@@ tail and send to client
	fileContents+="@@@"
	socket2.sendall (fileContents)
	print "Directory Information Sent To Client"
	
	

#########################################################
## Main Funtion
##
#########################################################
if __name__ == "__main__":

	#check command line parameters
	paramCheck()
	
	#fire up server
	serverPort = int(sys.argv[1])
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	serverSocket.bind(('', serverPort))
	serverSocket.listen(1)
	print "Server established on port: ", serverPort
	
	#assist client
	while 1:
		#accept connection
		connectionSocket, addr = serverSocket.accept()
		print "Incoming connection from ", addr
		#check what client is looking for
		connectType = ConnectionTypeCheck(connectionSocket)
		#return what client needs
		if connectType[0] == "-l":
			SendDir(connectType, addr)
		elif connectType[0] == "-g":
			SendFile(connectType, addr)
		connectionSocket.close()





