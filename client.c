#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "client.h"
#include "UI_library.h"

#define MAXIP 20


int main(int argc, char *argv[])
{
  //Client Variables
  char ip_addr[MAXIP];
  int port;
  struct sockaddr_in local_addr;
  struct sockaddr_in server_addr;
  int sock_fd = socket(AF_INET,SOCK_STREAM,0);

  //Game Variables
  int done = 0;
  SDL_Event event;
  int position [2];
  int map_size[2];


  //Testing argc
  if(argc < 3)
  {
    printf("Missing ip address and port\n");
    exit(-1);
  }
  if(sscanf(argv[1], "%s", ip_addr) == 0)
  {
    printf("Wrong IP");
    exit(-1);
  }
  if(sscanf(argv[2], "%d", &port) == 0)
  {
    printf("Port is not a number");
    exit(-1);
  }
  printf("%s\n",ip_addr);
  printf("%d\n",port );
  //Connecting to server
  connect_server(ip_addr,port,local_addr,server_addr,sock_fd);
  read(sock_fd,&map_size,sizeof(cols));
  create_board_window(map_size[0],map_size[1]);
  //create_board_window(cols,lines);
  while(!done)
  {
    while(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        done = SDL_TRUE;
      }
      //Movement
      //Update position
      //Send info

    }
  }





}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void connect_server(char ip_addr[MAXIP],int port,struct sockaddr_in local_addr,struct sockaddr_in server_addr, int sock_fd)
{
  if(sock_fd == -1)
  {
    perror("socket: ");
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton(ip_addr,&server_addr.sin_addr);

  if(connect(sock_fd,(const struct sockaddr *)&server_addr,sizeof(server_addr)) == -1)
  {
    printf("Error connecting\n");
    exit(-1);
  }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
