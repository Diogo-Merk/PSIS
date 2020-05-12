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
//https://github.com/Diogo-Merk/PSIS.git

int main(int argc, char *argv[])
{
  //Client Variables
  char ip_addr[MAXIP];
  int port;
  struct sockaddr_in local_addr;
  struct sockaddr_in server_addr;
  int sock_fd = socket(AF_INET,SOCK_STREAM,0);
  socklen_t size_server_addr;

  //Game Variables
  int done = 0;
  SDL_Event event;
  int position [2];
  int cols,lines;
  char **board_geral;
  int horizontal_move = 0, vertical_move = 0;
  Pacman pacman;


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
  size_server_addr = sizeof(struct sockaddr_storage);
  recvfrom(sock_fd,&cols,sizeof(cols),0,(struct sockaddr *)&server_addr,&size_server_addr);
  recvfrom(sock_fd,&lines,sizeof(lines),0,(struct sockaddr *)&server_addr,&size_server_addr);
  board_geral = malloc(sizeof(char *) * lines);
  for ( int i = 0 ; i < lines; i++)
  {
    board_geral[i] = malloc (sizeof(char) * (cols+1));
  }
  printf("Malloc good\n");

  for(int i=0;i<lines;i++)
  {
    for(int j=0;j<cols+1;j++)
    {
      recvfrom(sock_fd,&board_geral[i][j],sizeof(char),0,(struct sockaddr*)&server_addr,&size_server_addr);
    }
  }
  printf("recieve good\n");
  initialize_map(cols,lines,board_geral);

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
    switch( event.type )
    {
      /* Look for a keypress */
      case SDL_KEYDOWN:
          /* Check the SDLKey values and move change the coords */
          switch( event.key.keysym.sym )
          {
            case SDLK_LEFT:
              if (board[ID.x-1][ID.y]!='B')
              {
                horizontal_move = -1;
              }
              break;
            case SDLK_RIGHT:
              if (board[ID.x+1][ID.y]!='B')
              {
                horizontal_move = 1;
              }
              break;
            case SDLK_UP:
              if (board[ID.x][ID.y-1]!='B')
              {
                vertical_move = -1;
              }
              break;
            case SDLK_DOWN:
              if (board[ID.x][ID.y+1]!='B')
              {
                vertical_move = 1;
                pacman.
              }
              break;
            default:
                break;
          }
      break;
      /* Look for letting go of a key */
      case SDL_KEYUP:
        /* Check the SDLKey values and zero the movemnet when necessary */
        switch( event.key.keysym.sym )
        {
          case SDLK_LEFT:
              if( horizontal_move < 0 )
                  horizontal_move = 0;
              break;
          case SDLK_RIGHT:
              if( horizontal_move > 0 )
                  horizontal_move = 0;
              break;
          case SDLK_UP:
              if( vertical_move < 0 )
                  vertical_move = 0;
              break;
          case SDLK_DOWN:
              if( vertical_move > 0 )
                  vertical_move = 0;
              break;
          default:
              break;
        }
        break;

        default:
            break;
        }
      //Update position
     ID.x += horizontal_move;
     ID.y += vertical_move;
      //Send info

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
void initialize_map(int n_cols, int n_lines, char **board_geral)
{
  create_board_window(n_cols, n_lines);
  printf("create map good\n");
  //Preencher paredes
  for(int y=0;y<n_lines;y++)
  {
    for(int x=0;x<n_cols+1;x++)
    {
      if(board_geral[y][x] == 'B')
      {
        paint_brick(x,y);
      }
    }
    printf("\n");
  }
  printf("bricks good\n");
}
