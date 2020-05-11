#include <SDL2/SDL.h>

typedef struct Player_ID
{
  int x;
  int y;
  int ID;
}Player_ID;

void server_start(struct sockaddr_in local_addr, int sock_fd);
void initialize_map(int *cols, int *lines);
