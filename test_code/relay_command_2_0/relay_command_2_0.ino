/*-----( Declare Constants )-----*/
#define RELAY_ON 0
#define RELAY_OFF 1
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

#define PWM_Relay_1  2  // Arduino Digital I/O pin number
#define PWM_Relay_2  3
#define PWM_Relay_3  4
#define PWM_Relay_4  5

int relays[8];// = {Relay_1, Relay_2, Relay_3, Relay_4, Relay_5, Relay_6, Relay_7, Relay_8};

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
  digitalWrite(PWM_Relay_1, RELAY_OFF);
  digitalWrite(PWM_Relay_2, RELAY_OFF);
  digitalWrite(PWM_Relay_3, RELAY_OFF);
  digitalWrite(PWM_Relay_4, RELAY_OFF);  
  
  Serial.begin(115200);
  
  Serial.println("Relay command code version 1.2");
  Serial.println("Initalizing...");
//---( THEN set pins as outputs )----  
  for (int i = 0; i < 8; i++) {
    pinMode(relays[i], OUTPUT);
  }
  Serial.println("Done initalizing.");

}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{ 
  if (Serial.available() >= 2) {
    char c1 = Serial.read();
    delay(10);
    char c2 = Serial.read();
    
    Serial.print("Received hex packet: ");
    Serial.print(c1);
    Serial.print(c2);
    Serial.println();
    
    unsigned char data = hex_to_byte(c1, c2);
    
    Serial.print("Decimal conversion:  ");
    Serial.print(data, DEC);
    Serial.println();
    Serial.print("Binary conversion:   ");
    print_bin(data);
    Serial.println();
    
    Serial.print("Writing relays... ");
    write_relays(data);
    Serial.println("Done.  ");
  }
  else {
    if (Serial.available() > 0 && Serial.available() < 2) {
      Serial.flush();
      delay(50);
    }
    if (Serial.available() > 0 && Serial.available() < 2) {
      Serial.print("Incomplete packet detected: ");
      while (Serial.available()) {
        Serial.print((char)Serial.read());
      }
      Serial.println();
    }
  }
}//--(end main loop )---

void print_bin(unsigned char data) {
  int mod = 128;
  for (int i = 0; i < 8; i++) {
    Serial.print(data - mod >= 0, DEC);
    data = (data - mod >= 0)? data - mod : data;
    mod /= 2;
  }
}

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
    
unsigned char hex_to_byte(char c1, char c2) {
  return half_hex_to_byte(c2) + (half_hex_to_byte(c1) << 4);
}

