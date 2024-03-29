/*
Arduino_0-20mA_14-bit-main.ino

Interfaceamento de Transdutor 0-20mA com Resistor 250r e Arduino aplicando técnica de Oversampling e Decimation (14-bit).

Exemplo de aplicação de Resistor 250r 0.1% em saída de Transdutor 0-20mA (conversão de range para nível de tensão contínua
0-5Vcc apropriado às entradas analógicas do Arduino ou outro sistema microcontrolado).

Realiza leitura analógica (0-20mA) representando Tanque de 5000 litros. Dependendo do valor na Entrada Analógica A0, liga/desliga Saída Digital.

Obs.: Montar o(s) resistor(es) o mais próximo possível da(s) entrada(s) analógica(s) do sistema microcontrolado.
Resistor 250 0.1% Filme metálico 400mW:

https://produto.mercadolivre.com.br/MLB-1620033927-1-un-resistor-filme-metalico-250ohms-01-400mw-_JM?quantity=1

https://www.youtube.com/watch?v=Cz1FP2SAFvk

A. Inácio Morais - anderson.morais@protonmail.com - (35) 99161-9878 - 2022

*/

void setup()				
{
  pinMode(A0, INPUT);
  pinMode(4, OUTPUT);
  
  digitalWrite(4, LOW); 
}

float analog_14bit(byte pin) {  // Função para leituras em Entradas Analógicas (Oversampling e Decimation)
  int _adc;
  unsigned long _soma = 0;
  float _result;
        
  _adc = analogRead(pin);
  delay(25);
  _adc = analogRead(pin);  /* Leituras descartadas para eliminar valores parasitas (Ver datasheet ATmega e Arduino 
                            * Reference). "The ATmega datasheet [...] cautions against switching analog pins in close 
                            * temporal proximity to making A/D readings (analogRead) on other analog pins. This can cause 
                            * electrical noise and introduce jitter in the analog system." [Arduino Reference]. Como as 
                            * entradas analógicas compartilham um mesmo ADC, leituras em mais de um canal (principalmente
                            * com trocas rápidas entre os canais) podem gerar valores inesperados. Ocorrendo valores 
                            * inesperados entre leituras de canais diferentes (mesmo com o descarte das duas primeiras 
                            * leituras) uma solução é inserir algum delay entre as leituras em diferentes canais (entre 
                            * chamadas desta função para diferentes canais). 
                            */

  for(unsigned int i=0;i<256;i++) _soma += analogRead(pin);  //Oversampling          
    
  _result = ((float)_soma / 16.0);  // Decimation

  _result = (_result * 1.0158 + 35.281); 
  /* Função Linear de Correção de Desvio para Arduino Uno Rev3 [f(x) = 1,0158 * x + 35,281]
   *
   * Corrige erros inerentes ao microcontrolador ATmega328P (Ver páginas 254 a 256 da Datasheet do ATmega328P-PU)  
   * com uma aproximação linear aplicável ao Arduino Uno Rev3 em Oversampling para 14-bit. Uma função linear de
   * correção pode ser obtida para outras condições e sistemas microcontrolados com multiplas medições em comparação
   * aos valores esperados.
   */
    
  return _result;
}

void loop() {
  float _cal=1.000;  /* Valor multiplicador para calibração de leituras via firmware. Trabalhar dentro do range
                      * 0,900-1,100 (valores acima ou abaixo desse range indicam transdutor ou aferidor/padrão de 
                      * calibração muito descalibrado). Podem ser implementadas rotinas para calibração posterior in 
                      * loco com edição, salvamento e recuperação de valores em EEPROM. A criação de menus/rotinas 
                      * de calibração, leitura e escrita em EEPROM não são objeto deste Sketch. Calibrações devem 
                      * ser feitas com padrões calibrados.
                      */
  float _leitura;

  delay(250);  /* Principalmente para amostragens que não podem ocorrer indefinidamente (dependentes primeiramente da 
                * comutação de circuitos específicos e com posterior alimentação dos terminais de leitura do transdutor), 
                * aplica-se um delay referente ao tempo de aquisição estabelecido no manual do transdutor e ao tempo de 
                * comutação dos relés/contatores envolvidos. Ex.: Delay de 250 ms para tempo de aquisição apresentado 
                * no manual do transdutor como '< 200 ms' e pequeno delay considerando os tempos de comutação. Atenção: 
                * Sempre verifique o tempo de aquisição do transdutor aplicado.                
                */

  _leitura = analog_14bit(A0);

  // Máx. valor inteiro representado por 14 bits = 16383
  // ESCALONAMENTO (Considerando unidade/variável de processo como litros - Tanque1 máx. 5000 litros):
  //                    (_leitura - 0) / (16383 - 0) = (y[L] - 0[L]) / (5000[L] - 0[L]) 
  //                    _leitura / 16383 = y / 5000
  //                    y = (_leitura / 16383) * 5000 [L]

  // É possível realizar multiplas chamadas da função 'analog_14bit' após o tempo de aquisição/comutação, salvando os
  // retornos em diferentes variáveis e efetuando média das leituras obtidas para obter-se uma menor variação. Mas, o   
  // aumento na precisão também significa um aumento nos tempos de processamento envolvidos.
  
  _leitura = ((_leitura / 16383.0) * 5000.0) * _cal;
       
  if (_leitura >= 4750.0 ) {
    delay(1000); // Histerese ou Sensibilidade - evita trepidação do elemento comutador quando limiar é alcançado
    if (_leitura >= 4750.0 ) digitalWrite(4, HIGH); // Liga bomba1 quando Tanque1 atinge 95% da capacidade
  }
  
  if (_leitura <= 250.0 ) {
    delay(1000); // Histerese ou Sensibilidade - evita trepidação do elemento comutador quando limiar é alcançado
    if (_leitura <= 250.0 ) digitalWrite(4, LOW);  // Desliga bomba1 quando Tanque1 atinge 5% da capacidade
  }
}
