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
#define MAXIP 20
typedef struct Player
{
  int coord[2];
  int last_coord[2];
  int r,g,b;
  int type;
}Player;
typedef struct Fruta
{
  int x, y, tipo;
}Fruta;



void connect_server(char ip_addr[],int port,struct sockaddr_in local_addr,struct sockaddr_in server_addr, int sock_fd);
void initialize_map(int n_cols, int n_lines, char **board_geral);
void update_map(Player pacman,Player monster, int fruits, int sock);
char** update_fruits(int cols, int lines, char** board);
void recv_play(int sock_fd,int id);
void *game_loop(void* sock_fd);
void init_vars(int r,int g, int b);
