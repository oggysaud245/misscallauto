#include <SoftwareSerial.h>
#include <EEPROM.h>
#define relay1 10
#define relay2 9
#define sensor1 5
#define sensor2 6
#define buzzer 7
SoftwareSerial gsm(3, 4);
String _data, number, authNum, authNum1, authNum2;
bool inCall = false;
bool lastState = false;
bool oneTime = true;
bool outCall = false;
byte lastStateAddress = 35;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    gsm.begin(9600);
    delay(20);
    smsMode();
    Serial.println("Reading authorized Number...");
    readNum();
    Serial.print("Number 1 = ");
    Serial.println(authNum1);
    Serial.print("Number 2 = ");
    Serial.println(authNum2);
    Serial.print("Last State = ");
    Serial.println(lastState);
    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(sensor1, INPUT_PULLUP);
    pinMode(sensor2, INPUT_PULLUP);
    buzz(3000);
}

void loop()
{
    // Serial.println(digitalRead(sensor1));
    // Serial.println(digitalRead(sensor2));

    if (digitalRead(sensor1) == LOW && digitalRead(sensor2) == LOW)
    {
        // Serial.println("normal operation");
        serialEvent();
        if ((number == authNum1 || number == authNum2) && inCall == true)
        {
            gsm.println("ATH");
            Serial.println("authorized number");
            EEPROM.write(lastStateAddress, lastState = !lastState);
            inCall = false;
            outCall = true;
            oneTime = true;
        }
        else if ((number != authNum1 || number != authNum2) && inCall == true)
        {
            gsm.println("ATA");
            Serial.println("unauthorized number");
            inCall = false;
        }

        if (lastState == true && oneTime == true)
        {
            Serial.println("relay1 changes");
            digitalWrite(relay1, HIGH);
            delay(1000);
            digitalWrite(relay1, LOW);
            buzz(1000);
            if (outCall == true)
            {
                callBack();
            }
            outCall = false;
            oneTime = false;
        }
        else if (lastState == false && oneTime == true)
        {
            Serial.println("relay2 changes");
            digitalWrite(relay2, HIGH);
            delay(1000);
            digitalWrite(relay2, LOW);
            buzz(1000);
            oneTime = false;
        }
    }
    else
    {
        if (lastState == true)
        {
            digitalWrite(relay2, HIGH);
            delay(1000);
            digitalWrite(relay2, LOW);
            buzz(1000);
            oneTime = true;
        }
        Serial.println("Sensor Failure");
        while ((digitalRead(sensor1) == HIGH && digitalRead(sensor2) == HIGH) ||(digitalRead(sensor1) == HIGH && digitalRead(sensor2) == LOW) || (digitalRead(sensor1) == LOW && digitalRead(sensor2) == HIGH))
        {
            Serial.print(".");
            delay(1000);
        }
    }
}
void callBack()
{
    String cmd;
    Serial.println("Calling Back...");
    delay(5000);
    if (number == authNum1)
    {
        cmd = "ATD" + authNum1 + ";";
        Serial.println(cmd);
        gsm.println(cmd);
    }
    else if (number == authNum2)
    {
        cmd = "ATD" + authNum2 + ";";
        Serial.println(cmd);
        gsm.println(cmd);
    }
    delay(5000);
    gsm.println("ATH");
    Serial.println("Call End");
}
void serialEvent()
{
    if (gsm.available())
    {
        _data = gsm.readStringUntil('\r');
        delay(40);
        Serial.println(_data);
        delay(10);
        // Serial.flush();
        if (_data.length() > 10)
        {
            if (_data.indexOf("+977") >= 3 && _data.indexOf("+CLIP: \"") >= 0)
            {
                number = _data.substring(_data.indexOf("+977") + 4, _data.indexOf("+977") + 4 + 10);
                Serial.println(number);
                inCall = true;
            }
            else if (_data.indexOf("+CLIP: \"") >= 0)
            {
                number = _data.substring(_data.indexOf("+CLIP: \"") + 8, _data.indexOf("+CLIP: \"") + 8 + 10);
                Serial.println(number);
                inCall = true;
            }
            else if (_data.indexOf("SET NUM1") == 1)
            {
                Serial.println("sms received");
                authNum = _data.substring(_data.indexOf("9"), _data.indexOf("9") + 10);
                Serial.println(authNum);
                storeNum(1, authNum);
            }
            else if (_data.indexOf("SET NUM2") == 1)
            {
                Serial.println("sms received");
                authNum = _data.substring(_data.indexOf("9"), _data.indexOf("9") + 10);
                Serial.println(authNum);
                storeNum(2, authNum);
            }
        }
    }
    _data = "";
}

void smsMode()
{
    gsm.println("AT+CMGF=1\r");
    delay(10);
    gsm.println("AT+CNMI=2,2,0,0,0\r");
    delay(10);
}
void storeNum(byte post, String number)
{
    if (post == 1)
    {
        for (int j = 10; j < 20; ++j)
        {
            EEPROM.write(j, 0);
        }
        for (int i = 0; i < number.length(); ++i)
        {
            EEPROM.write(10 + i, number[i]);
        }
    }
    else if (post == 2)
    {
        for (int j = 21; j < 31; ++j)
        {
            EEPROM.write(j, 0);
        }
        for (int i = 0; i < number.length(); ++i)
        {
            EEPROM.write(21 + i, number[i]);
        }
    }
}
void readNum()
{
    for (int i = 10; i < 20; ++i)
    {
        authNum1 += (char)EEPROM.read(i);
    }
    for (int i = 21; i < 31; ++i)
    {
        authNum2 += (char)EEPROM.read(i);
    }
    lastState = EEPROM.read(lastStateAddress);
}

void buzz(int delayTime)
{
    digitalWrite(buzzer, HIGH);
    delay(delayTime);
    digitalWrite(buzzer, LOW);
}