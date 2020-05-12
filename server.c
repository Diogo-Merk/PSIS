#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "server.h"
#include "UI_library.h"

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
  int cols,lines;
  char **board_geral;

  //Starting server
  server_start(local_addr, sock_fd);
  //Starting Map
  board_geral = initialize_map(&cols,&lines);
  printf("%d %d",cols,lines);

  //child process
  if (fork()==0)
  {
    while(!done)
    {
      int client_fd = accept(sock_fd,(struct sockaddr *)&client_addr,&addr_len);
      if(client_fd == -1)
      {
        perror("accept: ");
        exit(-1);
      }
      sendto(client_fd,&cols,sizeof(cols),0,(struct sockaddr *)&client_addr,sizeof(client_addr));
      sendto(client_fd,&lines,sizeof(lines),0,(struct sockaddr *)&client_addr,sizeof(client_addr));

      for(int i=0;i<lines;i++)
      {
        for(int j=0;j<cols+1;j++)
        {
          sendto(client_fd,&board_geral[i][j],sizeof(char),0,(struct sockaddr *)&client_addr,sizeof(client_addr));
        }
      }

    }
    exit(0);
  }
  //Parent process
  else
  {
    while(!done)
    {
      while(SDL_PollEvent(&event))
      {
        if(event.type == SDL_QUIT)
        {
          done = SDL_TRUE;
        }
      }
    }
  }

  close_board_windows();
  printf("fuck you\n");
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
char** initialize_map(int *cols, int *lines)
{
  int n_lines, n_cols;
  //Map file
  FILE *map;
  char **board,ch;

  board = malloc(sizeof(char *) * n_lines);
  for ( int i = 0 ; i < n_lines; i++)
  {
    board[i] = malloc (sizeof(char) * (n_cols+1));
  }

  map = fopen("board.txt", "r");

  fscanf(map,"%d %d",&n_cols,&n_lines);
  fgetc(map);
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
      }
    }
    //printf("\n");
  }
  *cols = n_cols;
  *lines = n_lines;
  return board;

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
