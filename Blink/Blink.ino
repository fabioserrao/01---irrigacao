/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Blink/
*/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(7, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(7, HIGH);  // change state of the LED by setting the pin to the HIGH voltage level
  delay(1000);                      // wait for a second
  digitalWrite(7, LOW);   // change state of the LED by setting the pin to the LOW voltage level
  delay(1000);                      // wait for a second
}
