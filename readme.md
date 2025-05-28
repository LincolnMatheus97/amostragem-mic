# Projeto Amostragem de Microfone com Display OLED no Raspberry Pi Pico

## 1. Descrição Geral

Este projeto demonstra como realizar a amostragem de um sinal de áudio de um microfone utilizando o Conversor Analógico-Digital (ADC) e o Acesso Direto à Memória (DMA) em um microcontrolador Raspberry Pi Pico. O sistema calcula a tensão RMS (Root Mean Square) do sinal capturado, converte essa tensão para um nível sonoro simulado em decibéis (dB) e classifica a intensidade sonora em categorias como "Baixo", "Moderado" ou "Alto". Todas essas informações são exibidas em tempo real em um display OLED SSD1306.

Este projeto foi desenvolvido com base nas funcionalidades de amostragem de microfone via DMA, similar ao encontrado no projeto `microphone_dma` de BitDogLab, adaptando-o para incluir a análise de intensidade sonora e a integração com um display OLED para visualização dos dados.

## 2. Funcionalidades Principais

* **Amostragem de Áudio:** Utiliza o ADC do Raspberry Pi Pico para capturar o sinal analógico do microfone.
* **Transferência DMA:** Emprega o DMA para transferir eficientemente as amostras do ADC para um buffer na memória, minimizando a carga na CPU.
* **Cálculo de Tensão RMS:** Processa as amostras de áudio para calcular a tensão RMS, uma medida da magnitude efetiva do sinal AC.
* **Conversão para Nível de dB Simulado:** Converte a tensão RMS em um valor de decibéis (dB) relativo, utilizando uma tensão de referência configurável.
* **Classificação da Intensidade Sonora:** Categoriza o nível de dB em "Baixo", "Moderado" ou "Alto" com base em limiares personalizáveis.
* **Exibição em Display OLED:** Mostra o título "Intensidade Sonora", o nível de dB calculado, a classificação da intensidade e avisos adicionais (como "Nível Prejudicial!") em um display OLED SSD1306 conectado via I2C.
* **Mensagens de Status:** Exibe mensagens de inicialização e configuração no display.

## 3. Estrutura do Projeto

O código fonte está organizado da seguinte forma:

* `src/amostragem_mic.c`: Arquivo principal contendo a função `main()`, inicializações e o loop de controle que coordena a amostragem, processamento e exibição.
* `src/utils/microfone/mic.h` e `src/utils/microfone/mic.c`: Módulo responsável pela configuração do ADC, amostragem do microfone, cálculo da tensão RMS, conversão para dB e classificação do nível sonoro.
* `src/utils/dma/dma.h` e `src/utils/dma/dma.c`: Módulo para configuração do canal DMA utilizado para a transferência de dados do ADC.
* `src/utils/displayOLED/display.h` e `src/utils/displayOLED/display.c`: Módulo para inicialização e controle do display OLED SSD1306, incluindo funções para limpar a tela, desenhar texto e atualizar o display.
* `CMakeLists.txt`: Arquivo de configuração do CMake para compilação do projeto com o Pico SDK.
* `pico_sdk_import.cmake`: Script CMake para importar o SDK do Raspberry Pi Pico.

## 4. Como Funciona a Medição de dB

O processo para estimar o nível de decibéis (dB) do ambiente envolve algumas etapas chave, desde a captação do som até os cálculos finais:

1.  **Captação e Amplificação:** O microfone na placa (que usa um amplificador MAX4466) [cite: 88, 5] captura as ondas sonoras e as converte em um sinal elétrico analógico. Este sinal é amplificado para que possa ser lido adequadamente pelo ADC.
2.  **Conversão Analógico-Digital (ADC):** O Raspberry Pi Pico utiliza seu ADC interno (com resolução de 12 bits) para converter o sinal analógico do microfone em valores digitais. A função `init_config_adc` prepara o ADC, selecionando o pino (GP28) e configurando a taxa de amostragem (via `ADC_CLOCK_DIV`) e o buffer FIFO.
3.  **Amostragem via DMA:** Para coletar os dados do ADC de forma eficiente sem sobrecarregar a CPU, usamos o DMA (Acesso Direto à Memória). A função `sample_mic` configura o DMA para transferir um número definido de amostras (`SAMPLES`, atualmente 2500) do FIFO do ADC para um buffer na memória (`adc_buffer`).
4.  **Cálculo da Tensão RMS (`get_voltage_rms`):** Esta é a etapa central do processamento:
    * **Remoção do Offset DC:** O sinal do microfone possui um componente DC (nível de tensão médio quando não há som). Calculamos a média de todas as amostras no buffer para encontrar esse offset.
    * **Isolamento do Sinal AC:** Subtraímos o offset DC de cada amostra para obter apenas o componente AC, que representa o som.
    * **Cálculo do RMS:** O RMS (Root Mean Square) é uma medida da magnitude efetiva do sinal AC. Calculamos usando a fórmula:
        $V_{rms\_digital} = \sqrt{\frac{1}{N} \sum_{i=1}^{N} (Amostra_{AC_i})^2}$
        Onde N é o número de amostras (`SAMPLES`).
    * **Conversão para Tensão:** Convertemos o valor RMS digital para uma tensão RMS real (em Volts), considerando a tensão de referência do ADC (3.3V) e sua resolução (12 bits):
        $V_{rms} = V_{rms\_digital} \times \frac{3.3V}{4096}$
5.  **Cálculo do Nível de dB (`get_db_simulated`):** Com a tensão RMS ($V_{rms}$), estimamos o nível de decibéis usando a fórmula padrão:
    $dB = 20 \times \log_{10}\left(\frac{V_{rms}}{V_{REF\_DB}}\right)$
    * $V_{REF\_DB}$ é uma **tensão de referência crucial para a calibração**. É o valor que *definimos* como correspondendo a um nível de referência. Atualmente, estamos usando `0.00004f` (40µV). Ajustar este valor é a principal forma de calibrar as leituras do sistema com um medidor de dB real.
    * A função também aplica limites para garantir que o valor exibido permaneça dentro de uma faixa razoável (ex: não menor que `MIN_DB_DISPLAY_LEVEL` e não maior que 120 dB).
6.  **Classificação e Exibição:** Finalmente, o nível de dB calculado é classificado como "Baixo", "Moderado" ou "Alto" e exibido no display OLED.

Este processo nos permite obter uma estimativa da intensidade sonora do ambiente, que, embora não seja perfeitamente equivalente a um decibelímetro profissional (devido a fatores como ponderação de frequência e calibração precisa), fornece uma medida útil e relativa da intensidade sonora.

## 5. Calibração e Ajustes

Para obter leituras de intensidade sonora mais significativas e classificações adequadas ao seu ambiente e microfone, pode ser necessário ajustar algumas constantes definidas no arquivo `src/utils/microfone/mic.h`:

* **`V_REF_DB`**: Esta é a tensão de referência usada no cálculo de dB ($db = 20 * log10(voltage_rms / V_REF_DB)$).
    * Um valor menor de `V_REF_DB` resultará em valores de dB mais altos para a mesma `voltage_rms`.
    * Ajuste este valor para definir o "piso" de dB do seu sistema em silêncio. Por exemplo, se o silêncio resulta em 50dB e você deseja que seja 30dB, você precisará aumentar `V_REF_DB`. **Este é o principal fator de calibração.**

* **`LIMIAR_DB_BAIXO_MODERADO`**: Define o valor de dB que separa a classificação "Baixo" da "Moderado".
* **`LIMIAR_DB_MODERADO_ALTO`**: Define o valor de dB que separa a classificação "Moderado" da "Alto".
* **`SAMPLES`**: Aumentar este valor pode dar leituras mais estáveis, mas menos responsivas. Diminuir pode fazer o contrário.

**Processo de Calibração Sugerido:**
1.  **Escolha o Modo de Alimentação:** Decida se você vai usar predominantemente a bateria ou o USB, pois isso afeta o ruído de fundo. Calibre no modo principal.
2.  **Observe o Silêncio:** Coloque o dispositivo e um medidor de dB de referência no ambiente mais silencioso possível.
3.  **Ajuste `V_REF_DB`:** Modifique `V_REF_DB` até que a leitura de dB no silêncio na sua placa corresponda à leitura do medidor de referência (ou um valor desejado, ex: 35-40 dB). Lembre-se: `V_REF_DB` menor = dB maior; `V_REF_DB` maior = dB menor.
4.  **Teste com Sons:** Com o novo `V_REF_DB`, observe os valores para sons que você considera baixos, moderados e altos, comparando com o medidor.
5.  **Ajuste Fino:** Se necessário, faça pequenos ajustes em `V_REF_DB` para encontrar o melhor compromisso entre a precisão no silêncio e em volumes altos.
6.  **Ajuste Limiares:** Ajuste `LIMIAR_DB_BAIXO_MODERADO` e `LIMIAR_DB_MODERADO_ALTO` para que as classificações correspondam à sua percepção e às leituras calibradas.

## 6. Dependências

* Raspberry Pi Pico SDK
* Hardware I2C e ADC do Raspberry Pi Pico
* Biblioteca SSD1306 (geralmente incluída ou adaptada no projeto para o Pico)

---