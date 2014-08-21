#define ADDRESS_PIN A3

void setup() {
  Serial.begin(112500);
  pinMode(ADDRESS_PIN, INPUT_PULLUP);
  Serial.println("Value  Address");
}

void loop() {
  int value = analogRead(ADDRESS_PIN);
  Serial.print(value, DEC);
  Serial.print("   ");
  Serial.println(value / 256, DEC);
  delay(500);
}
