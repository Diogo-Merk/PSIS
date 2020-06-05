#ifndef SERVER_LIBRARY_H
#include "server_novo.h"
#endif

struct sockaddr_in local_addr,client_addr;
int n_players=0,n_lines,n_cols,client_exit,n_frutas;
char **board;
Player_ID *head = NULL;
Fruta_list *headf=NULL;
pthread_mutex_t mutex;
pthread_mutex_t **movement;

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
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
char** initialize_map(int *cols, int *lines,int *n_playersmax)
{
  int n_walls=0;
  //Map file
  FILE *map;

  map = fopen("board.txt", "r");

  fscanf(map,"%d %d",&n_cols,&n_lines);
  fgetc(map);
  board = malloc(sizeof(char *) * n_cols);
  for ( int i = 0 ; i < n_cols; i++)
  {
    board[i] = malloc (sizeof(char) * (n_lines));
  }

  //Receber posições das paredes
  for(int i=0;i<n_lines;i++)
  {
    for(int j=0;j<n_cols;j++)
    {
      board[j][i]=fgetc(map);
    }
    fgetc(map);
  }

  for(int x=0;x<n_cols;x++)
  {
    for(int y=0;y<n_lines;y++)
    {
      //printf("|>%c<|",board[y][x]);
      if(board[x][y] == 'B')
      {
        n_walls++;
      }
    }
    //printf("\n");
  }
  *cols = n_cols;
  *lines = n_lines;
  *n_playersmax = ((n_lines*n_cols)-n_walls)/2;

  movement = (pthread_mutex_t **)malloc(sizeof(pthread_mutex_t *) * n_cols);
  for ( int i = 0 ; i < n_cols; i++)
  {
    movement[i] = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * (n_lines));
  }

  return board;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Changes to list format
Player set_info(int colour[3],int type)
{
  Player new_player;
  new_player.r = colour[0];
  new_player.g = colour[1];
  new_player.b = colour[2];
  new_player.type = type;

  return new_player;
}
void disconnect_player()
{
  n_players = n_players-1;
  return;
}

//LIST OPERATIONS
Player_ID *create_node()
{
  Player_ID *node;
  return node = (Player_ID*) malloc(sizeof(Player_ID));
}
void insert_node(Player_ID *pnode)
{
  Player_ID *aux = head;
  if(head == NULL)
  {
    head = pnode;
  }
  else
  {
    while(aux->next != NULL)
    {
      aux = aux->next;
    }
    aux->next = pnode;
  }
}
void remove_node(int id)
{
  Player_ID *prev,*temp;
  temp = head;
  disconnect_player();
  if(temp != NULL && temp->id == id)
  {
    head = temp->next;
    free(temp);
    return;
  }
  while(temp != NULL && temp->id != id)
  {
    prev = temp;
    temp = temp->next;
  }
  if(temp == NULL)
    return;
  prev->next = temp->next;
  free(temp);
}
Player_ID *insert_player(int sock, int id,int colour[3])
{
  Player_ID *new = create_node();
  new->sock = sock;
  new-> id = id;
  new->pacman = set_info(colour,1);
  new->monster = set_info(colour,0);
  new->next = NULL;
  insert_node(new);
  n_players++;
  return new;
}
Player_ID *search_node(int x, int y,int type,int xnew, int ynew, int id)
{
    Player_ID *aux = head;
    while(aux != NULL)
    {
      if(type == 1)
      {
        printf("search pacman-> %d %d\n",aux->pacman.coord[0],aux->pacman.coord[1]);
        if(aux->pacman.coord[0] == x && aux->pacman.coord[1] == y && aux->id != id)
        {
          aux->pacman.coord[0] = xnew;
          aux->pacman.coord[1] = ynew;
          aux->pacman.last_coord[0] = xnew;
          aux->pacman.last_coord[1] = ynew;
          return aux;
        }
      }
      else if(type == 0)
      {
        if(aux->monster.coord[0] == x && aux->monster.coord[1] == y && aux->id != id)
        {
          aux->monster.coord[0] = xnew;
          aux->monster.coord[1] = ynew;
          aux->monster.last_coord[0] = xnew;
          aux->monster.last_coord[1] = ynew;
          return aux;
        }
      }
      aux = aux->next;

    }
    printf("player not found\n");
    return NULL;
}
Fruta_list *create_node_fruta()
{
  Fruta_list *node;
  return node = (Fruta_list*) malloc(sizeof(Fruta_list));
}
void insert_node_fruta(Fruta_list *pnode)
{
  Fruta_list *aux = headf;
  if(headf == NULL)
  {
    headf = pnode;
  }
  else
  {
    while(aux->next != NULL)
    {
      aux = aux->next;
    }
    aux->next = pnode;
  }
}
void remove_node_fruta()
{
  Fruta_list *prev,*temp;
  temp = headf;
  if(temp != NULL)
  {
    headf = temp->next;
    free(temp);
    return;
  }
  while(temp != NULL)
  {
    prev = temp;
    temp = temp->next;
  }
  if(temp == NULL)
    return;
  prev->next = temp->next;
  free(temp);
}
Fruta_list *insert_fruit(int x, int y, int tipo)
{
  Fruta_list *new = create_node_fruta();
  new->fruta.x=x;
  new->fruta.y=y;
  new->fruta.tipo=tipo;
  new->next = NULL;
  insert_node_fruta(new);
  return new;
}
void search_node_fruta(int x, int y,int xnew, int ynew)
{
    Fruta_list *aux = headf;
    while(aux != NULL)
    {
      if (aux->fruta.x==x &&aux->fruta.y==y)
      {
        aux->fruta.x=xnew;
        aux->fruta.y=ynew;
        return;
      }
      aux = aux->next;

    }
    printf("fruit not found\n");
    return;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Thread
void *game(void* client)
{
  int done = 0, a=0, b=0;
  SDL_Event event;
  int resp=-1, respm=-1;
  Player_ID *player = *(Player_ID**) client;
  Player_ID *other_player;
  random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
  player->pacman.last_coord[0]=player->pacman.coord[0];
  player->pacman.last_coord[1]=player->pacman.coord[1];
  board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
  random_coord(&player->monster.coord[0], &player->monster.coord[1]);
  player->monster.last_coord[0]=player->monster.coord[0];
  player->monster.last_coord[1]=player->monster.coord[1];
  board[player->monster.coord[0]][player->monster.coord[1]]='M';
  send_info(player);

  while(!done)
  {
    read(player->sock,&player->pacman,sizeof(player->pacman));
    read(player->sock,&player->monster,sizeof(player->monster));
    if(player->pacman.coord[0] == -1 && player->pacman.coord[1] == -1)
    {
      pthread_mutex_lock(&mutex);
      client_exit = 1;
      board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
      board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
      write(player->sock,&client_exit,sizeof(int));
      client_exit = 0;
      send_info(player);
      remove_node(player->id);
      pthread_mutex_unlock(&mutex);
      pthread_exit(NULL);
    }
    SDL_PollEvent(&event);
    //May go to switch case
    if(event.type == SDL_QUIT)
      done = SDL_TRUE;
    pthread_mutex_lock(&movement[player->pacman.coord[0]][player->pacman.coord[1]]);
    resp = check_interaction(player->pacman.coord, player->pacman.last_coord, player->pacman.type);
    printf("resp: %d\n", resp);
    switch (resp)
    {
      //Ficar parado
      case 1:
        player->pacman.coord[0] = player->pacman.last_coord[0];
        player->pacman.coord[1] = player->pacman.last_coord[1];
        break;
      //Knockback para a direita
      case 2:
        board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
        player->pacman.coord[0] = player->pacman.last_coord[0]+1;
        player->pacman.coord[1] = player->pacman.last_coord[1];
        if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='M'&&player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->monster.coord[0] = player->pacman.last_coord[0];
          player->monster.coord[1] = player->pacman.last_coord[1];
          player->monster.last_coord[0] = player->monster.coord[0];
          player->monster.last_coord[1] = player->monster.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='P')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]='P';
          other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],1, player->pacman.last_coord[0], player->pacman.last_coord[1],player->id);
          player->pacman.last_coord[0]=player->pacman.coord[0];
          player->pacman.last_coord[1]=player->pacman.coord[1];
          send_info(other_player);
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='M')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
          random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        }
        else
        {
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        }
        send_info(player);
        break;
      //Knockback para a esquerda
      case 3:
        board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
        player->pacman.coord[0] = player->pacman.last_coord[0]-1;
        player->pacman.coord[1] = player->pacman.last_coord[1];
        if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='M'&&player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->monster.coord[0] = player->pacman.last_coord[0];
          player->monster.coord[1] = player->pacman.last_coord[1];
          player->monster.last_coord[0] = player->monster.coord[0];
          player->monster.last_coord[1] = player->monster.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='P')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]='P';
          other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],1, player->pacman.last_coord[0], player->pacman.last_coord[1],player->id);
          player->pacman.last_coord[0]=player->pacman.coord[0];
          player->pacman.last_coord[1]=player->pacman.coord[1];
          send_info(other_player);
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='M')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
          random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        }
        else
        {
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        }
        send_info(player);
        break;
      //Knockback para baixo
      case 4:
        board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
        player->pacman.coord[0] = player->pacman.last_coord[0];
        player->pacman.coord[1] = player->pacman.last_coord[1]+1;
        if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='M'&&player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->monster.coord[0] = player->pacman.last_coord[0];
          player->monster.coord[1] = player->pacman.last_coord[1];
          player->monster.last_coord[0] = player->monster.coord[0];
          player->monster.last_coord[1] = player->monster.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='P')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]='P';
          other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],1, player->pacman.last_coord[0], player->pacman.last_coord[1],player->id);
          player->pacman.last_coord[0]=player->pacman.coord[0];
          player->pacman.last_coord[1]=player->pacman.coord[1];
          send_info(other_player);
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='M')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
          random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        }
        else
        {
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        }
        send_info(player);
        break;
      //Knockback para cima
      case 5:
        board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
        player->pacman.coord[0] = player->pacman.last_coord[0];
        player->pacman.coord[1] = player->pacman.last_coord[1]-1;
        if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='M'&&player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->monster.coord[0] = player->pacman.last_coord[0];
          player->monster.coord[1] = player->pacman.last_coord[1];
          player->monster.last_coord[0] = player->monster.coord[0];
          player->monster.last_coord[1] = player->monster.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='P')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]='P';
          other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],1, player->pacman.last_coord[0], player->pacman.last_coord[1],player->id);
          player->pacman.last_coord[0]=player->pacman.coord[0];
          player->pacman.last_coord[1]=player->pacman.coord[1];
          send_info(other_player);
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='M')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
          random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        }
        else
        {
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        }
        send_info(player);
        break;
      //Fruta
      case 6:
        board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
        board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        random_coord(&a, &b);
        search_node_fruta(player->pacman.coord[0],player->pacman.coord[1],a,b);
        board[a][b]='L';
        send_info(player);
        break;
      //Interaçoes entre personagens
      case 7:
        if (player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->monster.coord[0] = player->pacman.last_coord[0];
          player->monster.coord[1] = player->pacman.last_coord[1];
          player->monster.last_coord[0] = player->monster.coord[0];
          player->monster.last_coord[1] = player->monster.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
          send_info(player);
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='P')
        {
          other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],1, player->pacman.last_coord[0], player->pacman.last_coord[1],player->id);
          send_info(player);
          send_info(other_player);
        }
        else if(board[player->pacman.coord[0]][player->pacman.coord[1]]=='M')
        {
          board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
          random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          send_info(player);
        }
        break;
      //Ficar parado por variadas razoes
      case 8:
        player->pacman.coord[0] = player->pacman.last_coord[0];
        player->pacman.coord[1] = player->pacman.last_coord[1];
        send_info(player);
        break;
      //Andar para espaço livre
      case 0:
        board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
        board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
        send_info(player);
        break;

      default:
        break;
    }
    pthread_mutex_unlock(&movement[player->pacman.coord[0]][player->pacman.coord[1]]);
    pthread_mutex_lock(&movement[player->monster.coord[0]][player->monster.coord[1]]);
    respm = check_interaction(player->monster.coord, player->monster.last_coord, player->monster.type);
    printf("respm: %d\n", respm);
    switch (respm)
    {
      //Ficar parado pq n houve movimento
      case 1:
        player->monster.coord[0] = player->monster.last_coord[0];
        player->monster.coord[1] = player->monster.last_coord[1];
        break;
      //Knockback para a direita
      case 2:
        board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
        player->monster.coord[0] = player->monster.last_coord[0]+1;
        player->monster.coord[1] = player->monster.last_coord[1];
        if (board[player->monster.coord[0]][player->monster.coord[1]]=='P'&&player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->pacman.coord[0] = player->monster.last_coord[0];
          player->pacman.coord[1] = player->monster.last_coord[1];
          player->pacman.last_coord[0] = player->pacman.coord[0];
          player->pacman.last_coord[1] = player->pacman.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='M')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]='M';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],0, player->monster.last_coord[0], player->monster.last_coord[1],player->id);
          player->monster.last_coord[0]=player->monster.coord[0];
          player->monster.last_coord[1]=player->monster.coord[1];
          send_info(other_player);
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='P')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
          random_coord(&a, &b);
          board[a][b]='P';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],1, a, b,player->id);
          send_info(other_player);
        }
        else
        {
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }

        send_info(player);
        break;
      //Knockback para a esquerda
      case 3:
        board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
        player->monster.coord[0] = player->monster.last_coord[0]-1;
        player->monster.coord[1] = player->monster.last_coord[1];
        if (board[player->monster.coord[0]][player->monster.coord[1]]=='P'&&player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->pacman.coord[0] = player->monster.last_coord[0];
          player->pacman.coord[1] = player->monster.last_coord[1];
          player->pacman.last_coord[0] = player->pacman.coord[0];
          player->pacman.last_coord[1] = player->pacman.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='M')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]='M';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],0, player->monster.last_coord[0], player->monster.last_coord[1],player->id);
          player->monster.last_coord[0]=player->monster.coord[0];
          player->monster.last_coord[1]=player->monster.coord[1];
          send_info(other_player);
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='P')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
          random_coord(&a, &b);
          board[a][b]='P';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],1, a, b,player->id);
          send_info(other_player);
        }
        else
        {
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        send_info(player);
        break;
      //Knockback para baixo
      case 4:
        board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
        player->monster.coord[0] = player->monster.last_coord[0];
        player->monster.coord[1] = player->monster.last_coord[1]+1;
        if (board[player->monster.coord[0]][player->monster.coord[1]]=='P'&&player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->pacman.coord[0] = player->monster.last_coord[0];
          player->pacman.coord[1] = player->monster.last_coord[1];
          player->pacman.last_coord[0] = player->pacman.coord[0];
          player->pacman.last_coord[1] = player->pacman.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='M')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]='M';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],0, player->monster.last_coord[0], player->monster.last_coord[1],player->id);
          player->monster.last_coord[0]=player->monster.coord[0];
          player->monster.last_coord[1]=player->monster.coord[1];
          send_info(other_player);
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='P')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
          random_coord(&a, &b);
          board[a][b]='P';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],1, a, b,player->id);
          send_info(other_player);
        }
        else
        {
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        send_info(player);
        break;
      //Knockback para cima
      case 5:
        board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
        player->monster.coord[0] = player->monster.last_coord[0];
        player->monster.coord[1] = player->monster.last_coord[1]-1;
        if (board[player->monster.coord[0]][player->monster.coord[1]]=='P'&&player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->pacman.coord[0] = player->monster.last_coord[0];
          player->pacman.coord[1] = player->monster.last_coord[1];
          player->pacman.last_coord[0] = player->pacman.coord[0];
          player->pacman.last_coord[1] = player->pacman.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='M')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]='M';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],0, player->monster.last_coord[0], player->monster.last_coord[1],player->id);
          player->monster.last_coord[0]=player->monster.coord[0];
          player->monster.last_coord[1]=player->monster.coord[1];
          send_info(other_player);
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='P')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
          random_coord(&a, &b);
          board[a][b]='P';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],1, a, b,player->id);
          send_info(other_player);
        }
        else
        {
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
        }

        send_info(player);
        break;
      //Fruta
      case 6:
        board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
        board[player->monster.coord[0]][player->monster.coord[1]]='M';
        random_coord(&a, &b);
        search_node_fruta(player->monster.coord[0],player->monster.coord[1],a,b);
        board[a][b]='L';
        send_info(player);
        break;
      case 7:
        if (player->monster.coord[0]==player->pacman.coord[0]&&player->monster.coord[1]==player->pacman.coord[1])
        {
          player->pacman.coord[0] = player->monster.last_coord[0];
          player->pacman.coord[1] = player->monster.last_coord[1];
          player->pacman.last_coord[0] = player->pacman.coord[0];
          player->pacman.last_coord[1] = player->pacman.coord[1];
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
          send_info(player);
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='M')
        {
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],0, player->monster.last_coord[0], player->monster.last_coord[1],player->id);
          send_info(player);
          send_info(other_player);
        }
        else if(board[player->monster.coord[0]][player->monster.coord[1]]=='P')
        {
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
          board[player->monster.coord[0]][player->monster.coord[1]]='M';
          random_coord(&a, &b);
          board[a][b]='P';
          other_player = search_node(player->monster.coord[0], player->monster.coord[1],1, a, b,player->id);
          send_info(player);
          send_info(other_player);
        }
        break;
      //Ficar parado por variadas razoes
      case 8:
        player->monster.coord[0] = player->monster.last_coord[0];
        player->monster.coord[1] = player->monster.last_coord[1];
        send_info(player);
        break;
      //Andar para espaço livre
      case 0:
        board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
        board[player->monster.coord[0]][player->monster.coord[1]]='M';
        send_info(player);
        break;

      default:
        break;
    }
    pthread_mutex_unlock(&movement[player->monster.coord[0]][player->monster.coord[1]]);
    for (int i = 0; i < n_lines; i++) {
      for (int j = 0; j < n_cols ; j++) {
        printf("%c ", board[j][i]);
      }
      printf("\n");
    }
    printf("############\n");

    //draw interactions server side and
    //use send_info to send shit to clients
  }
  pthread_exit(NULL);
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void send_info(Player_ID *node_send)
{
  int local = 0;
  Player_ID *aux = head;
  Fruta_list *aux2 = headf;
  while(aux != NULL)
  {
    write(aux->sock,&client_exit,sizeof(int));
    write(aux->sock,&node_send->id,sizeof(int));
    write(aux->sock,&node_send->pacman,sizeof(aux->pacman));
    write(aux->sock,&node_send->monster,sizeof(aux->monster));
    write(aux->sock,&n_frutas,sizeof(int));
    for (int i = 0; i < n_frutas; i++)
    {
      write(aux->sock,&aux2->fruta,sizeof(aux2->fruta));
      aux2 = aux2->next;
    }
    aux = aux->next;
  }
  return;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
int check_interaction(int coord[2], int last_coord[2], int type)
{
    int i=coord[0];
    int j=coord[1];
    int x=last_coord[0];
    int y=last_coord[1];
    if (i < 0 || j < 0 || i >= n_cols || j >= n_lines)
    {
      if (x>i && board[x+1][y] != 'B')
      {
        return 2;
      }
      else if(x<i  && board[x-1][y] != 'B')
      {
        return 3;
      }
      else if(y>j  && board[x][y+1] != 'B')
      {
        return 4;
      }
      else if(y<j  && board[x][y-1] != 'B')
      {
        return 5;
      }
      else
      {
        return 8;
      }
    }
    else if(board[i][j] == 'B')
    {
      if (x+1 < n_cols)
      {
        if (x>i && board[x+1][y] != 'B')
        {
          return 2;
        }
      }

      if(x-1 >= 0)
      {
        if(x<i  && board[x-1][y] != 'B')
          return 3;
      }

      if(y+1 < n_lines)
      {
        if(y>j  && board[x][y+1] != 'B')
          return 4;
      }
      if(y-1 >= 0)
      {
        if(y<j  && board[x][y-1] != 'B')
          return 5;
      }

      return 8;
    }
    else if (board[i][j]=='C'||board[i][j]=='L')
    {
      return 6;
    }
    else if ((board[i][j]=='M'||board[i][j]=='P')&& (x != i || y!=j))
    {
      return 7;
    }
    else if (x == i && y==j)
    {
      return 1;
    }
    else
    {
      return 0;
    }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void random_coord(int *x, int *y)
{
  srand(time(NULL));
  *x = rand()%n_cols;
  *y = rand()%n_lines;
  while(board[*x][*y] == 'B'||board[*x][*y] == 'P'||board[*x][*y] == 'M'||board[*x][*y] == 'L'||board[*x][*y] == 'C')
  {
    *x = rand()%n_cols;
    *y = rand()%n_lines;
  }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
char** initialize_fruits(int cols, int lines,int n_players, char** board,int *n_fruits)
{
  srand(time(NULL));
  int i = *n_fruits, l = 0, c = 0, r= 0;
  while (i<((n_players-1)*2))
  {
    l = rand() % lines;
    c = rand() % cols;
    r = rand() % 1;
    if (board[c][l] == ' ')
    {
      if (r==1)
      {
        board[c][l] = 'C';
        insert_node_fruta(insert_fruit(c,l,0));
        n_fruits++;
        i++;
      }
      else
      {
        insert_node_fruta(insert_fruit(c,l,1));
        board[c][l] = 'L';
        n_fruits++;
        i++;
      }
    }
  }
  n_frutas=*n_fruits;
  return board;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
char** update_fruits(int cols, int lines, char** board)
{
  srand(time(NULL));
  int i = 0, l = 0, c = 0, r= 0;
  l = rand() % lines;
  c = rand() % cols;
  r = rand() % 1;
  if (board[c][l] == ' ')
  {
    if (r==1)
    {
      board[c][l] = 'C';
      i++;
    }
    else
    {
      board[c][l] = 'L';
      i++;
    }
  }
  return board;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
int get_n_players()
{
  return n_players;
}
