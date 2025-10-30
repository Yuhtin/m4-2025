# Ponderada 3 - M4-2025

&ensp;Nesta ponderada, criei um semáforo controlado por um ESP32, controlado por uma classe criada por mim chamada `ControleSemaforo` usando ponteiros.

## Objetivo

&ensp;A proposta da atividade foi dada na terceira semana do módulo 4 do Inteli. A ideia era imaginar que eu fosse um funcionário do Departamento de Engenharia de Trânsito e fiquei responsável por controlar o fluxo em uma via movimentada do bairro Butantã/SP. O desafio? Montar e programar um semáforo que garanta a segurança de pedestres e veículos, seguindo a lógica de tempo de cada fase das luzes.

## Instruções

**Parte 1: Montagem Física do Semáforo**
&ensp;Você deve realizar a montagem física de um semáforo utilizando LEDs e resistores em uma protoboard. Os LEDs devem representar as cores vermelho, amarelo e verde, seguindo o esquema de um semáforo convencional.

**Para completar esta etapa:**

- Conecte corretamente os LEDs e resistores na protoboard conforme o esquema.
- Certifique-se de usar os resistores adequadamente para proteger os LEDs.
- Organize a disposição dos fios para garantir clareza e facilidade de visualização.

**Parte 2: Programação e Lógica do Semáforo**
&ensp;Você deve programar o comportamento do semáforo para alternar entre as fases vermelho, amarelo e verde, seguindo a lógica abaixo:

- 6 segundos no vermelho
- 4 segundos no verde
- 2 segundos no amarelo

## Ir Além

&ensp;A atividade pedia apenas o semáforo, mas para complementar a atividade, decidi adicionar um LCD para mostrar o tempo restante de cada fase, e além disso, como tivemos uma aula de Ponteiros em C++, decidi implementar a lógica do semáforo usando ponteiros.

&ensp;Abaixo, está o código da classe `ControleSemaforo` que eu criei para controlar o semáforo.

```cpp
struct ControleSemaforo {
  uint8_t* pinVermelho; // Ponteiro para o pino do LED vermelho
  uint8_t* pinAmarelo; // Ponteiro para o pino do LED amarelo
  uint8_t* pinVerde; // Ponteiro para o pino do LED verde
  FaseSemaforo faseAtual; // Fase atual do semáforo
  unsigned long* tempoInicio; // Ponteiro para controle de tempo
};
```

## Montagem Física do Semáforo

&ensp;Nesta seção, está a montagem física do semáforo, com os LEDs e resistores conectados na protoboard. Vou passar um pouco do que eu fiz para montar o semáforo.

<div align="center">
    <sup>Figura 1 - Montagem Física do Semáforo</sup></br>
    <img src="image-2.png" width="500px"></br>
    <sup>Fonte: Própria</sup></br>
</div>

&ensp; Nesta imagem é possível ver a disposição dos LEDs e resistores na protoboard. Na parte à direita do ESP32, os LEDs estão conectados aos pinos 27, 26 e 25, respectivamente, e os resistores de 330Ω estão conectados aos LEDs para protegê-los.

<div align="center">
    <sup>Figura 2 - Montagem Física do Semáforo</sup></br>
    <img src="image-1.png" width="500px"></br>
    <sup>Fonte: Própria</sup></br>
</div>

&ensp;Nesta imagem, é possível ver o Semáforo funcionando, com o LCD mostrando o tempo restante de cada fase. Usei bastante o inferior da protoboard para conectar os fios do GND, para ligar o painel LCD e os LEDs. Além de ter usado as portas 21 e 22 para enviar os dados para o LCD.

## Tabela de Componentes

&ensp;Abaixo, está a tabela de componentes utilizados na montagem do semáforo.

| Componente | Quantidade |
| ---------- | ---------- |
| ESP32      | 1          |
| LED Vermelho | 1        |
| LED Amarelo | 1         |
| LED Verde  | 1          |
| Resistores 330Ω | 3          |
| Protoboard | 1          |
| LCD (Compatível com LiquidCrystal I2C)        | 1          |

## Vídeo Demonstrativo

&ensp;Vídeo demonstrando o funcionamento do semáforo: <https://youtu.be/kApF5Q_djHs>

## Avaliação de Pares

&ensp;A atividade foi avaliada por mais dois colegas da turma, estarei dispondo as opiniões e notas atribuídas ao meu trabalho por eles.

&ensp;O critério de avaliação foi o seguinte:

- Até **4 pontos** (Contempla/Contempla parcialmente/Não contempla) - Montagem física realizada com as cores corretas, boa disposição dos fios e uso adequado de resistores para proteção;
- Até **3 pontos** (Contempla/Contempla parcialmente/Não contempla) - Temporização adequada conforme tempos medidos com auxílio de algum instrumento externo (timer no celular por exemplo);
- Até **3 pontos** (Contempla/Contempla parcialmente/Não contempla) - O código implementa corretamente as fases do semáforo (vermelho, amarelo, verde) e as transições entre elas seguem a ordem correta. Além disso, o código tem boa estrutura, nomes representativos de variáveis e uso de comentários explicativos.

&ensp;Portanto, seguindo todos os critérios, é possível chegar em um total de 10 pontos.

| Avaliador | Nota | Observações |
| --------- | ---- | ----------- |
| Marcos        | 10    | "A implementação está muito bem feita. O código ficou organizado e funcional, principalmente pelo uso correto de ponteiros na estrutura `ControleSemaforo`, demonstra bom conhecimento conceitual do assunto. A montagem física também está bem executada." |
| Rayssa        | 10    | "Excelente trabalho! O projeto foi além do solicitado, incluindo um display LCD para mostrar o tempo restante de cada fase. A implementação com ponteiros e a documentação completa demonstram dedicação e domínio técnico da atividade." |

## Conclusão

&ensp;A atividade foi muito divertida e desafiadora. Foi a primeira vez que eu usei ponteiros em C++, e foi muito legal ver como eles funcionam. Além disso, foi muito legal ver o semáforo funcionando, e foi uma ótima oportunidade para aplicar os conhecimentos adquiridos em sala de aula.