/*
 * BlinkM Controller - A BlinkM network animator by Eric Kelley
 *
 * BlinkM connections to Arduino
 * PWR - -- gnd -- black -- Gnd
 * PWR + -- +5V -- red   -- 5V
 * I2C d -- SDA -- green -- Analog In 4
 * I2C c -- SCK -- blue  -- Analog In 5
 *
 * 2009, Eric Kelley, http://www.atmospheric.org
 *
 */

#include "Wire.h"
#include "BlinkM.h"

#define BLINKM_ARDUINO_POWERED 0

#define NUMX 6
#define NUMY 9

#define NODE_COUNT 39

#define NUM_ANIMS 7
#define NUM_TRANSITIONS 2

#define TRUE 1
#define FALSE 0

#define DEFAULT_FADE_SPEED 255
#define DEFAULT_TIME_ADJUST 0
#define MAX_TRIES 10

int max_t2=25;

//GLOBALS
int currentAnimation = 0;
int currentFadeSpeed = 255;
int currentTimeAdjust = 0;

byte t1;
int t2 = max_t2;
int ledPin = 13;
byte cmd;
byte debug = 1;

int nodeMap[NUMX][NUMY] = { { 1, 2, 3, 4, -1, -1, -1, -1, -1 }, { 5, 6, 7, 8, 9,
		-1, -1, -1, -1 }, { 10, 11, 12, 13, 14, 15, -1, -1, -1 }, { 16, 17, 18,
		19, 20, 21, 22, -1, -1 }, { 23, 24, 25, 26, 27, 28, 29, 30, -1 }, { 31,
		32, 33, 34, 35, 36, 37, 38, 39 } };

//Time for an object to fall 2 meters, in increments of 10cm
//Basically a snapshot of (t=sqrt{2d/g}) where g=9.8 m/s^2
int gravityDelays[10] = {202,84,64,54,48,43,40,37,35,33};

byte hueMap[NUMX][NUMY];
byte satMap[NUMX][NUMY];
byte brightMap[NUMX][NUMY];

byte redMap[NUMX][NUMY];
byte greenMap[NUMX][NUMY];
byte blueMap[NUMX][NUMY];
byte red,green,blue;

unsigned long current;

BlinkM blinkm = BlinkM();

char serInStr[30];  // array that will hold the serial input string

void setup() {
	if ( BLINKM_ARDUINO_POWERED) {
		blinkm.powerUp();
	} else {
		blinkm.begin();
	}

	if (debug) {
		Serial.begin(19200);
		help();
		Serial.print("cmd>");
	}

	//Set to black
	initNodes();
	setBufferToHSB(0, 255, 255);
}

void dumpDebug() {
	Serial.print("Current animation: ");
	Serial.println(currentAnimation);
	Serial.print("Current Fade Speed (Default 255): ");
	Serial.println(currentFadeSpeed);
	Serial.print("Current TimeAdjust (Default 0): ");
	Serial.println(currentTimeAdjust);
}

void initNodes() {
	for (int x = 0; x < NUMX; x++) {
		for (int y = 0; y < NUMY; y++) {
			int currentNode = nodeMap[x][y];
			if (currentNode > 0) {
				blinkm.stopScript(currentNode);
			}
		}
	}
}

//Our starting state.
boolean randomize = true;
boolean doAnimate = true;
boolean doTransition = false;

unsigned long startTime = 0;
unsigned long animationDuration = 45000;
unsigned long transitionDuration = 8000;

void loop() {
	t1++;
	if (t1 == 0) {
		t2--;
		if (t2 == 0) {
			t2 = max_t2;
			toggleLed();
			loopCommand();
			if (doAnimate) {
				renderAnimation();
			}

			if (doTransition) {
				renderTransition();
			}

		}
	}

}


//Render a single frame of whatever animation is currently selected.
void renderAnimation() {

	//If we starting a new animation cycle, pick a random animation
	if (startTime == 0) {
		if (randomize) {
			randomSeed(analogRead(1));
			int newAnim = -1;
			int i;
			//Try up to MAX_TRIES to not repeat animations.
			for( i=0; i < MAX_TRIES; i++) {
				newAnim = random(NUM_ANIMS);
				if ( newAnim != currentAnimation ) {
					currentAnimation = newAnim;
					break;
				}
			}

			if(i > 0) {
				Serial.print("Took tries: ");
				Serial.println(i,DEC);
			}
		}
//		if (debug) {
//			dumpDebug();
//		}
	}

	switch (currentAnimation) {
	case 0:
		renderRainbow();
		break;
	case 1:
		//Red Green Blue
		renderLightScriptAnimation(1, 0);
		break;
	case 2:
		//Thunderstorm
		renderLightScriptAnimation(16, 0);
		break;
	case 3:
		//Hue cycle
		if (startTime == 0) {
			modifyFadeSpeed(25);
		}
		renderLightScriptAnimation(10, 0);
		break;
	case 4:
		//Mood lamp
		if (startTime == 0) {
			modifyFadeSpeed(80);
		}
		renderLightScriptAnimation(11, 0);
		break;
	case 5:
		//color flash
		//renderLightScriptAnimation(3, 0);
		renderMultiFlash();
		break;
	case 6:
		//morse code white
		renderLightScriptAnimation(18, 0);
		break;
	}

}

void renderMultiFlash() {
	current = millis();
	if (startTime == 0) {
		startTime = millis();
		randomSeed(analogRead(0));
		// Scripts 2-8 are flashing different colors
		for (int x = 0; x < NUMX; x++) {
			for (int y = 0; y < NUMY; y++) {
				int currentNode = nodeMap[x][y];
				if (currentNode > 0) {
					int randomFlashScript = random(7) + 2;
					blinkm.playScript(randomFlashScript, 0, 0, currentNode);
				}
			}
		}

	}

	if (startTime + animationDuration < current) {
		doAnimate = false;
		doTransition = true;
		startTime = 0;
		blinkm.stopScript(0);
	}
}

int currentTransition = 1;
//Render a single frame of whatever transition is currently selected.
void renderTransition() {
	//If we starting a new transition, pick a random transition
	if (startTime == 0) {
		randomSeed(analogRead(0));
		currentTransition = random(NUM_TRANSITIONS);
	}

	switch (currentTransition) {
		case 0:
			renderFadeTransition(0, 0, 0, 2);
			break;
		case 1:
			renderDropOutTransition(15);
			break;
	}
}

void copyArrayColorToRGBBuffer() {
	//Read the current state of the blinkms.
	for (int x = 0; x < NUMX; x++) {
		for (int y = 0; y < NUMY; y++) {
			if ( nodeMap[x][y] > 0 ) {
				blinkm.getRGBColor(&red,&green,&blue,nodeMap[x][y]);
//				if(debug) {
//					Serial.print("nodeMap[");
//					Serial.print(x);
//					Serial.print(",");
//					Serial.print(y);
//					Serial.print("] = ");
//					Serial.print(nodeMap[x][y]);
//					Serial.print(" = RGB (");
//					Serial.print(red);
//					Serial.print(", ");
//					Serial.print(green);
//					Serial.print(", ");
//					Serial.print(blue);
//					Serial.println(")");
//				}
				redMap[x][y] = red;
				greenMap[x][y] = green;
				blueMap[x][y] = blue;
			}
		}
	}
}


int transitionStep = 0;
void renderDropOutTransition(byte fadespeed){
	//Starting at the bottom, copy the color value from the node above
	//If there's no node above, then set yourself to black
	current = millis();

	if (startTime == 0) {
		startTime = millis();
		initNodes();
		copyArrayColorToRGBBuffer();
	}

	if( transitionStep < 10 ) {
		delay(gravityDelays[transitionStep]*2);

		//set the transition step blinkms to black
		for (int x = 0; x < NUMX; x++) {
			if ( nodeMap[x][transitionStep] > 0 ) {
				//Fade to black slowly
				blinkm.setFadeSpeed(fadespeed, nodeMap[x][transitionStep]);
				blinkm.fadeToHSB(0,0,0,nodeMap[x][transitionStep]);
			}
		}

		for (int y = (transitionStep+1); y < NUMY; y++) {
			for (int x = 0; x < NUMX; x++) {
				if ( nodeMap[x][y] > 0 ) {
					//Fade to color instantly
					blinkm.setFadeSpeed(255, nodeMap[x][y]);
					blinkm.fadeToRGB(redMap[x][y-transitionStep],greenMap[x][y-transitionStep],blueMap[x][y-transitionStep],nodeMap[x][y]);
				}
			}
		}
	}
	transitionStep++;

	if (startTime + transitionDuration < current) {
		doAnimate = true;
		doTransition = false;
		startTime = 0;
		transitionStep = 0;
		//Reset the fadespeed back to default.
		modifyFadeSpeed(DEFAULT_FADE_SPEED);
	}

}

void renderFadeTransition(byte red, byte green, byte blue, byte fadespeed) {
	current = millis();

	if (startTime == 0) {
		startTime = millis();
		initNodes();
		blinkm.setFadeSpeed(fadespeed, 0);
		blinkm.fadeToRGB(red, green, blue, 0);
	}

	if (startTime + transitionDuration < current) {
		doAnimate = true;
		doTransition = false;
		startTime = 0;
		//Reset the fadespeed back to default.
		modifyFadeSpeed(DEFAULT_FADE_SPEED);
	}
}

//Simply tells all nodes to play the specified light script for the full duration of the animation cycle.

void renderLightScriptAnimation(byte script_id, byte addr) {
	current = millis();
	if (startTime == 0) {
		startTime = millis();

		blinkm.playScript(script_id, 0, 0, 0);
	}

	if (startTime + animationDuration < current) {
		doAnimate = false;
		doTransition = true;
		startTime = 0;
		blinkm.stopScript(0);
	}

}

void resetAnimation() {
	doAnimate = true;
	doTransition = false;
	startTime = 0;
}

float offset = 0;
float length = 2;
byte curSaturation = 255;

void renderRainbow() {
	current = millis();
	if (startTime == 0) {
		startTime = millis();
		initNodes();
		int analogValue = analogRead(1);
		randomSeed(analogValue);
		//One third of the time select a more pastel saturation
		if ( random(3) == 1){
			curSaturation = random(55) + 200;
		}
		else {
			curSaturation = 255;
		}
		modifyFadeSpeed(255);
		setTimeAdj(0);
		offset = 0;
	}
	if (startTime + animationDuration < current) {
		doAnimate = false;
		doTransition = true;
		startTime = 0;
	}

	for (int x = 0; x < NUMX; x++) {
		fillBufferWithSine(offset, length, x);
	}

	//If you add, the rainbow goes up.
	//if you subtract, the rainbow goes down.
	offset = offset - 0.08;
	renderBuffer();
}

void fillBufferWithSine(float offset, float length, int fillX) {
	float stepSize = length / NUMY;
	for (int Y = 0; Y < NUMY; Y++) {
		hueMap[fillX][Y] = (sin(offset + (Y * stepSize)) + 1) * 127.6;
		//I want the saturation to ramp up over the timespan of the animation.
		//		int calculatedSaturation = (int)(((current - startTime) / ( animationDuration / 100 )) * 2.55);
		satMap[fillX][Y] = curSaturation;
//		if (debug && fillX == 0 && Y == 0) {
//			Serial.print(hueMap[fillX][Y], DEC);
//			Serial.print(" | ");
//			Serial.println(satMap[fillX][Y], DEC);
//		}
	}
}

void setBufferToHSB(byte h, byte s, byte b) {
	for (int x = 0; x < NUMX; x++) {
		for (int y = 0; y < NUMY; y++) {
			hueMap[x][y] = h;
			satMap[x][y] = s;
			brightMap[x][y] = b;
		}
	}
}

void renderBuffer() {
	for (int x = 0; x < NUMX; x++) {

		for (int y = 0; y < NUMY; y++) {
			int currentNode = nodeMap[x][y];
			if (currentNode > 0) {
				blinkm.fadeToHSB(hueMap[x][y], satMap[x][y], brightMap[x][y],
						currentNode);
			}
		}
	}
}

void help() {
	Serial.println("\r\nColorfall!\n"
			"'s <num>' -- start running animation <num>\n"
			"'f <num> -- set fade speed ( 0 - 255 ) \n"
			"'a <num> -- set time adjust ( -255 - 255 | 0 to turn off )\n");
}

/**
 * Set Fade Speed
 This command sets the rate at which color fading happens.
 It takes one argument that is the fade speed from 1-255.
 The slowest fading occurs when the fade speed is 1. To change colors instantly, set the fade speed to 255.
 A value of 0 is invalid and is reserved for a future “Smart Fade” feature.
 */

void modifyFadeSpeed(int value) {
	currentFadeSpeed = value;
	for (int x = 1; x <= NODE_COUNT; x++) {
		blinkm.setFadeSpeed(value, x);
	}
}

/**
 * Set Time Adjust
 * This command adjusts the playback speed of a light script.
 * It takes one byte as an argument, a signed number between -128 and 127.
 * The argument is treated as an additive adjustment to all durations of the script being played.
 * A value of 0 resets the playback speed to the default.
 */
void setTimeAdj(int value) {
	currentTimeAdjust = value;
	for (int x = 1; x <= NODE_COUNT; x++) {
		blinkm.setTimeAdj(value, x);
	}
}

void loopCommand() {
	if (!readSerialString()) {  // did we read a string?
		return;
	}

	// yes we did. we can has serialz
	Serial.println(serInStr); // echo back string read
	char cmd = serInStr[0];  // first char is command
	char* str = serInStr + 1;  // get me a pointer to the first char

	// most commands are of the format "addr num"
	int num = strtol(str, &str, 10);

	switch (cmd) {
	case '?':
		help();
		break;
	case 's':
		if (num >= -1 && num < NUM_ANIMS) {
			Serial.print("Starting animation # ");
			Serial.println(num);
			if (num >= 0) {
				currentAnimation = num;
				randomize = false;
				resetAnimation();
			} else {
				randomize = true;
				resetAnimation();
			}
		}
		break;
	case 'f':
		if (num >= 0 && num <= 255) {
			Serial.print("Setting fade speed to: ");
			Serial.println(num);
			modifyFadeSpeed(num);
		}
		break;
	case 'a':
		Serial.print("Setting time adjust to: ");
		Serial.println(num);
		setTimeAdj(num);
		break;
	case 'm':
		Serial.print("Setting maxt2 to: ");
		Serial.println(num);
		max_t2 = num;
		break;
	case 'S':
		Serial.print("Setting sat to: ");
		Serial.println(num);
		curSaturation = num;
		break;

	default:
		Serial.println(" unknown cmd");
	}

	Serial.print("cmd> ");
}

//read a string from the serial and store it in an array
//you must supply the array variable
uint8_t readSerialString() {
	if (!Serial.available()) {
		return 0;
	}
	delay(10);  // wait a little for serial data
	int i = 0;
	while (Serial.available()) {
		serInStr[i] = Serial.read();   // FIXME: doesn't check buffer overrun
		i++;
	}
	serInStr[i] = 0;  // indicate end of read string
	return i;  // return number of chars read
}

void toggleLed() {
	digitalWrite(ledPin, digitalRead(ledPin) == HIGH ? LOW : HIGH);

}

void validateNode(int x) {

	//read a value from the node
	int address = blinkm.checkAddress();
	if (address < 0) {
		Serial.print("FAIL: ");
		Serial.print(x);
		Serial.print(" != ");
		Serial.println(address);
	}

}

