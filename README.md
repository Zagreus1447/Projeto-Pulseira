# Detector de Quedas com ESP32 e MPU6050

Este projeto implementa um sistema de detecção de quedas utilizando um microcontrolador ESP32 e um sensor acelerômetro/giroscópio MPU6050. O código foi desenvolvido utilizando a plataforma PlatformIO.

## Funcionalidades

- Leitura contínua dos dados do acelerômetro.
- Aplicação de um filtro passa-baixa para suavizar o sinal.
- Algoritmo de detecção baseado na sequência de queda-livre seguida por um impacto de alta aceleração.
- Saída de status via Monitor Serial para depuração e calibração.

## Hardware Necessário

- Placa de desenvolvimento ESP32
- Módulo MPU6050
- Jumpers para conexão

## Como Compilar e Usar

1. Clone este repositório.
2. Abra o projeto no VSCode com a extensão PlatformIO.
3. Conecte o hardware conforme a pinagem I2C padrão (SDA: GPIO 21, SCL: GPIO 22).
4. Compile e faça o upload do código para a ESP32.
5. Abra o Monitor Serial com baud rate de `115200` para ver os resultados.

## Status do Projeto

- Funcional. Próximos passos podem incluir o envio de alertas via Wi-Fi.