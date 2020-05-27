#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
unsigned long t0;
unsigned long t_int;
unsigned int val;
byte buf[2];
byte command;

void setup() {
  // put your setup code here, to run once:
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
  Serial.begin(115200);
  pinMode(A0, INPUT);
  t_int = 1000;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    command = Serial.read();
    switch (command) {
      case 0:
        // query for program name
        Serial.print("Breathing Sensor");
        break;
      case 1:
        // change sampling rate
        t_int = changeValue();
        Serial.print(t_int);
        break;
      case 2:
        // sample and print data
        t0 = micros();
        while (!Serial.available()) {
          if ((micros() - t0) > t_int) {
            val = analogRead(A0);
            buf[0] = val & 255;
            buf[1] = (val >> 8) & 255;
            //Serial.println(val);
            Serial.write(buf, 2);
            t0 = t0 + t_int;
          }
        }
        Serial.read();
        break;
    }
  }
}

unsigned long changeValue() {
  unsigned long output = 0;
  for (byte n = 0; n < 4; n++) {
    if (!checkTimeout()) {
      output = (output << 8) + Serial.read();
    }
  }
  return output;
}

boolean checkTimeout() {
  boolean timeout;
  unsigned long tlast = millis();
  while (!Serial.available() && ((millis() - tlast) <= 10))
    timeout = (millis() - tlast) > 10;
  return timeout;
}
