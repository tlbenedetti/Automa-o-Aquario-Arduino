 /*
Thiagogo - automaçnao aquario 01

indice para busca facil


-MENU PRINCIPAL
-BACKLIGHT LCD
-MOSTRA DATA E HORA
-SENSOR DE TEMPERATURA DS18B20
-SISTEMA AUTOMATICO
-MENSAGENS DE ACIONAMENTO LCD
-ACIONAMENTO POR AGENDAMENTO ALIMENTADOR
-MENU CONTROLES MANUAIS
-LED E SAIDAS MANUAL
-MENU AJUSTA HORA
-MENU AGENDAMENTO ALIMENTADOR
-MENU AGENDAMENTO SAIDA 1
-MENU AGENDAMENTO SAIDA 2
-MENU AGENDAMENTO SAIDA LED
-MENU TEMPERATURA DE EMERGENCIA
-MOSTRA AGENDAMENTOS

Sintaxe F() serve para guaradar a informação na ide do arduino liberando meoria ram
exemplo: Serial.println(F("S1 On")); 

*/



#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//---termometro DS18B20---
#define ONE_WIRE_BUS 4             // Porta do pino de sinal do DS18B20
OneWire oneWire(ONE_WIRE_BUS);     // Define uma instancia do oneWire para comunicacao com o sensor

float tempMin = 999;               // Armazena temperaturas minima e maxima
float tempMax = 0;

DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

byte termometru[8] = //icone termometro
{
    B00100,
    B01010,
    B01010,
    B01110,
    B01110,
    B11111,
    B11111,
    B01110
};
//--- fim termometro DS18B20---

//Inicializando LCD
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);


int buttonPin = A6; //Pino ligado ao push-button
int buttonState; //Variavel para fazer a checagem
//valores dos botões: botão 1 = 0, botão 2 = 509, botão 3 = 681 e nada apertado 767 (nada do >767) 

 int NumMin_i = 0; //numero de minutos
 int NumMin_f = 0;
 int MinhaHora_i = 0; //conversão de hora + minutos
 int MinhaHora_f = 0;

unsigned long lastMillis = 0; //contador backlight
int N = 0; //monstra momentaneo hora editada no lcd
int QtyAli = 0; //controla a quantidade de comida do alimentador
char menu = 1, menuhora = 0x01, menuAjusteM = 0x01, menuAlimentador = 1, menusaida1 = 0x01, menusaida2 = 0x01, menusaidaLED = 0x01; //Variável para selecionar o menus
char set1 = 0x00, set2 = 0x00, set3 = 0x00, set4 = 0x01, set5 = 0x01;  //Controles dos botõs on/off no mesmo botão
char T_hora = 0, T_minuto = 0, T_dia = 0, T_mes = 0, T_ano = 0; //conta hora para atualizar o relogio
char T_agendaHI = 0, T_agendaMI = 0, T_agendaHF = 0, T_agendaMF = 0; //grava hora inserida para gravar na eeprom
boolean t_butUp, t_butDown, t_butSet, t_Allbuttons; //Flags para armazenar o estado dos botões

//------------- Convert normal decimal numbers to binary coded decimal -------
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

//Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers
void setDateDs1307(byte second,        // 0-59
                   byte minute,        // 0-59
                   byte hour,          // 1-23
                   byte dayOfWeek,     // 1-7
                   byte dayOfMonth,    // 1-28/29/30/31
                   byte month,         // 1-12
                   byte year)          // 0-99
{
   Wire.beginTransmission(DS1307_I2C_ADDRESS);
   Wire.write(0);
   Wire.write(decToBcd(second));    // 0 to bit 7 starts the clock
   Wire.write(decToBcd(minute));
   Wire.write(decToBcd(hour));      // If you want 12 hour am/pm you need to set
                                   // bit 6 (also need to change readDateDs1307)
   Wire.write(decToBcd(dayOfWeek));
   Wire.write(decToBcd(dayOfMonth));
   Wire.write(decToBcd(month));
   Wire.write(decToBcd(year));
   Wire.endTransmission();
}

// Gets the date and time from the ds1307
void getDateDs1307(byte *second,
          byte *minute,
          byte *hour,
          byte *dayOfWeek,
          byte *dayOfMonth,
          byte *month,
          byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  // A few of these need masks because certain bits are control bits
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());



}


void setup()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  Wire.begin();
  Serial.begin(9600);

//--- termometro DS18B20 ---
  sensors.begin();
  // Localiza e mostra enderecos dos sensores
  Serial.println("Localizando sensores DS18B20...");
  Serial.print("Foram encontrados ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensores.");
  if (!sensors.getAddress(sensor1, 0)) 
     Serial.println("Sensores nao encontrados !"); 
  // Mostra o endereco do sensor encontrado no barramento
  Serial.print("Endereco sensor: ");
  mostra_endereco_sensor(sensor1);
  Serial.println();
  Serial.println();

 //--- fim termometro DS18B20 ---


  
  lcd.begin(16, 2); //minha tela 16x1 funciona (0,0)01234567 (0,1)01234567 que soma os 16.
  
  pinMode(A0, OUTPUT); //SAida para relês
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);


  pinMode(buttonPin, INPUT);    // Define o pino do botao como entrada
  

  digitalWrite(A0, HIGH);     // iniciar reles desligados
  digitalWrite(A1, HIGH);     // iniciar reles desligados
  digitalWrite(A2, HIGH);     // iniciar reles desligados
  digitalWrite(A3, HIGH);     // iniciar reles desligados
    
  analogWrite(6, 0); // led desligado

  // AJUSTAR HORARIO DO RELOGIO AQUI
  
  second = 00;
  minute = 16;
  hour = 01;
  dayOfWeek = 2;
  dayOfMonth = 21;
  month = 7;
  year = 16;
 //setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);

//Descomentar a lina acima para ajustar hora, upar, comentar novamente e upar de novo.

  t_butUp   = 0x00; //limpa flag do botão Up
  t_butDown = 0x00; //limpa flag do botão Down
  t_butSet  = 0x00; //limpa flag do botão Set


 lcd.createChar(1,termometru); //chama o caractere do termometro

//Mostrar nome na icialização
              analogWrite(3, 255);
              lcd.clear();
              lcd.setCursor(4, 0); 
              lcd.print("THIAGOGO");              
               delay(3000);
              lcd.clear();
              analogWrite(3, 30);

}

void loop()
{
    changeMenu(); //chama variavel do botão para funcionamento menu
    dispMenu();   //chama menu
    Backlight(); //acende por 000 segundos o lcd
}

//--------------------- MENU PRINCIPAL -----------------------------------

void changeMenu()                                       //Modifica o menu atual
{
  buttonState = analogRead(buttonPin);
    
      if(buttonState <20)    t_butSet    = 0x01;          //Botão Set pressionado? Seta flag
   
      if((buttonState >750) && t_butSet)                      //Botão Set solto e flag setada?
   {                                                    //Sim...
      t_butSet = 0x00;                                     //Limpa flag

//zera todas as flags ao precionar set
menuhora = 0x01;
menuAjusteM = 0x01;
menuAlimentador = 1;
menusaida1 = 0x01;
menusaida2 = 0x01;
menusaidaLED = 0x01;
set5 = 0x01;
T_agendaHI = 0;
T_agendaMI = 0;
T_agendaHF = 0;
T_agendaMF = 0;
      
      lcd.clear();                                     //Limpa display
      menu++;                                           //Incrementa menu
      
      if(menu > 10) menu = 1;                      //Se menu maior que 10, volta a ser 1
   
} //end changeMenu

}

void dispMenu()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

switch (menu) {

case 1: //nenhum botão apertado

      //nenhum botão apertado
      MostraData();
      if(second ==0)
      {
      Temperatura_leitura(); //mostra temperatura no lcd
      }
      Temperatura();
      AtualizaSaidas(); //modo Automático
      Alimentador(); //modo automático
      

  break;

case 2: 

      //função alimentador
      AlimentadorManual();
      
  break;

case 3:

      //CONTROLE manual
      MenuControleManual();
      dispMenuControleManual();

  break;

case 4: 

      //visualização agendamentos
    mostraconfg_agenda();
    contmostraconfg_agenda();

  break;

case 5: 

      //Ajusta relogio
      MenuHora();
      dispMenuHora();

  break;
  
case 6: 

      //agendamento alimentador
      MenuAgendaAlimentador();
      dispMenuAgendaAlimentador();

  break;
    
case 7: 

      //agendamento saida 1
      MenuSaida1();
      dispMenuSaida1();

  break;

case 8: 

      //agendamento saida 2
      MenuSaida2();
      dispMenuSaida2();

  break;

case 9: 

      //agendamento saida 2
      MenuSaidaLED();
      dispMenuSaidaLED();
  
  break;

  case 10: 

      //aviso de temperatura
      MenuTemperatura();
      dispMenuTemperatura();
  
  break;
  delay (100);


  
  } 

}


//--------------------- FIM MENU PRINCIPAL --------------------

//------------------BACKLIGHT LCD-------------------
void Backlight()
{

  buttonState = analogRead(buttonPin);

      if(buttonState <720)    t_Allbuttons    = 0x01;            //Botão Set pressionado? Seta flag

      if((buttonState >750) && t_Allbuttons)                    //Botão Set solto e flag setada?
   {                                                      //Sim...
      t_Allbuttons = 0x00;
 
                //Serial.println("BACKLIGHT ON");        
                analogWrite(3, 255);
                lastMillis = millis();
   }
        if (lastMillis > 0 && (millis() - lastMillis > 15000))
        {
                //Serial.println("BACKLIGHT Off");        
                analogWrite(3, 30);
                lastMillis = 0;
                
   }

}

//---------------------FIM BACKLIGHT LCD-------------------

//------------------------MOSTRA DATA E HORA--------------------------------------

void MostraData(){
    
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  lcd.setCursor(0, 0);   
  if (hour < 10){
  lcd.print("0");       
  }
  lcd.print(hour,DEC);   
  lcd.setCursor(2, 0); 
  lcd.print(":");
  lcd.setCursor(3, 0); 
  if (minute < 10){
  lcd.print("0");       
  }
  lcd.print(minute,DEC); 
  lcd.setCursor(5, 0); 
  lcd.print(":");             
  lcd.setCursor(6, 0); 
  if (second < 10){
  lcd.print("0");       
  }
  lcd.print(second,DEC);
  
  lcd.setCursor(11, 0);
  if (dayOfMonth < 10){
  lcd.print("0");         
  }
  lcd.print(dayOfMonth,DEC);
  lcd.setCursor(13, 0);
  lcd.print("/"); 
  lcd.setCursor(14, 0);
  if (month < 10){
  lcd.print("0");       
  }
  lcd.print(month,DEC);
  lcd.setCursor(16, 0);



/*
  Serial.print(hour, DEC);
  Serial.print(":");
  Serial.print(minute, DEC);
  Serial.print(":");
  Serial.print(second, DEC);
  Serial.print("  ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.println(year, DEC);
  Serial.print("Day of week ");
  Serial.println(dayOfWeek, DEC);
  */
}
//------------------------FIM MOSTRA DATA E HORA--------------------------------------

//---------------------SENSOR DE TEMPERATURA DS18B20----------

  void mostra_endereco_sensor(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // Adiciona zeros se necessário
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void Temperatura_leitura()
{
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  
  // Le a informacao do sensor
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(sensor1);
  // Atualiza temperaturas minima e maxima
  if (tempC < tempMin)
  {
    tempMin = tempC;
  }
  if (tempC > tempMax)
  {
    tempMax = tempC;
  }
  if (hour == 00 && minute==00 && second==00)
  {
    tempMin = tempC;
    tempMax = tempC;
  }
}
void Temperatura()
{
//mostrar temperatura de acordo com o botão
       buttonState = analogRead(buttonPin);
       float tempC = sensors.getTempC(sensor1);
       
   if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
   if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
   {                                                    //Sim...
      t_butUp = 0x00;                                    //Limpa flag

      lcd.setCursor(0, 1);
      lcd.print("                ");
      set5++;                                           //Incrementa set5
      
      if(set5 > 3) set5 = 1; 
   }

    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      lcd.setCursor(0, 1);
      lcd.print("                ");
      set5--; 
          
      if(set5 < 1) set5 = 3; 
   }
   
             
      switch(set5)                                      //Controle do set5
      {
          case 0x01:                                    //Caso 1      
                
         // escreve A1

               // Mostra temperatura  
              lcd.setCursor(0,1);
              lcd.write(1);
              lcd.setCursor(1,1);
              lcd.print("Temp.:");
              lcd.setCursor(9,1);
              lcd.print(tempC);
              lcd.setCursor(14,1);
              //Simbolo grau
              lcd.write(223);
              lcd.print("C");

                    
                //---avisos de emergencia temperatura
                if(tempC < EEPROM.read(18))
                { 
                    lcd.setCursor(0, 1);
                    lcd.print("                ");
                    set5 = 4;
                }
                if(tempC > EEPROM.read(19))
                { 
                    lcd.setCursor(0, 1);
                    lcd.print("                ");
                    set5 = 5;
                }

             
                break;
                
          case 0x02:

         // escreve A2

               // Mostra temperatura minima
               lcd.setCursor(0,1);
               lcd.write(1);
               lcd.setCursor(1,1);
               lcd.print("T min:");
               lcd.setCursor(8,1);
               lcd.print(tempMin);        
                break;
                
          case 0x03:

               // Mostra temperatura maxima
              lcd.setCursor(0,1);
              lcd.write(1);
              lcd.setCursor(1,1);
              lcd.print("T max:");
              lcd.setCursor(8,1);
              lcd.print(tempMax);
                break;
                
          case 0x04:

            //caso a temp baixo mostra essa mensagem
              analogWrite(3, 255); //acende backlight
              lcd.setCursor(0,1);
              lcd.write(1);
              lcd.setCursor(1,1);
              lcd.print("TEMP. BAIXA");
              if(tempC >= EEPROM.read(18))
                { 
                    lcd.setCursor(0, 1);
                    lcd.print("                ");
                    set5 = 1;
                }  
             
                break;

          case 0x05:

            //caso a temp alta mostra essa mensagem
              analogWrite(3, 255); //acende backlight
              lcd.setCursor(0,1);
              lcd.write(1);
              lcd.setCursor(1,1);
              lcd.print("TEMP. ALTA");
              if(tempC <= EEPROM.read(19))
                { 
                    lcd.setCursor(0, 1);
                    lcd.print("                ");
                    set5 = 1;
                }  
             
                break;

                
               delay(100);
      }
}
//---------------------FIM SENSOR DE TEMPERATURA DS18B20----------
//---------------------AVISO DE TEMPERATURA ALTA/BAIXA------------





//------------------------SISTEMA AUTOMATICO------------------------------------------
void AtualizaSaidas(){
    
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

// --- LER EEPROM E ATUALIZAR DADOS ---
  
  //Saída S1

  if (hour >=EEPROM.read(8) && hour <EEPROM.read(9))
  {
    //Serial.println("S1 On");    
    digitalWrite(A0, LOW);
  }
  else
  {
    //Serial.println("S1 Off");
    digitalWrite(A0, HIGH);      
  }
    

 NumMin_i = 0; //zera contador
 NumMin_f = 0;
 MinhaHora_i = 0; 
 MinhaHora_f = 0;

NumMin_i = (hour*60)+minute;
NumMin_f = (hour*60)+minute;

MinhaHora_i = (EEPROM.read(10)*60)+ EEPROM.read(11);
MinhaHora_f = (EEPROM.read(12)*60)+ EEPROM.read(13);
  
  if (NumMin_i >= MinhaHora_i && NumMin_f < MinhaHora_f)  
  {
    //Serial.println("S2 On");    
    digitalWrite(A1, LOW); 
  }
  else
  {
    //Serial.println("S2 Off");
    digitalWrite(A1, HIGH);      
  }         

  // -- Programação Luz led 


    if (hour >=EEPROM.read(14) && hour < EEPROM.read(15))  
  {
     //Serial.println("LED ON");        
     analogWrite(6, 255);  
  }
   else if (hour >=EEPROM.read(16) && hour < EEPROM.read(17))  
  {
     //Serial.println("LED ON");            
     analogWrite(6, 255);  
  }
  else {
     //Serial.println("LED OFF");            
     analogWrite(6, 0);
  }
  
//------------------------FIM SISTEMA AUTOMATICO------------------------------------------



//---------------------MENSAGENS DE ACIONAMENTO LCD -----------------
    
        if (hour == EEPROM.read(0) && hour == EEPROM.read(2) && hour == EEPROM.read(4)
        && hour == EEPROM.read(6) && minute==00 && second==00)
    {
              lcd.setCursor(0, 1);
              lcd.print(F("                "));
              lcd.setCursor(0, 1);
              lcd.print(F("ALIMENTANDO"));
               delay(3000);
              lcd.clear();  
    }    
    
    if (hour == EEPROM.read(8) && minute==00 && second==00)
    {
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1); 
              lcd.print("SAIDA S1");
              lcd.setCursor(11, 1); 
              lcd.print("ON");
               delay(2000);
              lcd.clear();
    }
    if (hour == EEPROM.read(9) && minute==00 && second==00)
    {
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1); 
              lcd.print("SAIDA S1");
              lcd.setCursor(11, 1); 
              lcd.print(" OFF");
               delay(2000);
              lcd.clear();
    }
    if (NumMin_i == MinhaHora_i && minute==00 && second==00)
    {
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1); 
              lcd.print("SAIDA S2");
              lcd.setCursor(11, 1); 
              lcd.print(" ON");
               delay(2000);
              lcd.clear();
    }
    if (NumMin_f == MinhaHora_f && minute==00 && second==00)
    {
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1); 
              lcd.print("SAIDA S2");
              lcd.setCursor(11, 1); 
              lcd.print(" OFF");
               delay(2000);
              lcd.clear();
    }

    if (hour == EEPROM.read(14) && hour == EEPROM.read(16) && minute==00 && second==00)
    {
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1); 
              lcd.print("LED On");
               delay(2000);
              lcd.clear();
    }

    if (hour == EEPROM.read(15) && hour == EEPROM.read(17) && minute==00 && second==00)
    {
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1); 
              lcd.print("LED Off");
               delay(2000);
              lcd.clear();

    }
        if (hour == 07 && minute == 00 && second == 03)
    {
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1); 
              lcd.print("BOM DIA");
               delay(5000);
              lcd.clear();
    }
        if (hour == 22 && minute == 10 && second == 03)
    {
              lcd.setCursor(0, 1);
              lcd.print("                ");
              lcd.setCursor(0, 1); 
              lcd.print("BOA NOITE");
               delay(5000);
              lcd.clear();
    }
  }
  //---------------- FIM MENSAGENS DE ACIONAMENTO LCD -----------------




//----------------------ACIONAMENTO POR AGENDAMENTO ALIMENTADOR-----------

void Alimentador()
  { 
   byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

//agendamento A1

  if (hour ==EEPROM.read(0) && minute==00 && second==00)  
  {
     //Serial.println("servo ON");            
     analogWrite(5, 100); //velocidade 0 - 255
     delay(EEPROM.read(1)*250); //tempo
     analogWrite(5, 0); //desliga
  }

//agendamento A2

  if (hour ==EEPROM.read(2) && minute==00 && second==00)  
  {
     //Serial.println("servo ON");            
     analogWrite(5, 100); //velocidade 0 - 255
     delay(EEPROM.read(3)*250); //tempo
     analogWrite(5, 0); //desliga  
  }

  //agendamento A3

  if (hour ==EEPROM.read(4) && minute==00 && second==00)  
  {
     //Serial.println("servo ON");            
     analogWrite(5, 100); //velocidade 0 - 255
     delay(EEPROM.read(5)*250); //tempo
     analogWrite(5, 0); //desliga  
  }

  //agendamento A4

  if (hour ==EEPROM.read(6) && minute==00 && second==00)  
  {
     //Serial.println("servo ON");            
     analogWrite(5, 100); //velocidade 0 - 255
     delay(EEPROM.read(7)*250); //tempo
     analogWrite(5, 0); //desliga  
  }

}

//----------------------ACIONAMENTO POR AGENDAMENTO ALIMENTADOR-----------


//----------------------ALIMENTADOR MANUAL---------------------

void AlimentadorManual()
  { 
   
   lcd.setCursor(0,0);                                 
   lcd.print("ALIMENTAR MAN.");


  
  
   if(buttonState >620 && buttonState <720)
   {
                lcd.setCursor(0, 1);
                lcd.print("                ");
                lcd.setCursor(0, 1);
                lcd.print("Alimentando");
                //Serial.println("servo ON");            
                analogWrite(5, 100); //velocidade 0 - 255
   }

   
      
   else{
    analogWrite(5, 0); //desliga
    lcd.print("                ");
    lcd.setCursor(0, 1);                                
    lcd.print("Alimentar ???");
  }
}

//----------------------FIM ALIMENTADOR MANUAL---------------------

//---------------------MENU CONTROLES MANUAIS---------------------

void MenuControleManual()                                       //Modifica o menu atual
{
  buttonState = analogRead(buttonPin);
    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      
         lcd.setCursor(0, 1);
         lcd.print("                ");                           //Limpa display linha 2
      menuAjusteM++;                                                         //Incrementa menu
      
      if(menuAjusteM > 0x03) menuAjusteM = 0x01;                      //Se menu maior que 3, volta a ser 1
   
} //end changeMenu

}

void dispMenuControleManual()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

 
     lcd.setCursor(0,0);
     lcd.print("CONTROLE MANUAL");
             

switch (menuAjusteM) {

case 0x01: 

      //controle manual Led
      led();
         
      
  break;

case 0x02: 

      //controle manual saida 1/
      S1();
     
  break;

case 0x03: 

      //controle manual saida 2
      S2();
         

  break;
    
  delay (100);
  
  } 

}
//---------------------FIM MENU CONTROLES MANUAIS---------------------

//----------------------LED E SAIDAS MANUAL---------------------

void led()                                           //Acionamento (menu3)
{
     
   lcd.setCursor(0,1);                                 
   lcd.print("Controle LED");

  
   if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
   if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
   {                                                    //Sim...
      t_butUp = 0x00;                                    //Limpa flag
      
      set1++;                                           //Incrementa set1
      
      if(set1 > 2) set1 = 0x01;                         //Se maior que 2, volta a ser 1
      
      switch(set1)                                      //Controle do set1
      {
          case 0x01:                                    //Caso 1      
                lcd.setCursor(0, 1);
                lcd.print("                ");  
                lcd.setCursor(0, 1); 
                lcd.print("             ON ");
                //Serial.println("LED ON");        
                analogWrite(6, 255);
             
                break;                                  //Break
          case 0x02:                                    //Caso 2
                lcd.setCursor(0, 1);
                lcd.print("                "); 
                lcd.setCursor(0, 1); 
                lcd.print("             OFF");
                //Serial.println("LED Off");        
                analogWrite(6, 0);

                break;                                  //Break
      
      } //end switch set1
   
   } //end butP
}

void S1()
  {
     
   lcd.setCursor(0,1);                               
   lcd.print("Saida 1");                                  //Imprime mensagem do menu 4
  
  
   if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
   if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
   {                                                    //Sim...
      t_butUp = 0x00;                                     //Limpa flag
      
      set2++;                                           //Incrementa set2
      
      if(set2 > 2) set2 = 0x01;                         //Se maior que 2, volta a ser 1
      
      switch(set2)                                      //Controle do set2
      {
          case 0x01:                                    //Caso 1      
                lcd.setCursor(0, 1);
                lcd.print("                ");
                lcd.setCursor(0,1);
                lcd.print("             ON ");
                //Serial.println("S1 On");    
                digitalWrite(A0, LOW);
             
                break;                                  //Break
          case 0x02:                                    //Caso 2
                lcd.setCursor(0, 1);
                lcd.print("                ");
                lcd.setCursor(0,1);
                lcd.print("             OFF");
                //Serial.println("S1 Off");
                digitalWrite(A0, HIGH);  

                break;                                  //Break
      
      } //end switch set3
   
   } //end butP
}

void S2()
  {
     
   lcd.setCursor(0,1);                               
   lcd.print("Saida 2");                                  //Imprime mensagem do menu 5
  
  
   if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
   if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
   {                                                    //Sim...
      t_butUp = 0x00;                                     //Limpa flag
      
      set3++;                                           //Incrementa set3
      
      if(set3 > 2) set3 = 0x01;                         //Se maior que 2, volta a ser 1
      
      switch(set3)                                      //Controle do set3
      {
          case 0x01:                                    //Caso 1      
                lcd.setCursor(0, 1);
                lcd.print("                ");
                lcd.setCursor(0,1);
                lcd.print("             ON ");
                //Serial.println("S1 On");    
                digitalWrite(A1, LOW);
             
                break;                                  //Break
          case 0x02:                                    //Caso 2
                lcd.setCursor(0, 1);
                lcd.print("                ");
                lcd.setCursor(0,1);
                lcd.print("             OFF");
                //Serial.println("S1 Off");
                digitalWrite(A1, HIGH);  

                break;                                  //Break
      
      } //end switch set3
   
   } //end butP
}

//----------------------FIM LED E SAIDAS MANUAL---------------------

//---------------------MENU AJUSTA HORA---------------------

void MenuHora()                                       //Modifica o menu atual
{
  buttonState = analogRead(buttonPin);
    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      
      lcd.setCursor(0, 1);
      lcd.print("                ");                                                   //Limpa display
      menuhora++;                                                         //Incrementa menu
      
      if(menuhora > 0x06) menuhora = 0x01;                              //Se menu maior que 6, volta a ser 1
   
} //end changeMenu

}

void dispMenuHora()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

             lcd.setCursor(0,0);
             lcd.print("SETUP RELOGIO");

switch (menuhora) {
case 0x01: 

      //muda hora
             lcd.setCursor(0,1);                                 
             lcd.print("Hora");
             
         if(buttonState >620 && buttonState <720)
         { T_hora++;
          
            if(T_hora > 23) T_hora = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_hora;
            
             lcd.setCursor(8,1);
             if (T_hora < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);   
 
      
  break;

case 0x02: 

      //muda minuto
             lcd.setCursor(0,1);                                 
             lcd.print("Minuto");
             
             
         if(buttonState >620 && buttonState <720)
         { T_minuto++;
            if(T_minuto >59) T_minuto = 0;
         }
            N = 0;
            N = T_minuto;
            
             lcd.setCursor(8,1);
             if (T_minuto < 10){
             lcd.print("0");       
             }                                 
             lcd.print(N);
             delay(100);

             
  break;

case 0x03: 

      //muda dia do mes
             lcd.setCursor(0,1);                                 
             lcd.print("Dia");
             
         if(buttonState >620 && buttonState <720)
         { T_dia++;
            if(T_dia >31) T_dia = 0;
         }
            N = 0;
            N = T_dia;

             lcd.setCursor(8,1);
             if (T_dia < 10){
             lcd.print("0");       
             }                                 
             lcd.print(N);
             delay(100);

  break;
  
case 0x04: 

      //muda mes
             lcd.setCursor(0,1);                                 
             lcd.print("Mes");
             
         if(buttonState >620 && buttonState <720)
         { T_mes++;
            if(T_mes >12) T_mes = 0;
         }
            N = 0;
            N = T_mes;
            
             lcd.setCursor(8,1);
             if (T_mes < 10){
             lcd.print("0");       
             }                                 
             lcd.print(N);
             delay(100);
  break;
    
case 0x05: 

      //muda ano
             lcd.setCursor(0,1);                                 
             lcd.print("Ano");
             
         if(buttonState >620 && buttonState <720)
         { T_ano++;
         
            if(T_ano >99) T_ano = 0;
         }
            N = 0;
            N = T_ano;
            
             lcd.setCursor(8,1);
             if (T_ano < 10){
             lcd.print("0");       
             }                              
             lcd.print(N);
             delay(100);


  break;
  
  case 0x06: 

      //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar ?");


             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 

  //second = 00;
  minute = T_minuto;
  hour = T_hora;
  //dayOfWeek = 2;
  dayOfMonth = T_dia;
  month = T_mes;
  year = T_ano;
 setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);

             lcd.setCursor(8,1);                                 
             lcd.print("  SALVO");
             delay(1000);
              } 


  break;
  delay (100);
  
  } 

}
//---------------------FIM MENU AJUSTA HORA---------------------


//---------------------MENU AGENDAMENTO ALIMENTADOR---------------------

void MenuAgendaAlimentador()                                       //Modifica o menu atual
{
  buttonState = analogRead(buttonPin);
    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      
      lcd.setCursor(0, 1);
      lcd.print("                ");                                                   //Limpa display
      menuAlimentador++;                                                         //Incrementa menu
      
      if(menuAlimentador > 12) menuAlimentador = 1;                      //Se menu maior que 12, volta a ser 1
   
} //end changeMenu
}

void dispMenuAgendaAlimentador()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

             lcd.setCursor(0,0);
             lcd.print("SETUP ALMENTADOR");

switch (menuAlimentador) {


case 1: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Hora inic. A1");

             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++;
          
            if(T_agendaHI > 23) T_agendaHI = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHI;
            
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");
             }                         
             lcd.print(N);
             delay(100);   
      
  break;

case 2:


             lcd.setCursor(0,1);                                 
             lcd.print("Quantidade A1");

   if(buttonState >620 && buttonState <720)
   {  
      QtyAli++;
      if(QtyAli >15) QtyAli = 0;
          }
            N = 0;
            N = QtyAli;
             lcd.setCursor(14,1);
             if (QtyAli < 10){
             lcd.print("0");
             }                            
             lcd.print(N);
             delay(100);   
   
  break;


case 3: 

       //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar A1?");

             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 

 EEPROM.write(0,T_agendaHI); //gravando hora de inicio alimentador A1 eeprom endereço '0'
 EEPROM.write(1,QtyAli);     //gravando quantidade de comida eeprom endereço '1'

             lcd.setCursor(10,1);                                 
             lcd.print("SALVO");
             delay(1000);
              } 
  break;
  
case 4: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Hora inic. A2");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++;
          
            if(T_agendaHI > 23) T_agendaHI = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHI;
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");       
             }                                  
             lcd.print(N);
             delay(100);   
      
  break;

case 5:


             lcd.setCursor(0,1);                                 
             lcd.print("Quantidade A2");

   if(buttonState >620 && buttonState <720)
   {
      QtyAli++;
      if(QtyAli >15) QtyAli = 0;
   }
            N = 0;
            N = QtyAli;
             lcd.setCursor(14,1);
             if (QtyAli < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);
   

  break;


case 6: 

       //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar A2?");


             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 

 EEPROM.write(2,T_agendaHI); //gravando hora de inicio alimentador A2 eeprom endereço '0'
 EEPROM.write(3,QtyAli);     //gravando quantidade de comida eeprom endereço '1'

             lcd.setCursor(10,1);                                 
             lcd.print("SALVO");
             delay(1000);
              } 
  break;

case 7: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Hora inic. A3");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++;
          
            if(T_agendaHI > 23) T_agendaHI = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHI;
            
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);   
      
  break;

case 8:


             lcd.setCursor(0,1);                                 
             lcd.print("Quantidade A3");

   if(buttonState >620 && buttonState <720)
   { 
      QtyAli++;
      if(QtyAli >15) QtyAli = 0;
   }
            N = 0;
            N = QtyAli;
             lcd.setCursor(14,1);
             if (QtyAli < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);
   
  break;


case 9: 

       //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar A3?");


             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 

 EEPROM.write(4,T_agendaHI); //gravando hora de inicio alimentador A3 eeprom endereço '0'
 EEPROM.write(5,QtyAli);     //gravando quantidade de comida eeprom endereço '1'

             lcd.setCursor(10,1);                                 
             lcd.print("SALVO");
             delay(1000);
              } 
  break;

case 10: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Hora inic. A4");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++;
          
            if(T_agendaHI > 23) T_agendaHI = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHI;
            
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);
      
  break;

case 11:


             lcd.setCursor(0,1);                                 
             lcd.print("Quantidade A4");

   if(buttonState >620 && buttonState <720)
   { 
      QtyAli++;
      if(QtyAli >15) QtyAli = 0;
   }
            N = 0;
            N = QtyAli;
             lcd.setCursor(14,1);
             if (QtyAli < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);
   
  break;


case 12: 

       //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar A4?");


             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 

 EEPROM.write(6,T_agendaHI); //gravando hora de inicio alimentador A4 eeprom endereço '0'
 EEPROM.write(7,QtyAli);     //gravando quantidade de comida eeprom endereço '1'

             lcd.setCursor(10,1);                                 
             lcd.print("SALVO");
             delay(1000);
              } 
  break;


}
  delay (100);
  
  } 



//---------------------FIM MENU AGENDAMENTO ALIMENTADOR---------------------



//---------------------MENU AGENDAMENTO SAIDA 1---------------------

void MenuSaida1()                                       //Modifica o menu atual
{
  buttonState = analogRead(buttonPin);
    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      
      lcd.setCursor(0, 1);
      lcd.print("                ");                                                   //Limpa display
      menusaida1++;                                                         //Incrementa menu
      
      if(menusaida1 > 0x03) menusaida1 = 0x01;                      //Se menu maior que 3, volta a ser 1
   
} //end changeMenu

}

void dispMenuSaida1()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

             lcd.setCursor(0,0);
             lcd.print("SETUP SAIDA 1");

switch (menusaida1) {

case 0x01: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Hora inic.");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++;

          
            if(T_agendaHI > 23) T_agendaHI = 0; //consertar, limitar até 23 e voltar a 0
          }
            N = 0;
            N = T_agendaHI;
            
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);   
      
  break;


case 0x02: 

      //agendamento hora final
             lcd.setCursor(0,1);                                 
             lcd.print("Hora finl.");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHF++;
          
            if(T_agendaHF > 23) T_agendaHF = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHF;
            
             lcd.setCursor(14,1);
             if (T_agendaHF < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);   

  break;
  
  case 0x03: 

      //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar ?");


             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 

            //Serial.print("Gravando numero na memoria EEPROM : ");

 EEPROM.write(8,T_agendaHI); //gravando hora de inicio saida 1 eeprom endereço '8'
 EEPROM.write(9,T_agendaHF); //gravando hora final saida 1 eeprom endereço '9'

             lcd.setCursor(8,1);                                 
             lcd.print("  SALVO");
             delay(1000);
              } 
  break;


  delay (100);
  
  } 

}
//---------------------FIM MENU AGENDAMENTO SAIDA 1---------------------

//------------------------MENU AGENDAMENTO SAIDA 2---------------------

void MenuSaida2()                                       //Modifica o menu atual
{
  buttonState = analogRead(buttonPin);
    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      
      lcd.setCursor(0, 1);
      lcd.print("                ");                                                    //Limpa display
      menusaida2++;                                                         //Incrementa menu
      
      if(menusaida2 > 0x05) menusaida2 = 0x01;                      //Se menu maior que 5, volta a ser 1
   
} //end changeMenu

}

void dispMenuSaida2()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

             lcd.setCursor(0,0);
             lcd.print("SETUP SAIDA 2");

switch (menusaida2) {
case 0x01: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Hora inic.");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++;
          
            if(T_agendaHI > 23) T_agendaHI = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHI;
            
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);
      
  break;

case 0x02: 

      //agendamento minuto inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Min. inic.");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaMI++;
          
            if(T_agendaMI > 59) T_agendaMI = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaMI;
            
             lcd.setCursor(14,1);
             if (T_agendaMI < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);
      
  break;

case 0x03: 

      //agendamento hora final
             lcd.setCursor(0,1);                                 
             lcd.print("Hora finl.");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHF++;
          
            if(T_agendaHF > 23) T_agendaHF = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHF;
            
             lcd.setCursor(14,1);
             if (T_agendaHF < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);

  break;

case 0x04: 

      //agendamento minuto final
             lcd.setCursor(0,1);                                 
             lcd.print("Min. finl.");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaMF++;
          
            if(T_agendaMF > 59) T_agendaMF = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaMF;
            
             lcd.setCursor(14,1);
             if (T_agendaMF < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);

  break;
  
  case 0x05: 

      //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar ?");


             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 


 EEPROM.write(10,T_agendaHI); //gravando hora de inicio saida 1 eeprom endereço '10'
 EEPROM.write(11,T_agendaMI); //gravando hora final saida 1 eeprom endereço '11'
 EEPROM.write(12,T_agendaHF); //gravando hora de inicio saida 1 eeprom endereço '12'
 EEPROM.write(13,T_agendaMF); //gravando hora final saida 1 eeprom endereço '13'

             lcd.setCursor(8,1);                                 
             lcd.print("  SALVO");
             delay(1000);
              } 

  break;


  delay (100);
  
  } 

}
//---------------------FIM MENU AGENDAMENTO SAIDA 2---------------------

//------------------------MENU AGENDAMENTO SAIDA LED---------------------

void MenuSaidaLED()                                       //Modifica o menu atual
{
  buttonState = analogRead(buttonPin);
    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      
      lcd.setCursor(0, 1);
      lcd.print("                ");                                                     //Limpa display
      menusaidaLED++;                                                         //Incrementa menu
      
      if(menusaidaLED > 0x06) menusaidaLED = 0x01;                      //Se menu maior que 6, volta a ser 1
   
} //end changeMenu

}

void dispMenuSaidaLED()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

             lcd.setCursor(0,0);
             lcd.print("SETUP SAIDA LED");

switch (menusaidaLED) {
case 0x01: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Hora inic. L1");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++;
          
            if(T_agendaHI > 23) T_agendaHI = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHI;
            
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);
      
  break;


case 0x02: 

      //agendamento hora final
             lcd.setCursor(0,1);                                 
             lcd.print("Hora finl. L1");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHF++;
          
            if(T_agendaHF > 23) T_agendaHF = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHF;
            
             lcd.setCursor(14,1);
             if (T_agendaHF < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);

  break;
  
  case 0x03: 

      //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar ?");


             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 


 EEPROM.write(14,T_agendaHI); //gravando hora de inicio saida 1 eeprom endereço '14'
 EEPROM.write(15,T_agendaHF); //gravando hora final saida 1 eeprom endereço '15'

             lcd.setCursor(8,1);                                 
             lcd.print("  SALVO");
             delay(1000);
              } 


  break;

case 0x04: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("Hora inic. L2");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++;
          
            if(T_agendaHI > 23) T_agendaHI = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHI;
            
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);
      
  break;


case 0x05: 

      //agendamento hora final
             lcd.setCursor(0,1);                                 
             lcd.print("Hora finl. L2");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHF++;
          
            if(T_agendaHF > 23) T_agendaHF = 0; //consertar, limitar até 23 e voltar a 0
         }
            N = 0;
            N = T_agendaHF;
            
             lcd.setCursor(14,1);
             if (T_agendaHF < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);

  break;
  
  case 0x06: 

      //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar ?");

             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 

 EEPROM.write(16,T_agendaHI); //gravando hora de inicio saida 1 eeprom endereço '16'
 EEPROM.write(17,T_agendaHF); //gravando hora final saida 1 eeprom endereço '17'

             lcd.setCursor(8,1);                                 
             lcd.print("  SALVO");
             delay(1000);
              } 


  break;


}          
  delay (100);
} 

//---------------------FIM MENU AGENDAMENTO LED---------------------

//---------------------MENU TEMPERATURA DE EMERGENCIA---------------
void MenuTemperatura()                                       //Modifica o menu atual
{
  buttonState = analogRead(buttonPin);
    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      
      lcd.setCursor(0, 1);
      lcd.print("                ");                                                   //Limpa display
      set5++;                                                         //Incrementa menu
      
      if(set5 > 0x03) set5 = 0x01;                      //Se menu maior que 3, volta a ser 1
   
} //end changeMenu

}

void dispMenuTemperatura()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

             lcd.setCursor(0,0);
             lcd.print("ALERTA TEMP.");
             lcd.setCursor(12,0);
             lcd.write(1);
             lcd.setCursor(13,0);
             lcd.write(223);
             lcd.setCursor(14,0);
             lcd.write("C");
             

switch (set5) {

case 0x01: 

      //agendamento hora inicio
             lcd.setCursor(0,1);                                 
             lcd.print("temp. minima:");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHI++; //usando a mesma variavel das horas...

          
            if(T_agendaHI > 40) T_agendaHI = 0; //consertar, limitar até 40 e voltar a 0
          }
            N = 0;
            N = T_agendaHI;
            
             lcd.setCursor(14,1);
             if (T_agendaHI < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);   
      
  break;


case 0x02: 

      //agendamento hora final
             lcd.setCursor(0,1);                                 
             lcd.print("temp. maxima:");
             
         if(buttonState >620 && buttonState <720)
         {
          T_agendaHF++; //usando a mesma variavel das horas...
          
            if(T_agendaHF > 40) T_agendaHF = 0; //consertar, limitar até 40 e voltar a 0
         }
            N = 0;
            N = T_agendaHF;
            
             lcd.setCursor(14,1);
             if (T_agendaHF < 10){
             lcd.print("0");       
             }                                   
             lcd.print(N);
             delay(100);   

  break;
  
  case 0x03: 

      //Confirmar e salva
             lcd.setCursor(0,1);                                 
             lcd.print("Salvar ?");


             if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
             if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
              {                                                    //Sim...
               t_butUp = 0x00; 

            Serial.print("Gravando numero na memoria EEPROM : ");

 EEPROM.write(18,T_agendaHI); //gravando temp. minima eeprom endereço '18'
 EEPROM.write(19,T_agendaHF); //gravando temp. maxima eeprom endereço '19'

             lcd.setCursor(8,1);                                 
             lcd.print("  SALVO");
             delay(1000);
              } 
  break;


  delay (100);
  
  } 

}

//---------------------FIM MENU TEMPERATURA DE EMERGENCIA---------------

//---------------------MOSTRA AGENDAMENTOS-------------------------

void mostraconfg_agenda()
{

       buttonState = analogRead(buttonPin);
       
   if(buttonState >620 && buttonState <720)   t_butUp   = 0x01;          //Botão Up pressionado? Seta flag
      
   if((buttonState >750) && t_butUp)                    //Botão Up solto e flag setada?
   {                                                    //Sim...
      t_butUp = 0x00;                                    //Limpa flag

      lcd.setCursor(0, 1);
      lcd.print("                ");
      set4++;                                           //Incrementa set4
      
      if(set4 > 9) set4 = 1; 
   }

    
   if(buttonState >450 && buttonState <550)   t_butDown = 0x01;          //Botão Down pressionado? Seta flag

   if((buttonState >750) && t_butDown)                                  //Botão Down solto e flag setada?
   {                                                                    //Sim...
      t_butDown = 0x00;                                               //Limpa flag
      lcd.setCursor(0, 1);
      lcd.print("                ");
      set4--; 
          
      if(set4 < 1) set4 = 9; 
   }
   
}

void contmostraconfg_agenda()
{

             lcd.setCursor(0,0);
             lcd.print("AGENDAMENTOS");
             
      switch(set4)                                      //Controle do set4
      {
          case 0x01:                                    //Caso 1      
                
         // escreve A1

             lcd.setCursor(0,1);                                 
             lcd.print("A1");            
             lcd.setCursor(3,1);
             if (EEPROM.read(0) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(0));
             lcd.setCursor(5,1);                                 
             lcd.print("H");
             lcd.setCursor(7,1);                                 
             lcd.print("-");
             lcd.setCursor(10,1);                                 
             lcd.print("Qty:");
             lcd.setCursor(14,1);
             if (EEPROM.read(1) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(1));             
             
                break;
                
          case 0x02:

         // escreve A2

             lcd.setCursor(0,1);                                 
             lcd.print("A2");            
             lcd.setCursor(3,1);
             if (EEPROM.read(2) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(2));
             lcd.setCursor(5,1);                                 
             lcd.print("H");
             lcd.setCursor(7,1);                                 
             lcd.print("-");
             lcd.setCursor(10,1);                                 
             lcd.print("Qty:");
             lcd.setCursor(14,1);
             if (EEPROM.read(3) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(3));             
             
                break;
                
          case 0x03:

         // escreve A3

             lcd.setCursor(0,1);                                 
             lcd.print("A3");            
             lcd.setCursor(3,1);
             if (EEPROM.read(4) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(4));
             lcd.setCursor(5,1);                                 
             lcd.print("H");
             lcd.setCursor(7,1);                                 
             lcd.print("-");
             lcd.setCursor(10,1);                                 
             lcd.print("Qty:");
             lcd.setCursor(14,1);
             if (EEPROM.read(5) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(5));             
             
                break; 

          case 0x04:

         // escreve A4

             lcd.setCursor(0,1);                                 
             lcd.print("A4");            
             lcd.setCursor(3,1);
             if (EEPROM.read(6) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(6));
             lcd.setCursor(5,1);                                 
             lcd.print("H");
             lcd.setCursor(7,1);                                 
             lcd.print("-");
             lcd.setCursor(10,1);                                 
             lcd.print("Qty:");
             lcd.setCursor(14,1);
             if (EEPROM.read(7) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(7));             
             
                break; 

          case 0x05: 

      //mostra agendamento agendamento saida 1
             lcd.setCursor(0,1);                                 
             lcd.print("S1");
             lcd.setCursor(3,1);                                 
             lcd.print("I");          
             lcd.setCursor(4,1);
             if (EEPROM.read(8) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(8));
             lcd.setCursor(6,1);                                 
             lcd.print(":");
             lcd.setCursor(7,1);
             lcd.print("00");       
             lcd.setCursor(9,1);
             lcd.print(" ");
             lcd.setCursor(10,1);                                 
             lcd.print("F");
             lcd.setCursor(11,1);
             if (EEPROM.read(9) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(9));
             lcd.setCursor(13,1);
             lcd.print(":");
             lcd.setCursor(14,1);
             lcd.print("00");        
                              
             
             delay(100);
             
  break;

           case 0x06: 

      //mostra agendamento agendamento saida 2
             lcd.setCursor(0,1);                                 
             lcd.print("S2");
             lcd.setCursor(3,1);                                 
             lcd.print("I");          
             lcd.setCursor(4,1);
             if (EEPROM.read(10) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(10));
             lcd.setCursor(6,1);                                 
             lcd.print(":");
             lcd.setCursor(7,1);
             if (EEPROM.read(11) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(11));      
             lcd.setCursor(9,1);
             lcd.print(" ");
             lcd.setCursor(10,1);                                 
             lcd.print("F");
             lcd.setCursor(11,1);
             if (EEPROM.read(12) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(12));
             lcd.setCursor(13,1);
             lcd.print(":");
             lcd.setCursor(14,1);
             if (EEPROM.read(13) < 10){
             lcd.print("0");       
             }
             lcd.print(EEPROM.read(13));                                             
             
             delay(100);
             
  break;
      
      //mostra agendamento agendamento led L1
          case 0x07:   
                
             //L1
             lcd.setCursor(0,1);                                 
             lcd.print("L1");
             lcd.setCursor(3,1);                                 
             lcd.print("I");         
             lcd.setCursor(4,1);
             if (EEPROM.read(14) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(14));
             lcd.setCursor(6,1);                                 
             lcd.print(":");
             lcd.setCursor(7,1);
             lcd.print("00");       
             lcd.setCursor(9,1);
             lcd.print(" ");
             lcd.setCursor(10,1);                                 
             lcd.print("F");
             lcd.setCursor(11,1);
             if (EEPROM.read(15) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(15));
             lcd.setCursor(13,1);
             lcd.print(":");
             lcd.setCursor(14,1);
             lcd.print("00");                               
             
                break;
                
      //mostra agendamento agendamento led L2
          case 0x08:

             //L2
             lcd.setCursor(0,1);                                 
             lcd.print("L2");
             lcd.setCursor(3,1);                                 
             lcd.print("I");
            lcd.setCursor(4,1);
             if (EEPROM.read(16) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(16));
             lcd.setCursor(6,1);                                 
             lcd.print(":");
             lcd.setCursor(7,1);
             lcd.print("00");       
             lcd.setCursor(9,1);
             lcd.print(" ");
             lcd.setCursor(10,1);                                 
             lcd.print("F");
             lcd.setCursor(11,1);
             if (EEPROM.read(17) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(17));
             lcd.setCursor(13,1);
             lcd.print(":");
             lcd.setCursor(14,1);
             lcd.print("00");                               
                        
             
                break; 
                
      //mostra temperatura de emergencia
          case 0x09:

             lcd.setCursor(0,1);
             lcd.write(1);
             lcd.setCursor(1,1);                                 
             lcd.print("min:");
             lcd.setCursor(6,1);
             if (EEPROM.read(18) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(18));

             lcd.setCursor(9,1);
              lcd.write(1);
              lcd.setCursor(10,1);                                 
             lcd.print("max:");
             lcd.setCursor(14,1);
             if (EEPROM.read(19) < 10){
             lcd.print("0");       
             }                                 
             lcd.print(EEPROM.read(19));
                             
                break; 
      }
}

//---------------------FIM MOSTRA AGENDAMENTOS-------------------------



/* adicionar hora e minuto na if do agendamento
 *  
 *  int NumMin = 0;
 *  int MinhaHora_i = 0;
 *  int MinhaHora_f = 0;
 *  
 *  void loop()
 *  {
 *  
 *  (hour*60)+ minute = NumMin;
 *  
 *  /* levando em consideração que as memorias EEPROM gravadas são:
 *  (51)hora inicial
 *  (52)minuto inicial
 *  (53)hora final
 *  (54)minuto final
 * 
 *  
 *  (EEPROM.read(51)*60)+ EEPROM.read(52) = MinhaHora_i;
 *  (EEPROM.read(54)*60)+ EEPROM.read(55) = MinhaHora_f;
 *  }
 */

/* Mapa EEPROM:
 *  
 *  EEPROM.read(0) hora alimentador A1
 *  EEPROM.read(1) Qty alimentador A1
 *  EEPROM.read(2) hora alimentador A2
 *  EEPROM.read(3) Qty alimentador A2
 *  EEPROM.read(4) hora alimentador A3
 *  EEPROM.read(5) Qty alimentador A3
 *  EEPROM.read(6) hora alimentador A4
 *  EEPROM.read(7) Qty alimentador A4
 *  EEPROM.read(8) hora inicio saida S1
 *  EEPROM.read(9) hora fim saida S1
 *  EEPROM.read(10) hora inicio saida S2 
 *  EEPROM.read(11) minuto inicio saida S2
 *  EEPROM.read(12) hora fim saida S2
 *  EEPROM.read(13) minuto fim saida S2
 *  EEPROM.read(14) hora inicio LED L1
 *  EEPROM.read(15) hora fim LED L1
 *  EEPROM.read(16) hora inicio LED L2
 *  EEPROM.read(17) hora fim LED L2
 *  EEPROM.read(18) temperatura minima
 *  EEPROM.read(19) temperatura maxima
 *  
 */

