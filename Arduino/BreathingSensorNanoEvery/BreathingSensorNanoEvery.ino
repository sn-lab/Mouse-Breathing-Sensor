//#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
//#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
unsigned long t0;
unsigned long t_int;
unsigned int val;
byte buf[2];
byte command;

void setup() {
  //
  //sbi(ADCSRA, ADPS2);
  //cbi(ADCSRA, ADPS1);
  //cbi(ADCSRA, ADPS0);
  ADC0_CTRLC = 0x54;
  Serial.begin(115200);
  pinMode(A0, INPUT);
  t_int = 1000; // default sampling rate of 1/1000us (1 kHz)
  pulseNum = 10;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    command = byte(Serial.read());
    switch (command) {
      case 0:
        // query for program name
        Serial.print("Breathing Sensor");
        break;
      case 1:
        // change sampling rate
        t_int = changeValue(4);
        Serial.print(t_int);
        break;
      case 2:
        // sample and print data, no trigger
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
        //Serial.read();
        break;
    }
  }
}

unsigned long changeValue(byte n) {
  unsigned long output = 0;
  for (n = 0; n < 4; n++) {
    if (!checkTimeout()) {
      output = (output << 8) + Serial.read();
    }
  }
  return output;
}

boolean checkTimeout() {
  boolean timeout = true;
  unsigned long tlast = millis();
  while (Serial.available() < 1 && ((millis() - tlast) <= 10))
    timeout = (millis() - tlast) > 10;
  return timeout;
}
