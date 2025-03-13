#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
unsigned long t0;
unsigned long t_int;
unsigned int val;
byte buf[2];
byte command;
byte trigPin = 10; //digital pin 10
byte pulseNum;
byte counter;

void setup() {
  //
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
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
        Serial.print("Breathing Sensor With Trigger");
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
      case 3:
        // read in external trigger
        pinMode(trigPin, INPUT);
        t0 = micros();
        while (!Serial.available()) {
          if ((micros() - t0) > t_int) {
            val = analogRead(A0);
            val = val | (digitalRead(trigPin) << 15);
            buf[0] = val & 255;
            buf[1] = (val >> 8) & 255;
            //Serial.println(val);
            Serial.write(buf, 2);
            t0 = t0 + t_int;
          }
        }
        break;
      case 4:
        // output trigger until stopped
        pinMode(trigPin, OUTPUT);
        digitalWrite(trigPin, HIGH);
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
        digitalWrite(trigPin, LOW);
        break;
      case 5:
        // output trigger every N samples while reading, up to 255
        if (!checkTimeout()) {
          pulseNum = Serial.read();
          counter = 0;
          pinMode(trigPin, OUTPUT);
          digitalWrite(trigPin, LOW);
          t0 = micros();
          while (!Serial.available()) {
            if ((micros() - t0) > t_int) {
              if (counter == pulseNum) {
                digitalWrite(trigPin, OUTPUT);
                counter = 0;
              }
              digitalWrite(trigPin, HIGH);
              val = analogRead(A0);
              buf[0] = val & 255;
              buf[1] = (val >> 8) & 255;
              //Serial.println(val);
              Serial.write(buf, 2);
              t0 = t0 + t_int;
              digitalWrite(trigPin, LOW);
              counter = counter + 1;
            }
          }
        }
        break;
      case 6:
        // toggle for every N samples, up to 255
        if (!checkTimeout()) {
          pulseNum = Serial.read();
          counter = 0;
          pinMode(trigPin, OUTPUT);
          digitalWrite(trigPin, LOW);
          t0 = micros();
          while (!Serial.available()) {
            if ((micros() - t0) > t_int) {
              if (counter == pulseNum) {
                digitalWrite(trigPin, !digitalRead(trigPin));
                counter = 0;
              }
              val = analogRead(A0);
              buf[0] = val & 255;
              buf[1] = (val >> 8) & 255;
              //Serial.println(val);
              Serial.write(buf, 2);
              t0 = t0 + t_int;
              counter = counter + 1;
            }
          }
        }
        digitalWrite(trigPin, LOW);
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
    Serial.print(millis() - tlast);
  timeout = (millis() - tlast) > 10;
  return timeout;
}
