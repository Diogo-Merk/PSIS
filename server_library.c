#ifndef SERVER_LIBRARY_H
#include "server_novo.h"
#endif

struct sockaddr_in local_addr,client_addr;
int n_players,n_lines,n_cols;
char **board;
Player_ID *all_players;

void server_start(int sock_fd)
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
char** initialize_map(int *cols, int *lines,int *n_playersmax)
{
  int n_walls=0;
  //Map file
  FILE *map;

  map = fopen("board.txt", "r");

  fscanf(map,"%d %d",&n_cols,&n_lines);
  fgetc(map);
  board = malloc(sizeof(char *) * n_cols);
  for ( int i = 0 ; i < n_cols; i++)
  {
    board[i] = malloc (sizeof(char) * (n_lines));
  }

  //Receber posições das paredes
  for(int i=0;i<n_lines;i++)
  {
    for(int j=0;j<n_cols;j++)
    {
      board[j][i]=fgetc(map);
    }
    fgetc(map);
  }

  for(int x=0;x<n_cols;x++)
  {
    for(int y=0;y<n_lines;y++)
    {
      //printf("|>%c<|",board[y][x]);
      if(board[x][y] == 'B')
      {
        n_walls++;
      }
    }
    //printf("\n");
  }
  *cols = n_cols;
  *lines = n_lines;
  printf("number walls %d\n",n_walls);
  *n_playersmax = ((n_lines*n_cols)-n_walls)/2;
  return board;
}
//Changes to list format
Player_ID set_info(int *colour, int id,int sock)
{
  Player_ID new_player;
  for(int i=0;i<3;i++)
  {
    //new_player.colour[i]=colour[i];

  }
    new_player.id = id;
    new_player.sock = sock;

  return new_player;
}
void *game(void* client)
{
  int done = 0;
  int coord[2],last_coord[2];
  int *rand;
  SDL_Event event;
  Player_ID player = *(Player_ID*) client;
  n_players = player.n_players;
  rand = random_coord();
  coord[0] = rand[0];
  coord[1] = rand[1];
  write(player.sock,&coord,sizeof(coord));

  while(!done)
  {
    if(read(player.sock,&coord,sizeof(coord))>0)
    {
      printf("recieved coordinates: %d %d\n",coord[0],coord[1]);
      int resp = check_interaction(coord, last_coord);
      SDL_PollEvent(&event);
      //May go to switch case
      if(event.type == SDL_QUIT)
        done = SDL_TRUE;
      switch (resp)
      {
        case 1:
          write(player.sock,&last_coord,sizeof(last_coord));
          break;
        case 2:
          last_coord[0]=last_coord[0]+1;
          write(player.sock,&last_coord,sizeof(last_coord));
          break;
        case 3:
          last_coord[0]=last_coord[0]-1;
          write(player.sock,&last_coord,sizeof(last_coord));
          break;
        case 4:
          last_coord[1]=last_coord[1]+1;
          write(player.sock,&last_coord,sizeof(last_coord));
          break;
        case 5:
          last_coord[1]=last_coord[1]-1;
          write(player.sock,&last_coord,sizeof(last_coord));
          break;
        case 69:
          write(player.sock,&coord,sizeof(coord));
          break;

        default:
          break;
      }
      last_coord[0] = coord[0];
      last_coord[1] = coord[1];
      //draw interactions server side and
      //use send_info to send shit to clients
    }
  }
  pthread_exit(NULL);
}
void send_info()
{
  printf("fuck you\n");
}
int check_interaction(int coord[2], int last_coord[2])
{
    int i=coord[0];
    int j=coord[1];
    int x=last_coord[0];
    int y=last_coord[1];
    if (i < 0 || j < 0 || i >= n_cols || j >= n_lines)
    {
      if (x>i && board[x+1][y] != 'B')
      {
        return 2;
      }
      else if(x<i  && board[x-1][y] != 'B')
      {
        return 3;
      }
      else if(y>j  && board[x][y+1] != 'B')
      {
        return 4;
      }
      else if(y<j  && board[x][y-1] != 'B')
      {
        return 5;
      }
      else
      {
        return 1;
      }
    }
    else if(board[i][j] == 'B')
    {
      if (x+1 < n_cols)
      {
        if (x>i && board[x+1][y] != 'B')
          return 2;
      }

      if(x-1 >= 0)
      {
        if(x<i  && board[x-1][y] != 'B')
          return 3;
      }

      if(y+1 < n_lines)
      {
        if(y>j  && board[x][y+1] != 'B')
          return 4;
      }
      if(y-1 >= 0)
      {
        if(y<j  && board[x][y-1] != 'B')
          return 5;
      }

      return 1;
    }
    else if(board[i][j] == ' ')
      return 69;
}
int *random_coord()
{
  int *coord;
  coord = malloc(sizeof(int)*2);
  coord[0] = random()%n_cols;
  coord[1] = random()%n_lines;
  if(board[coord[0]][coord[1]] == 'B')
  {
    coord = random_coord();
  }
  return coord;
}
