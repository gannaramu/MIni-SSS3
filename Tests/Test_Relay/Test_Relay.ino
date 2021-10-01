/* RGB LED Blink, Teensyduino Tutorial #2
   http://www.pjrc.com/teensy/tutorial2.html

   This example code is in the public domain.
*/

const int relaypin =  16;
//const int greenPin =  21;

// The setup() method runs once, when the sketch starts

void setup()   {                
  // initialize the digitals pin as an outputs
  pinMode(relaypin, OUTPUT);
//  pinMode(greenPin, OUTPUT);
}

// the loop() method runs over and over again,

void loop()                     
{
  digitalWrite(relaypin, HIGH);
  delay(500);
//  digitalWrite(greenPin, HIGH);
//  delay(500);
//  digitalWrite(redPin, LOW);
//  delay(500);
  digitalWrite(relaypin, LOW);
  delay(500);

}
