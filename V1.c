//Bibliotecas
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Carrega biblioteca do LCD com I2C
#include <Servo.h>              // Carrega a biblioteca do Servo Motor
#include <Ultrasonic.h>        // Carrega a biblioteca para o sensor ultrasonico
#include <SPI.h>                // Carrega biblioteca auxiliar do leitor RIFD
#include <MFRC522.h>            // Carrega biblioteca do leitor RIFD


//Constantes
#define QTD_VAGAS           3   // Quantidade de vagas do sistema
#define MAX_DISTANCIA       30  // Valor da maxima distancia em centrimetro do carro na saida

//Pinos do sensor ultrasonico
#define PIN_ECHO            5   // Pino Echo responsavel por receber o som do sensor ultrasonico 
#define PIN_TRIGGER         6   // Pino Trigger responsável por emitir o som do sensor ultrasonico

//Pinos dos servos
#define PIN_SERVO_SAIDA     8   // Pino que está conectado o servo motor DE SAIDA
#define PIN_SERVO_ENTRADA   7   // Pino que está conectado o servo motor DE ENTRADA


//Pinos do leitor
#define PIN_RST             9   // Configurando pino do Leitor
#define PIN_SDA             10  // Configurando pino do Leitor
#define PIN_MOSI            11  // Configurando pino do Leitor
#define PIN_MISO            12  // Configurando pino do Leitor
#define PIN_SCK             13  // Configurando pino do Leitor

//Pinos analógicos do LCD
#define PIN_LCD1             5  // Configurando pino do LCD A5
#define PIN_LCD2             4  // Configurando pino do LCD A4

//Pinos analógicas dos sensores 
int senVags[QTD_VAGAS] = {0,2,1};       // Pinos que estão localizados os sensores fotoresistores

//Pinos dos leds das vagas 
int ledsLivVaga[QTD_VAGAS] = {3,4,2};   // Pinos que estão localizados os leds que avisam se a vaga está liberada (verde)

//Instâncias dos objetos das bibliotecas

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);
Servo servo_Ent;
Servo servo_Sai;
MFRC522 mfrc522(PIN_SDA, PIN_RST);   // Criando instancia MFRC522
Ultrasonic ultrasonic(PIN_TRIGGER,PIN_ECHO); // // Criando instancia Sensor ultrasonico 

//variaveis independentes
int situ_Vaga = 0;                  // Situação das vagas (valores analogicos)
int vagas_Liv = 0;                  // Quantidade de vagas livres
int posi_VagaLiv = 0;               // Posição da vaga livre
int movi_Servo_Sai = 180;           // Movimento do Servo da saida- Graus 
int movi_Servo_Ent = 0;             // Movimento do Servo da entrada - Graus 
unsigned long tempo;                // Armazena a duração do pulso do som sem sinais 
double distancia;                   // Armazenara a distancia do objeto (carro)

String autorizado = " 1e 4d 43 4b"; // UID do motorista autorizado a passar
String conteudo= "";                // UID do cartão rf lido no momento da entrada

void setup()
{
    Serial.begin(9600);
    lcd.begin (16,2);       // Inicializando instância LCD
    SPI.begin();            // Init SPI bus
    mfrc522.PCD_Init();     // Inicializando  MFRC522

    servo_Ent.attach(PIN_SERVO_ENTRADA);// Associar o servo de entrada ao pino
    servo_Ent.write(movi_Servo_Ent);    // iniciando a posição do servo da entrada;
    servo_Sai.attach(PIN_SERVO_SAIDA);  // Associar o servo de saida ao pino
    servo_Sai.write(movi_Servo_Sai);    // iniciando a posição do servo da saida;
    
    mfrc522.PCD_DumpVersionToSerial();  // Mostrando detalhes do MFRCC522
    Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));


    for (int i = 0; i < QTD_VAGAS; i++) // Definindo os pinos dos leds como de Saida 
    {
        pinMode(ledsLivVaga[i], OUTPUT);
    }

    for (int i = 0; i < QTD_VAGAS; i++) // Inicializando os leds em valor baixo (desligados)
    {
        digitalWrite(ledsLivVaga[i], HIGH);
    }


}



void loop()
{

// --- + --- + --- Verificando as vagas --- + --- + ---- //
    VerificarVagas();
    ExibindoLCD();
// --- + --- + --- Verificação das vagas Concluida  --- + --- + ---- //

// --- + --- + --- Verificando a saida dos veiculos  --- + --- + ---- //

    VerificarSaida();
  
    if (distancia < MAX_DISTANCIA ) // Se a distancia for menor que 50 centimetros se afirma que o carro está querendo passar 
    {
        AbrirCancelaSai();      
    }
    else{ // Se a distancia for maior não existe mais carro e pode fechar a cancela 
        FecharCancelaSai();
    }
// --- + --- + --- Verificação da saida dos veiculos concluida --- + --- + ---- //

// --- + --- + --- Verificando a Entrada dos veiculos  --- + --- + ---- //

    conteudo = "";

    VerificarEntrada();
    
    if (conteudo == autorizado){
        Serial.println("\n------------ Veiculo autorizado------"); 
        AbrirCancelaEnt();    
    }else{
        Serial.println("\n-------Não autorizado --------");
        FecharCancelaEnt();
    }

// --- + --- + --- Verificação da entrada dos veiculos concluida --- + --- + ---- //

}

void VerificarVagas(){

    vagas_Liv = 0;
    posi_VagaLiv = 0;
    
    for (int i = 0; i < QTD_VAGAS; i++)         // Lendo os valores dos sensores
    {
        situ_Vaga = analogRead(senVags[i]);     // Valores de 0 a 1023
        Serial.println("Vaga ");
        Serial.println(i);
        Serial.println(situ_Vaga);
        if(situ_Vaga <= 3){                     // Menor valor = pouca luz = vaga ocupada = LED apagado
            digitalWrite(ledsLivVaga[i], LOW);
        }else{                                  // Maiores valores = maior nível de luz = vaga livre = LED ligado
            digitalWrite(ledsLivVaga[i], HIGH);
            vagas_Liv += 1;
            posi_VagaLiv = i+1;
        }
    }
}

void VerificarSaida(){

    float cmMsec;
    long microsec = ultrasonic.timing();
    cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
                                           
    distancia = cmMsec;
  
    Serial.print("Distancia: ");
    Serial.println(distancia);
    delay(500);
}

void VerificarEntrada(){
    if (mfrc522.PICC_IsNewCardPresent()) {              // Verificando se há algun cartão no leitor
        Serial.println("Detectando veiculo na entrada... ");
        if (mfrc522.PICC_ReadCardSerial()){             // Lendo o cartão 
            Serial.println("Lendo ID do veiculo ... ");
                                                        //Salvar UID
            for (byte i = 0; i < mfrc522.uid.size; i++) 
            {
                conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
                conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
            }
            Serial.println("\nLeitura completa !");
            Serial.println("\nID do usuário : ");
            Serial.println(conteudo);

        }else{
            Serial.println("Erro na leitura do ID do veiculo ... ");
        }
    }else{
    }   
}

void ExibindoLCD(){

    lcd.setBacklight(HIGH);     // Ligando LCD
    lcd.clear();                //Limpa a tela
    lcd.setCursor(0,0);         //Posiciona o cursor na coluna 3, linha 0;
    lcd.print("Livres : ");     //Envia o texto entre aspas para o LCD
    lcd.print(vagas_Liv);
    lcd.setCursor(0,1);
    lcd.print(" Local:");
    lcd.print(posi_VagaLiv);    // Posição da vaga livre "mais proxima da saida"

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
}



void AbrirCancelaSai(){
    if (movi_Servo_Sai == 90){                          // Se a cancela estiver aberta deve-se continuar  
    }else{                                              // Mas se ela estiver fechada ...
        for (; movi_Servo_Sai >=90 ; movi_Servo_Sai--){ // Deve-se abrir
            servo_Sai.write(movi_Servo_Sai);
            delay(30);                                  // Aguarda milisegundo a cada grau graus
        }
    }
    delay(60);                                          // Aguarda 60 milisegundo em 90 graus antes de verificar se o carro ainda estar na saida
}

void FecharCancelaSai(){
    if (movi_Servo_Sai != 180){                          // Se a cancela estiver aberta  ... 
        for (; movi_Servo_Sai <= 180; movi_Servo_Sai++){ // Deve ser fechada 
            servo_Sai.write(movi_Servo_Sai);
            delay(30);                                 // Aguarda milisegundo a cada grau
        }
    }else{                                             // Mas se já estiver fechada deve-se continuar 
    }
}


void AbrirCancelaEnt(){
    if (movi_Servo_Ent == 90){                          // Se a cancela estiver aberta deve-se continuar  
    }else{                                              // Mas se ela estiver fechada ...
        for (; movi_Servo_Ent <=90 ; movi_Servo_Ent++){ // Deve-se abrir
            servo_Ent.write(movi_Servo_Ent);
            delay(30);                                  // Aguarda milisegundo a cada grau graus
        }
    }
    delay(50);                                          // Aguarda 60 milisegundo em 90 graus antes de verificar se o carro ainda estar na saida
}

void FecharCancelaEnt(){
    if (movi_Servo_Ent != 0){                           // Se a cancela estiver aberta  ... 
        for (; movi_Servo_Ent >= 0; movi_Servo_Ent--){  // Deve ser fechada 
            servo_Ent.write(movi_Servo_Ent);
            delay(30);                                  // Aguarda milisegundo a cada grau
        }
    }else{                                              // Mas se já estiver fechada deve-se continuar 
    }
}