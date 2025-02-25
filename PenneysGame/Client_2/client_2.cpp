//============= PLAYER 2 ======================
#include <Arduino.h>
#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>   
#include <stdbool.h>


#define SEND_BUFFER_SIZE_ONE         1      
#define SEND_BUFFER_SIZE_THREE       3      


#define CLIENT_PORT                  1204   
#define LOCAL_PORT                   49001  


#define SEQUENCE_LEN                 4      



#define REGISTER                     0x80   //1000 0000
#define GAME                         0x40   //0100 0000
#define WINNER                       0x00   //0000 0000
#define END                          0xC0   //1100 0000

#define SERVER                       0x00   //0000 0000

#define TYPE_MASK                    0xC0   //1100 0000 
#define PLAYER_NUM                   0x38   //0011 1000
#define GAIN_NUM                     0x07   //0000 0111
#define GAIN_DATA                    0x01   //0000 0001

#define CLOCK            ZSUT_PIN_D0        
#define VALUE            ZSUT_PIN_D1        
#define MASK_EDGE        0x0001             
#define MASK_VALUE       0x0002             

unsigned char send_one_buffer[SEND_BUFFER_SIZE_ONE];
unsigned char send_three_buffer[SEND_BUFFER_SIZE_THREE];
unsigned char recv_buffer[SEND_BUFFER_SIZE_THREE];


ZsutIPAddress address_ip = ZsutIPAddress(192,168,0,3); 
unsigned int localPort = LOCAL_PORT;                      
byte mac[] = {0x02, 0x31, 0x54, 0x45, 0x89, 0xAB};        
ZsutEthernetUDP Udp;


uint8_t player_number;
uint8_t result;


int sequence[SEQUENCE_LEN]; 
int list_pos = 0;
int prevEdge = 0;
int edge, val;


// ------------ FUNCTIONS ------------
void get_seq();
void send_register();
void listenUDP();
void send_seq();
void send_ack();
void send_win();
void send_rdy();
bool check_the_sequence(struct Queue *q);
uint16_t list_to_hex(int[]);




struct Queue {
    uint8_t items[SEQUENCE_LEN];
    int front;
    int rear;
};
void init_queue(struct Queue *q);
int is_full(struct Queue *q);
int is_empty(struct Queue *q);
void enqueue(struct Queue *q, int value);
int dequeue(struct Queue *q);
void display(struct Queue *q);
Queue q; //zaczep do kolejki



void setup() {
  Serial.begin(115200);
  ZsutPinMode(CLOCK, OUTPUT);
  ZsutPinMode(VALUE, OUTPUT);
  
  Serial.println("Start gracza 1");

  Serial.print(F("Player init... [")); 
  Serial.print(F(__FILE__));
  Serial.print(F(", "));
  Serial.print(F(__DATE__));
  Serial.print(F(", ")); 
  Serial.print(F(__TIME__)); 
  Serial.println(F("]"));

  ZsutEthernet.begin(mac);
  Serial.print("IP Address: "); Serial.println(ZsutEthernet.localIP());
  
  get_seq();
  init_queue(&q);

  Udp.begin(localPort);

  Serial.println("Registering for the game... ");
  send_register();
  
  Serial.println("\nListening started...\n");
}

void loop() {
  listenUDP();
}

void get_seq() {
  while(list_pos < SEQUENCE_LEN){
    edge = (int) ZsutDigitalRead() & MASK_EDGE;
      if(edge != prevEdge){
        val = (int) (ZsutDigitalRead() & MASK_VALUE) >> 1;
        sequence[list_pos++] = val;
        Serial.println(val);
      }
    prevEdge = edge;
  }
}


void send_register() {
  send_one_buffer[0] = REGISTER | SERVER; 
  Udp.beginPacket(address_ip, CLIENT_PORT);
  Udp.write(send_one_buffer, SEND_BUFFER_SIZE_ONE);
  Udp.endPacket(); 
}


void listenUDP() {
  int packetSize=Udp.parsePacket();
  
  if(packetSize>0) {
    int len = Udp.read(recv_buffer, SEND_BUFFER_SIZE_THREE);
    
	
    if(len<=0){
      Udp.flush(); return;
    }
    
    if((recv_buffer[0] & TYPE_MASK) == REGISTER){
      //TYPE 10
      if((recv_buffer[0] & PLAYER_NUM) == SERVER){
        player_number = recv_buffer[0] & GAIN_NUM;
        if(player_number == 0){
          Serial.print("\n=====SORRY, YOU CANNOT JOIN RIGHT NOW: =====\n");
          Serial.print("too many players OR the game is pending OR your sequence is wrong!!!=====\n");
          Serial.print("\n=====END GAME!!!=====\n");
        }
        else{
          Serial.print("Given player number: "); Serial.print(player_number, HEX);
          send_seq();
        }
      }
      else{
        Serial.print("---- UNKNOWN message type 10: DROPPED ----\n");
      }
    }
    
    //TYPE: 01
    else if((recv_buffer[0] & TYPE_MASK) == GAME){
      if( ((recv_buffer[0] & PLAYER_NUM) >> 3) == player_number){
        result = recv_buffer[0] & GAIN_DATA; 
        if (is_full(&q)){
           dequeue(&q);
        }
        enqueue(&q, result);
        
        if(check_the_sequence(&q)){
          send_win();
          Serial.print("---- Sending a winner msg ----\n");
        }
        else{
          send_ack();
        }
      }
      else{
        Serial.print("---- UNKNOWN message type 01: DROPPED ----\n");
      }
    }
    
    //TYPE: 00
    else if((recv_buffer[0] & TYPE_MASK) == WINNER){
      if( ((recv_buffer[0] & PLAYER_NUM)  >> 3) == player_number){
        Serial.print("\n---- There is a WINNER and sending RDY----\n");
        send_rdy();
        init_queue(&q);
      }
      else{
        Serial.print("---- UNKNOWN message type 00: DROPPED ----\n");
      }
    }
    
    //TYPE: 11
    else if((recv_buffer[0] & TYPE_MASK) == END){
      if( ((recv_buffer[0] & PLAYER_NUM) >> 3) == player_number){
        Serial.print("\n=====END GAME!!!=====\n");
      }
      else{
        Serial.print("---- UNKNOWN message type 11: DROPPED ----\n");
      }
    }
  }
}


void send_seq(){
  uint16_t seq = list_to_hex(sequence); 
  uint8_t header = REGISTER | (player_number<<3);
  uint8_t seqMSB = (seq >> 8) & 0xFF;
  uint8_t seqLSB = seq & 0xFF;
  send_three_buffer[0] = header;
  send_three_buffer[1] = seqMSB;
  send_three_buffer[2] = seqLSB;
  
  Udp.beginPacket(address_ip, CLIENT_PORT);
  Udp.write(send_three_buffer, SEND_BUFFER_SIZE_THREE);
  Udp.endPacket(); 
  
  Serial.print("\n---- Messeage SEQ was sent ----\n");
  Serial.print("\n---- Waiting for the start ----\n");
}

void send_ack(){
  uint8_t header = GAME | (player_number<<3);
  send_one_buffer[0] = header; 
  
  Udp.beginPacket(address_ip, CLIENT_PORT);
  Udp.write(send_one_buffer, SEND_BUFFER_SIZE_ONE);
  Udp.endPacket();  
  Serial.print("\n---- Messeage ACK was sent ----");
}

void send_win(){
  uint8_t header = WINNER | (player_number<<3);
  send_one_buffer[0] = header; 
  
  Udp.beginPacket(address_ip, CLIENT_PORT);
  Udp.write(send_one_buffer, SEND_BUFFER_SIZE_ONE);
  Udp.endPacket();  
  Serial.print("\n---- Messeage WIN was sent ----\n");
  
}

void send_rdy(){
  uint8_t header = WINNER | SERVER | player_number;
  send_one_buffer[0] = header; 
  
  Udp.beginPacket(address_ip, CLIENT_PORT);
  Udp.write(send_one_buffer, SEND_BUFFER_SIZE_ONE);
  Udp.endPacket();  
  Serial.print("\n---- Messeage RDY was sent ----\n");
}


bool check_the_sequence(struct Queue *q){
  int i = q->front;
  int pos = 0;
  
  while (i != q->rear) {
    if(q->items[i] != sequence[pos++]){return false;}
    i = (i + 1) % SEQUENCE_LEN;
  }
  if(q->items[i] != sequence[pos]){return false;}
  
  if(pos == SEQUENCE_LEN-1) {return true;}
  return false;
}

uint16_t list_to_hex(int seq[]){
  uint16_t hex_value = 0;
  for (uint8_t i = 0; i < SEQUENCE_LEN; i++) {
    hex_value = (hex_value << 1) | seq[i];
  }
  return hex_value;
}



void init_queue(struct Queue *q) {
    q->front = -1;
    q->rear = -1;
}
int is_full(struct Queue *q) {
    return (q->rear + 1) % SEQUENCE_LEN == q->front;
}
int is_empty(struct Queue *q) {
    return q->front == -1;
}
void enqueue(struct Queue *q, int value) {
    if (is_full(q)) {
        return;
    }
    if (q->front == -1) {
        q->front = 0;
    }
    q->rear = (q->rear + 1) % SEQUENCE_LEN;
    q->items[q->rear] = value;
}
int dequeue(struct Queue *q) {
    if (is_empty(q)) {
        return -1;
    }
    int value = q->items[q->front];
    if (q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % SEQUENCE_LEN;
    }
    return value;
}
void display(struct Queue *q) {
    if (is_empty(q)) {
        return;
    }
    int i = q->front;
    while (i != q->rear) {
        Serial.print(q->items[i], HEX);
        i = (i + 1) % SEQUENCE_LEN;
    }
    Serial.print(q->items[i], HEX);
}