#include "server_novo.h"

int main()
{
  int n_players,n_playersmax,cols,lines;
  int colour[3];
  int client_sock;
  int sock_fd = socket(AF_INET,SOCK_STREAM,0);
  pthread_t client_connect;
  Player_ID clients;
  char **board_geral;

  server_start(sock_fd);
  board_geral = initialize_map(&cols,&lines,&n_playersmax);

  while(n_players < n_playersmax)
  {
    //May have changes
    client_sock = accept(sock_fd,NULL,NULL);
    n_players++;
    //recieve colour of pacman
    for(int i=0;i<3;i++)
    {
      read(client_sock,&colour[i],sizeof(int));
    }
    clients = set_info(colour,n_players);
    //sending map size
    write(client_sock,&cols,sizeof(int));
    write(client_sock,&lines,sizeof(int));
    //sending board
    for(int i=0;i< lines;i++)
    {
      for(int j=0;j<cols+1;j++)
      {
        write(client_sock,&board_geral[i][j],sizeof(char));
      }
    }
    pthread_create(&client_connect,NULL,game,(void *)clients);
    printf("client joined game\n");
  }
}
