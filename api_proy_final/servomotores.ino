// Código Servos
#include <Servo.h>

// Pines del ultrasonido
const int trigPin = D6;
const int echoPin = D7;

// Pines de los 4 servos
const int servoPin1 = D1; 
const int servoPin2 = D2;
const int servoPin3 = D3;
const int servoPin4 = D4;

long duration;
int distance;

// Objetos Servo
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

void setup() {
  Serial.begin(9600);

  // Pines del sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Conectar servos
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
  servo4.attach(servoPin4);

  // Posición inicial
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);

  delay(500);
}

void loop() {
  distance = getDistance();
  Serial.print("Distancia: ");
  Serial.println(distance);

  if (distance >= 50 && distance <= 85) {
    caminar();     // Camina si el objeto está ENTRE 50 y 85 cm
} else {
    parar();       // Fuera del rango → se detiene
}
}

// Patrón de caminata
void caminar() {
  // Paso 1
  servo1.write(20);
  servo2.write(180);
  servo3.write(20);
  servo4.write(180);
  delay(1000);

  // Paso 2
  servo1.write(20);
  servo2.write(180);
  servo3.write(20);
  servo4.write(180);
  delay(1000);
}

// Detener servos en posición neutral
void parar() {
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
}

// Medir distancia con ultrasonido
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  int dist = duration * 0.034 / 2;  // Convertir a cm
  return dist;
}
