#ifndef SERVER_LIBRARY_H
#include "client.h"
#endif

int done = 0,client_exit = 0, cols=0, lines=0, fruits=0;
Player pacman_local;
Player monster_local;
char **board_geral;
int flag=0,pac_horizontal_move = 0, pac_vertical_move = 0, mon_horizontal_move = 0, mon_vertical_move = 0, xaux = 0, yaux = 0, mouse_x = 0, mouse_y = 0, one_tapmon=0, one_tappac=0;
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
void initialize_map(int n_cols, int n_lines, char **board)
{
  create_board_window(n_cols, n_lines);
  cols=n_cols;
  lines=n_lines;
  //Preencher paredes
  for(int x=0;x<n_cols;x++)
  {
    for(int y=0;y<n_lines;y++)
    {
      if(board[x][y] == 'B')
      {
        fruits++;
        paint_brick(x,y);
      }
      if(board[x][y] == 'L')
      {
        paint_lemon(x,y);
      }
      if(board[x][y] == 'C')
      {
        paint_cherry(x,y);
        fruits++;
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
  if(pacman.coord[0] == -1 && pacman.coord[1] == -1)
  {
      clear_place(monster.coord[0],monster.coord[1]);
      return NULL;
  }
  for (int i = 0; i < cols; i++)
  {
    for (int j = 0; j < lines; j++)
    {
      if (board_geral[i][j]=='L')
      {
        paint_lemon(i,j);
      }
      if (board_geral[i][j]=='C')
      {
        paint_cherry(i,j);
      }
    }
  }
  if (pacman.type==1)
  {
    paint_pacman(pacman.coord[0], pacman.coord[1],pacman.r,pacman.g,pacman.b);
  }
  else
  {
    paint_powerpacman(pacman.coord[0], pacman.coord[1],pacman.r,pacman.g,pacman.b);
  }
  paint_monster(monster.coord[0], monster.coord[1],monster.r,monster.g,monster.b);
}
void recv_play(int sock_fd,int id)
{
	int current_id, i=0, j=0, score;
  Player pacman, monster;
  board_geral = malloc(sizeof(char *) * cols);
  for ( i = 0 ; i < cols; i++)
  {
    board_geral[i] = malloc (sizeof(char) * lines);
  }
	while(1)
  {
    read(sock_fd,&client_exit,sizeof(int));
    if(client_exit == 1)
      break;
    read(sock_fd,&current_id,sizeof(int));
    read(sock_fd,&pacman,sizeof(pacman));
    read(sock_fd,&monster,sizeof(monster));
    for (i = 0; i < cols; i++)
    {
      for (j = 0; j < lines; j++)
      {
        read(sock_fd,&board_geral[i][j],sizeof(char));
      }
    }

    update_map(pacman,monster);
    read(sock_fd,&fruits,sizeof(int));
    read(sock_fd,&score,sizeof(int));
    if(current_id == id)
    {
      pacman_local = pacman;
      monster_local = monster;
    }
    if(flag == 0)
      flag = 1;
	}
  for (i = 0 ; i < cols; i++)
  {
    free(board_geral[i]);
  }
  free(board_geral);
}
void *game_loop(void *sock_fd)
{
  SDL_Event event;
  int sock = *((int*) sock_fd);
  int ordem=0;
  while(!done)
  {
    if(flag == 0)
      continue;
    while(SDL_PollEvent(&event))
    {
      ordem=0;
      if(event.type == SDL_QUIT)
      {
        pacman_local.coord[0] = -1;
        pacman_local.coord[1] = -1;
        write(sock,&pacman_local,sizeof(pacman_local));
        write(sock,&monster_local,sizeof(monster_local));
        done = SDL_TRUE;
        pthread_exit(NULL);
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
                ordem=1;
                one_tapmon = 1;
                break;
              case SDLK_RIGHT:
                if (one_tapmon==0)
                  mon_horizontal_move = 1;
                ordem=1;
                one_tapmon = 1;
                break;
              case SDLK_UP:
                if(one_tapmon==0)
                  mon_vertical_move = -1;
                ordem=1;
                one_tapmon=1;
                break;
              case SDLK_DOWN:
                if(one_tapmon==0)
                  mon_vertical_move = 1;
                ordem=1;
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
              ordem=1;
              mon_horizontal_move = 0;
              one_tapmon=0;
              break;
            case SDLK_RIGHT:
              ordem=1;
              mon_horizontal_move = 0;
              one_tapmon=0;
              break;
            case SDLK_UP:
              ordem=1;
              mon_vertical_move = 0;
              one_tapmon=0;
              break;
            case SDLK_DOWN:
              ordem=1;
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
              ordem=1;
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
            ordem=1;
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
      if (ordem==1)
      {
        write(sock,&pacman_local,sizeof(pacman_local));
        write(sock,&monster_local,sizeof(monster_local));
      }
    }
  }
}
void init_vars(int r,int g, int b)
{
  pacman_local.type=1;
  monster_local.type=0;
  pacman_local.last_coord[0] = -1;
  monster_local.last_coord[0] = -1;
  pacman_local.last_coord[1] = -1;
  monster_local.last_coord[1] = -1;
  pacman_local.r = r;
  pacman_local.g = g;
  pacman_local.b = b;
  monster_local.r = r;
  monster_local.g = g;
  monster_local.b = b;
}
