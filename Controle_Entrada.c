#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurando pino RST do Leitor
#define SS_PIN          10         // Configurando pino SS do Leitor
String autorizado = " 1e 4d 43 4b"; // UID do motorista responsável


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Criando instancia MFRC522

void setup() {
  Serial.begin(9600);   // Inicializando Seria Monitor
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Inicializando  MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Mostrando detalhes do MFRCC522
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  
  pinMode(7,OUTPUT);
}

void loop() {
  String conteudo= "";
 
  if (mfrc522.PICC_IsNewCardPresent()) { // Verificando se há algun cartão no leitor
    Serial.println("Detectando veiculo na entrada... ");
    if (mfrc522.PICC_ReadCardSerial()){ // Lendo o cartão 
      Serial.println("Lendo ID do veiculo ... ");
      //Salvar UID
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
        conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.println("\nLeitura completa !");
      Serial.println(conteudo);
      if (conteudo == autorizado)
      {
        Serial.println("\n------------ Veiculo autorizado------");
        //liga o buzzer
        tone(7,1500);   
        delay(500);
        //Desligando o buzzer.
        noTone(7);       
      }else
      {
        Serial.println("\n-------Não autorizado --------");
      }
    } 
  }else{
  }  
}
