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

typedef struct Fruta
{
  int x, y, tipo;
}Fruta;

typedef struct Fruta_list
{
  Fruta fruta;
  struct Fruta_list *next;
}Fruta_list;




void server_start(int sock_fd);
char** initialize_map(int *cols, int *lines, int *n_playersmax);
Player set_info(int colour[3],int type);
void *game(void*);
int check_interaction(int coord[2], int last_coord[2], int type);
void send_info(Player_ID *node_send);
void random_coord(int *x, int *y);
char** initialize_fruits(int cols, int lines,int n_players, char** board, int *n_fruits);
Player_ID *create_node();
void disconnect_player();
void insert_node(Player_ID *pnode);
void remove_node(int id);
Player_ID *search_node(int x, int y,int type,int xnew, int ynew,int id);
Player_ID *insert_player(int sock, int id,int colour[3]);
int get_n_players();
void remove_node_fruta();
Fruta_list *create_node_fruta();
void insert_node_fruta(Fruta_list *pnode);
Fruta_list *insert_fruit(int x, int y, int tipo);
void search_node_fruta(int x, int y,int xnew, int ynew);
