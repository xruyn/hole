/* Подсветка в коридоре на Pro Mini, DS1307, PIR датчике. Для индикации основного света
   используется оптопара. Лампы управляются  4-канальным модулем реле
   Три лампочки. Все три работают только если нет основного света.
   Один датчик движения. При входе в квартиру включается подсветка.
   В интервале времени с 6 до 20  включаются все три лампы. В другое время включается только средняя.

   Стоим лицом ко входу в зал, слева направо - (коридор) 1, 2, 3 (кухня)

 */
#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>
RTC_DS1307 rtc;       //выбор DS


byte pin_relay_1 = 4; //пин для управление реле1 (выход)
byte pin_relay_2 = 5; //пин для управление реле2 (выход)
byte pin_relay_3 = 8; //пин для управление реле3 (выход)
byte pin_relay_4 = 9; //пин для управление реле4 (выход)
byte pin_220v = A0; //пин для подключения оптопары (вход ADC)
byte pin_PIR = A1;    //пин для подключаения PIR (вход)
byte pin_button = 10; //пин для подключения кнопки без фиксации
int pirState = LOW;
int val_PIR = 0;
int flag = 0;
int val=0;
int button_state = 0;
float delta;
float time1;

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
void setup() {
        Serial.begin(9600);
        Wire.begin();
        rtc.begin();
        pinMode(pin_relay_1, OUTPUT);
        pinMode(pin_relay_2, OUTPUT);
        pinMode(pin_relay_3, OUTPUT);
        pinMode(pin_relay_4, OUTPUT);
        pinMode(pin_220v, INPUT);
        pinMode(pin_PIR, INPUT);
        pinMode(pin_button, INPUT);
        pinMode(13, OUTPUT);

}

void loop() {
        DateTime now = rtc.now();
        if (button_state == 0) {     //основной, "умный режим, включение по датчику движения и только при выключенном основном в"
                if (digitalRead(pin_PIR) == HIGH && digitalRead(pin_220v) == LOW) {
                        Serial.println("Motion, svet");
                        if (now.hour() > 5 && now.hour() < 19) {
                                lighting_enable();
                                Serial.println(now.hour());
                                Serial.println("Svet vse");
                        }
                        else { lighting_middle(); Serial.println("Gorit sredni"); }
                }
                else { lighting_disable(); Serial.println("Bez podsvetki"); }
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
                lighting_enable();
                Serial.println("Vse +");
        }

        if (button_state == 2) {  // режим "просто выключить все"
                lighting_disable();
                Serial.println("Vse -");
        }

}
