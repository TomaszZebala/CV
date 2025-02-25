//UDP Server 2

#include <sys/socket.h>
#include <sys/types.h>
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

#define MAX_BUFF 128
#define KEEP_ALIVE "s2 OK?"
#define KEEP_ALIVE_DC "s2 DC!"
#define KEEP_ALIVE_RESP "s2 YES"
#define PORT_NUMBER "1204" 
#define CLIENT_IP "192.168.89.4"
#define TOKEN "s2" 


void say_hello();
void keep_alive_responder();
char *random_response(const char msg[], int len);
bool is_correct_random_message(const char msg[], int len);

struct addrinfo h, *r=NULL;
struct sockaddr_in c;
int s, c_len=sizeof(c);
char buffer[MAX_BUFF];


int main(){
  printf("Starting UDP server...\n");
  
  memset(&h, 0, sizeof(struct addrinfo));
  h.ai_family=PF_INET; 
  h.ai_socktype=SOCK_DGRAM; 
  if(getaddrinfo(CLIENT_IP, PORT_NUMBER, &h, &r)!=0){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    return -1;
  }
  if((s=socket(r->ai_family, r->ai_socktype, r->ai_protocol))==-1){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
    return -1;
  }
  
  
  say_hello();
  
  for(;;){
    int pos=recvfrom(s, buffer, MAX_BUFF, 0, (struct sockaddr*)&c, &c_len);
    if(pos<0){printf("ERROR: %s\n", strerror(errno));exit(-4);}
    if(c_len>0){
      buffer[pos]='\0';}
      
    if(!strcmp(buffer, KEEP_ALIVE)){
      printf("Received KEEP_ALIVE: %s\n", buffer);
      keep_alive_responder();
    }
    
   
    else if(buffer[0] == 's' && buffer[1] == TOKEN[1] && buffer[2] == '-' && buffer[3] == '>'){
      srand(time(NULL));
      
      printf("RECEIVED: %s\n", buffer);
      char *response = random_response(buffer, pos);
      printf("SEND: %s\n", response);
      
      int pos=sendto(s, response, strlen(response), 0, r->ai_addr, r->ai_addrlen);
      if(pos<0){
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);//...
      }
      if(pos!=strlen(response)){
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
      }
    }
    
    else if(!strcmp(buffer, KEEP_ALIVE_DC)){
      
      printf("\nRozlaczono z serwerem: %s\n", buffer);
      printf("Ponawiam probe polaczenia:\n");
      say_hello();
    }else{
      printf("\nOdebrano niezrozumiala wiadomosc!\n");
    }
  }
  
  freeaddrinfo(r);
  close(s);
}


char *random_response(const char msg[], int len) { 
  if(is_correct_random_message(msg,len)){
    
    char temp1[5]; 
    char temp2[len - 4 + 1];
    char temp3[3]; 

    for (int i = 0; i < 4; i++) {
        temp1[i] = msg[i];
    }
    temp1[4] = '\0';
    

    for (int j = 0; j < len - 4; j++) {
        temp2[j] = msg[4 + j];
    }
    temp2[len - 4] = '\0';  

    int random_number = (rand() % 90) + 10;
    snprintf(temp3, sizeof(temp3), "%d", random_number);
    
    size_t total_length = strlen(temp1) + strlen(temp2) + strlen(temp3) + 1;
    char* result = malloc(total_length);
    if (!result) {
        return "ERROR";
    }
    
    strcpy(result, temp1);
    strcat(result, temp3);
    strcat(result, temp2);
    

    return result;
  }else{
    printf("ODRZUCONO: %s\n", msg);
    return "ERROR";
  }
}


void keep_alive_responder(){
  char response[MAX_BUFF];
  snprintf(response, MAX_BUFF, KEEP_ALIVE_RESP);
  printf("Sending a KEEP_ALIVE response %s\n", response);
  
  int pos=sendto(s, response, strlen(response), 0, r->ai_addr, r->ai_addrlen);
  if(pos<0){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
  if(pos!=strlen(response)){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
    
}


void say_hello(){
  unsigned char hello_message[MAX_BUFF];
  snprintf(hello_message, MAX_BUFF, TOKEN);
  
  printf("Sending a HELLO message: %s\n\n", hello_message);
  int pos=sendto(s, hello_message, strlen(hello_message), 0, r->ai_addr, r->ai_addrlen);

  if(pos<0){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }
  if(pos!=strlen(hello_message)){
    printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
  }

}


bool is_correct_random_message(const char msg[], int len) {
    for (int i = 0; i < len; i++) {
        if (!( (msg[i] == '-') || (msg[i] == '>') || (msg[i] >= '0' && msg[i] <= '9') || (msg[i] >= 'a' && msg[i] <= 'z'))) {
            return false;
        }
    }
    return true;
}