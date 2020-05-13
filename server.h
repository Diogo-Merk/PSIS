#include <SDL2/SDL.h>

typedef struct Player_ID
{
  int x,y;
  int r,g,b;
  int ID;
}Player_ID;

void server_start(struct sockaddr_in local_addr, int sock_fd);
char** initialize_map(int *cols, int *lines, int *n_playersmax);
