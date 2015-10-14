#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#include <windows.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#include <netinet/tcp.h> //For TCP_NODELAY

#endif
#include <fcntl.h>
#include <cstdlib>
#include <stdio.h>

#include <unistd.h> // close()
//#include <sys/socket.h>
//#include  <sys/types.h>
//#include  <sys/times.h>
//#include  <sys/select.h>
namespace neosocket
{

}

int main(void)
{
	printf("main\n");
	//Asign FD_SET fds
	fd_set fds;
	int MAX_SOCKET_FD = 0;
	int port = 1234;
	struct timeval timeout={3,0};
	struct sockaddr_in server_address;
	printf("socket(AF_INET, SOCK_STREAM, 0);\n");
	//tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    //udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	#ifdef _WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2),&wsa);
	#endif
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (MAX_SOCKET_FD < listen_fd) MAX_SOCKET_FD = listen_fd;
	if (listen_fd == -1)
        {
		  printf("listen_fd fd=%d\n",listen_fd);
          perror("socket");
		  //WSACleanup();
          return 1;
        }
	printf("listen_fd fd=%d\n",listen_fd);
	printf("Set non Blocking\n");
	#ifdef _WIN32
		u_long enable = 1;
		ioctlsocket(listen_fd, FIONBIO, &enable);
	#else
		fcntl(listen_fd, F_SETFL, O_NONBLOCK);		
	#endif
	const char yes = 1;
	// SO_REUSEADDR should be set for TCP
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
	// IPPROTO_TCP for TCP sockets
	// IPPROTO_UDP for UDP sockets
  	setsockopt(listen_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof yes);
	// sin_family is always set to AF_INET.
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        server_address.sin_port = htons(port);
        if (bind(listen_fd, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) == -1)
	{
          perror("bind");
		  //WSACleanup();
          exit(1);
	}
        if (listen(listen_fd, 5) == -1)
        {
          perror("listen");
		  //WSACleanup();
          exit(1);
	}
	// fd should small than FD_SETSIZE
	FD_SET(listen_fd, &fds);
	printf("Server start listen:%d\n",port);
	
	 printf("server: waiting for connections...\n");
	//----------------- 
	char buf[256];
	int nbytes;
	fd_set read_fds;
	FD_ZERO(&fds);
    FD_SET(listen_fd, &fds);
	for( ; ; ) {
		read_fds = fds; 
		struct timeval timeout = {3,0};
		int SelectTiming = select(MAX_SOCKET_FD+1, &read_fds, 0, 0, &timeout);
		switch (SelectTiming)
        {
			case 0:
				printf("Timeout!\n");
				break;
			break;
			case -1:
			 perror("select");
			 //WSACleanup();
			 exit(4);
			break;
			default:
			{
				//struct sockaddr_storage  client_addr;
                //socklen_t addrlen = sizeof(client_addr);
		        //int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
			    for(int i = 0; i <= MAX_SOCKET_FD; i++) { 
			        if (FD_ISSET(i, &read_fds)) {
					    if (i == listen_fd) {
					  	    struct sockaddr_storage  client_addr;
						    socklen_t addrlen = sizeof(client_addr);
						    int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
						    if (client_fd == -1) {
								printf("Try to accept a new connection fail!\n");
							    perror("accept");
						    } else {
							    FD_SET(client_fd, &fds);
							    if (client_fd > MAX_SOCKET_FD) {
							      MAX_SOCKET_FD = client_fd;
		                        }
		                        printf("Server: new connection on socket %d\n", client_fd);
						    }
					    }else {
						// not listen_fd means client data coming
							  if ((nbytes = recv(i, buf, sizeof buf, 0)) > 0)
							  {
								//
								//j should init to 1?
								for(int j = 0; j <= MAX_SOCKET_FD; j++) {
									if (FD_ISSET(j, &fds)) {
										if (j != listen_fd && j != i) {
											//Don't need to send server and self again!
											if (send(j, buf, nbytes, 0) == -1) {
												printf("Send message to fd: %d\n",j);
												perror("send");
											}
										}
									}
								}
								//
							  } else{
								  //Error handle
								  if (nbytes == 0) {
                                       // 關閉連線
								       printf("Server: socket %d hung up\n", i);
								   } else {
									    perror("recv");
                                   }
                                   close(i); // bye!
                                   FD_CLR(i, &fds);
							    }
						}
				    }
			    }
	        }
			break;
		}

		/*
          read_fds = fds; // 複製 fds
	  //Only select read_fds
	  #ifdef _WIN32
	 if (select(0, &read_fds, NULL, NULL, NULL) == -1) {
	#else
	  if (select(MAX_SOCKET_FD+1, &read_fds, NULL, NULL, NULL) == -1) {  
	#endif
             printf("MAX_SOCKET_FD=%d\n",MAX_SOCKET_FD);
			perror("select");
			 //WSACleanup();
            exit(4);
          }

	 for(int i = 0; i <= MAX_SOCKET_FD; i++) {
           if (FD_ISSET(i, &read_fds)) {
             if (i == listen_fd) {
		//printf("We found one connection, %d\n",fd);
		struct sockaddr_storage  client_addr;
        socklen_t addrlen = sizeof(client_addr);
		int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);
		if (client_fd == -1) {
                  perror("accept");
                } else {
                  FD_SET(client_fd, &fds);
                  if (client_fd > MAX_SOCKET_FD) {
                    MAX_SOCKET_FD = client_fd;
		  }
		  printf("Server: new connection on socket %d\n", client_fd);
                }
	     } else {
	       // not listen_fd means client data coming
               if ((nbytes = recv(i, buf, sizeof buf, 0)) > 0)
	       {
			//j should init to 1?
			  for(int j = 0; j <= MAX_SOCKET_FD; j++) {
              		    if (FD_ISSET(j, &fds)) {
		              if (j != listen_fd && j != i) {
				//Don't need to send server and self again!
				if (send(j, buf, nbytes, 0) == -1) {
                    		  printf("Send message to fd: %d\n",j);
 				  perror("send");
                                }
			      }
			    }
			  }
	       } else{
		//Error handle
		  if (nbytes == 0) {
                    // 關閉連線
                    printf("Server: socket %d hung up\n", i);
                  } else {
                    perror("recv");
                  }
                  close(i); // bye!
                  FD_CLR(i, &fds);
	       }
             }
	  }
	 }
	 */
	}
        exit(0);
}