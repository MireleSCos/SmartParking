//Adicionado ao controle de vagas a parte do controle da cancela de saida 
//Falta o controle de entrada com o RFID e outro servo motor 

#include <LiquidCrystal.h> //Carrega a biblioteca LiquidCrystal
#include <Servo.h> //Carrega a biblioteca do Servo Motor
//#include <NewPing.h> //Carrega a biblioteca para o sensor ultrasonico

//Constantes
#define QTD_VAGAS  3 //Quantidade de vagas do sistema
#define PIN_SERVO 3  // Pino que está conectado o servo motor
#define PIN_TRIGGER 7 // Pino Trigger responsável por emitir o som do sensor ultrasonico
#define PIN_ECHO 5 // Pino Echo responsavel por receber o som do sensor ultrasonico 
#define MAX_DISTANCIA 30 // Valor da maxima distancia em centrimetro

//Parametros para objetos das bibliotecas
LiquidCrystal lcd(13, 12, 11, 10, 9, 8); //Define os pinos que serão utilizados para ligação ao display
Servo servo;

//variaveis independentes
int situ_Vaga = 0;//Situação das vagas (valores analogicos)
int vagas_Liv = 0; // Quantidade de vagas livres
int posi_VagaLiv = 0; // Posição da vaga livre
int movi_Servo = 0; // Movimento do Servo - Graus 
unsigned long tempo; // Armazena a duração do pulso do som sem sinais 
double distancia; // Armazenara a distancia do objeto (carro)



//variavel da entradas analógicas dos sensores

int senVags[QTD_VAGAS] = {0,1,2}; // Pinos que estão localizados os sensores fotoresistores

//Saida dos leds das vagas 

int ledsLivVaga[QTD_VAGAS] = {6,4,2}; // Pinos que estão localizados os leds que avisam se a vaga está liberada (verde)

void setup()
{

  lcd.begin(16, 2);
  servo.attach(PIN_SERVO); // Associar o servo a porta 
  servo.write(movi_Servo); // iniciando a posição do servo;
  Serial.begin(9600);
  
  for (int i = 0; i < QTD_VAGAS; i++) // Definondo os pinos dos leds como de Saida 
  {
    pinMode(ledsLivVaga[i], OUTPUT);
  }

  pinMode(PIN_TRIGGER,OUTPUT); // Definir o pino trigger como de saida
  pinMode(PIN_ECHO,INPUT); //Definir o pino echo como entrada

  for (int i = 0; i < QTD_VAGAS; i++) // Inicializando os leds em valor baixo (desligados)
  {
    digitalWrite(ledsLivVaga[i], LOW);
  }

  digitalWrite(PIN_TRIGGER,LOW);

}

void loop()
{

// --- + --- + --- Verificando as vagas --- + --- + ---- //

  vagas_Liv = 0;
  posi_VagaLiv = 0;
  
  for (int i = 0; i < QTD_VAGAS; i++) // Lendo os valores dos sensores
  {
    situ_Vaga = analogRead(senVags[i]); // valores de 0 a 1023

    if(situ_Vaga < 223){ // menor valor = menor luz = vaga ocupada = LED apagado
      digitalWrite(ledsLivVaga[i], LOW);
    }else{              // maiores valores = maior nível de luz = vaga livre = LED ligado
      digitalWrite(ledsLivVaga[i], HIGH);
      vagas_Liv += 1;
      posi_VagaLiv = i+1;
    }

  }

  
  lcd.clear();//Limpa a tela
  
  lcd.setCursor(3, 0);//Posiciona o cursor na coluna 3, linha 0;
  
  lcd.print("Livres : ");//Envia o texto entre aspas para o LCD
  lcd.print(vagas_Liv);
  lcd.setCursor(3, 1);
  lcd.print(" Local:");
  lcd.print(posi_VagaLiv); // Posição da vaga livre "mais proxima"

  delay(5000);
   
  //Rolagem para a esquerda
  for (int posicao = 0; posicao < 3; posicao++)
  {
    lcd.scrollDisplayLeft();
    delay(300);
  }
   
  //Rolagem para a direita
  for (int posicao = 0; posicao < 6; posicao++)
  {
    lcd.scrollDisplayRight();
    delay(300);
  }

// --- + --- + --- Verificação das vagas Concluida  --- + --- + ---- //


// --- + --- + --- Verificando a saida dos veiculos  --- + --- + ---- //

 
  digitalWrite(PIN_TRIGGER, HIGH); // Ativa por 10 milisegundo faz com que seja emitido 8 pulsos de 40KHz
  delayMicroseconds(10);
  digitalWrite(PIN_TRIGGER, LOW);

  tempo =  pulseIn(PIN_ECHO, HIGH); // Espera o pino ir para high e começa a temporizar em uma contagem de microsegundos, quando volta a onda o pino vai para low e o valor do tempo é salvo na variavel

  distancia = tempo/56; // Para obter a distancia do obstaculo

  if (distancia < MAX_DISTANCIA ) // Se a distancia for menor que 50 centimetros se afirma que o carro está querendo passar 
  {
    if (movi_Servo == 90) // Se a cancela estiver aberta deve-se continuar  
    {
    }else{//mas se ela estiver fechada deve-se abrir
      for (; movi_Servo <=90 ; movi_Servo++) // A cancela abre
      {
        servo.write(movi_Servo);
        delay(30); // Aguarda milisegundo em 90 graus
      }
    }
      
  }
  else{ // Se a distancia for maior não existe mais carro e pode fechar a cancela 
  
    if (movi_Servo != 0){
      for (; movi_Servo >= 0; movi_Servo--)
      {
        servo.write(movi_Servo);
        delay(30); // Aguarda milisegundo em 90 graus
      }
    }else{
    }
  }
  // --- + --- + --- Verificação da saida dos veiculos concluida --- + --- + ---- //
}