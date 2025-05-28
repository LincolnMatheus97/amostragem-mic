# Projeto Amostragem de Microfone com Display OLED no Raspberry Pi Pico

## 1. Descrição Geral

Este projeto demonstra como realizar a amostragem de um sinal de áudio de um microfone utilizando o Conversor Analógico-Digital (ADC) e o Acesso Direto à Memória (DMA) em um microcontrolador Raspberry Pi Pico. O sistema calcula a tensão RMS (Root Mean Square) do sinal capturado, converte essa tensão para um nível sonoro simulado em decibéis (dB) e classifica a intensidade sonora em categorias como "Baixo", "Moderado" ou "Alto". Todas essas informações são exibidas em tempo real em um display OLED SSD1306.

Este projeto foi desenvolvido com base nas funcionalidades de amostragem de microfone via DMA, similar ao encontrado no projeto `microphone_dma` de BitDogLab, adaptando-o para incluir a análise de intensidade sonora e a integração com um display OLED para visualização dos dados.

## 2. Funcionalidades Principais

*   **Amostragem de Áudio:** Utiliza o ADC do Raspberry Pi Pico para capturar o sinal analógico do microfone.
*   **Transferência DMA:** Emprega o DMA para transferir eficientemente as amostras do ADC para um buffer na memória, minimizando a carga na CPU.
*   **Cálculo de Tensão RMS:** Processa as amostras de áudio para calcular a tensão RMS, uma medida da magnitude efetiva do sinal AC.
*   **Conversão para Nível de dB Simulado:** Converte a tensão RMS em um valor de decibéis (dB) relativo, utilizando uma tensão de referência configurável.
*   **Classificação da Intensidade Sonora:** Categoriza o nível de dB em "Baixo", "Moderado" ou "Alto" com base em limiares personalizáveis.
*   **Exibição em Display OLED:** Mostra o título "Intensidade Sonora", o nível de dB calculado, a classificação da intensidade e avisos adicionais (como "Nível Prejudicial!") em um display OLED SSD1306 conectado via I2C.
*   **Mensagens de Status:** Exibe mensagens de inicialização e configuração no display.

## 3. Estrutura do Projeto

O código fonte está organizado da seguinte forma:

*   `src/amostragem_mic.c`: Arquivo principal contendo a função `main()`, inicializações e o loop de controle que coordena a amostragem, processamento e exibição.
*   `src/utils/microfone/mic.h` e `src/utils/microfone/mic.c`: Módulo responsável pela configuração do ADC, amostragem do microfone, cálculo da tensão RMS, conversão para dB e classificação do nível sonoro.
*   `src/utils/dma/dma.h` e `src/utils/dma/dma.c`: Módulo para configuração do canal DMA utilizado para a transferência de dados do ADC.
*   `src/utils/displayOLED/display.h` e `src/utils/displayOLED/display.c`: Módulo para inicialização e controle do display OLED SSD1306, incluindo funções para limpar a tela, desenhar texto e atualizar o display.
*   `CMakeLists.txt`: Arquivo de configuração do CMake para compilação do projeto com o Pico SDK.
*   `pico_sdk_import.cmake`: Script CMake para importar o SDK do Raspberry Pi Pico.


## 4. Calibração e Ajustes

Para obter leituras de intensidade sonora mais significativas e classificações adequadas ao seu ambiente e microfone, pode ser necessário ajustar algumas constantes definidas no arquivo `src/utils/microfone/mic.h`:

*   **`V_REF_DB`**: Esta é a tensão de referência usada no cálculo de dB (`db = 20 * log10(voltage_rms / V_REF_DB)`).
    *   Um valor menor de `V_REF_DB` resultará em valores de dB mais altos para a mesma `voltage_rms`.
    *   Ajuste este valor para definir o "piso" de dB do seu sistema em silêncio. Por exemplo, se o silêncio resulta em 50dB e você deseja que seja 30dB, você precisará aumentar `V_REF_DB`.

*   **`LIMIAR_DB_BAIXO_MODERADO`**: Define o valor de dB que separa a classificação "Baixo" da "Moderado".
*   **`LIMIAR_DB_MODERADO_ALTO`**: Define o valor de dB que separa a classificação "Moderado" da "Alto".

**Processo de Calibração Sugerido:**
1.  Observe o valor de `db_level` exibido no display em um ambiente o mais silencioso possível.
2.  Ajuste `V_REF_DB` até que este valor de "silêncio" corresponda a um nível de dB desejado (ex: 20-30dB, alinhando com referências de "Muito Baixo" ou "Biblioteca").
3.  Com o novo `V_REF_DB` configurado, observe os valores de `db_level` para sons que você considera baixos, moderados e altos.
4.  Ajuste `LIMIAR_DB_BAIXO_MODERADO` e `LIMIAR_DB_MODERADO_ALTO` para que as classificações correspondam à sua percepção.

## 5. Dependências

*   Raspberry Pi Pico SDK
*   Hardware I2C e ADC do Raspberry Pi Pico
*   Biblioteca SSD1306 (geralmente incluída ou adaptada no projeto para o Pico)

---