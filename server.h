#include <SDL2/SDL.h>

typedef struct Player_ID
{
  int x;
  int y;
  int ID;
};
void server_start(struct sockaddr_in local_addr, int sock_fd);
void initialize_map();
