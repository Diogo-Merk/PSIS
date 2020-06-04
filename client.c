#ifndef SERVER_LIBRARY_H
#include "client.h"
#endif


//https://github.com/Diogo-Merk/PSIS.git

int main(int argc, char *argv[])
{
  //Client Variables
  char ip_addr[MAXIP];
  int port;
  struct sockaddr_in local_addr;
  struct sockaddr_in server_addr;
  int sock_fd = socket(AF_INET,SOCK_STREAM,0);
  pthread_t play_thread;

  //Game Variables

  int cols,lines,n_players=0;
  char **board_geral;
  int id,r,g,b;

  //garante primeira iteração  do loop


  //Testing argc
  if(argc < 6)
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
  if(sscanf(argv[3], "%d", &r) == 0)
  {
    printf("Red not a number");
    exit(-1);
  }
  if(sscanf(argv[4], "%d", &g) == 0)
  {
    printf("Green not number");
    exit(-1);
  }
  if(sscanf(argv[5], "%d", &b) == 0)
  {
    printf("Blue not number");
    exit(-1);
  }
  init_vars(r,g,b);

  //Connecting to server
  connect_server(ip_addr,port,local_addr,server_addr,sock_fd);
  write(sock_fd,&r,sizeof(int));
  write(sock_fd,&g,sizeof(int));
  write(sock_fd,&b,sizeof(int));
  read(sock_fd,&cols,sizeof(int));
  read(sock_fd,&lines,sizeof(int));
  read(sock_fd,&id,sizeof(int));
  //board malloc
  board_geral = malloc(sizeof(char *) * cols+1);
  for ( int i = 0 ; i < cols; i++)
  {
    board_geral[i] = malloc (sizeof(char) * lines);
  }
  //Puts walls on map
  for(int i=0;i<cols;i++)
  {
    for(int j=0;j<lines;j++)
    {
      read(sock_fd,&board_geral[i][j],sizeof(char));
    }
  }
  //Draws map and walls
  initialize_map(cols,lines,board_geral);
  if(pthread_create(&play_thread, NULL, game_loop, &sock_fd)){
    printf("Failed to create receive thread\n");
    exit(-1);
  }
  //Game loop
  recv_play(sock_fd,id);

  for ( int i = 0 ; i < cols; i++)
  {
    free(board_geral[i]);
  }
  free(board_geral);
  close_board_windows();
  exit(0);
}
