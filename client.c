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

  //Game Variables
  SDL_Event event;
  int done = 0;
  int cols,lines,n_players;
  char **board_geral;
  int pac_horizontal_move = 0, pac_vertical_move = 0, mon_horizontal_move = 0, mon_vertical_move = 0, xaux = 0, yaux = 0, mouse_x = 0, mouse_y = 0, one_tapmon=0, one_tappac=0;
  int coord[2];
  int last_coord[2];
  last_coord[0] = -1;

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

  read(sock_fd,&cols,sizeof(int));
  read(sock_fd,&lines,sizeof(int));
  printf("%d %d\n",lines,cols );
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

  //Game loop
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
      //printf("recieved %d %d\n",coord[0],coord[1] );
      update_map(coord[0],coord[1],last_coord[0],last_coord[1],n_players);
      last_coord[0] = coord[0];
      last_coord[1] = coord[1];
      mon_horizontal_move = 0;
      mon_vertical_move = 0;
      pac_horizontal_move = 0;
      pac_vertical_move = 0;
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
                if (one_tapmon==0)
                  mon_horizontal_move = -1;
                one_tapmon = 1;
                break;
              case SDLK_RIGHT:
                if (one_tapmon==0)
                  mon_horizontal_move = 1;
                one_tapmon = 1;
                break;
              case SDLK_UP:
                if(one_tapmon==0)
                  mon_vertical_move = -1;
                one_tapmon=1;
                break;
              case SDLK_DOWN:
                if(one_tapmon==0)
                  mon_vertical_move = 1;
                one_tapmon=1;
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
              mon_horizontal_move = 0;
              one_tapmon=0;
              break;
            case SDLK_RIGHT:
              mon_horizontal_move = 0;
              one_tapmon=0;
              break;
            case SDLK_UP:
              mon_vertical_move = 0;
              one_tapmon=0;
              break;
            case SDLK_DOWN:
              mon_vertical_move = 0;
              one_tapmon=0;
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
                SDL_GetMouseState(&mouse_x, &mouse_y);
                get_board_place(mouse_x,mouse_y, &xaux, &yaux);
                xaux=xaux-coord[0];
                yaux=yaux-coord[1];
                if (xaux>0 && yaux ==0)
                {
                  if (one_tappac==0)
                    pac_horizontal_move = 1;
                  one_tappac=1;
                }
                if (xaux<0 && yaux ==0)
                {
                  if (one_tappac==0)
                    pac_horizontal_move = -1;
                  one_tappac=1;
                }
                if (yaux>0 && xaux==0)
                {
                  if (one_tappac==0)
                    pac_vertical_move = 1;
                  one_tappac=1;
                }
                if (yaux<0 && xaux==0)
                {
                  if (one_tappac==0)
                    pac_vertical_move = -1;
                  one_tappac=1;
                }
              }
          break;
          /* Look for letting go of a key */
          case SDL_MOUSEBUTTONUP:
            /* Check the SDLKey values and zero the movemnet when necessary */
            if(event.button.button == SDL_BUTTON_LEFT)
            {
              pac_horizontal_move = 0;
              pac_vertical_move = 0;
              one_tappac=0;
            }
            break;

          default:
            break;
        }
      //Update position
      coord[0] += mon_horizontal_move;
      coord[1] += mon_vertical_move;
      //coord[0] += pac_horizontal_move;
      //coord[1] += pac_vertical_move;

      //Send info to server
      write(sock_fd,&coord,sizeof(coord));
      coord[0] = last_coord[0];
      coord[1] = last_coord[1];
    }
  }
  close_board_windows();
  exit(0);
}
