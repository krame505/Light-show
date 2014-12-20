//Utility functions for hex conversion and bit math.  
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

void print_bin(uint8_t data) {
  int mod = 128;
  for (int i = 0; i < 8; i++) {
    Serial.print(data - mod >= 0, DEC);
    data = (data - mod >= 0)? data - mod : data;
    mod /= 2;
  }
}

void print_hex(uint8_t data) {
   if (data < 16) {Serial.print("0");}

   Serial.print(data, HEX);
}
