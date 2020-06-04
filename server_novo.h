#include <time.h>
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


typedef struct Player
{
  int coord[2];
  int last_coord[2];
  int r,g,b;
  int type;
}Player;

typedef struct Player_ID
{
  int sock;
  int id;
  Player pacman;
  Player monster;
  struct Player_ID *next;
}Player_ID;




void server_start(int sock_fd);
char** initialize_map(int *cols, int *lines, int *n_playersmax);
Player set_info(int colour[3],int type);
void *game(void*);
int check_interaction(int coord[2], int last_coord[2], int type);
void send_info(Player_ID *node_send);
void random_coord(int *x, int *y);
char** initialize_fruits(int cols, int lines,int n_players, char** board);
Player_ID *create_node();
void disconnect_player();
void insert_node(Player_ID *pnode);
void remove_node(int id);
Player_ID *insert_player(int sock, int id,int colour[3]);
