#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

#define BACKLOG 20
#define REQSIZE 2048  //size of GET request, 2048th byte is NULL
#define PATHSIZE 1024 //size of file path in get_request

int socket_fd, client_fd;
struct sockaddr_in server_addr; /* my address */
struct sockaddr_in client_addr; /* client's address */
unsigned int addr_size;
int enable = 1;

//  find an occurence of .. or // in the path
int isLegalPath(char *path){
  int i;
  int s = strlen(path);

  for(i=1; i<s; i++){
    if(path[i] == '/' && path[i-1] == '/')
      return 0;
    if(path[i] == '.' && path[i-1] == '.')
      return 0;
  }
  return 1;
}

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
     but exactly one time, in the main thread of your program)
     BEFORE USING ANY OF THE FUNCTIONS BELOW
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port) { // Creates server
  //  Create socket
  socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  if(socket_fd == -1){
    perror("In init(): Error creating socket.");
    exit(1);
  }
  //  Bind the socket to a network address
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  //  Set the reuse option
  if( setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int)) == -1) {
    perror("In init(): Cannot set socket reuse option.");
    exit(1);
  }
  // Bind the socket command
  if ( bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("In init(): Unable to bind.");
    exit(1);
  }
  // Setup a queue to listen to oncoming requests (backlog upto 20 requests in queue)
  if (  listen(socket_fd, BACKLOG) == -1) {
    perror("In init(): Unable to listen.");
    exit(1);
  }
  return;
}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the request should be ignored.
***********************************************/
int accept_connection(void) { // Connection handle

  // reutrn this file descriptor 
  int connection_fd;
  addr_size = sizeof(client_addr);
  // accept socket
  connection_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addr_size);

  // Error check
  if(connection_fd == -1){
    perror("In accept_connection(): Unable to accept connection.");
    return -1;
  }
  return connection_fd;
}

/**********************************************
 * get_request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        from where you wish to get a request
      - filename is the location of a character buffer in which
        this function should store the requested filename. (Buffer
        should be of size 1024 bytes.)
   - returns 0 on success, nonzero on failure. You must account
     for failures because some connections might send faulty
     requests. This is a recoverable error - you must not exit
     inside the thread that called get_request. After an error, you
     must NOT use a return_request or return_error function for that
     specific 'connection'.
************************************************/
int get_request(int fd, char *filename) { // Data handle

  // init request data
  char reqMsg[REQSIZE];
  char reqName[10];       // aka GET
  char reqPath[PATHSIZE]; // aka file path
  char reqType[10];       // aka HTTP/1.1
  int readSize = 0;
  readSize = read(fd, reqMsg, REQSIZE-1);

  if(readSize < 0){
    perror("In get_request(): Request read problem.");
    return -1;  //if read() returns negative, a read error must have occured
  }
  //  This should separate the important info from the request into three char buffers
  //  These three, in order, should be the first line of the request if printed in its entirety
  sscanf(reqMsg, "%s %s %s", reqName, reqPath, reqType);

  if(strcmp(reqName,"GET"))   // Not a get request
    return -1;
  if(strlen(reqPath) <= 0 || !isLegalPath(reqPath))    // Empty file path case or faulty read from sscanf()
    return -1;                                         

  //  If reach this point, the request contains the two strings as specified

  reqMsg[readSize] = '\0';  //Null terminate end of message
  reqPath[PATHSIZE-1] = '\0'; // Null terminate file path
  strcpy(filename, reqPath);

  return 0;
}

/**********************************************
 * return_result
   - returns the contents of a file to the requesting client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()     --> Socket descriptor
        to where you wish to return the result of a request
      - content_type is a pointer to a string that indicates the
        type of content being returned. possible types include
        "text/html", "text/plain", "image/gif", "image/jpeg" cor-
        responding to .html, .txt, .gif, .jpg files.
      - buf is a pointer to a memory location where the requested
        file has been read into memory (the heap). return_result
        will use this memory location to return the result to the
        user. (remember to use -D_REENTRANT for CFLAGS.) you may
        safely deallocate the memory after the call to
        return_result (if it will not be cached).
      - numbytes is the number of bytes the file takes up in buf
   - returns 0 on success, nonzero on failure.
************************************************/
// Send data
int return_result(int fd, char *content_type, char *buf, int numbytes) {
  char header[REQSIZE];

  // This header will be sent to the client giving all useful information
  // about the server and the requested file (size, connection len, con. typ.)
  int size = sprintf(header, 
      "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %d\nConnection: Close\n\n",
      content_type, numbytes);

  // Writes the data to the client's file descriptor
  write(fd, header, size);
  write(fd, buf, numbytes);
  
  // Closes the connection 
  if(close(fd) < 0){
    printf("\nSocket didn't close right\n");
    return -1;
  }

  return 0;
}

/**********************************************
 * return_error
   - returns an error message in response to a bad request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the error
      - buf is a pointer to the location of the error text
   - returns 0 on success, nonzero on failure.
************************************************/
int return_error(int fd, char *buf) { // Send Error(s)
  char header[REQSIZE];

  // Same as header from 'return_request' but with an error code instead.
  int size = sprintf(header, 
  "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: %ld\nConnection: Close\n\n",
      strlen(buf));
      
  // Writes the msg. and the header to client
  write(fd, header, size);
  write(fd, buf, sizeof(buf));

  // Closes the connection
  if(close(fd) < 0){
    printf("\nSocket didn't close right\n");
    return -1;
  }

  return 0;
}
