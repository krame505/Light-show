
#include <SPI.h>
//#include "nRF24L01.h"
//#include "RF24.h"
//#include "ids.h"

//packet id defs
#define ID_M  0x00  //full assignment
#define ID_DN 0x10  //digital
#define ID_D  0x20  //full assignment to digital
#define ID_PN 0x30  //pwm
#define ID_S  0x40  //status request

/*-----( Declare Constants )-----*/
#define RELAY_ON 0
#define RELAY_OFF 1
#define PWM_RELAY_ON 1
#define PWM_RELAY_OFF 0
/*-----( Declare objects )-----*/
/*-----( Declare Variables )-----*/
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
  
  Serial.println("Relay command code version 2.1");
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
  address = analogRead(ADDRESS_PIN) / 256; //change to change number of possible addresses.  
  Serial.print("Found address ");
  Serial.println(address, DEC);
  
  Serial.println("Done initalizing.");

}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  char c[PACKET_LEN * 2];
  if (Serial.available() >= PACKET_LEN * 2) {
    for (int i = 0; i < PACKET_LEN * 2; i++) {
      delay(10);
      c[i] = Serial.read();
    }
    
    Serial.print("Received hex packets: ");
    for (int i = 0; i < PACKET_LEN * 2; i += 2) {
      Serial.print(c[i]);
      Serial.print(c[i + 1]);
      Serial.print(' ');
    }
    Serial.println();
    
    unsigned char data[PACKET_LEN];
    for (int i = 0; i < PACKET_LEN; i++) {
      data[i] = hex_to_byte(c[i * 2], c[i * 2 + 1]);
    }
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
      print_bin(data[i]);
      Serial.print(" ");
    }
    Serial.println();
    
    switch (data[0]) {
      case ID_DN:
        if (mode && data[1] == address) {
          Serial.print("Writing relays... ");
          write_relays(data[2]);
          Serial.println("Done.  ");
        }
        break;
      case ID_D:
        if (mode) {
          Serial.print("Writing relays... ");
          write_relays(data[address + 1]);
          Serial.println("Done.  ");
        }
        break;
      case ID_PN:
        if (!mode && data[1] == address) {
          Serial.print("Writing relays... ");
          write_relays_pwm(data + 2);
          Serial.println("Done.  ");
        }
        break;
      case ID_M:
        if (mode) {
          Serial.print("Writing relays... ");
          write_relays(data[address + 1]);
          Serial.println("Done.  ");
        }
        else {
          Serial.print("Writing relays... ");
          write_relays_pwm(data + NUM_D + address * 4 + 1);
          Serial.println("Done.  ");
        }
        break;
      case ID_S:
        Serial.println("Status packets not yet implimented.");
        break;
      default:
        Serial.print("Invalid message ID: ");
        Serial.println(data[1]);
        break;
    }
  }
  else {
    if (Serial.available() > 0 && Serial.available() < PACKET_LEN * 2) {
      Serial.flush();
      delay(500);
    }
    if (Serial.available() > 0 && Serial.available() < PACKET_LEN * 2) {
      Serial.print("Incomplete packet detected: ");
      while (Serial.available()) {
        Serial.print((char)Serial.read());
      }
      Serial.println();
    }
  }
}//--(end main loop )---
void write_relays(unsigned char data) {
  int mod = 128;
  for (int i = 0; i < 8; i++) {
    write_relay(i, data - mod >= 0);
    data = (data - mod >= 0)? data - mod : data;
    mod /= 2;
  }
}

void write_relay(int relay, boolean value) {
  if (value)
    digitalWrite(relays[relay], RELAY_ON);
  else
    digitalWrite(relays[relay], RELAY_OFF);
}

void write_relays_pwm(unsigned char data[4]) {
  for (int i = 0; i < 4; i++) {
    write_relay_pwm(i, data[i]);
  }
}

void write_relay_pwm(int relay, unsigned char value) {
  analogWrite(relays[relay], value);
}
