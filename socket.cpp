#include <stdio.h>
#include "socket.hpp"

using namespace NEO;

void Socket::abc(void){
        printf("\nabc\n");
}

void Socket::makelisten(int listen_port, void (*new_connection_func)(Session* s)){
 struct sockaddr_in server_address;
  printf("socket(AF_INET, SOCK_STREAM, 0);\n");
  //tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
  //udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  #ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);
  #endif
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (Socket::MAX_SOCKET_FD < listen_fd) Socket::MAX_SOCKET_FD = listen_fd;
  if (listen_fd == -1)
  {
    printf("listen_fd fd=%d\n",listen_fd);
    perror("socket");
    //WSACleanup();
    exit(-1);
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
  server_address.sin_port = htons(listen_port);
  if (bind(listen_fd, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) == -1)
  {
    perror("bind");
    //WSACleanup();
     exit(-1);
  }
  if (listen(listen_fd, 5) == -1)
  {
    perror("listen");
    //WSACleanup();
     exit(-1);
  }
  //
  FD_ZERO(&(Socket::fds));
  Socket::listen_fd = listen_fd;
  FD_SET(Socket::listen_fd, &(Socket::fds));
  Socket::connection_func = new_connection_func;
}

void Socket::do_recv_send(int timeout_sec){
  //
  for(int z=0;z<sessions.size();z++){
    Session* p = sessions[z];
    if (p->wdata_size){
      if (p->send_func) p->send_func(p);
      int len;
      #ifdef _WIN32
         len = send(p->GetFD(), p->wdata, p->wdata_size, 0);
      #else
         len = send(p->GetFD(), p->wdata, p->wdata_size, MSG_NOSIGNAL);
      #endif
      if (len > 0){
        p->wdata_size -= len;
        if (p->wdata_size){
          memmove(&p->wdata[0], &p->wdata[len], p->wdata_size);
        }
      }else{

      }
    }
  }
  //        
	char buf[256];
  int nbytes;
	fd_set read_fds;
	FD_ZERO(&read_fds);
	read_fds = Socket::fds;
	//FD_SET(Socket::listen_fd, &read_fds);
    struct timeval timeout = {timeout_sec,0};
    int SelectTiming = select(Socket::MAX_SOCKET_FD+1, &(read_fds), 0, 0, &timeout);
    switch (SelectTiming)
    {
      case 0:
        //printf("Timeout!\n");
        break;
      case -1:
        perror("select");
        //WSACleanup();
        exit(4);
        break;
      default:
      {
      	for(int i = 0; i <= Socket::MAX_SOCKET_FD; i++) {
      	  //檢查read_fds
          if (FD_ISSET(i, &read_fds)) {
            if (i == Socket::listen_fd) {
              struct sockaddr_storage  client_addr;
              socklen_t addrlen = sizeof(client_addr);
              int client_fd = accept(Socket::listen_fd, (struct sockaddr *)&client_addr, &addrlen);
             if (client_fd == -1) {
                printf("Try to accept a new connection fail!\n");
                perror("accept");
              } else {
                FD_SET(client_fd, &(Socket::fds));               
                if (client_fd > Socket::MAX_SOCKET_FD) {
                  Socket::MAX_SOCKET_FD = client_fd;
                }  
                //printf("Server: new connection on socket %d\n", client_fd);
                Session* s1 = (Session*)malloc(sizeof(Session));
                s1->socketfd = client_fd;
                s1->rdata_size = 0;
                s1->wdata_size = 0;
                s1->connected = 0;
                s1->rdata = NULL;
                s1->wdata = NULL;
                sessions.push_back(s1);
                for(int z=0;z<sessions.size();z++){
   					Session* p = sessions[z];
   					if (p->GetFD() == client_fd){
   						if (Socket::connection_func){
   							p->connected = 1;
                			Socket::connection_func(p);
                		}
   					}
   				}
              }
            }else {
              //printf("i=%d,Socket::listen_fd=%d\n",i,Socket::listen_fd);
              // not listen_fd means client data coming
              if ((nbytes = recv(i, buf, sizeof buf, 0)) > 0)
              {
        		for(int z=0;z<sessions.size();z++){
   					Session* p = sessions[z];
   					if (p->GetFD() == i){
   					  //printf("nbytes=%d,p->rdata_size=%d,\n",nbytes,p->rdata_size);
   					  if (nbytes > p->rdata_size){
              	p->rdata = (char*)realloc((p->rdata) ? p->rdata:NULL, nbytes);
   					  }
   					  p->rdata_size = nbytes;
   					  memset(p->rdata,0,p->rdata_size);
   					  memcpy(p->rdata,buf,p->rdata_size);
   					  if (p->recv_func){
   		                p->recv_func(p);
                      }
   					  break;
   					}
   				}		
              } else{
                //Error handle
                if (nbytes == 0) {
                  // 關閉連線
                  printf("Server: socket %d hung up\n", i);
                } else {
                  printf("Server: socket %d recv error, nbytes=%d\n", i,nbytes);
                  perror("recv");
                }
                close(i); // bye!
                FD_CLR(i, &(Socket::fds));
                //remove Session
                for(int z=0;z<sessions.size();z++){
   					Session* p = sessions[z];
   					if (p->GetFD() == i){
   						sessions.erase(sessions.begin()+z);
   					}
   					free(p);
   				}
              }            
            }
          }
        }
        
      }
	}
}

void Socket::do_parse_package(void)
{
  for(int z=0;z<Socket::sessions.size();z++){
    Session* p = Socket::sessions[z];
    if (p->parse_func && p->rdata_size) {
      p->parse_func(p);
    }
  }
}



//template<unsigned short id, unsigned short size>
//void Socket::send_fpacket(Session *s, const Packet_Fixed<id>& fixed)
//{
    //Buffer pkt = create_fpacket<id, size>(fixed);
    //send_buffer(s, pkt);
//}


unsigned short Session::peek_package_id(void){
  assert(rdata_size >= 2);
  unsigned short package_id = 0;
  package_id = rdata[0] + (rdata[1] << 8);
  return package_id;
}

bool Session::send_wdata(const char* data, int len)
{
    if (wdata_size + len > max_wdata)
    {
        max_wdata = wdata_size + len;
        wdata = (char*)realloc((wdata) ? wdata:NULL, max_wdata);
        //realloc_fifo(s, s->max_rdata, s->max_wdata << 1);
        printf("socket: %d wdata expanded to %d bytes.\n", socketfd, max_wdata);
    }
    
    wdata_size += len;
    //先計算出矩陣的起始位置當作end, 減去要填入的資料長度當作start
    char *end = reinterpret_cast<char *>(&wdata[wdata_size + 0]);
    char *start = end - len;
    //copy(資料起始位置, 資料結束位置, start)
    std::copy(data, data + len, start);

}

void Session::wdata_dump(void)
{
   printf(
          "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F\n");
    for (int i = 0; i < wdata_size; i++)
    {
      if ((i & 15) == 0)
            printf("%04X ", i);
      if ((i - 8) % 16 == 0)  // -8 + 1
            printf(" ");  
      printf("%02X ", (unsigned char) wdata[i]);
      if ((i+1)% 16 == 0){
        printf("|\n");
      }
    }
    printf("\n");
    printf(
          "---- -----------------------  ------------------------\n");
}