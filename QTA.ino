// Definições de pinos
const int SensorRede = A3;
const int SensorGerador = A4;
const int Ignicao = 11;
const int Partida = 12;

const int sinalRede = A0;
const int sinalRedeGerador = 2;
const int botaoDesligar = A2;  // NOVO

 bool redePresente = 0;
  bool redeGeradorPresente = 0;
  bool botaoDesliga = 0;


bool LigaPartida = 0;
bool LigaIgnicao = 0;
bool DesligaIgnicao = 0;
bool Desl_Pulso_partida = 1;
//bool Gerador = 1;
bool AtivaDesligamentoManual = 1;
bool PartidaManual = 0;

bool geradorLigado = false;
bool geradorDesligado = false;
long tempoRepetirPartida = 0;

unsigned long tempoLigarGerador = 0;
long val = 10000;//tempo para gerador ligar apos falta da rede
unsigned int EsperaPartida = val;
unsigned long tempoDesligarGerador = 0;
unsigned long tempoLigarPartida = 0;
unsigned long PulsoPartida = 0;
unsigned long Atualizcao = 0;

int tentativasFalhaGerador = 0;
bool falhaFinalGerador = false;
unsigned long tempoPiscaErro = 0;
bool estadoPiscaErro = false;

// Substituição do delay
bool aguardandoFeedback = false;
unsigned long tempoInicioAguardar = 0;
const unsigned long tempoEsperaGerador = 2000;

void setup() {
  pinMode(SensorRede, INPUT);
  pinMode(SensorGerador, INPUT_PULLUP);
  pinMode(botaoDesligar, INPUT_PULLUP);  // BOTÃO A5

  pinMode(Ignicao, OUTPUT);
  pinMode(Partida, OUTPUT);
  pinMode(sinalRede, OUTPUT);
  pinMode(sinalRedeGerador, OUTPUT);
  
 
 
  
  digitalWrite(Ignicao, LOW);
  digitalWrite(Partida, LOW);
  digitalWrite(sinalRede, LOW);
  digitalWrite(sinalRedeGerador, LOW);
  

  Serial.begin(115200);
  
}

void loop() {


  if (millis() - Atualizcao > 500) {
    Atualizcao = millis();
    Serial.print("tentativasFalhaGerador= ");
    Serial.println(tentativasFalhaGerador);
    
   // Serial.print(" falhaFinalGerador= ");
    //Serial.println( falhaFinalGerador);

    Serial.print("   Desl_Pulso_partida =");
    Serial.println(  Desl_Pulso_partida);
   
Serial.print(" aguardandoFeedback ");
 Serial.println( aguardandoFeedback);


 Serial.print("   EsperaPartida=");
Serial.println( EsperaPartida);


  }
     //falhaFinalGerador = false;
    //aguardandoFeedback = false;
  redePresente = digitalRead(SensorRede) == LOW;
  redeGeradorPresente = digitalRead(SensorGerador) == LOW;
  botaoDesliga = digitalRead(botaoDesligar) == LOW;

  // ---- DESLIGAMENTO MANUAL DO GERADOR ----
  if (botaoDesliga && redeGeradorPresente == 1 && redePresente == 0) {//desliga gerador manual
    digitalWrite(Ignicao, LOW);
    digitalWrite(Partida, LOW);
   
    DesligaIgnicao = 1;
    LigaIgnicao = 0;
    LigaPartida = 0;
    Desl_Pulso_partida = 0;
    AtivaDesligamentoManual = 0;
    aguardandoFeedback = false;
    falhaFinalGerador = false;
    tentativasFalhaGerador = 0;
    
    
  }

   if (botaoDesliga && redeGeradorPresente == 0 && redePresente == 0) {//liga gerador manual
   
  
     PartidaManual = 1;   
     EsperaPartida = 1000;
     tempoLigarPartida = millis();
     tempoLigarGerador = millis();
   
     
   }

   if(PartidaManual ==1){//chama função de ignção e partida
     ligarGerador();
    
   }

  // ---- CONTROLE AUTOMÁTICO ----
  if (redePresente) {
    delay(300);
    digitalWrite(sinalRede, HIGH);
    desligarGerador();
    DesligaIgnicao = 0;
    tempoLigarGerador = millis();
    tempoLigarPartida = millis();
     
    tentativasFalhaGerador = 0;
    falhaFinalGerador = false;
    aguardandoFeedback = false;
    AtivaDesligamentoManual = 1;     
    PartidaManual = 0;
    EsperaPartida = val;//8 segundos para gerador entrar apos quede de rede
  } 
  else {
  if(AtivaDesligamentoManual ==1){
    ligarGerador();
   
    
  }
 
    piscarLED_rede();
    tempoDesligarGerador = millis();
    
  }

  // LED do gerador (porta 2)
  if (redeGeradorPresente && !falhaFinalGerador) {
    digitalWrite(sinalRedeGerador, HIGH);
    geradorLigado = true;
   
  } 
  else if (!redeGeradorPresente && !falhaFinalGerador) {
    digitalWrite(sinalRedeGerador, LOW);
    geradorDesligado = true;
     
  }

  
  if (falhaFinalGerador) {
    piscarLED_erro();
    EsperaPartida = val;//8 segundos para gerador entrar apos quede de rede
  }

  // Aguarda 5s após partida para verificar se gerador funcionou
  if (aguardandoFeedback && millis() - tempoInicioAguardar >= tempoEsperaGerador) {
    aguardandoFeedback = false;

    if (digitalRead(SensorGerador) == HIGH) {
      tentativasFalhaGerador++;

       if( tentativasFalhaGerador <=3){

         PartidaManual = 1;
         
       }
      
      if (tentativasFalhaGerador >3) {
        falhaFinalGerador = true;
      } else {
        LigaPartida = 0;
        LigaIgnicao = 0;
        tempoLigarGerador = millis();
      }
    } else {
      tentativasFalhaGerador = 0;
      falhaFinalGerador = false;
    }
  }
}

// ------------------ Funções auxiliares ------------------------

void ligarGerador() {
  if (LigaIgnicao == 0 && LigaPartida == 0) {
    if (millis() - tempoLigarGerador > EsperaPartida) {//tempo de espera para da partida, caso seja uma queda de rde rapida
      digitalWrite(Ignicao, HIGH);
    
      LigaPartida = 1;
      LigaIgnicao = 1;
     
      tempoLigarPartida = millis();
    }
  }

  if (LigaPartida == 1) {
    if (millis() - tempoLigarPartida > 4000) {
      digitalWrite(Partida, HIGH);
      LigaPartida = 0;
      Desl_Pulso_partida = 1;
      PulsoPartida = millis();
    }
  }

  if (Desl_Pulso_partida == 1) {
    if (millis() - PulsoPartida > 4000 && LigaPartida == 0) {
      digitalWrite(Partida, LOW);
      Desl_Pulso_partida = 0;
       PartidaManual = 0;//manter rotina circular do modo manual   
       EsperaPartida = val;
     

      if (!aguardandoFeedback) {
        tempoInicioAguardar = millis();
        aguardandoFeedback = true;
        
      }
    }
  }
}

void desligarGerador() {
  if (DesligaIgnicao == 0) {
    if (millis() - tempoDesligarGerador > 15000) {//15s para desligar gerador após reestabelecimento da rede
      digitalWrite(Ignicao, LOW);
      
      DesligaIgnicao = 1;
      LigaIgnicao = 0;
      Desl_Pulso_partida = 0;
      LigaPartida = 0;
      
    }
  }
}

void piscarLED_rede() {
  static unsigned long ultimoPisca = 0;
  static bool estadoLED = false;

  if (millis() - ultimoPisca >300) {
    estadoLED = !estadoLED;
    digitalWrite(sinalRede, estadoLED);
    ultimoPisca = millis();
  }
}

void piscarLED_erro() {
  if (millis() - tempoPiscaErro >= 1000) {
    estadoPiscaErro = !estadoPiscaErro;
    digitalWrite(sinalRedeGerador, estadoPiscaErro);
    tempoPiscaErro = millis();
  }
}
