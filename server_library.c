#include "server_novo.h"

struct sockaddr_in local_addr,client_addr;
int n_players;
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
  int n_lines, n_cols,n_walls;
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
  create_board_window(n_cols, n_lines);

  //Receber posições das paredes
  for(int i=0;i<n_lines;i++)
  {
    for(int j=0;j<n_cols+1;j++)
    {
      board[i][j]=fgetc(map);
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
//Changes to list format
Player_ID set_info(int *colour, int id)
{
  Player_ID new_player;
  for(int i=0;i<3;i++)
  new_player.colour[i]=colour[i];
  new_player.id = id;

  return new_player;
}
void *game(void* client)
{
  int done = 0;
  int coord[2];
  SDL_Event event;
  Player_ID player = *(Player_ID*) client;
  n_players = player.n_players;

  while(!done)
  {
    if(read(player.sock,&coord,sizeof(coord))>0)
    {
      printf("recieved coordinates: %d %d\n",coord[0],coord[1]);

      while(SDL_PollEvent(&event))
      {
        if(event.type == SDL_QUIT)
          done = SDL_TRUE;
      }
    }
  }
  pthread_exit(NULL);
}
void send_info()
{

}
