#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include "server.h"
#include "UI_library.h"

//Global Variables
int n_players=0;
int *clients;
struct sockaddr_in *all_clients_addr;
Player_ID *player_pacmans;
new_connect first_connect;
int client_fd;

int main(int agrc, char *argv[])
{
  //Server Variables
  pthread_t connections;
  first_connect.sock_fd = socket(AF_INET,SOCK_STREAM,0);

  //Game Variables
  int done = 0;
  SDL_Event event;
  int n_playersmax;



  //Starting server
  server_start(first_connect.local_addr, first_connect.sock_fd);
  //Starting Map
  first_connect.board_geral = initialize_map(&first_connect.cols,&first_connect.lines,&n_playersmax);
  player_pacmans = malloc(sizeof(Player_ID)*n_playersmax);
  clients = malloc(sizeof(int)*n_playersmax);
  all_clients_addr = malloc(sizeof(struct sockaddr_in)*n_playersmax);

  //Thread for connections


  printf("test1\n");
  //Parent process
    while(!done)
    {
      while(SDL_PollEvent(&event))
      {
        if(event.type == SDL_QUIT)
        {
          done = SDL_TRUE;
        }
        //pthread_create(&connections,NULL,new_connections,&first_connect);
        if((client_fd = accept4(first_connect.sock_fd,(struct sockaddr*)&first_connect.client_addr,&first_connect.addr_len,SOCK_NONBLOCK))>0)
        {
          new_connections(client_fd);
        }
        printf("%d\n",n_players);
        for(int i=0;i<n_players;i++)
        {
          for(int j=0;j<n_players;j++)
          {
            sendto(clients[i],&player_pacmans[j],sizeof(Player_ID),0,(struct sockaddr*)&all_clients_addr[i],sizeof(all_clients_addr[i]));
            printf("segfault aqui\n");
          }
        }
        for(int i=0;i<n_players;i++)
        {
          recvfrom(first_connect.sock_fd,&player_pacmans[i],sizeof(Player_ID),0,(struct sockaddr*)&first_connect.client_addr,&first_connect.addr_len);
        }
          //pthread_join(connections,NULL);
      }
    }
  close_board_windows();
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
void new_connections(int client_fd)
{
    sendto(client_fd,&first_connect.cols,sizeof(int),0,(struct sockaddr *)&first_connect.client_addr,sizeof(first_connect.client_addr));
    sendto(client_fd,&first_connect.lines,sizeof(int),0,(struct sockaddr *)&first_connect.client_addr,sizeof(first_connect.client_addr));
    for(int i=0;i<first_connect.lines;i++)
    {
      for(int j=0;j<first_connect.cols+1;j++)
      {
        sendto(client_fd,&first_connect.board_geral[i][j],sizeof(char),0,(struct sockaddr*)&first_connect.client_addr,sizeof(first_connect.client_addr));
      }
    }
    n_players++;
    clients[n_players] = client_fd;
    all_clients_addr[n_players] = first_connect.client_addr;
    player_pacmans[n_players].ID = n_players;
    //Testing only
    player_pacmans[n_players].x = 0;
    player_pacmans[n_players].y = 2;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
char** initialize_fruits(int cols, int lines,int n_players, char** board)
{
  srand(time(NULL));
  int i = 0, l = 0, c = 0, r= 0;

  while (i<((n_players-1)*2))
  {
    l = rand() % lines;
    c = rand() % cols;
    r = rand() % 1;
    if (board[c][l] == ' ')
    {
      if (r==1)
      {
        board[c][l] = 'C';
        paint_cherry(c,l);
        i++;
      }
      else
      {
        board[c][l] = 'L';
        paint_lemon(c,l);
        i++;
      }
    }
  }
  return board;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
