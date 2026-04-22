//LIBRERIAS//
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

/* Apartado 1 - Control de luz mediante LED y sensor LDR */

const byte pinLedRed = 2;  // pinLed -> Apartado 1

/* Apartado 2 - Regulacion de luz mediante deslizador PWM */

const byte pinLedSlider = 3;

// LDR
const byte LdrPin = A15;
int sensorValor;
int sensorLow = 1023;
int sensorHigh = 0;
int lecturaLimitada;

byte brillo;
byte minPWM = 0;
byte maxPWM = 255;

// MOTOR AIRE ACONDICIONADO //
const byte motorIn1 = 5;
const byte motorIn2 = 6;
boolean motorEncendido = false;


//DHT//
#define DHTPIN 9
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temperatura = 0.0;
float humedad = 0.0;
unsigned long tiempoActualDHT;
unsigned long tiempoAnteriorDHT = 0;
const unsigned long PERIODODHT = 2000;
const byte tRef = 26;

// PANTALLA LCD 0X3F//
LiquidCrystal_I2C lcd(0x27, 16, 2);
const unsigned long PERIODO_LCD = 2000;
unsigned long tiempoActualLCD;
unsigned long tiempoAnteriorLCD = 0;


// RGB
const byte pinRGB_R = 11;
const byte pinRGB_G = 12;
const byte pinRGB_B = 13;

/*ServoMotor*/

// Servo
Servo servo;
const byte pinservo = 22;
byte angulo;

//RELÉ
const byte pinRele = 30;
boolean estadoRele = false;


boolean ledLuzAut = false;
boolean motorAut = false;
boolean datoBT;


char lecturaComando;
int lecturaValor;


boolean debug = true;
boolean calibra_ON = true;
boolean debugLDR = false;
boolean dhtOK = false;

void sliderR(int);
void sliderG(int);
void sliderB(int);

boolean lecturaValorRGB();
void accionSliderR();
void accionSliderG();
void accionSliderB();


void setup() {
  Serial.begin(9600);  
  Serial1.begin(9600);
  configuraPines();
  dht.begin();
  Wire.begin();
  lcd.begin(16, 2);
  lcd.backlight();
  if (calibra_ON) calibraLDR();
  servo.attach(pinservo);
  servo.write(0);
}


void loop() {
  lecturaSerial();
  calculoTemperatura();
  fAutomaticas();
  gestionLCD();
}

void configuraPines() {
  pinMode(pinLedRed, OUTPUT);
  digitalWrite(pinLedRed, LOW);

  pinMode(pinLedSlider, OUTPUT);
  digitalWrite(pinLedSlider, LOW);

  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);

  pinMode(pinRele, OUTPUT);
  digitalWrite(pinRele, LOW);
}

void lecturaSerial() {
  if (Serial.available() > 0) {
    lecturaComando = Serial.read();
    if (debug) Serial.print("Comando leido Serial: ");
    if (debug) Serial.println(lecturaComando);

    datoBT = false;
    actuaciones();
  } else if (Serial1.available() > 0) {
    lecturaComando = Serial1.read();
    if (debug) Serial.print("Comando leido BT: ");
    if (debug) Serial.println(lecturaComando);

    datoBT = true;
    actuaciones();
  }
}

void fAutomaticas() {
  if (ledLuzAut) controlLedLuz();
  if (motorAut) controlMotorAuto();
}


void gestionLCD() {
  tiempoActualLCD = millis();
  if (tiempoActualLCD - tiempoAnteriorLCD >= PERIODO_LCD) {
    tiempoAnteriorLCD = tiempoActualLCD;
    mostrarTempHumedad();
  }
}

void actuaciones() {
  if ((lecturaComando == 'l') && (!ledLuzAut)) apagaLedLuz();
  else if ((lecturaComando == 'L') && (!ledLuzAut)) enciendeLedLuz();
  else if (lecturaComando == 'a') modoLedLuz(false);
  else if (lecturaComando == 'A') modoLedLuz(true);
  else if (lecturaComando == 'S') accionLedSlider(true);
  else if (lecturaComando == 's') accionLedSlider(false);

  else if (lecturaComando == 'F') motorManualOn();
  else if (lecturaComando == 'f') motorManualOff();
  else if (lecturaComando == 'T') motorModoAutomatico();
  else if (lecturaComando == 't') motorModoManual();

  else if (lecturaComando == 'E') releOn();
  else if (lecturaComando == 'e') releOff();

  else if (lecturaComando == 'C') escena1();
  else if (lecturaComando == 'M') escena2();
  else if (lecturaComando == 'Y') escena3();

  else if (lecturaComando == 'o') rgbOff();

  else if (lecturaComando == 'R') accionSliderR();
  else if (lecturaComando == 'G') accionSliderG();
  else if (lecturaComando == 'B') accionSliderB();

  else if (lecturaComando == 'O') abrirPuerta();
  else if (lecturaComando == 'W') cerrarPuerta();
  else if (lecturaComando == 'P') accionSliderPuerta();

  vaciaSerial();
}

void vaciaSerial() {
  while (Serial.available() > 0) Serial.read();
  while (Serial1.available() > 0) Serial1.read();
  if (debug) Serial.println("Serial vaciado ... esperando nuevo comando");
}

//FUNCIONES//

//On/OFF/Slider luces Led
void apagaLedLuz() {
  if (debug) Serial.println("Led Luz -> OFF");
  digitalWrite(pinLedRed, LOW);
}

void enciendeLedLuz() {
  if (debug) Serial.println("Led Luz -> ON");
  digitalWrite(pinLedRed, HIGH);
}

void modoLedLuz(boolean modoLuz) {
  if (modoLuz) {
    if (debug) Serial.println("Led Luz automatico -> ON");
    ledLuzAut = true;
  } else {
    if (debug) Serial.println("Led Luz automatico -> OFF ");
    ledLuzAut = false;
  }
}

void accionLedSlider(boolean actuar) {
  if (actuar) {
    if (lecturaValorLedSlider()) analogWrite(pinLedSlider, lecturaValor);
  } else {
    if (debug) Serial.println("Led Slider -> OFF ");
    digitalWrite(pinLedSlider, LOW);
  }
}

boolean lecturaValorLedSlider() {
  delay(5);

  if ((Serial.available() > 0) && (!datoBT)) {
    lecturaValor = Serial.parseInt();

    if (debug) Serial.print("Valor leido Serial led Slider: ");
    if (debug) Serial.println(lecturaValor);

    if ((lecturaValor < 0) || (lecturaValor > 255)) {
      if (debug) Serial.print("Valor incorrecto (1-255): ");
      if (debug) Serial.println(lecturaValor);
      return false;
    } else return true;

  } else if ((Serial1.available() > 0) && (datoBT)) {
    lecturaValor = Serial1.parseInt();

    if (debug) Serial.print("Valor leido BT led Slider: ");
    if (debug) Serial.println(lecturaValor);

    if ((lecturaValor < 1) || (lecturaValor > 255)) {
      if (debug) Serial.print("Valor incorrecto (1-255): ");
      if (debug) Serial.println(lecturaValor);
      return false;
    } else return true;
  } else {
    if (debug) Serial.println("Comando incompleto - S + numero (1-255)");
    return false;
  }
}

void controlLedLuz() {
  sensorValor = analogRead(LdrPin);
  lecturaLimitada = constrain(sensorValor, sensorLow, sensorHigh);
  brillo = map(lecturaLimitada, sensorLow, sensorHigh, 0, 255);
  analogWrite(pinLedRed, brillo);
}

//CALIBRACIÓN LDR//
void calibraLDR() {
  unsigned long periodo = millis();
  while (millis() - periodo < 2000) {
    sensorValor = analogRead(LdrPin);
    if (sensorValor > sensorHigh)
      sensorHigh = sensorValor;
    if (sensorValor < sensorLow)
      sensorLow = sensorValor;
  }
}

//CALCULO TEMPERATURA DHT
void calculoTemperatura() {
  tiempoActualDHT = millis();  // Actualiza el tiempoActual con el valor de millis()

  if (tiempoActualDHT - tiempoAnteriorDHT >= PERIODODHT) {
    tiempoAnteriorDHT = tiempoActualDHT;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(t) && !isnan(h)) {
      temperatura = t;
      humedad = h;
      dhtOK = true;
    } else {
      dhtOK = false;
      if (debug) Serial.println("Error lectura DHT");
    }
  }
}

//MOSTRAR TEMPERATURA EN LCD 03XF//

void mostrarTempHumedad() {

  lcd.setCursor(0, 0);

  if (dhtOK) {
    lcd.print("Temp:");
    lcd.print(temperatura);
    lcd.write(223);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Hum:");
    lcd.print(humedad);
    lcd.print("   ");
  } else {
    lcd.print("Error DHT     ");

    lcd.setCursor(0, 1);
    lcd.print("Sin datos     ");
  }
}

//MOTOR ON/OFF

void motorManualOn() {
  if (!motorAut) motorOn();
}

void motorManualOff() {
  if (!motorAut) motorOff();
}

void motorOn() {
  digitalWrite(motorIn1, HIGH);
  digitalWrite(motorIn2, LOW);
  motorEncendido = true;
  if (debug) Serial.println("Motor -> ON");
}

void motorOff() {
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
  motorEncendido = false;
  if (debug) Serial.println("Motor -> OFF");
}

//MOTOR AUTOMATICO O MANUAL
void motorModoManual() {
  motorAut = false;
  if (debug) Serial.println("Motor -> MODO MANUAL");
}

void motorModoAutomatico() {
  motorAut = true;
  if (debug) Serial.println("Motor -> MODO AUTOMATICO");
}

void controlMotorAuto() {
  if (dhtOK) {
    if (temperatura > tRef) motorOn();
    else motorOff();
  } else {
    if (debug) Serial.println("Error DHT");
  }
}
//RELE ON/OFF
void releOn() {
  digitalWrite(pinRele, HIGH);
  estadoRele = true;
  if (debug) Serial.println("Rele -> ON");
}

void releOff() {
  digitalWrite(pinRele, LOW);
  estadoRele = false;
  if (debug) Serial.println("Rele -> OFF");
}

//RGB ON/OFF ESCENAS//
void escena1() {
  analogWrite(pinRGB_R, 255);
  analogWrite(pinRGB_G, 0);
  analogWrite(pinRGB_B, 0);
}

void escena2() {
  analogWrite(pinRGB_R, 0);
  analogWrite(pinRGB_G, 255);
  analogWrite(pinRGB_B, 255);
}

void escena3() {
  analogWrite(pinRGB_R, 255);
  analogWrite(pinRGB_G, 255);
  analogWrite(pinRGB_B, 0);
}

void rgbOff() {
  analogWrite(pinRGB_R, 0);
  analogWrite(pinRGB_G, 0);
  analogWrite(pinRGB_B, 0);
}

//RGB SLIDERS //

void sliderR(int v) {
  v = constrain(v, 0, 255);
  analogWrite(pinRGB_R, v);
}

void sliderG(int v) {
  v = constrain(v, 0, 255);
  analogWrite(pinRGB_G, v);
}

void sliderB(int v) {
  v = constrain(v, 0, 255);
  analogWrite(pinRGB_B, v);
}

void accionSliderR() {
  if (lecturaValorRGB()) sliderR(lecturaValor);
}

void accionSliderG() {
  if (lecturaValorRGB()) sliderG(lecturaValor);
}

void accionSliderB() {
  if (lecturaValorRGB()) sliderB(lecturaValor);
}

boolean lecturaValorRGB() {

  delay(5);

  if ((Serial.available() > 0) && (!datoBT)) {
    lecturaValor = Serial.parseInt();
  } else if ((Serial1.available() > 0) && (datoBT)) {
    lecturaValor = Serial1.parseInt();
  } else return false;

  lecturaValor = constrain(lecturaValor, 0, 255);
  return true;
}

//Servo abrir y cerrar//

void abrirPuerta() {
  servo.write(180);
  if (debug) Serial.println("Puerta -> ABIERTA");
}

void cerrarPuerta() {
  servo.write(0);
  if (debug) Serial.println("Puerta -> CERRADA");
}

//Servo SLider//

void moverPuerta(int angulo) {
  angulo = constrain(angulo, 0, 180);
  servo.write(angulo);
}

void accionSliderPuerta() {
  if (lecturaValorServo())
    moverPuerta(lecturaValor);
}

boolean lecturaValorServo() {

  delay(5);

  if ((Serial.available() > 0) && (!datoBT)) {
    lecturaValor = Serial.parseInt();
  } else if ((Serial1.available() > 0) && (datoBT)) {
    lecturaValor = Serial1.parseInt();
  } else return false;

  lecturaValor = constrain(lecturaValor, 0, 180);
  return true;
}
