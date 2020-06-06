#ifndef SERVER_LIBRARY_H
#include "server_novo.h"
#endif

struct sockaddr_in local_addr,client_addr;
int n_players=0, fruits=0, fruits_on_board=0,n_lines,n_cols,client_exit;
char **board;
Player_ID *head = NULL;
pthread_mutex_t mutex;
pthread_mutex_t **movement;
int pacflag = 0, monflag = 0;

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
  fruits_on_board = fruits;
  n_players = n_players-1;
  if(n_players > 0)
    fruits=(n_players-1)*2;
  printf("notboard->%d\nboard->%d\n",fruits,fruits_on_board);
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

int search_type(int x, int y,int id)
{
    Player_ID *aux = head;
    while(aux != NULL)
    {
      printf("pacmanaux:%d %d\n", aux->pacman.coord[0],aux->pacman.coord[1]);
      printf("idaux:%d  typeaux:%d\n", aux->id,aux->pacman.type);
      printf("pacman:%d %d\n", x,y);
      printf("type:%d\n", id);
      if(aux->pacman.coord[0] == x && aux->pacman.coord[1] == y && aux->id != id&&aux->pacman.type>1)
      {
        return 1;
      }
      aux = aux->next;
    }
    return 0;
}
void delete_list()
{
  Player_ID *next;
  while(head != NULL)
  {
    next = head->next;
    free(head);
    head = next;
  }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Thread
void *game(void* client)
{
  int done = 0, a=0, b=0,xlock,ylock;
  SDL_Event event;
  int resp=-1, respm=-1;
  Player_ID *player = *(Player_ID**) client;
  Player_ID *other_player;
  Player playbuffer;
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
    SDL_PollEvent(&event);
    printf("pac flag = %d\n ",pacflag);
    if(pacflag == 0)
    {
      read(player->sock,&player->pacman,sizeof(player->pacman));
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

      if(player->pacman.coord[0]>0 && player->pacman.coord[0]<n_cols && player->pacman.coord[1]>0 && player->pacman.coord[1]<n_lines)
      {
        xlock = player->pacman.coord[0];
        ylock = player->pacman.coord[1];
      }
      pthread_mutex_lock(&movement[xlock][ylock]);
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
            if (player->pacman.type==1)
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
            }
            else
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
              random_coord(&a, &b);
              board[a][b]='M';
              other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],0, a, b,player->id);
              player->pacman.type=  player->pacman.type-1;
              send_info(other_player);
            }
          }
          else if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'||board[player->pacman.coord[0]][player->pacman.coord[1]]=='C')
          {
            board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
            if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='L';
            }
            if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='C'&&fruits_on_board<=fruits)
            {
              random_coord(&a, &b);
              board[a][b]='C';
            }
            if(fruits_on_board>fruits)
              fruits_on_board--;
            board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
            player->pacman.type=player->pacman.type+2;
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
            if (player->pacman.type==1)
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
            }
            else
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
              random_coord(&a, &b);
              board[a][b]='M';
              other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],0, a, b,player->id);
              player->pacman.type=  player->pacman.type-1;
              send_info(other_player);
            }
          }
          else if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'||board[player->pacman.coord[0]][player->pacman.coord[1]]=='C')
          {
            board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
            if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='L';
            }
            if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='C'&&fruits_on_board<=fruits)
            {
              random_coord(&a, &b);
              board[a][b]='C';
            }
            if(fruits_on_board>fruits)
              fruits_on_board--;
            board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
            player->pacman.type=player->pacman.type+2;
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
            if (player->pacman.type==1)
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
            }
            else
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
              random_coord(&a, &b);
              board[a][b]='M';
              other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],0, a, b,player->id);
              player->pacman.type=  player->pacman.type-1;
              send_info(other_player);
            }
          }
          else if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'||board[player->pacman.coord[0]][player->pacman.coord[1]]=='C')
          {
            board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
            if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='L';
            }
            if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='C'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='C';
            }
            if(fruits_on_board>fruits)
              fruits_on_board--;
            board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
            player->pacman.type=player->pacman.type+2;
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
            if (player->pacman.type==1)
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
            }
            else
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
              random_coord(&a, &b);
              board[a][b]='M';
              other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],0, a, b,player->id);
              player->pacman.type=  player->pacman.type-1;
              send_info(other_player);
            }
          }
          else if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'||board[player->pacman.coord[0]][player->pacman.coord[1]]=='C')
          {
            board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
            if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='L';
            }

            if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='C'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='C';
            }
            if(fruits_on_board>fruits)
              fruits_on_board--;
            board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
            player->pacman.type=player->pacman.type+2;
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
          if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='L'&&fruits_on_board<=fruits)
          {
            random_coord(&a, &b);
            board[a][b]='L';
          }
          if (board[player->pacman.coord[0]][player->pacman.coord[1]]=='C'&&fruits_on_board<=fruits)
          {
            random_coord(&a, &b);
            board[a][b]='C';
          }
          if(fruits_on_board>fruits)
            fruits_on_board--;
          board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
          player->pacman.type=player->pacman.type+2;
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
            if (player->pacman.type==1)
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              random_coord(&player->pacman.coord[0], &player->pacman.coord[1]);
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
              send_info(player);
            }
            else
            {
              board[player->pacman.last_coord[0]][player->pacman.last_coord[1]]=' ';
              board[player->pacman.coord[0]][player->pacman.coord[1]]='P';
              random_coord(&a, &b);
              board[a][b]='M';
              other_player = search_node(player->pacman.coord[0], player->pacman.coord[1],0, a, b,player->id);
              player->pacman.type=  player->pacman.type-1;
              send_info(player);
              send_info(other_player);
            }
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
      //pacflag = 1;
      //ualarm(500000,0);
      //signal(SIGALRM,wait_playp);
      pthread_mutex_unlock(&movement[xlock][ylock]);
    }
    else if(pacflag == 1)
    {
      read(player->sock,&playbuffer,sizeof(Player));
      send_info(player);
    }
    if(monflag == 0)
    {
      read(player->sock,&player->monster,sizeof(player->monster));
      if(player->monster.coord[0]>0 && player->monster.coord[0]<n_cols && player->monster.coord[1]>0 && player->monster.coord[1]<n_lines)
      {
        xlock = player->monster.coord[0];
        ylock = player->monster.coord[1];
      }
      pthread_mutex_lock(&movement[xlock][ylock]);
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
            if (search_type(player->monster.coord[0], player->monster.coord[1],player->id)==0)
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
              board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
              random_coord(&player->monster.coord[0], &player->monster.coord[1]);
              board[player->monster.coord[0]][player->monster.coord[1]]='M';
            }
          }
          else if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'||board[player->monster.coord[0]][player->monster.coord[1]]=='C')
          {
            board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
            if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='L';
            }
            if (board[player->monster.coord[0]][player->monster.coord[1]]=='C'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='C';
            }
            if(fruits_on_board>fruits)
              fruits_on_board--;
            board[player->monster.coord[0]][player->monster.coord[1]]='P';
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
            if (search_type(player->monster.coord[0], player->monster.coord[1],player->id)==0)
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
              board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
              random_coord(&player->monster.coord[0], &player->monster.coord[1]);
              board[player->monster.coord[0]][player->monster.coord[1]]='M';
            }
          }
          else if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'||board[player->monster.coord[0]][player->monster.coord[1]]=='C')
          {
            board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
            if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='L';
            }
            if (board[player->monster.coord[0]][player->monster.coord[1]]=='C'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='C';
            }
            if(fruits_on_board>fruits)
              fruits_on_board--;
            board[player->monster.coord[0]][player->monster.coord[1]]='P';
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
            if (search_type(player->monster.coord[0], player->monster.coord[1],player->id)==0)
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
              board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
              random_coord(&player->monster.coord[0], &player->monster.coord[1]);
              board[player->monster.coord[0]][player->monster.coord[1]]='M';
            }
          }
          else if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'||board[player->monster.coord[0]][player->monster.coord[1]]=='C')
          {
            board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
            if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='L';
            }
            if (board[player->monster.coord[0]][player->monster.coord[1]]=='C'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='C';
            }
            if(fruits_on_board>fruits)
              fruits_on_board--;
            board[player->monster.coord[0]][player->monster.coord[1]]='P';
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
            if (search_type(player->monster.coord[0], player->monster.coord[1],player->id)==0)
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
              board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
              random_coord(&player->monster.coord[0], &player->monster.coord[1]);
              board[player->monster.coord[0]][player->monster.coord[1]]='M';
            }
          }
          else if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'||board[player->monster.coord[0]][player->monster.coord[1]]=='C')
          {
            board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
            if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='L';
            }
            if (board[player->monster.coord[0]][player->monster.coord[1]]=='C'&&fruits_on_board<=fruits)
            {

              random_coord(&a, &b);
              board[a][b]='C';
            }
            if(fruits_on_board>fruits)
              fruits_on_board--;
            board[player->monster.coord[0]][player->monster.coord[1]]='P';
          }
          else
          {
            board[player->monster.coord[0]][player->monster.coord[1]]='M';
          }

          send_info(player);
          break;
        //Fruta
        case 6:
          printf("actual->%d\n on board->%d\n",fruits,fruits_on_board );
          board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
          if (board[player->monster.coord[0]][player->monster.coord[1]]=='L'&&fruits_on_board<=fruits)
          {
            alarm(2);
            signal(SIGALRM,bota_frutaL);

          }
          if (board[player->monster.coord[0]][player->monster.coord[1]]=='C'&&fruits_on_board<=fruits)
          {
            alarm(2);
            signal(SIGALRM,bota_frutaC);

          }
          if(fruits_on_board>fruits)
            fruits_on_board--;
          board[player->monster.coord[0]][player->monster.coord[1]]='M';

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
            if (search_type(player->monster.coord[0], player->monster.coord[1],player->id)==0)
            {
              board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
              board[player->monster.coord[0]][player->monster.coord[1]]='M';
              random_coord(&a, &b);
              board[a][b]='P';
              other_player = search_node(player->monster.coord[0], player->monster.coord[1],1, a, b,player->id);
              send_info(player);
              send_info(other_player);
            }
            else
            {
              board[player->monster.last_coord[0]][player->monster.last_coord[1]]=' ';
              random_coord(&player->monster.coord[0], &player->monster.coord[1]);
              board[player->monster.coord[0]][player->monster.coord[1]]='M';
              send_info(player);
            }
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
      //monflag = 1;
      //ualarm(500000,0);
      //signal(SIGALRM,wait_playm);
      pthread_mutex_unlock(&movement[xlock][ylock]);
    }
    else if(monflag == 1)
    {
      read(player->sock,&playbuffer,sizeof(player->monster));
      send_info(player);
    }
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
  int local = 0, i=0, j=0;
  Player_ID *aux = head;
  while(aux != NULL)
  {
    write(aux->sock,&client_exit,sizeof(int));
    write(aux->sock,&node_send->id,sizeof(int));
    write(aux->sock,&node_send->pacman,sizeof(aux->pacman));
    write(aux->sock,&node_send->monster,sizeof(aux->monster));
    for (i = 0; i < n_cols; i++)
    {
      for (j = 0; j < n_lines; j++)
      {
        write(aux->sock,&board[i][j],sizeof(char));
      }
    }
    write(aux->sock,&fruits_on_board,sizeof(int));
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
      printf("mamas");
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
      printf("boobs");
      if (x+1 < n_cols)
      {
        if (x>i && board[x+1][y] != 'B')
          return 2;
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
char** initialize_fruits(int cols, int lines,int n_frutas)
{
  printf("fruits->%d\nfrutas-> %d",fruits,n_frutas);
  srand(time(NULL));
  int l = 0, c = 0, r= 0;
  while (fruits<n_frutas)
  {
    l = rand() % lines;
    c = rand() % cols;
    if (board[c][l] == ' ')
    {
      if (r==1)
      {
        board[c][l] = 'C';
        r=0;
        fruits++;
      }
      else
      {
        board[c][l] = 'L';
        r=1;
        fruits++;
      }
    }
  }
  return board;
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
int get_n_players()
{
  return n_players;
}
void wait_playp(int signum)
{
  pacflag = 0;
}
void wait_playm(int signum)
{
  monflag = 0;
}
void bota_frutaL(int signum)
{
  int a,b;
  random_coord(&a, &b);
  board[a][b]='L';
}
void bota_frutaC(int signum)
{
  int a,b;
  random_coord(&a, &b);
  board[a][b]='C';
}
