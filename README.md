============ Projeto EmbarcaTech ============
--------------  INTERRUPÇÕES  ---------------

-> Objetivos:
• Compreender o funcionamento e a aplicação de interrupções em microcontroladores.
• Identificar e corrigir o fenômeno do bouncing em botões por meio de debouncing via software.
• Manipular e controlar LEDs comuns e LEDs endereçáveis WS2812.
• Fixar o estudo do uso de resistores de pull-up internos em botões de acionamento.
• Desenvolver um projeto funcional que combine hardware e software.

-> Descrição do Projeto:
O projeto consiste no uso da placa BitDogLab e do Pico SDK para programação de um contador de 0 a 9, 
exibido na matriz de LEDs e controlado por dois botões A e B que decrementam e incrementam um número, respectivamente. 
Paralelo ao contador, um led RGB foi programado para piscar continuamente, 5 vezes por segundo. O projeto inclui técnicas
de interrupção e debouncing.

-> Componentes:
• Matriz 5x5 de LEDs (endereçáveis) WS2812, conectada à GPIO 7.
• LED RGB, com os pinos conectados às GPIOs (11, 12 e 13).
• Botão A conectado à GPIO 5.
• Botão B conectado à GPIO 6.

-> Link do vídeo de demonstração do projeto
https://youtube.com/shorts/d4ApofD8oyI
