#include <NewPing.h>

#define TRIG_PIN A1  // TRIG connected to A1
#define ECHO_PIN A2  // ECHO connected to A2
#define MAX_DISTANCE 200  // Maximum distance in cm

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {
  Serial.begin(9600);  // Start Serial Monitor for debugging
}

void loop() {
  int distance = sonar.ping_cm();  // Measure distance in cm
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(500);  // Wait for 500ms before the next reading
}
