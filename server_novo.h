#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include "UI_library.h"

typedef struct Player_ID
{
  int sock;
  int id;
  int colour[3];
  int n_players;
}Player_ID;

void server_start(int sock_fd);
char** initialize_map(int *cols, int *lines, int *n_playersmax);
Player_ID set_info(int *colour, int id);
void *game(void*);
int check_interaction(int *coord);
void send_info();
