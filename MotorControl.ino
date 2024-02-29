#include <LiquidCrystal.h>
const int RS = 12, EN = 11, D4 = 10, D5 = 6, D6 = 5, D7 = 4;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

#define adc_pin A1
#define led 13
#define IN1 8
#define IN2 7
#define pwm_pin 9
#define T2_initialVal 156

const int SIZE = 10;
int adcArray[SIZE];
int elementsRead = 0;
volatile bool allElementsRead = false;

// interrupt service routine that wraps a user defined function
ISR(TIMER2_OVF_vect) {
  TCNT2 = T2_initialVal; // preload timer
  digitalWrite(led, digitalRead(led) ^ 1); // toggle the led (^ 1 means XOR function)

  if (elementsRead < SIZE) {
    adcArray[elementsRead] = analogRead(adc_pin);
    elementsRead++;
  } else {
    allElementsRead = true;
  }
}

float calculateAverageAdc(int elementsRead) {
  float sum = 0;
  float divider = (float)elementsRead;
  float averageAdc = 0;
  for (int i = 0; i < elementsRead; i++) {
    sum += (float)adcArray[i];
  }
  averageAdc = (sum/divider);
  return averageAdc;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  lcd.begin(20, 4); // since we used LM044L on proteus
  
  pinMode(led,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);

  // setup Timer2
  noInterrupts(); // disable all interrupts
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = T2_initialVal; // preload timer 65536-16MHz/8/1kHz
  TCCR2B |= (1 << CS21)|(1 << CS20); // 32 prescaler
  TIMSK2 |= (1 << TOIE2); // enable timer overflow interrupt
  interrupts(); // enable all interrupts
}

void loop() {
  // put your main code here, to run repeatedly:
  if (allElementsRead) {
    noInterrupts(); // Disable interrupts while calculating average ADC
    int currentElementsRead = elementsRead; // Save the value of elementsRead for future usage, i.e. calculating average ADC
    float adcAverage = calculateAverageAdc(currentElementsRead); // Use the saved value for calculating average ADC
    interrupts(); // Enable interrupts after calculating average ADC
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AVG MOTOR CURRENT");
    lcd.setCursor(0, 1);
    lcd.print(adcAverage);
    lcd.print("mA");

    for (int i = 0; i < currentElementsRead; i++) {
      Serial.print(adcArray[i]);
      if (i < currentElementsRead - 1) {
        Serial.print(", ");
      }
    }
    Serial.println();

    elementsRead = 0; // Reset read element count, for ISR
    allElementsRead = false; // reset the state, for ISR and loop()
    TCNT2 = 0; // preload timer 0
    TCCR2B |= (1 << CS21); // 8 prescaler
  }
  analogWrite(pwm_pin, abs(analogRead(A0)/4-1));
  delay(500);
}
