#include  <Timer.h>
#include <ESP32Servo.h>

Servo servo1;  // create servo object to control the servo
int pos = 0;  //Seervo angle variable
int count = 0;  //LED blink count	
struct pulse_input  //Setting a structure to initialise a pin for pulse capture
{
	const uint8_t PIN;
	bool state;
};

pulse_input input1 = {21, false};  //Setting GPIO Pin 19 to record the pulses
Timer timer1;  //Initialising timer for pulse measurement;

uint32_t timer_value;  //variable to store pulse length
bool flag1 = false;  //Flag1 & Flag2 used to denote when interrupts have been raised in main program
bool flag2 = false;
int blinkdelay;  //Variable to store the time delay for LED blinking

void IRAM_ATTR isr1()  //Setting up the ISR to set flag1 to true when raised
{
	flag1 = true;
}

void LEDBlink(int timer_value)  //function to blink and control servo, pulse length is passed to the function
{
	int i=0;
	count = 0;
	blinkdelay = (2000/timer_value);  //Setting delay value based on pulse width
	if(blinkdelay>timer_value)
	{blinkdelay=timer_value;}
	timer1.start();
  while(timer1.read()<=timer_value)  //Used to set the time that the LED toggles to be the same length as the origianl pulse
	{
		attachInterrupt(input1.PIN, isr1, CHANGE);  //Interrupt re enabled
		if(pos>=180)  //sets servo position to 0 if it is more than or equal to 180
    {
      pos=0;
    }  
    else
    {
      pos=pos+30;  //increases the servo position variable by 30deg every iteration
    }
		if(timer1.read()%25==0)  //servo commands are given every 25ms because of the frequency of the PWM control signal  
    {
			servo1.write(pos);  //sets the servo position based on the current value 
		}
		Serial.print("Servo position:");
		Serial.println(pos);
    digitalWrite(22, HIGH);    // Turn LED on
    delay(blinkdelay);        // Wait time
    digitalWrite(22, LOW);     // Turn LED off
    delay(blinkdelay);				// Wait time
		count++;
		Serial.print("LED blink count:");
		Serial.println(count);   // Print pulse width
		if(flag1){timer1.stop();return;}
  }
	servo1.write(pos);  //writing servo position incase the pulse width less than 25ms 
	timer1.stop();  //stops timer after blinking sequence
}

void setup() {
	Serial.begin(115200);  //Set serial baud rate
  servo1.attach(25, 500, 2400);  //Attaching the servo at pin 18 and setting the maximum and minimum PWM values
	pinMode(input1.PIN, INPUT_PULLDOWN);  //pin 19 set as Pulldown input to read the pulse
	pinMode(22,OUTPUT);  //setting pin 14 as LED output
	attachInterrupt(input1.PIN, isr1, CHANGE);  //Attaching an interrupt to pin 19 that is triggered by any edge change
	input1.state = true;
	servo1.write(0);  //setting servo to 0 deg
}

void loop() 
{
		if(flag1)
	{
		flag1 = 0;
		if(flag2 == false)  //starting timer after ISR is executed, flag 2 is raised to denote measurement start
		{
				flag2 = true;
				timer1.start();
				flag1 = false;
		}	
		else if(flag2 == true)  //stops pulse width measurement after the ISR triggers on pulse stop
		{
				timer1.stop();  //stops timer
				detachInterrupt(18);  //Interrupt temporarily disabled until blinking completed
				flag2 = false;  
				timer_value = timer1.read();  //reading and saving timer value in ms
				if(timer_value<10)  //limiting the pulse values between 10 - 1000 ms
				{timer_value = 10;}
				else if(timer_value>1000)
				{timer_value = 1000;} 
				Serial.print("Pulse width in ms:"); 
				Serial.println(timer_value);
				LEDBlink(timer_value);  //function to blink the LEDs
				
		}
	}
}