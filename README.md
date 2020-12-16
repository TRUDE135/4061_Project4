CSCI4061 Fall 2020
Project 4 - Group 41
Test Machine: csel-kh4250-05
Group members:
  Andrew Trudeau  trude135
  Minh Bui        bui00011
  John Kimlinger  kimli020

• The purpose of your program:
  The goal of this project is to integrate the c socket/server functionality of project 3. This includes writing sockets and accepting requests for file transfer using http/wget.  

• How to compile the program
  Within the file diretory, if you run 'make', the ./web_server program will be generated using the code found in the Makfile provided. To run the program you can use the how_to_test file provided in the testing folder. I will briefly explain it here:
    After the program is compiled, you can run it with the following command: 
        ./web_server <port> <path_to_testing>/testing <num_dispatch> <num_worker> <dynamic_flag> <queue_len> <cache_entries>
    Example:
        ./web_server 9000 ./testing 100 100 1 100 0
  Now that the server is running, you can make a wget http request for a file of the server using:
        wget http://127.0.0.1:9000/image/jpg/29.jpg
    This finds a file using the local IP adress and a port that is chosen above when running the web_server

• Breif explanation of how the program works:
  The init function creates a web-server using crucial functions like: socket(), htons(), htonl(), setsockopt(), bind, listen. The accept_connection() function takes a connection and returns a file decriptor to write the file request data to (triggered by wget and listen). get_request retrieves the data needed to send. return_result will send the data given the fd and file size type and data to the fd (sends the client data). return_error will send an error back to the client if an error occurs. More in-depth explanation in the coments/src code.
  
Unfortunately we did not have enough time to start on the extra credit portion of this project. 

• Contribution by each member of the team
  
  Minh Bui: Set the framework for the intermin report and wrote init/get_request functions. Wrote accept_connection function. Comments. Effectivly communicated with the group. Finished polishng for the program. Helped with README.
  
  Andrew Trudeau: Helped create base code for intermin report. Wrote the writeup for the intermin report. Debugged code to finish the return_result method. Wrote the return_error method. Effectivly communicated with group.
    
  John Kimlinger: Set up github project. Wrote the README. Finished up the commenting and polishing of the program. Effectivly communicated with the group.