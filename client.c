#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT "3490"
#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *);

int main(int argc,char *argv[]) {

    struct addrinfo *ps_address;
    int               i_addrlen;
    char            ac_buf[MAXDATASIZE];
    WSADATA         wsaData;
    struct addrinfo s_hints;
    int             status;
    struct addrinfo *ps_servinfo;
    SOCKET              sockfd;
    char             ac_server[INET6_ADDRSTRLEN];
    int             numbytes;

    //Check for host name command line argument
    if (argc != 2) {
        fprintf(stderr,"usage: WSclient hostname\n");
        return 1;
    }

    //intialize WinSock instance, request version 2.2
    status = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (status != 0) {
        //todo get more detailed error message
        fprintf(stderr,"WSAStartup failed\n");
        return 2;
    }

    //check if winsock 2.2 was available
    if (HIBYTE(wsaData.wVersion) < 2 || LOBYTE(wsaData.wVersion) < 2) {
        fprintf(stderr, "Version 2.2 of Winsock is not available.\n");
        WSACleanup();
        return 3;
    }

    //set ip address characteristics
    memset(&s_hints,0,sizeof(s_hints));
    s_hints.ai_family   = AF_UNSPEC;   // AF_INET or AF_INET6 to force version
    s_hints.ai_socktype = SOCK_STREAM; // Streaming socket

    //request list of matching ip addresses for specified host
    status = getaddrinfo(argv[1],PORT,&s_hints,&ps_servinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo failed\n");
        WSACleanup();
        return 4;
    }

    //loop through results from getaddrinfo
    for (ps_address = ps_servinfo ;
        ps_address != NULL ;
        ps_address = ps_address->ai_next) {

        sockfd = socket(ps_address->ai_family,ps_address->ai_socktype,
                        ps_address->ai_protocol);
        if (sockfd == INVALID_SOCKET)
        {
          //  dw_error = (DWORD)WSAGetLastError();
          //  get_msg_text(dw_error,
          //               &nc_error);
          //  fprintf(stderr,"socket failed with code %ld.\n",dw_error);
          //  fprintf(stderr,"%s\n",nc_error);
          //  LocalFree(nc_error);
        
           continue;
        }

        i_addrlen = (int)ps_address->ai_addrlen;
        status = connect(sockfd,ps_address->ai_addr,i_addrlen);

        if (status == SOCKET_ERROR)
        {
          //  dw_error = (DWORD)WSAGetLastError();
          //  get_msg_text(dw_error,
          //               &nc_error);
          //  fprintf(stderr,"connect failed with code %ld.\n",dw_error);
          //  fprintf(stderr,"%s\n",nc_error);
          //  LocalFree(nc_error);
          closesocket(sockfd);
        
           continue;
        }

        break;
    }

    //check for successful connection
    if (ps_address == NULL) {
    fprintf(stderr,"Failed to connect.\n");
      freeaddrinfo(ps_servinfo);
      WSACleanup();
      return 5;
    }

    //inet_ntop converts ip address into string with proper format
    inet_ntop(ps_address->ai_family,
        get_in_addr((struct sockaddr *)ps_address->ai_addr),
        ac_server,sizeof(ac_server));

    printf("Connecting to %s\n",ac_server);
    
    //free list of addresses
    freeaddrinfo(ps_servinfo);

    //recieve message from server
    numbytes = recv(sockfd,ac_buf,MAXDATASIZE-1,0);
    if (numbytes == SOCKET_ERROR) {
        fprintf(stderr, "recieve failed\n");
        closesocket(sockfd);
        WSACleanup();
        return 6;
    } else if (numbytes == 0) {
        fprintf(stderr, "socket closed by server\n");
        closesocket(sockfd);
        WSACleanup();
        return 7;
    }

    //display data recieved
    ac_buf[numbytes] = '\0';
    printf("Recieved '%s'",ac_buf);

    //terminate winsock
    closesocket(sockfd);
    WSACleanup();
    return 0;
}


void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET)
   {
      return &(((struct sockaddr_in*)sa)->sin_addr);
   }

   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}