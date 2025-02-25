//============= Kod Serwera obslugujacego gry (POSIX) =============
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>


#define NUM_GAMES                    100    
#define SEQUENCE_LEN                 4      
#define MAX_PLAYERS                  3      
#define WAITING_TIME                 20     
#define CONFIGURATION_TIME           0.2    


#define MAX_BUFF                     3      
#define PORT_NUMBER                  "1204" 
#define PORT_LEN                     48


#define REGISTER                     0x80   //1000 0000
#define GAME                         0x40   //0100 0000
#define WINNER                       0x00   //0000 0000
#define END                          0xC0   //1100 0000


#define SERVER                       0x00   //0000 0000


#define TYPE_MASK                    0xC0   //1100 0000 
#define PLAYER_CODE                  0x38   //0011 1000
#define DATA_MASK                    0x07   //0000 0111


//============= FUNCTIONS ========================
void make_one_game();                          
void monitor_sockets();                        
void send_back_reg(int, int, char[20]);        
void send_sorry(int, char[20]);               
void send_data();                              
void announce_winner(int);                     
void send_end();                               
uint8_t toss_a_coin();                         
bool check_ack();                              
void hex_to_list(uint16_t, int[SEQUENCE_LEN]); 
float prepare_average();                       

//============= VARIABLES ======================

struct player {
    char ip [20];
    int port;
    int number;
    uint16_t seq;
    int win_counter;
  };
  
struct player connected_players[MAX_PLAYERS];
int num_players = 0;


int rounds_to_win[NUM_GAMES];      
bool is_winner;                    
bool has_to_wait_for_players;      
int currnent_round_counter=0;      
int game_counter;                  
int players_with_ack[MAX_PLAYERS]; 
bool registering;                  


struct addrinfo h, *r=NULL;
struct sockaddr_in c;
int s, c_len=sizeof(c);


unsigned char buffer[MAX_BUFF];
unsigned char msg[MAX_BUFF];


fd_set master_set, working_set;


int main(){
  printf("\nStarting game's server...\n");

  memset(&h, 0, sizeof(struct addrinfo));
  h.ai_family=PF_INET;
  h.ai_socktype=SOCK_DGRAM;
  h.ai_flags=AI_PASSIVE;
  if(getaddrinfo(NULL, PORT_NUMBER, &h, &r)!=0){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__); return -1;
  }
  if((s=socket(r->ai_family, r->ai_socktype, r->ai_protocol))==-1){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__); return -1;
  }
  if(bind(s, r->ai_addr, r->ai_addrlen)!=0){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__); return -1;
  }
  
  FD_ZERO(&master_set);
  FD_SET(s, &master_set);
  
  registering = true;
  time_t start_waiting = time(NULL);
  printf("Waiting for players for %d seconds...\n\n", WAITING_TIME);
  while (difftime(time(NULL), start_waiting) < WAITING_TIME) {
    monitor_sockets(); 
  }
  registering = false;
  
  printf("\n=======Starting game============\n");
  srand(time(NULL));
  
  for(game_counter = 0; game_counter < NUM_GAMES; game_counter++){ 
    printf("\n\n----Starting %d round!----\n", game_counter + 1);
    make_one_game(); 
  }

  send_end();
  
  printf("\n\n===========================\n");
  printf("==========SUMMARY==========\n");
  printf("===========================\n\n");
  printf("====NUMBERS OF GAMES:%d====\n", game_counter);
  
  float avg = prepare_average();
  
  printf("AVERAGE NUMBER OF ROUNDS: %.1f\n", avg);
  
  for(int player = 0; player < num_players; player++){
    int number = connected_players[player].number;
    int wins = connected_players[player].win_counter;
    float probability = (float) wins/game_counter * 100;
    int list[SEQUENCE_LEN];
    hex_to_list(connected_players[player].seq, list);
    
    printf("\n========PLAYER %d========\n", number);
    printf("Sequence: [");
    for(int i = 0; i < SEQUENCE_LEN-1; i++){
      printf("%d, ", list[i]);
    }
    printf("%d]\n", list[SEQUENCE_LEN-1]);
    printf("Wins: %d\n", wins);
    printf("Probability: %.1f %\n", probability);
  }
  
  freeaddrinfo(r);
  close(s);
  return 0;
}

void make_one_game(){
  is_winner = false;
  has_to_wait_for_players = false;
  currnent_round_counter = 0;
  
  for(;;){
    monitor_sockets();
    if(is_winner){break;}
    
    has_to_wait_for_players = check_ack();
    if(!has_to_wait_for_players){
      send_data();
      memset(players_with_ack, 0, sizeof(players_with_ack));
      has_to_wait_for_players = true;
      currnent_round_counter++;
    }
  }
  printf("----End of %d round!----\n\n", game_counter + 1);
  rounds_to_win[game_counter] = currnent_round_counter;
  

  time_t start_waiting = time(NULL);
  printf("Just a moment for configuration...\n\n");
  while (difftime(time(NULL), start_waiting) < CONFIGURATION_TIME) {
    monitor_sockets(); 
  }
}


void monitor_sockets(){
  FD_ZERO(&working_set);
  memcpy(&working_set, &master_set, sizeof(master_set));

  struct timeval timeout = {0, 50000}; // Timeout 50ms
  int activity = select(FD_SETSIZE, &working_set, NULL, NULL, &timeout);
  if (activity < 0) {
        perror("select() error");
        return;
  }
  
  if (FD_ISSET(s, &working_set)) {
    int pos=recvfrom(s, buffer, MAX_BUFF, 0, (struct sockaddr*)&c, &c_len);
    if(pos<0){printf("ERROR: %s\n", strerror(errno));exit(-4);}
    if(c_len>0){
      
      //TYPE 10
      if( ((buffer[0] & TYPE_MASK) == REGISTER)){
        if(registering){
          if((buffer[0] & PLAYER_CODE) == SERVER){
            printf("Received HELLO from (%s:%d)\n", inet_ntoa(c.sin_addr),ntohs(c.sin_port));
            if(num_players < MAX_PLAYERS){
              struct player temp_player;
              temp_player.port = ntohs(c.sin_port);
              strcpy(temp_player.ip, inet_ntoa(c.sin_addr));
              temp_player.number = num_players+1;
            
              connected_players[num_players] = temp_player;
              send_back_reg(temp_player.number, temp_player.port, temp_player.ip);
              num_players++;
            }else{
              printf("Too many players!!!\n");
              printf("Sending rejection msg!!!\n\n");
              send_sorry(ntohs(c.sin_port), inet_ntoa(c.sin_addr));
            }
          }
          else{
            int temp_player_num = (buffer[0] & PLAYER_CODE) >> 3;
            connected_players[temp_player_num-1].seq = buffer[1] << 8 | buffer[2]; //zapisywana sekwencja w strukturze
            connected_players[temp_player_num-1].win_counter = 0;
            players_with_ack[temp_player_num-1] = 1;  //gracz jest gotowy na odbior kolejnej wiadomosci - ROZPOCZECIE GRY
            
            int temp_list[SEQUENCE_LEN];
            hex_to_list(connected_players[temp_player_num-1].seq, temp_list);
            if(sizeof(temp_list)/sizeof(int) == SEQUENCE_LEN){
              printf("Otrzymano sekwencje: [");
              for(int i = 0; i < SEQUENCE_LEN-1; i++){
                printf("%d, ", temp_list[i]);
              }
              printf("%d]\n\n", temp_list[SEQUENCE_LEN-1]);
            }
            else{
              printf("SORRY, YOUR SEQUENCE IS WRONG!!!\n");
              send_sorry(ntohs(c.sin_port), inet_ntoa(c.sin_addr));
            }
          } 
        }
        else{
        printf("The game is pending!!!\n");
        printf("Sending rejection msg!!!\n\n");
        send_sorry(ntohs(c.sin_port), inet_ntoa(c.sin_addr));
        }
      }
      //TYPE 01
      else if((buffer[0] & TYPE_MASK) == GAME){
        int temp_number = ((buffer[0] & PLAYER_CODE) >> 3);
        players_with_ack[temp_number-1] = 1;
      }
      
      //TYPE 00
      else if((buffer[0] & TYPE_MASK) == WINNER){
        if((buffer[0] & PLAYER_CODE) == SERVER){
          int temp_number = buffer[0] & DATA_MASK;
          players_with_ack[temp_number-1] = 1;
          printf("---- Got RDY from: %x ----\n", temp_number);
          
        }else{
          is_winner = true;
          int win_number = ((buffer[0] & PLAYER_CODE) >> 3);
          connected_players[win_number-1].win_counter++;
          announce_winner(win_number);
        }
      }
      
      else{
        printf("---- UNKNOWN message: DROPPED ----\n");
      }
    }
  }
}


void send_back_reg(int num_player, int port, char ip[20]){
  msg[0] = REGISTER | SERVER | num_player;
  
  char port_str[PORT_LEN];
  snprintf(port_str, PORT_LEN, "%d", port);
  if (getaddrinfo(ip, port_str, &h, &r) != 0) {
      printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
  int pos = sendto(s, msg, MAX_BUFF, 0, r->ai_addr, r->ai_addrlen); 
  if (pos < 0) {
      printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
  if (pos != MAX_BUFF) {
      printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
}

void send_sorry(int port, char ip[20]){
  msg[0] = REGISTER | SERVER;
  
  char port_str[PORT_LEN];
  snprintf(port_str, PORT_LEN, "%d", port);
  if (getaddrinfo(ip, port_str, &h, &r) != 0) {
      printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
  int pos = sendto(s, msg, MAX_BUFF, 0, r->ai_addr, r->ai_addrlen); 
  if (pos < 0) {
      printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
  if (pos != MAX_BUFF) {
      printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
}

void send_data(){ 
  uint8_t result = toss_a_coin();
  printf("Sending data: %d\n", result);
  
  for(int player = 0; player < num_players; player++){
    char ip[20];
    int port = connected_players[player].port;
    strcpy(ip, connected_players[player].ip);
    int num_player = connected_players[player].number;
    
    msg[0] = GAME | (num_player << 3) | result;
    char port_str[PORT_LEN];
    snprintf(port_str, PORT_LEN, "%d", port);
    if (getaddrinfo(ip, port_str, &h, &r) != 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
    int pos = sendto(s, msg, MAX_BUFF, 0, r->ai_addr, r->ai_addrlen); 
    if (pos < 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
    if (pos != MAX_BUFF) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
  }
}

void announce_winner(int winner){
  printf("ANNOUNCE WINNER... PLAYER: %d !!!\n", winner);
  for(int player = 0; player < num_players; player++){
    char ip[20];
    int port = connected_players[player].port;
    strcpy(ip, connected_players[player].ip);
    int num_player = connected_players[player].number;
    
    msg[0] = WINNER | (num_player << 3) | winner;
    char port_str[PORT_LEN];
    snprintf(port_str, PORT_LEN, "%d", port);
    if (getaddrinfo(ip, port_str, &h, &r) != 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
    int pos = sendto(s, msg, MAX_BUFF, 0, r->ai_addr, r->ai_addrlen); 
    if (pos < 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
    if (pos != MAX_BUFF) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
    
  }
}

void send_end(){
  printf("\n=====END GAME!!!=====\n");
  for(int player = 0; player < num_players; player++){
    char ip[20];
    int port = connected_players[player].port;
    strcpy(ip, connected_players[player].ip);
    int num_player = connected_players[player].number;
    
    msg[0] = END | (num_player << 3);
    char port_str[PORT_LEN];
    snprintf(port_str, PORT_LEN, "%d", port);
    if (getaddrinfo(ip, port_str, &h, &r) != 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
    int pos = sendto(s, msg, MAX_BUFF, 0, r->ai_addr, r->ai_addrlen); 
    if (pos < 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
    if (pos != MAX_BUFF) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    }
  }
}


uint8_t toss_a_coin(){
  int result = rand() % 2;  
  if (result == 0) {
    return 0x01;  
  }else{
    return 0x00; 
  }
}

bool check_ack(){
  for(int i = 0; i < num_players; i++){
    if(players_with_ack[i] == 0){return true;}
  }
  return false;
}

void hex_to_list(uint16_t hex_value, int list[]){
  uint8_t pos;
  for(pos = 0; pos < SEQUENCE_LEN; pos++){
    list[SEQUENCE_LEN - pos - 1] = hex_value & 0x1;
    hex_value = hex_value >> 1;
  }
}

float prepare_average(){
  int sum = 0;
  for(int i = 0; i < game_counter; i++){
    sum = sum + rounds_to_win[i];
  }
  return (float) sum/game_counter;
}