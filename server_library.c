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
  board = malloc(sizeof(char *) * n_lines);
  for ( int i = 0 ; i < n_lines; i++)
  {
    board[i] = malloc (sizeof(char) * (n_cols+1));
  }

  //Receber posições das paredes
  for(int i=0;i<n_lines;i++)
  {
    for(int j=0;j<n_cols+1;j++)
    {
      board[i][j]=fgetc(map);
    }
  }
  for(int y=0;y<n_lines;y++)
  {
    for(int x=0;x<n_cols+1;x++)
    {
      //printf("|>%c<|",board[y][x]);
      if(board[y][x] == 'B')
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
  int coord[2];
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
      int resp = check_interaction(coord);
      SDL_PollEvent(&event);
      //May go to switch case
      if(event.type == SDL_QUIT)
        done = SDL_TRUE;
      switch (resp)
      {
        case 1:
          //ingnores plays
          break;
        case 2:
          write(player.sock,&coord,sizeof(coord));
          break;

        default:
          break;
      }

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
int check_interaction(int coord[2])
{
    int i=coord[0];
    int j=coord[1];
    if(board[i][j] == 'B')
      return 1;
    else if(board[i][j] == ' ')
      return 2;
}
int *random_coord()
{
  int *coord;
  coord = malloc(sizeof(int)*2);
  coord[0] = random()%n_lines;
  coord[1] = random()%n_cols;
  if(board[coord[0]][coord[1]] == 'B')
  {
    coord = random_coord();
  }
  return coord;
}
