/* -------------Trigger of Robots-------------- v1.3
    Un codigo para una arracador para robots que soporta cualquier control IR (Universal)
    basado de en microcotrolador attiny13A.
                           ________
    (Reset/ADC0)PB5(D5) --|1°     8|-- VCC
          (ADC3)PB3(D3) --|2      7|-- (D2)PB2(SCK)
          (ADC2)PB4(D4) --|3      6|-- (D1)PB1(MISO)
                GND     --|4______5|-- (D0)PB0(MOSI)

  -----[20/08/2020]----------------------------------------------------------------
  * En esta version se toman muestras de la señal cada cierto tiempo durante un tiempo de 
  * muestreo total guardandola en 12bytes donde se va poniento el estado logico medido en 
  * muetra y se va haciendo un corriemiento de bits hasta llenar los 12bytes (12x8 bi/muestras).
  ---------------------------------------------------------------------------------
*/
#include <EEPROM.h>
#include <Arduino.h>

const byte ledG = PB0, ledR = PB2, button = PB3, IRS = PB1, Sout = PB4;
const int t1 = 1000, t3 = 500, time_M=500;  //time_M es el tiempo entre cada muestra
const byte Start=0, Stop=1, Ready=2;
byte counter = 0, i, j, state = 0, Courrent_senal[12];


void setup() {

//  DDRB |= ((1 << ledG) | (1 << ledR) | (1 << Sout)); //definiendo entradas y salidas, por defecto las salidas estan en BAJO
//  DDRB &= ~((1 << IRS) | (1 << button));
//  PORTB |= (1 << button);       //Activando el pull_up para el botton
  pinMode(ledG, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(Sout,OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(IRS, INPUT);
  state = 1;
}

void loop() {
  switch (state) {
    case 0: {
//        PORTB |= ((1 << ledG) | (1 << Sout)); //START
//        PORTB &= ~(1 << ledR);
        digitalWrite(ledG, HIGH);
        digitalWrite(Sout, HIGH);
        digitalWrite(ledR,LOW);
        break;
      }
    case 1: {
//        PORTB |=  (1 << ledR); //STOP
//        PORTB &= ~((0 << ledR) | (1<<ledG));
        digitalWrite(ledG, LOW);
        digitalWrite(Sout, LOW);
        digitalWrite(ledR,HIGH);
        break;
      }
    case 2: {
//        PORTB |= ((1<<ledG) | (1<<ledR));    //READY
//        PORTB &= ~(1 << Sout);
        digitalWrite(ledG, HIGH);
        digitalWrite(Sout, LOW);
        digitalWrite(ledR,HIGH);
        break;
    }
    default:{
      break;
    }
  }
  waitSenal();
  StateTrigger();
  ProgramingActions();
}

void StateTrigger() {
  readSenalBytes();
  if (Compare_Senals(Start)) {
    state = Start;        //start
  }
  else if (Compare_Senals(Stop)) {
    state = Stop;        //stop
  }
  else if (Compare_Senals(Ready)) {
    state = Ready;        //ready
  }
  else{
  }
}


void readSenalBytes() {
  for(j=0;j<12;j++){
    for(i=0;i<8;i++){
      if(!(PINB & (1<<IRS))){
        Courrent_senal[j] << 1;   //corrimiento antes de meter el dato
        Courrent_senal[j] = 0x01; //intriduciendo 1 en el LSB.
      }
      else{
        Courrent_senal[j] << 1; //corrimiento de 1 espacion (añade 0 en LSB)
      }
      _delay_us(time_M);
    }
  }
}


void waitSenal() {
  while (digitalRead(IRS)==HIGH && digitalRead(button)==HIGH){  // miestras que IRS esta en alto y el boton no se precione.
  }
}
void waitSenal1() {
  while (digitalRead(IRS)==HIGH){  // miestras que IRS esta en alto y el boton no se precione.
  }
}

  
void ProgramingActions(){
  if(bit_is_clear(PINB,button)){
    i=0;
    while(bit_is_clear(PINB,button)&& i<3 ){
      _delay_ms(t1);
      i++;
    }
    if(i==3){
      PORTB |= ((1<<ledG) | (1<<ledR));

      waitSenal1();
      readSenalBytes();
      WriteSenal_eeprom(Start);      //escribiedo en la EEPROM el valor de START
      
      inticationLED();
      waitSenal1();
      readSenalBytes();
      WriteSenal_eeprom(Stop);      //escribiedo en la EEPROM el valor de Stop
      
      inticationLED();
      waitSenal1();
      readSenalBytes();
      WriteSenal_eeprom(Ready);      //escribiedo en la EEPROM el valor de Ready

      PORTB &= ~((1<<ledG) | (1<<ledR));     //Apagando led verde Y rojo
      _delay_ms(100);
      
    }
    i=0;
    state=1;
  }
}

void inticationLED(){
//    PORTB &= ~((1<<ledG) | (1<<ledR));
    digitalWrite(ledG, LOW);
    digitalWrite(ledR, LOW);
    delay(t3);
    digitalWrite(ledG, HIGH);
    digitalWrite(ledR, HIGH);
//    PORTB |=((1<<ledG) | (1<<ledR));
}

bool Compare_Senals(byte n){
  j=0;
  for(i=0;i<12;i++){
    if(Courrent_senal[i]=EEPROM.read(i+(12*n))){
      j++;
    }
    else{
      i=12;
    }
  }
  if(j==12){
    return(true);
  }
  else{
    return(false);
  }
}
void WriteSenal_eeprom(byte a){
  for(i=0;i<12;i++){
    EEPROM.write(i+(12*a),Courrent_senal[i]);
  }
}