#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
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
  SDL_Event event;
  int done = 0;
  int cols,lines,n_players;
  char **board_geral;
  int pac_horizontal_move = 0, pac_vertical_move = 0, mon_horizontal_move = 0, mon_vertical_move = 0, xaux = 0, yaux = 0;
  Player pacman_local, monster_local;
  Player *pacman_others;
  int coord[2];

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
  //Connecting to server
  connect_server(ip_addr,port,local_addr,server_addr,sock_fd);
  size_server_addr = sizeof(struct sockaddr_storage);
  read(sock_fd,&cols,sizeof(int));
  read(sock_fd,&lines,sizeof(int));
  printf("%d %d\n",cols,lines);
  board_geral = malloc(sizeof(char *) * lines);
  for ( int i = 0 ; i < lines; i++)
  {
    board_geral[i] = malloc (sizeof(char) * (cols+1));
  }
  for(int i=0;i<lines;i++)
  {
    for(int j=0;j<cols+1;j++)
    {
      read(sock_fd,&board_geral[i][j],sizeof(char));
    }
  }
  printf("Board kinda good\n");
  initialize_map(cols,lines,board_geral);
  printf("map good\n");

  while(!done)
  {
    while(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        done = SDL_TRUE;
      }

      //recieving player positions
      read(sock_fd,&coord,sizeof(coord));
      printf("recieved %d %d\n",coord[0],coord[1] );
      update_map(coord[0],coord[1],n_players);
      //Movement
      switch( event.type )
      {
        //Monster Movement
        /* Look for a keypress */
        case SDL_KEYDOWN:
            /* Check the SDLKey values and move change the coords */
            switch( event.key.keysym.sym )
            {
              case SDLK_LEFT:
                if (board_geral[monster_local.x-1][monster_local.y]!='B')
                {
                  mon_horizontal_move = -1;
                }
                else if (board_geral[monster_local.x-1][monster_local.y]=='B' && board_geral[monster_local.x+1][monster_local.y]!='B')
                {
                  mon_horizontal_move = 1;
                }
                break;
              case SDLK_RIGHT:
                if (board_geral[monster_local.x+1][monster_local.y]!='B')
                {
                  mon_horizontal_move = 1;
                }
                else if (board_geral[monster_local.x+1][monster_local.y]=='B' && board_geral[monster_local.x-1][monster_local.y]!='B')
                {
                  mon_horizontal_move = -1;
                }
                break;
              case SDLK_UP:
                if (board_geral[monster_local.x][monster_local.y-1]!='B')
                {
                  mon_vertical_move = -1;
                }
                else if (board_geral[monster_local.x][monster_local.y-1]=='B' && board_geral[monster_local.x][monster_local.y+1]!='B')
                {
                  mon_vertical_move = 1;
                }
                break;
              case SDLK_DOWN:
                if (board_geral[monster_local.x][monster_local.y+1]!='B')
                {
                  mon_vertical_move = 1;
                }
                else if (board_geral[monster_local.x][monster_local.y+1]=='B' && board_geral[monster_local.x][monster_local.y-1]!='B')
                {
                  mon_vertical_move = -1;
                }
                break;
              default:
                break;
            }
        break;
        /* Look for letting go of a key */
        case SDL_KEYUP:
          /* Check the SDLKey values and zero the movemnet when necessary */
          switch( event.key.keysym.sym)
          {
            case SDLK_LEFT:
              if( mon_horizontal_move < 0 )
                  mon_horizontal_move = 0;
              break;
            case SDLK_RIGHT:
              if( mon_horizontal_move > 0 )
                  mon_horizontal_move = 0;
              break;
            case SDLK_UP:
              if( mon_vertical_move < 0 )
                  mon_vertical_move = 0;
              break;
            case SDLK_DOWN:
              if( mon_vertical_move > 0 )
                  mon_vertical_move = 0;
              break;
            default:
              break;
          }
          break;

          //Pacman Movement
          /* Look for a keypress */
          case SDL_MOUSEBUTTONDOWN:
              /* Check the SDLKey values and move change the coords */
              if( event.button.button == SDL_BUTTON_LEFT)
              {
                SDL_GetMouseState(&xaux, &yaux);
                if (xaux>pacman_local.x && yaux < xaux && yaux > -xaux && board_geral[pacman_local.x+1][pacman_local.y]!='B')
                {
                  pac_horizontal_move = 1;
                }
                else if (board_geral[pacman_local.x+1][pacman_local.y]=='B' && board_geral[pacman_local.x-1][pacman_local.y]!='B')
                {
                  pac_horizontal_move = -1;
                }
                if (xaux<pacman_local.x && yaux > xaux && yaux < -xaux && board_geral[pacman_local.x-1][pacman_local.y]!='B')
                {
                  pac_horizontal_move = -1;
                }
                else if (board_geral[pacman_local.x-1][pacman_local.y]=='B' && board_geral[pacman_local.x+1][pacman_local.y]!='B')
                {
                  pac_horizontal_move = 1;
                }
                if (yaux>pacman_local.y && yaux > xaux && yaux > -xaux && board_geral[pacman_local.x][pacman_local.y+1]!='B')
                {
                  pac_vertical_move = 1;
                }
                else if (board_geral[pacman_local.x][pacman_local.y+1]=='B' && board_geral[pacman_local.x][pacman_local.y-1]!='B')
                {
                  pac_vertical_move = -1;
                }
                if (yaux>pacman_local.y && yaux < xaux && yaux < -xaux && board_geral[pacman_local.x][pacman_local.y-1]!='B')
                {
                  pac_vertical_move = -1;
                }
                else if (board_geral[pacman_local.x][pacman_local.y-1]=='B' && board_geral[pacman_local.x-1][pacman_local.y+1]!='B')
                {
                  pac_vertical_move = 1;
                }
              }
          break;
          /* Look for letting go of a key */
          case SDL_MOUSEBUTTONUP:
            /* Check the SDLKey values and zero the movemnet when necessary */
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if( pac_horizontal_move < 0 )
                {
                  pac_horizontal_move = 0;
                }
                if( pac_horizontal_move > 0 )
                {
                  pac_horizontal_move = 0;
                }

                if( pac_vertical_move < 0 )
                {
                  pac_vertical_move = 0;
                }

                if( pac_vertical_move > 0 )
                {
                  pac_vertical_move = 0;
                }
            }
            break;

          default:
            break;
        }
      //Update position
      monster_local.x += mon_horizontal_move;
      monster_local.y += mon_vertical_move;
      pacman_local.x += pac_horizontal_move;
      pacman_local.y += pac_vertical_move;

      //condicoes de teleporte
      /*if (monster_local.x>=n_cols)
      {
        monster_local.x = 0;
      }
      if (monster_local.x<0)
      {
        monster_local.x = n_cols-1;
      }
      if (monster_local.y>=n_lines)
      {
        monster_local.y = 0;
      }
      if (monster_local.y<0)
      {
        monster_local.y = n_lines-1;
      }
      if (pacman_local.x>=n_cols)
      {
        pacman_local.x = 0;
      }
      if (pacman_local.x<0)
      {
        pacman_local.x = n_cols-1;
      }
      if (pacman_local.y>=n_lines)
      {
        pacman_local.y = 0;
      }
      if (pacman_local.y<0)
      {
        pacman_local.y = n_lines-1;
      }*/

      //Send info
      pacman_local.x = coord[0];
      pacman_local.y = coord[1];
      write(sock_fd,&coord,sizeof(coord));
      printf("Is it working?\n");
    }
  }
  close_board_windows();
  exit(0);
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
  printf("map kinda good\n");
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
  }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void update_map(int x,int y,int n_players)
{
    printf("%d %d\n",x, y);
    paint_pacman(x,y,0,0,0);
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
char** update_fruits(int cols, int lines, char** board)
{
  srand(time(NULL));
  int i = 0, l = 0, c = 0, r= 0;
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
  return board;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
