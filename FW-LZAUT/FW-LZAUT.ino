// Definições do programa
#define rele 0 // Pino para ligar/desligar o relé
#define entrada 1 // Sensor mais perto da porta
#define saida 2 // Sensor mais longe da porta
#define botao 3 // Botão para ligar a luz manualmente
#define ldr A2 // Pino ligado ao divisor de tensao LDR + Resistencia

// Macro para facilitar a leitura do LDR
#define leituraLDR analogRead(ldr)

// Variaveis do programa:
bool entrando1 = false; // Indica que a pessoa está entrando e passou do primeiro sensor
bool entrando2 = false; // Indica que a pessoa está entrando e passou do segundo sensor
bool saindo1 = false; // Indica que a pessoa esta saiando e passou do segundo sensor (1º da saida)
bool saindo2 = false; // Indica que a pessoa esta saiando e passou do primeiro sensor (2º da saida)
bool escureceu = false; // Indica se está escuro ou nao
bool acende = false; // Indica se a lampada deve ser acionada ou nao
int contagem = 0; // Contar quantas pessoas tem no quarto
unsigned long tempoE = 0; // Tempo de espera para aceitar se a pessoa esta entrando ou nao
unsigned long tempoS = 0; // Tempo de espera para aceitar se a pessoa esta saindo ou nao
unsigned long tempoL = 0; // Tempo de espera para aceitar se escureceu (tempoldr)

// Funções
void verifica_tempoS(void); // Manipula as variaveis de tempo
bool leitura_sensor(char sensor); // Indica se alguém passou na frente do sensor ultrassônico

void setup() {
  // Configura os pinos
  pinMode(rele, OUTPUT);
  pinMode(botao, INPUT_PULLUP);
  pinMode(ldr, INPUT);
  digitalWrite(rele, LOW);
}

void loop() {  
  // Verifica se os sensores detectaram algo
  bool leituraE = leitura_sensor(entrada);
  bool leituraS = leitura_sensor(saida);
 
  // Logica da pessoa entrando
  if (leituraE && !leituraS && !saindo1) { // Condiçao da pessoa estar entrando (nao esta saindo e leituraE é true)
    entrando1 = true;
    tempoE = millis();
  }
  if (!leituraE && leituraS && entrando1 ) { // Condiçao para a pessoa estar entrando no meio do caminho
    entrando2 = true;
    tempoE = millis();
  }
  if (leituraE && !leituraS && entrando2){ // Condição para a pessoa desistir de entrar e voltar
    entrando2 = false;
    tempoE = millis();
  }  
  if (!leituraE && !leituraS && entrando2) { //Condiçao para a pessoa ter entrado (nenhum sensor le nada)
    contagem++;
    entrando1 = false;
    entrando2 = false;
    delay(500); // Delay para nao duplicar a leitura)
  }


  // Logica da pessoa saindo
  if (leituraS && !leituraE && !entrando1) { //Condiçao da pessoa estar saindo (nao esta entrando e leituraS é true)
    saindo1 = true;
    tempoS = millis();
  }
  if (!leituraS && leituraE && saindo1) { //Condiçao para a pessoa estar entrando no meio do caminho
    saindo2 = true;
    tempoS = millis();
  }
  if (!leituraE && leituraS && saindo2){ // Condição para a pessoa desistir de sair e voltar
    saindo2 = false;
    tempoS = millis();
  }  
  if (!leituraS && !leituraE && saindo2) { //Condiçao para a pessoa ter saido (nenhum sensor le nada)
    if (contagem > 0) { //So diminui se tiver alguem dentro do quarto
      contagem--;
    }
    saindo1 = false;
    saindo2 = false;
    delay(500); //delay para nao duplicar a leitura
  }
 

  // Logica da pessoa parada no meio do caminho
  if (leituraE && leituraS) { //Condiçao para a pessoa estar parada no meio do caminho
    if (entrando1 || entrando2) {
      tempoE = millis();
    }

    if (saindo1 || saindo2) {
      tempoS = millis();
    }
  }


  // Manipula os tempos
  verifica_tempos();


  // Logica do acendimento da lampada
  if (leituraLDR < 8) { //se o ldr indicou que escureceu
    if (!escureceu){
      escureceu = true;
      tempoL = millis(); // tempo para saber se nao foi apenas uma sombra
    }
  } else { //se o ldr indicou que esta claro
    escureceu = false;
  }
 
  if(digitalRead(botao)){ // Se o botão está desligado, funciona normalmente
    if (contagem >= 1 && acende) {
      digitalWrite(rele, HIGH);
    } else {
      digitalWrite(rele, LOW);
      acende = false;
    }
  }else{ // Senão, apenas acende o relé
    digitalWrite(rele, HIGH);
  }
}

/*
 * Verifica se os intervalos limites de tempo de entrada e saída foram ultrapassados para reiniciar as variáveis
 * Verifica se o LDR ficou  escuro por no mínimo 5 segundos
 */
void verifica_tempos(void) {
  // Entrada
  if (millis() - tempoE >= 500) {
    entrando1 = false;
    entrando2 = false;
  }

  // Saida
  if (millis() - tempoS >= 500) {
    saindo1 = false;
    saindo2 = false;
  }

  // Espera 5 segundos para saber se está de fato escuro ou se foi uma sombra que passou no sensor
  if (escureceu && millis() - tempoL >= 5000) {
    acende = true;
  }
}

/* Informa se o sensor especificado detectou algum obstáculo.
 * Parâmetros:      
 *     sensor: pino do sensor que deve ser lido
 * Retorna: 1 se detectou obstáculo e 0 se não.  
 */
bool leitura_sensor(char sensor) {
  // Inicia a leitura mandando um pulso de 10uS
  pinMode(sensor, OUTPUT);
  digitalWrite(sensor, HIGH);
  delayMicroseconds(10);
  digitalWrite(sensor, LOW);

  // Lê o sinal de resposta e converte para distância
  pinMode(sensor, INPUT);
  float dist = pulseIn(sensor, HIGH);
  dist = dist * 0.034 / 2.0;

  // Verifica se há um objeto na frente do sensor
  if (dist != 0 && (dist < 50 || dist > 200))
    return true;
  else
    return false;
}
