#include <SDL2/SDL.h>

typedef struct Player_ID
{
  int x,y;
  int r,g,b;
  int ID;
}Player_ID;

typedef struct new_connect
{
  //server side
  int sock_fd;
  struct sockaddr_in local_addr;
  struct sockaddr_in client_addr;
  socklen_t addr_len;
  //game side
  int cols;
  int lines;
  char **board_geral;
}new_connect;



void server_start(struct sockaddr_in local_addr, int sock_fd);
char** initialize_map(int *cols, int *lines, int *n_playersmax);
char** initialize_fruits(int cols, int lines,int n_players, char** board);
void new_connections(int client_fd);
