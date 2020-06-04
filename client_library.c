#ifndef SERVER_LIBRARY_H
#include "client.h"
#endif

int done = 0;
Player pacman_local;
Player monster_local;
int pac_horizontal_move = 0, pac_vertical_move = 0, mon_horizontal_move = 0, mon_vertical_move = 0, xaux = 0, yaux = 0, mouse_x = 0, mouse_y = 0, one_tapmon=0, one_tappac=0;
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
  //Preencher paredes
  for(int x=0;x<n_cols;x++)
  {
    for(int y=0;y<n_lines;y++)
    {
      if(board_geral[x][y] == 'B')
      {
        paint_brick(x,y);
      }
      if(board_geral[x][y] == 'L')
      {
        paint_lemon(x,y);
      }
      if(board_geral[x][y] == 'C')
      {
        paint_cherry(x,y);
      }
    }
  }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void update_map(Player pacman,Player monster)
{
  if(pacman.last_coord[0] != -1)
    clear_place(pacman.last_coord[0],pacman.last_coord[1]);
  if(monster.last_coord[0] != -1)
    clear_place(monster.last_coord[0],monster.last_coord[1]);
  printf("coordenadas monstro: %d %d\n", monster.coord[0],monster.coord[1]);
  printf("coordenadas pacman: %d %d\n", pacman.coord[0],pacman.coord[1]);
  paint_pacman(pacman.coord[0], pacman.coord[1],255,0,0);
  paint_monster(monster.coord[0], monster.coord[1],0,255,0);
}
void recv_play(int sock_fd,int id)
{
	int current_id;
  Player pacman, monster;
	while(1)
  {
    read(sock_fd,&current_id,sizeof(int));
    read(sock_fd,&pacman,sizeof(pacman));
    read(sock_fd,&monster,sizeof(monster));
    update_map(pacman,monster);
    if(current_id == id)
    {
      pacman_local = pacman;
      monster_local = monster;
    }
	}
}
void *game_loop(void *sock_fd)
{
  SDL_Event event;
  int sock = *((int*) sock_fd);
  while(!done)
  {
    while(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        done = SDL_TRUE;
      }
      pacman_local.last_coord[0] = pacman_local.coord[0];
      pacman_local.last_coord[1] = pacman_local.coord[1];
      monster_local.last_coord[0] = monster_local.coord[0];
      monster_local.last_coord[1] = monster_local.coord[1];
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
            /* Check the SDLKey values and move change the coordps */
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
              /* Check the SDLKey values and move change the coordps */
              if( event.button.button == SDL_BUTTON_LEFT)
              {
                SDL_GetMouseState(&mouse_x, &mouse_y);
                get_board_place(mouse_x,mouse_y, &xaux, &yaux);
                xaux=xaux-pacman_local.coord[0];
                yaux=yaux-pacman_local.coord[1];
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
      monster_local.coord[0] += mon_horizontal_move;
      monster_local.coord[1] += mon_vertical_move;
      pacman_local.coord[0] += pac_horizontal_move;
      pacman_local.coord[1] += pac_vertical_move;
      write(sock,&pacman_local,sizeof(pacman_local));
      write(sock,&monster_local,sizeof(monster_local));
    }
  }
}
void init_vars()
{
  pacman_local.type=1;
  monster_local.type=0;
  pacman_local.last_coord[0] = -1;
  monster_local.last_coord[0] = -1;
  pacman_local.last_coord[1] = -1;
  monster_local.last_coord[1] = -1;
}
