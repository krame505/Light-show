
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "ids.h"
#include "DHT.h"

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
#define PWM_RELAY_ON 1
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
D_t digital_status;
uint8_t pwm_status[4] = {0, 0, 0, 0};

//#define PACKET_LEN 8
#define PACKET_LEN sizeof(msg_t)
#define NUM_D 3 //for message_M parsing
#define NUM_P 1

boolean mode = false;//false = pwm, true = digital
char address = 0;

//auto-fade
boolean auto_fade[4] = {false, false, false, false};
#define auto_fade_delay 5
long int last_auto_fade = 0;

//Radio stuff
RF24 radio(15,16);
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};

//DHT
#define DHTPIN A0     // what pin we're connected to
// Uncomment whatever type you're using
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

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
  digital_status.byte = 0;

/*
  digitalWrite(PWM_Relay_1, PWM_RELAY_OFF);
  digitalWrite(PWM_Relay_2, PWM_RELAY_OFF);
  digitalWrite(PWM_Relay_3, PWM_RELAY_OFF);
  digitalWrite(PWM_Relay_4, PWM_RELAY_OFF);  
*/
  Serial.begin(115200);
  printf_begin();
  
  Serial.println("Relay command code version 3.7");
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
    
    radio.read(msg.bytes, PACKET_LEN);
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
    Serial.print("Received hex bytes: ");
    for (int i = 0; i < PACKET_LEN; i++) {
      print_hex(msg.bytes[i]);
      Serial.print(" ");
    }
    Serial.println();
    
    Serial.print("Binary conversion:   ");
    for (int i = 0; i < PACKET_LEN; i++) {
      print_bin(msg.bytes[i]);
      Serial.print(" ");
    }
    Serial.println();
    
    msg_t status_message;
    status_message.msg_SR.id = ID_SR;
 //   status_message.res[0] = 0;
 
    switch (msg.msg_GEN.id) {
      case ID_DN:
      case ID_DSN:
      case ID_DCN:
        if (mode && (msg.msg_DN.N == address || msg.msg_DN.N == 0)) {
          D_t assign = msg.msg_DN.D;
          if (msg.msg_GEN.id == ID_DN)
            digital_status.byte = assign.byte;
          else if (msg.msg_GEN.id == ID_DSN)
            digital_status.byte |= assign.byte;
          else if (msg.msg_GEN.id == ID_DCN)
            digital_status.byte &= ~assign.byte;
          write_relays(digital_status);
        }
        break;
      case ID_D:
      case ID_DS:
      case ID_DC:
        if (mode) {
          D_t assign = msg.msg_D.D[address - 1];
          if (msg.msg_GEN.id == ID_D)
            digital_status.byte = assign.byte;
          else if (msg.msg_GEN.id == ID_DS)
            digital_status.byte |= assign.byte;
          else if (msg.msg_GEN.id == ID_DC)
            digital_status.byte &= ~assign.byte;
          write_relays(digital_status);
        }
        break;
      case ID_PN:
      case ID_PNA:
        if (!mode && (msg.msg_PN.N == address || msg.msg_PN.N == 0)) {
          if (msg.msg_GEN.id == ID_PNA)
            for (int i = 0; i < 4; i++)
              auto_fade[i] = true;
          write_relays_pwm(msg.msg_PN.P);
        }
        break;
      case ID_PNC:
      case ID_PNCA:
        if (!mode && (msg.msg_PNC.N == address || msg.msg_PNC.N  == 0)) {
          if (msg.msg_PNC.C == 0)
            for (int i = 1; i <= 4; i++) {
              auto_fade[i - 1] = true;
              write_relay_pwm(i, msg.msg_PNC.P);
            }
          else {
            auto_fade[(int)msg.msg_PNC.C - 1] = true;
            write_relay_pwm((int)msg.msg_PNC.C, msg.msg_PNC.P);
          }
        }
        break;
      case ID_M:
      case ID_MA:
        if (mode) {
          write_relays(msg.msg_M.D[address - 1]);
        }
        else {
          if (msg.msg_GEN.id == ID_MA)
            for (int i = 0; i < 4; i++)
              auto_fade[i] = true;
          if (address == 1) //Update this if more than 1 pwm relay is added
            write_relays_pwm(msg.msg_M.P);
        }
        break;
      case ID_S:
        if (msg.msg_S.N == address || msg.msg_S.N == 0) {
          status_message.msg_SR.N = address;
          status_message.msg_SR.temp = (int)(dht.readTemperature());
          status_message.msg_SR.humidity = dht.readHumidity();
  
          // Check if read failed and exit early (to try again).
          if (isnan(status_message.msg_SR.humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
          }
          
          Serial.print("Temperatute: ");
          Serial.print(status_message.msg_SR.temp, DEC);
          Serial.println(" C");
          Serial.print("Humidity: ");
          Serial.print(status_message.msg_SR.humidity);
          Serial.println("% RH");
          
          Serial.print("Sending hex bytes: ");
          for (int i = 0; i < PACKET_LEN; i++) {
            print_hex(status_message.bytes[i]);
            Serial.print(" ");
          }
          Serial.println();
          radio.stopListening();
          radio.write(status_message.raw, PACKET_LEN);
          radio.startListening();
          Serial.println("Done.  ");
        }
        break;
    default:
        Serial.print("Invalid message ID: ");
        print_hex(msg.msg_GEN.id);
        Serial.println();
        break;
    }
    Serial.println();
    data_ready = false;
  }
  
  if (last_auto_fade + auto_fade_delay < millis()) {
    last_auto_fade = millis();
    for (int i = 0; i < 4; i++) {
      if (auto_fade[i]) {
        if (pwm_status[i] <= 0) {
          auto_fade[i] = false;
        }
        else {
          write_relay_pwm(i + 1, pwm_status[i] - 1);
        }
      }
    }
  }
      
}//--(end main loop )---

void write_relays(D_t data) {
  Serial.println("Writing relays... ");
//  for (int i = 0; i < 8; i++) {
//    write_relay(i, data.bits[i]);
//  }
  Serial.print("Set digital channels 1-8 to ");
  print_bin(data.byte);
  Serial.println();
#if RELAY_OFF
  data.byte = ~data.byte;
#endif
  digitalWrite(relays[0], data.relay.in1);
  digitalWrite(relays[1], data.relay.in2);
  digitalWrite(relays[2], data.relay.in3);
  digitalWrite(relays[3], data.relay.in4);
  digitalWrite(relays[4], data.relay.in5);
  digitalWrite(relays[5], data.relay.in6);
  digitalWrite(relays[6], data.relay.in7);
  digitalWrite(relays[7], data.relay.in8);
  Serial.println("Done.  ");
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
  Serial.println("Writing relays... ");
  for (int i = 1; i <= 4; i++) {
    write_relay_pwm(i, data[i - 1]);
  }
  Serial.println("Done.  ");
}

void write_relay_pwm(int relay, uint8_t value) {
  Serial.print("Set pwm channel ");
  Serial.print(relay, DEC);
  Serial.print(" to ");
  Serial.println(value, DEC);
  pwm_status[relay - 1] = value;
  analogWrite(relays[relay - 1], value);
}
