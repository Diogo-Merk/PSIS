#ifndef SERVER_LIBRARY_H
#include "client.h"
#endif

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void connect_server(char ip_addr[MAXIP],int port,struct sockaddr_in local_addr,struct sockaddr_in server_addr, int sock_fd)
{
  if(sock_fd == -1)
  {
    perror("socket: ");
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_aton(ip_addr,&server_addr.sin_addr);

  if(connect(sock_fd,(const struct sockaddr *)&server_addr,sizeof(server_addr)) == -1)
  {
    printf("Error connecting\n");
    exit(-1);
  }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void initialize_map(int n_cols, int n_lines, char **board_geral)
{
  create_board_window(n_cols, n_lines);
  //Preencher paredes
  for(int x=0;x<n_cols;x++)
  {
    for(int y=0;y<n_lines;y++)
    {
      if(board_geral[x][y] == 'B')
      {
        paint_brick(x,y);
      }
      if(board_geral[x][y] == 'L')
      {
        paint_lemon(x,y);
      }
      if(board_geral[x][y] == 'C')
      {
        paint_cherry(x,y);
      }
    }
  }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void update_map(Player pacman,Player monster,int n_players)
{
  if(pacman.last_coord[0] != -1)
    clear_place(pacman.last_coord[0],pacman.last_coord[1]);
  if(monster.last_coord[0] != -1)
    clear_place(monster.last_coord[0],monster.last_coord[1]);

  paint_pacman(pacman.coord[0], pacman.coord[1],255,0,0);
  paint_monster(monster.coord[0], monster.coord[1],0,255,0);
}
