#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

//Ultrassonico
const int pino_trigger = 30;
const int pino_echo = 31;

//Relé
const int pino_Rele = 28; //PINO DIGITAL UTILIZADO PELO MÓDULO RELÉ

//Controle de nivel de agua
bool diminuindoReservatorio = false;

//Ethernet Shield MAC
byte mac[] = { 0x70, 0xB3, 0xD5, 0x0A, 0xCA, 0xC3 };

//Ethernet Shield IP
byte ip[] = { 10, 0, 0, 102 };

EthernetServer server(80);

void setup() {
  //Inicializa Ethernet Shield
  Ethernet.begin(mac, ip);
  server.begin();
  
  // Inicializa LCD
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Aqua House");

  //Inicializa Ultrassonico
  pinMode(pino_trigger, OUTPUT); // Arduino envia sinal para o componente
  pinMode(pino_echo, INPUT); //Componente envia sinal para o arduino
  pinMode(pino_Rele, OUTPUT); //DEFINE O PINO COMO SAÍDA
  
  //Inicializa Relé
  digitalWrite(pino_Rele, HIGH); //MÓDULO RELÉ INICIA DESLIGADO
  
  Serial.begin(9600); //Velocidade de comunicação entre o arduino e o computador
}

void loop() {
  //Obtém Distancia
  float distancia = CalculaDistancia();

  //Calcula os litros
  float litros = (10*30)/distancia;

  //Seta o LCD
  lcd.setCursor(0,1);

  //Lógica para escrever no LCD os litros e acionar relé para liberação de agua
  if(litros<=30){    
    lcd.print((String)litros+"L              ");
  }
  else{
    lcd.print("30L                ");
    diminuindoReservatorio = true;
    digitalWrite(pino_Rele, LOW);
  }

  //Informa status do solenoide e desaciona relé de liberação de agua
  lcd.setCursor(0,0);
  if(diminuindoReservatorio == true){
    lcd.print("Aqua House     +");
    if(litros<25){
      diminuindoReservatorio = false;
      digitalWrite(pino_Rele, HIGH);
    }
  }
  else{
    lcd.print("Aqua House     -");
  }
  
  Serial.println(distancia);

  //Aguarda conexao do browser
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == 'n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 2"); //Recarrega a pagina a cada 2seg
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.print("<p style=\"font-size:300%;background-color:powderblue;text-align: center;\">Litros Aqua House : ");
          client.print("<b>");
          client.print((String)litros+"L");
          client.println("</b></p>");
          client.println("</html>");
          break;
        }
        if (c == 'n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != 'r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    }
  delay(200);
}

float CalculaDistancia() { 
  digitalWrite(pino_trigger, LOW);
  delay(5);
  digitalWrite(pino_trigger, HIGH);
  delay(10);
  digitalWrite(pino_trigger, LOW);
  float cm = pulseIn(pino_echo, HIGH);
  cm = cm / 58;
  return cm;
}
