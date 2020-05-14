#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "server.h"
#include "UI_library.h"

//Global Variables
int n_players=0;
int *clients;
struct sockaddr_in *all_clients_addr;

int main(int agrc, char *argv[])
{
  //Server Variables
  struct sockaddr_in local_addr;
  struct sockaddr_in client_addr;
  socklen_t addr_len;
  int sock_fd = socket(AF_INET,SOCK_STREAM,0);



  //Game Variables
  int done = 0;
  SDL_Event event;
  int cols,lines,n_playersmax;
  char **board_geral;
  Player_ID *player_pacmans;


  //Starting server
  server_start(local_addr, sock_fd);
  //Starting Map
  board_geral = initialize_map(&cols,&lines,&n_playersmax);
  player_pacmans = malloc(sizeof(Player_ID)*n_playersmax);
  clients = malloc(sizeof(int)*n_playersmax);
  all_clients_addr = malloc(sizeof(struct sockaddr_in)*n_playersmax);


  //Mudar para thread
  int child;
  if(child = fork() == 0)
  {
    while(1)
    {
      int client_fd = accept(sock_fd,(struct sockaddr*)&client_addr,&addr_len);
      sendto(client_fd,&cols,sizeof(cols),0,(struct sockaddr *)&client_addr,sizeof(client_addr));
      sendto(client_fd,&lines,sizeof(lines),0,(struct sockaddr *)&client_addr,sizeof(client_addr));
      for(int i=0;i<lines;i++)
      {
        for(int j=0;j<cols+1;j++)
        {
          sendto(client_fd,&board_geral[i][j],sizeof(char),0,(struct sockaddr*)&client_addr,sizeof(client_addr));
        }
      }
      n_players++;
      clients[n_players] = client_fd;
      all_clients_addr[n_players] = client_addr;
      player_pacmans[n_players].ID = n_players;
      //Testing only
      player_pacmans[n_players].x = 0;
      player_pacmans[n_players].y = 2;
    }
  }



  printf("test1\n");
  printf("%d\n",player_pacmans[n_players].y);
  //Parent process
    while(!done)
    {
      while(SDL_PollEvent(&event))
      {
        if(event.type == SDL_QUIT)
        {
          done = SDL_TRUE;
        }
        for(int i=0;i<n_players;i++)
        {
          for(int j=0;j<n_players;j++)
          {
            sendto(clients[i],&player_pacmans[j],sizeof(Player_ID),0,(struct sockaddr*)&all_clients_addr[i],sizeof(all_clients_addr[i]));
          }
        }
        for(int i=0;i<n_players;i++)
        {
          recvfrom(sock_fd,&player_pacmans[i],sizeof(Player_ID),0,(struct sockaddr*)&client_addr,&addr_len);
        }
      }
    }
  close_board_windows();
  kill(child,SIGKILL);
  exit(0);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void server_start(struct sockaddr_in local_addr, int sock_fd)
{
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = INADDR_ANY;
  local_addr.sin_port = htons(10000);

  int error = bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr));
  if(error == -1)
  {
    perror("bind");
    exit(-1);
  }
  if(listen(sock_fd,2) == -1)
  {
    perror("listen");
    exit(-1);
  }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
char** initialize_map(int *cols, int *lines,int *n_playersmax)
{
  int n_lines, n_cols,n_walls;
  //Map file
  FILE *map;
  char **board;

  map = fopen("board.txt", "r");

  fscanf(map,"%d %d",&n_cols,&n_lines);
  fgetc(map);
  board = malloc(sizeof(char *) * n_lines);
  for ( int i = 0 ; i < n_lines; i++)
  {
    board[i] = malloc (sizeof(char) * (n_cols+1));
  }
  create_board_window(n_cols, n_lines);

  //Receber posições das paredes
  for(int i=0;i<n_lines;i++)
  {
    for(int j=0;j<n_cols+1;j++)
    {
      board[i][j]=fgetc(map);
      //printf("%d %d = %c\n",i,j,board[i][j]);
    }
  }

  //Preencher paredes
  for(int y=0;y<n_lines;y++)
  {
    for(int x=0;x<n_cols+1;x++)
    {
      //printf("|>%c<|",board[y][x]);
      if(board[y][x] == 'B')
      {
        paint_brick(x,y);
        n_walls++;
      }
    }
    //printf("\n");
  }
  *cols = n_cols;
  *lines = n_lines;
  *n_playersmax = (n_lines*n_cols)-n_walls;
  return board;

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
