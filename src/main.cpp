/* Подсветка в коридоре на Pro Mini, DS1307, PIR датчике. Для индикации основного света
   используется оптопара. Лампы управляются  4-канальным модулем реле
   Три лампочки. Все три работают только если нет основного света.
   Один датчик движения. При входе в квартиру включается подсветка.
   В интервале времени с 6 до 20  включаются все три лампы. В другое время включается только средняя.

   Стоим лицом ко входу в зал, слева направо - (коридор) 1, 2, 3 (кухня)

   режим 1 - умный режим, включение по датчику движения и только при выключенном основном
   режим 2 - просто включить все, без датчиков
   режим 3 - просто выключить все
 */
#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>
#include <MsTimer2.h>
RTC_DS1307 rtc;       //выбор DS
void lighting_enable();
void lighting_disable();
void lighting_middle();
void pit_test();
void pulsout(byte x, int y);
void digitOut(byte j);
void digit_mode(int digit);
void low_battery();


#define pin_relay_1 9 //пин для управление реле1 (выход)
#define pin_relay_2 8 //пин для управление реле2 (выход)
#define pin_relay_3 7 //пин для управление реле3 (выход)
#define pin_relay_4 6 //пин для управление реле4 (выход)
#define pin_220v  5 //пин для подключения оптопары (вход)
#define pin_PIR 11    //пин для подключаения PIR (вход)
#define pin_button 10 //пин для подключения кнопки без фиксации
#define pin_data 12 //data HC164
#define pin_CLK A1 //CLK HC164
#define pin_nCLR A0 //nCLR HC164
#define pin_ADC A7 // вход АЦП для измерения напряжения батарейки DS1307

int pirState = LOW;
int val_PIR = 0;
int flag = 0;
int val = 0;
int button_state = 0;
float scale = 0.00447; // реальное напряжение Vcc/1023
float delta;
float time1;
byte Digit[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};




void setup() {
        Serial.begin(9600);
        Wire.begin();
        rtc.begin();
        MsTimer2::set(7000, low_battery); // 7s период
        MsTimer2::start();
        pinMode(pin_relay_1, OUTPUT);
        pinMode(pin_relay_2, OUTPUT);
        pinMode(pin_relay_3, OUTPUT);
        pinMode(pin_relay_4, OUTPUT);
        pinMode(pin_220v, INPUT);
        pinMode(pin_PIR, INPUT);
        pinMode(pin_button, INPUT);
        pinMode(pin_data, OUTPUT);
        pinMode(pin_CLK, OUTPUT);
        pinMode(pin_nCLR, OUTPUT);
        pinMode(13, OUTPUT);
        analogReference(DEFAULT);
        pinMode(pin_ADC, INPUT);
        digitalWrite(pin_nCLR, HIGH);
        for (int p = 9; p >= 0; p--) {
                digitOut(Digit[p]);
                delay(50);
        }
}


void loop() {
        //digitalWrite(pin_nCLR, HIGH);
        DateTime now = rtc.now();




        if (button_state == 0) {     //основной, "умный режим, включение по датчику движения и только при выключенном основном"
                //digit_mode(1);
                if (digitalRead(pin_PIR) == HIGH && digitalRead(pin_220v) == LOW) {
                        Serial.println("Motion, svet");
                        if (now.hour() > 5 && now.hour() < 19) {
                                lighting_enable();
                                Serial.println(now.hour());
                                Serial.println("Svet vse");
                        }
                        else {
                                lighting_enable();
                                lighting_middle();
                                Serial.println("Gorit sredni");
                        }
                }
                else {
                        lighting_disable();
                        Serial.println("Bez podsvetki");
                }
        }

        if(digitalRead(pin_button) == HIGH && flag == 0) {
                button_state++;
                flag = 1;
                if(button_state>2)    {
                        button_state = 0;
                }
        }

        if(digitalRead(pin_button) == LOW && flag == 1) {
                flag = 0;
        }

        if (button_state == 1) {  // режим "просто включить все, без датчиков"
                //digit_mode(2);
                lighting_enable();
                Serial.println("Vse +");
        }

        if (button_state == 2) {  // режим "просто выключить все"
                //digit_mode(3);
                lighting_disable();
                Serial.println("Vse -");
        }

}


void lighting_enable() {
        digitalWrite(pin_relay_1, LOW);
        digitalWrite(pin_relay_2, LOW);
        digitalWrite(pin_relay_3, LOW);
        digitalWrite(pin_relay_4, LOW);

}
void lighting_disable() {
        digitalWrite(pin_relay_4, HIGH);
        digitalWrite(pin_relay_3, HIGH);
        digitalWrite(pin_relay_2, HIGH);
        digitalWrite(pin_relay_1, HIGH);
}
void lighting_middle() {
        digitalWrite(pin_relay_1, HIGH);
        digitalWrite(pin_relay_2, LOW);
        digitalWrite(pin_relay_3, HIGH);
        digitalWrite(pin_relay_4, HIGH);
}

void pir_test(){
        val_PIR = digitalRead(pin_PIR);
        if (val_PIR == HIGH) { // проверяем, соответствует ли считанное значение HIGH
                digitalWrite(13, HIGH); // включаем светодиод
                if (pirState == LOW) {
                        // мы только что включили
                        Serial.println("Motion detected!");
                        time1 = millis();
                        // мы выводим на серийный монитор изменение, а не состояние
                        pirState = HIGH;
                }

        } else {
                digitalWrite(13, LOW); // выключаем светодиод
                if (pirState == HIGH) {
                        // мы только что его выключили
                        Serial.println("Motion ended!");
                        delta = millis() - time1;
                        Serial.println(delta);
                        // мы выводим на серийный монитор изменение, а не состояние
                        pirState = LOW;

                }
        }
}

void digitOut(byte j)   {       // вывод цифра на индикатор
        byte k;
        for(int i=0; i < 8; i++)   {
                k = j & 0x01;
                digitalWrite(pin_data, !k); // для общего катода убрать !
                pulsout(pin_CLK, 1);
                j = j >> 1;
        }

}



void pulsout(byte x, int y)   { // функция CLK для HC164
        byte z = digitalRead(x);
        z = !z;
        digitalWrite(x, z);
        delayMicroseconds(10* y);
        z = !z; // return to original state
        digitalWrite(x, z);
        return;
}


void digit_mode(int digit){
        for (int p = digit; p <= digit; p++) {
                digitOut(Digit[p]);
                delay(30);
        }
}

void low_battery(){
  static boolean output = HIGH;

  digitalWrite(13, output);
  output = !output;
      // float v_bat = analogRead(pin_ADC); // читаем значение на аналоговом входе
      // if (v_bat*scale < 2.7) {
      //
      // }
}
