/*-----( Declare Constants )-----*/
#define RELAY_ON 1
#define RELAY_OFF 0
/*-----( Declare objects )-----*/
/*-----( Declare Variables )-----*/
#define Relay_1  3  // Arduino Digital I/O pin number
#define Relay_2  5
#define Relay_3  6
#define Relay_4  9

int relays[4] = {Relay_1, Relay_2, Relay_3, Relay_4};

void setup() {
//-------( Initialize Pins so relays are inactive at reset)----
  digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);
  
  Serial.begin(115200);
  
  Serial.println("PWM relay command code version 1.0");
  Serial.println("Initalizing...");
//---( THEN set pins as outputs )----  
  for (int i = 0; i < 4; i++) {
    pinMode(relays[i], OUTPUT);
  }
  Serial.println("Done initalizing.");

}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{ 
  char c[8];
  if (Serial.available() >= 2) {
    for (int i = 0; i < 8; i++) {
      delay(10);
      c[i] = Serial.read();
    }
    
    Serial.print("Received hex packets: ");
    for (int i = 0; i < 8; i += 2) {
      Serial.print(c[i]);
      Serial.print(c[i + 1]);
      Serial.print(' ');
    }
    Serial.println();
    
    unsigned char data[4];
    for (int i = 0; i < 4; i++) {
      data[i] = hex_to_byte(c[i * 2], c[i * 2 + 1]);
    }
    
    Serial.print("Decimal conversion:  ");
    for (int i = 0; i < 4; i++) {
      Serial.print(data[i], DEC);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("Binary conversion:   ");
    for (int i = 0; i < 4; i++) {
      print_bin(data[i]);
      Serial.print(" ");
    }
    Serial.println();
    
    Serial.print("Writing relays... ");
    write_relays(data);
    Serial.println("Done.  ");
  }
  else {
    if (Serial.available() > 0 && Serial.available() < 8) {
      Serial.flush();
      delay(500);
    }
    if (Serial.available() > 0 && Serial.available() < 8) {
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

void write_relays(unsigned char data[4]) {
  for (int i = 0; i < 4; i++) {
    write_relay(i, data[i]);
  }
}

void write_relay(int relay, unsigned char value) {
  analogWrite(relays[relay], value);
}
    
unsigned char hex_to_byte(char c1, char c2) {
  return half_hex_to_byte(c2) + (half_hex_to_byte(c1) << 4);
}

unsigned char half_hex_to_byte(char c) {
  switch (c) {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'A':
    case 'a':
      return 10;
    case 'B':
    case 'b':
      return 11;
    case 'C':
    case 'c':
      return 12;
    case 'D':
    case 'd':
      return 13;
    case 'E':
    case 'e':
      return 14;
    case 'F':
    case 'f':
      return 15;
    default:
      return 0;
  }
}
