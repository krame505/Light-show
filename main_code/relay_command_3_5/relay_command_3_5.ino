
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "ids.h"

//packet id defs
//See ids.h
/*
#define ID_M  0x00  //full assignment
#define ID_DN 0x10  //digital
#define ID_D  0x20  //full assignment to digital
#define ID_PN 0x30  //pwm
#define ID_S  0x40  //status request
#define ID_SR 0x50  //status reply
*/

#define RELAY_ON 0
#define RELAY_OFF 1
#define PWM_RELAY_OsN 1
#define PWM_RELAY_OFF 0

#define Relay_1  2  // Arduino Digital I/O pin number
#define Relay_2  3
#define Relay_3  4
#define Relay_4  5
#define Relay_5  6
#define Relay_6  7
#define Relay_7  8
#define Relay_8  9

#define PWM_Relay_1  3  // Arduino Digital I/O pin number
#define PWM_Relay_2  5
#define PWM_Relay_3  6
#define PWM_Relay_4  9

#define DEFAULT_Relay 3  //pin 3 is used for both relays.  

#define MODE_PIN 10
#define ADDRESS_PIN A3

int relays[8];// = {Relay_1, Relay_2, Relay_3, Relay_4, Relay_5, Relay_6, Relay_7, Relay_8};

#define PACKET_LEN 8
#define NUM_D 3 //for message_M parsing
#define NUM_P 1

boolean mode = false;//false = pwm, true = digital
char address = 0;

//Radio stuff
RF24 radio(15,16);
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//Misc
boolean data_ready = false;
msg_t msg;

void setup() {
//-------( Initialize Pins so relays are inactive at reset)----

  digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  digitalWrite(Relay_5, RELAY_OFF);
  digitalWrite(Relay_6, RELAY_OFF);
  digitalWrite(Relay_7, RELAY_OFF);
  digitalWrite(Relay_8, RELAY_OFF);  

/*
  digitalWrite(PWM_Relay_1, PWM_RELAY_OFF);
  digitalWrite(PWM_Relay_2, PWM_RELAY_OFF);
  digitalWrite(PWM_Relay_3, PWM_RELAY_OFF);
  digitalWrite(PWM_Relay_4, PWM_RELAY_OFF);  
*/
  Serial.begin(115200);
  printf_begin();
  
  Serial.println("Relay command code version 3.5");
  Serial.println("Initalizing...");
  pinMode(MODE_PIN, INPUT_PULLUP);
  pinMode(ADDRESS_PIN, INPUT_PULLUP);
  Serial.print("Reading mode...");
  mode = digitalRead(MODE_PIN);
  if (mode)
    Serial.println("Found HIGH\nConfiguring as digital...");
  else {
    Serial.println("Found LOW\nConfiguring as pwm...");
    digitalWrite(PWM_Relay_1, PWM_RELAY_OFF);
    digitalWrite(PWM_Relay_2, PWM_RELAY_OFF);
    digitalWrite(PWM_Relay_3, PWM_RELAY_OFF);
    digitalWrite(PWM_Relay_4, PWM_RELAY_OFF);  
  }
  
  if (mode) {
    int relays_tmp[] = {Relay_1, Relay_2, Relay_3, Relay_4, Relay_5, Relay_6, Relay_7, Relay_8};
    for (int i = 0; i < 8; i++) {
      relays[i] = relays_tmp[i];
    }
  }
  else {
    int relays_tmp[] = {PWM_Relay_1, PWM_Relay_2, PWM_Relay_3, PWM_Relay_4, DEFAULT_Relay, DEFAULT_Relay, DEFAULT_Relay, DEFAULT_Relay};
    for (int i = 0; i < 8; i++) {
      relays[i] = relays_tmp[i];
    }
    digitalWrite(PWM_Relay_1, PWM_RELAY_OFF);
    digitalWrite(PWM_Relay_2, PWM_RELAY_OFF);
    digitalWrite(PWM_Relay_3, PWM_RELAY_OFF);
    digitalWrite(PWM_Relay_4, PWM_RELAY_OFF);  
  }
  
//---( THEN set pins as outputs )----  
  for (int i = 0; i < 8; i++) {
    pinMode(relays[i], OUTPUT);
  }
  Serial.println("Done configuring.");
  Serial.print("Reading address...");
  int analog_address = analogRead(ADDRESS_PIN);
  address = analog_address / 256 + 1; //change to change number of possible addresses.  
  Serial.print("Found address ");
  Serial.print(address, DEC);
  Serial.print(" (analog value ");
  Serial.print(analog_address, DEC);
  Serial.println(")");
  
  Serial.println("Starting radio...");
  radio.begin();
  radio.setRetries(15,100); //15, 15
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  radio.printDetails();
  
  Serial.println("Done initalizing.");

}//--(end setup )---

/****** LOOP: RUNS CONSTANTLY ******/
void loop() {
  if (radio.available()) {
    Serial.println("Radio packet incoming...");
    radio.read(msg.raw, PACKET_LEN);
    
    Serial.print("Received hex bytes: ");
    for (int i = 0; i < PACKET_LEN; i++) {
      print_hex(msg.bytes[i]);
      Serial.print(" ");
    }
    Serial.println();
    /*
    Serial.print("Decimal conversion:  ");
    for (int i = 0; i < PACKET_LEN; i++) {
      Serial.print(data[i], DEC);
      Serial.print(" ");
    }
    Serial.println();
    */
    Serial.print("Binary conversion:   ");
    for (int i = 0; i < PACKET_LEN; i++) {
      print_bin(msg.bytes[i]);
      Serial.print(" ");
    }
    Serial.println();
    
    data_ready = true;
  }
  else if (Serial.available() >= PACKET_LEN * 2) {
    char c[PACKET_LEN * 2];
    
    Serial.print("Serial message incoming: ");
    for (int i = 0; i < PACKET_LEN * 2; i++) {
      delay(10);
      c[i] = Serial.read();
      Serial.print(c[i]);
    }
    Serial.println();
    
    for (int i = 0; i < PACKET_LEN; i++) {
      msg.bytes[i] = hex_to_byte(c[i * 2], c[i * 2 + 1]);
    }
    
    Serial.print("Received hex bytes: ");
    for (int i = 0; i < PACKET_LEN; i++) {
      print_hex(msg.bytes[i]);
      Serial.print(" ");
    }
    Serial.println();
    
    /*
    Serial.print("Decimal conversion:  ");
    for (int i = 0; i < PACKET_LEN; i++) {
      Serial.print(msg.bytes[i], DEC);
      Serial.print(" ");
    }
    Serial.println();
    */
    Serial.print("Binary conversion:   ");
    for (int i = 0; i < PACKET_LEN; i++) {
      print_bin(msg.bytes[i]);
      Serial.print(" ");
    }
    Serial.println();
    data_ready = true;
  }
  else if (Serial.available() > 0 && Serial.available() < PACKET_LEN * 2) {
    Serial.flush();
    delay(500);
    if (Serial.available() > 0 && Serial.available() < PACKET_LEN * 2) {
      Serial.print("Incomplete Serial packet detected: ");
      while (Serial.available()) {
        Serial.print((char)Serial.read());
      }
      Serial.println();
    }
  }
  
  if (data_ready) { 
    msg_t status_message;
    status_message.msg_SR.id = ID_SR;
 //   status_message.res[0] = 0;
 /* Serial.print("Received hex packets: ");
    for (int i = 0; i < PACKET_LEN; i++) {
      print_hex(msg.bytes[i]);
      Serial.print(" ");
    }
    Serial.println();*/
    switch (msg.bytes[0]) {
      case ID_DN:
        if (mode && (msg.msg_DN.N == address || msg.msg_DN.N == 0)) {
          Serial.println("Writing relays... ");
          write_relays(msg.msg_DN.D1);
          Serial.println("Done.  ");
        }
      break;
      case ID_D:
        if (mode) {
          Serial.println("Writing relays...");
          write_relays(msg.msg_D.D[address]);
          Serial.println("Done.  ");
        }
        break;
      case ID_PN:
        if (!mode && (msg.msg_PN.N == address || msg.msg_PN.N  == 0)) {
          Serial.println("Writing relays...");
          write_relays_pwm(msg.msg_PN.P);
          Serial.println("Done.  ");
        }
        break;
      case ID_M:
        if (mode) {
          Serial.println("Writing relays...");
          write_relays(msg.msg_M.D[address]);
          Serial.println("Done.  ");
        }
        else {
          Serial.println("Writing relays...");
          if (address == 1) //Change this if more than 1 pwm relay is added
            write_relays_pwm(msg.msg_M.P);
          Serial.println("Done.  ");
        }
        break;
      case ID_S:
        if (msg.msg_S.N == address || msg.msg_S.N == 0) {
          Serial.print("Sending status... ");
          radio.stopListening();
          status_message.msg_SR.N = address;
          //TODO: temperature and humidity
          radio.write(status_message.raw, PACKET_LEN);
          radio.startListening();
          Serial.println("Done.  ");
        }
        break;
    default:
        Serial.print("Invalid message ID: ");
        print_hex(msg.bytes[0]);
        Serial.println();
        break;
    }
    Serial.println();
    data_ready = false;
  }
}//--(end main loop )---

void write_relays(D_t data) {
  for (int i = 0; i < 8; i++) {
    write_relay(i, data.bits[i]);
  }
}

void write_relay(int relay, boolean value) {
  Serial.print("Set digital channel ");
  Serial.print(relay, DEC);
  Serial.print(" to ");
  if (value) {
    Serial.println("ON");
    digitalWrite(relays[relay], RELAY_ON);
  }
  else {
    Serial.println("OFF");
    digitalWrite(relays[relay], RELAY_OFF);
  }
}

void write_relays_pwm(uint8_t data[4]) {
  for (int i = 0; i < 4; i++) {
    write_relay_pwm(i, data[i]);
  }
}

void write_relay_pwm(int relay, uint8_t value) {
  Serial.print("Set pwm channel ");
  Serial.print(relay, DEC);
  Serial.print(" to ");
  Serial.println(value, DEC);
  analogWrite(relays[relay], value);
}
