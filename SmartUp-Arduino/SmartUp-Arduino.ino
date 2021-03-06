#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHT_PIN   6     // Pin which is connected to the DHT sensor.
#define SERVO_PIN 5     // Servo motor pin
#define RELAY_PIN 4     // Relay pin
#define DOOR_PIN  3     // Door pin
#define PIR_PIN	  2		// PIR pin

#define DHT_TYPE  DHT22   // DHT 22 (AM2302)
#define DHT_DELAY 2000    // DHT recommended time between measurements

#define MAX_ANGLE     125 //max angle of the motor
#define MIN_ANGLE     45   //min angle of the motor
#define DEFAULT_ANGLE 90  //default angle of the motor
#define SERVO_DEALY 1000  //duration of every motor action (defined by kinect frequency)

#define INIT "init"     //const for init
#define SCAN "scan"     //const for scan
#define ANGLE "angle"   //const for angle

#define RELAY_TEMPERTURE 30//temperature to turn on the relay
#define RELAY_HUMIDITY   95 //humidity to turn on the relay

int curAngle = DEFAULT_ANGLE, scan_speed = 60; //define current angle and scanning speed
DHT_Unified dht(DHT_PIN, DHT_TYPE); //create dht object
Servo servo;//create servo motor object
int DhtTimeElapsed = 0; //represent the time elapsed from last dht check
int doorVal = HIGH;

void setup() {
	//initialize PIR sensor with Interrupt
	pinMode(PIR_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(DOOR_PIN), DOOR_Interrupt, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIR_PIN), PIR_Interrupt, CHANGE);
	//Initialize servo
	servo.attach(SERVO_PIN);
	servo.write(DEFAULT_ANGLE);

	//Initialize device.
	dht.begin();

	//Open serial communication
	Serial.begin(9600);
}

void loop() {
	//Serial.print("Hello");
	String msg = readLine();

	if (isNumeric(msg));
		moveMotorTo(msg.toInt());
	
	else if (msg.equals(INIT))  //connect to program from PC
		Serial.print((String)INIT + "\n");
	
	else if (msg.equals(SCAN))
		scan();
	else if (msg.equals(ANGLE))
		Serial.print((String)curAngle + "\n");
	
	else if (DhtTimeElapsed < DHT_DELAY)  //no valid message, read DHT with delay
	{
		int tmp = min(DHT_DELAY - DhtTimeElapsed, 500);
		delay(tmp);
		DhtTimeElapsed += tmp;
	}
	
	
	if (DhtTimeElapsed >= DHT_DELAY) //can measure DHT without delay
	{
		DhtTimeElapsed = 0;
		sensors_event_t event;
		float t = getTemperture(event);
//    Serial.print("Temperture " + ((String)t) + " ");
		float h = getHumidity(event);
//    Serial.print("Humidity " + ((String)h) + "\n");
		if (t > RELAY_TEMPERTURE && h > RELAY_HUMIDITY)
			digitalWrite(RELAY_PIN, HIGH);
		else
			digitalWrite(RELAY_PIN, LOW);
	}
	
}

void DOOR_Interrupt() {
  int val = digitalRead(DOOR_PIN);
  //if (val!=doorVal)
  //{
	    if (val != LOW)    
    Serial.print("OPENED\n");
  else    
    Serial.print("CLOSED\n");
  //}
}

void PIR_Interrupt () {
	int val = digitalRead(PIR_PIN);
	if (val == HIGH)
		Serial.print("ON\n");
	else
		Serial.print("OFF\n");
}


float getTemperture(sensors_event_t event)
{
	dht.temperature().getEvent(&event);
	if (isnan(event.temperature)) return 0;  //Error reading temperture
	else return event.temperature;
}

float getHumidity(sensors_event_t event)
{
	dht.humidity().getEvent(&event);
	if (isnan(event.relative_humidity)) return 0;  //Error reading humidity
	else  return event.relative_humidity;
}

void scan()
{
	if (curAngle == MAX_ANGLE || curAngle == MIN_ANGLE) 
		scan_speed *= -1;
	moveMotorTo(curAngle + scan_speed);
	//Serial.print((String)curAngle + "\n");
}

boolean isNumeric(String str) {
	int stringLength = str.length();

	if (stringLength == 0)
		return false;
	if (str.charAt(0) == '-')
		return isNumeric(str.substring(1));

	for (int i = 0; i < stringLength; i++) {
		if (!isDigit(str.charAt(i)))
			return false;
	}
	return true;
}

String readLine()
{
	String msg1 = "";
	while (Serial.available() > 0)
	{
		char inChar = Serial.read();

		if (inChar == '\n')
		{
			String msg2 = readLine();
			if (msg2.equals(""))
				return msg1;
			return msg2;
		}
		msg1 += (String)inChar;
		delay(2);
	}
	return msg1;
}

void moveMotorTo(int newAngle)
{
	DhtTimeElapsed += SERVO_DEALY;

	if (newAngle > MAX_ANGLE) newAngle = MAX_ANGLE;
	if (newAngle < MIN_ANGLE) newAngle = MIN_ANGLE;

	int delayTime = SERVO_DEALY / abs(newAngle - curAngle) + 1;

	for (; curAngle < newAngle; curAngle++)
	{
		servo.write(curAngle);
		delay(delayTime);
	}
	for (; curAngle > newAngle; curAngle--)
	{
		servo.write(curAngle);
		delay(delayTime);
	}
	//Serial.print((String)curAngle + "\n");
}
