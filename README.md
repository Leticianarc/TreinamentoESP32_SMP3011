 # Treinamento: Sensor SMP3011 com ESP32 usando ESP-IDF 5.3.0

## Pré-requisitos:
### Software
- Visual Studio Code
- ESP-IDF 5.3.0 (instalado e configurado)
- Python 3.9+
- Ambiente de desenvolvimento **https://www.youtube.com/watch?v=1u9h4_O1yQg**

## 📟 O sensor SMP3011  

O **SMP3011** é um **sensor de pressão** que se comunica via protocolo **I²C**.  

### 🔑 Características principais  
- Conversor Analógico-Digital (**ADC**) de **24 bits** para **pressão**  
- Conversor Analógico-Digital (**ADC**) de **16 bits** para **temperatura**  

---

## 🧮 Conversão da leitura do ADC (Pressão)  

Um conversor de **24 bits** entrega valores entre **0** e **16.777.216**  
\[(2^{24})\].  

- **0** → representa **0%**  
- **16.777.216** → representa **100%**  

Para converter o valor lido em percentual:  

**Fórmula:**  
```
PercentualAD = ValorAD / 16777215
```

---

## 📊 Faixa útil de leitura (Pressão)  

O sensor não utiliza toda a escala de 0% a 100%.  
Ele entrega uma faixa de **15% a 85%**, correspondendo a:  

- **15% → 0 Pa**  
- **85% → Range do sensor (pressão máxima medida)**  

---

## ✏️ Montando a equação (regressão linear da pressão)  

Usamos a equação de 1º grau:  

```
y(x) = a * x + b
```

Onde:  
- **x** → percentual obtido (PercentualAD)  
- **y(x)** → pressão correspondente  

### Definindo os pontos:
1. `0 Pa = a * 15% + b`  
2. `Range = a * 85% + b`  

---

## 🔍 Resolvendo o sistema (Pressão)  

1. De (1):  
```
b = -15% * a
```

2. Substituindo em (2):  
```
Range = 85% * a - 15% * a
Range = 70% * a
a = Range / 70%
```

3. Calculando `b`:  
```
b = -15% * (Range / 70%)
```

---

## ✅ Fórmula final da pressão  

Substituindo na equação geral:  

```
Pressão(PercentualAD) = (Range * (PercentualAD - 15%)) / 70%
```

---

## 🌡️ Leitura de Temperatura  

Para obter a leitura de **temperatura**, o processo é semelhante, porém o **range é fixo**:  

- **Faixa de medição:** `-40 °C até +150 °C`  
- **Conversor ADC:** 16 bits (máx. 65.536 valores possíveis)  

### Passos:  

1. Converter a leitura bruta em percentual:  
```
PercentualADTemp = ValorADTemp / 65535
```

2. Aplicar a faixa de temperatura:  
```
Temperatura = ((150.0 - (-40.0)) * PercentualADTemp) - 40.0
```

Ou simplificando:  
```
Temperatura = (190.0 * PercentualADTemp) - 40.0
```

---

👉 Assim, o SMP3011 fornece tanto **pressão** (com faixa ajustada de 15% a 85%) quanto **temperatura** (em -40 °C a +150 °C) a partir dos valores lidos pelos conversores ADC.  

## Leitura do sensor

O endereço do sensor é o 0x78  
O processo de leitura do sensor é:
   - Escrever 0xAC para iniciar o processo de conversão da pressão e temperatura
   - Ler 6 bytes do sendor
   - Verificar se o bit de busy (bit 5 do byte[0]) esta em zero
   - Se o busy em zero, separar os bytes 1, 2 e 3 para represesntar a pressão e os bytes 4 e 5 a temperatura
   - Transformar as leituras em float de percentual do AD da pressão e temperatura
   - Converter para Pa e °C
   - Escrever 0xAC para iniciar o processo de conversão da pressão e temperatura e repetir o processo

### Converter as leituras para float:
Para organizar os valores, considerando que a pressão é 24bits ou seja, 3 bytes, precisamos agrupar esses bytes para formar uma variável de 32bits.



## 📖 Leitura do sensor SMP3011  

O endereço I²C do sensor é **0x78**.  

### 🔄 Processo de leitura  
1. **Escrever `0xAC`** para iniciar o processo de conversão da pressão e temperatura.  
2. **Ler 6 bytes** do sensor.  
3. **Verificar o bit de busy**:  
   - O **bit 5** do `byte[0]` indica se o sensor ainda está processando.  
   - Quando o **busy = 0**, os dados estão prontos.  
4. **Separar os dados**:  
   - **Pressão** → bytes **[1], [2], [3]** (24 bits)  
   - **Temperatura** → bytes **[4], [5]** (16 bits)  
5. **Converter para float**:  
   - Transformar os valores lidos em **percentual do ADC**.  
   - Aplicar as fórmulas para obter **Pa** (pressão) e **°C** (temperatura).  
6. **Reiniciar o processo**:  
   - Escrever `0xAC` novamente para iniciar nova conversão.  

---

## ⚙️ Conversão dos bytes em valores  

### 🔢 Pressão (24 bits)  
A pressão é representada em **3 bytes (24 bits)**. Para manipular esses valores em código, normalmente agrupamos em um **inteiro de 32 bits**.

Seja o buffer de leitura:  
```
byte[1], byte[2], byte[3]
```

Podemos montar o valor de 24 bits assim:  

```c
uint32_t rawPressure = ((uint32_t)byte[1] << 16) |
                       ((uint32_t)byte[2] << 8)  |
                       (uint32_t)byte[3];
```

### 🔢 Temperatura (16 bits)  
A temperatura vem em **2 bytes (16 bits)**:  
```
byte[4], byte[5]
```

Conversão:  

```c
uint16_t rawTemp = ((uint16_t)byte[4] << 8) |
                   (uint16_t)byte[5];
```

---

## 📌 O que é Shift?  

**Shift** é a operação de **deslocar os bits de um número para a esquerda ou para a direita**.  

- `<<` → **shift para a esquerda**: desloca os bits, multiplicando o número por potências de 2.  
- `>>` → **shift para a direita**: desloca os bits, dividindo o número por potências de 2.  

Exemplo:  

```
00000001 (1 decimal)
<< 8
00000001 00000000 (256 decimal)
```

---

## 🧩 Explicação do agrupamento (Pressão 24 bits)  

Queremos que:  
- `byte[1]` → ocupe os bits **23 a 16**  
- `byte[2]` → ocupe os bits **15 a 8**  
- `byte[3]` → ocupe os bits **7 a 0**  

Isso é feito com **shifts**:  

1. `byte[1] << 16` → desloca `byte[1]` para ficar nos bits 23–16.  
2. `byte[2] << 8` → desloca `byte[2]` para os bits 15–8.  
3. `byte[3]` → já está nos bits 7–0.  

Somando com **OR bit a bit (`|`)**, obtemos o valor completo de 24 bits.  

---

👉 Dessa forma, a leitura do sensor é organizada em variáveis inteiras que depois podem ser convertidas em **percentual do ADC** e, por fim, em **Pa** e **°C**.  



## 📦 Pré-requisitos
### Hardware
- ESP32 AM-032
- Sensor SMP3011

### Software
- Visual Studio Code
- ESP-IDF 5.3.0 (instalado e configurado)
- Python 3.9+
  

## 🧱 Etapa 1 – Criar Projeto Base no VSCode
Vamos criar a base do projeto que configura e compila o básico para apenas  iniciar a placa  
- Crie uma pasta chamada **TreinamentoESP32_SMP3011** em **Documentos**
- Abra o VSCode.  
- Vá em *File → Open folder*  e selecione a pasta criada  **TreinamentoESP32_SMP3011**  
- Clique no ícone da *Espressif*
- Clique em **Select current ESP-IDF version** e aguarde alguns segundos até aparecer uma lista no centro da tela
- No centro da tela irá exibir a lista de versões dispiníveis. Selecione **Version: v5.3.0**
- Pressione *F1* → digite *ESP-IDF: Create Project from Extension Template*.  
- Selecione **Use current folder**
- Selecione **template-app**
- Assim que carregar o novo projeto, clique no ícone da *Espressif*
- Em seguida,  clique em **Build project** e aguarde finalizar a compilação

## 🧱 Etapa 2 - Scanner I2C e leitura do sensor
Essa etapa iremos configurar a porta I2C e construir um scanner para identificar o endereço do sensor na porta I2C e ler o sensor

- No *Explorer* do VSCcode abra o arquivo **main.c**
- E insira o seguinte código:
```c++
#include "driver/i2c_master.h"
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "example";

//I2C Port Configuration
#define I2C_SENSOR_BUS_PORT     1
#define I2C_SENSOR_SDA          33
#define I2C_SENSOR_SCL          32

//I2C Bus Handler and Configuration
i2c_master_bus_handle_t i2c_sensor_bus = NULL;    
i2c_master_bus_config_t sensor_bus_config = 
{
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .i2c_port = I2C_SENSOR_BUS_PORT,
    .sda_io_num = I2C_SENSOR_SDA,
    .scl_io_num = I2C_SENSOR_SCL,
    .flags.enable_internal_pullup = true,
};

//I2C for SMP3011 Pressure Sensor configuration and handler
i2c_master_dev_handle_t i2c_smp3011_handle = NULL;
i2c_device_config_t i2c_smp3011_config = 
{
    .dev_addr_length  = I2C_ADDR_BIT_LEN_7,         /*!< Select the address length of the slave device. */
    .device_address  = 0x78,                        /*!< I2C device raw address. (The 7/10 bit address without read/write bit) */
    .scl_speed_hz  = 400000,                        /*!< I2C SCL line frequency. */
    .scl_wait_us = 1000000,                         /*!< Timeout value. (unit: us). Please note this value should not be so small that it can handle stretch/disturbance properly. If 0 is set, that means use the default reg value*/
    .flags = 
    {
        .disable_ack_check = 0                       /*!< Disable ACK check. If this is set false, that means ack check is enabled, the transaction will be stopped and API returns error when nack is detected. */
    }
};

/*
    PROTOTYPES
*/
void smp3011Init();
void smp3011Poll();


/**
 * @brief Entry point of the application.
 *
 * This function configures the I2C master mode and scans the bus for devices.
 * The bus is configured to use GPIO 5 for SDA and GPIO 4 for SCL, and the
 * clock speed is set to 100000 Hz. The scan starts from address 1 and goes
 * to address 126 (inclusive). If a device is found at an address, a message
 * is printed to the console with the address of the device.
 */
void app_main() 
{

    //------------------------------------------------
    // I2C Initialization
    //------------------------------------------------
    ESP_LOGI(TAG, "Initialize I2C bus");
    ESP_ERROR_CHECK(i2c_new_master_bus(&sensor_bus_config, &i2c_sensor_bus));


    //------------------------------------------------
    // I2C Scan
    //------------------------------------------------
    printf("Scanning I2C bus...\n");
    for (int i = 1; i < 127; i++) 
    {
        esp_err_t err = i2c_master_probe(i2c_sensor_bus, i, -1);
        if (err == ESP_OK) 
        {
            printf("Found device at 0x%02x\n", i);                
        }                    
    }    
    

    //------------------------------------------------
    // SMP3011 Initialization
    //------------------------------------------------    
    smp3011Init();

     while(1)
    {
        smp3011Poll(); 
        vTaskDelay(100/portTICK_PERIOD_MS);
    }    
}

void smp3011Init()
{
    i2c_master_bus_add_device(i2c_sensor_bus, &i2c_smp3011_config, &i2c_smp3011_handle);    
    
    uint8_t PressSensorCommand = 0xAC;  //Comando para iniciar conversor ADC
    i2c_master_transmit(i2c_smp3011_handle, (uint8_t *)(&PressSensorCommand), 1, 20); 
}

void smp3011Poll()
{
    uint8_t PressSensorBuffer[6];
    i2c_master_receive(i2c_smp3011_handle, (uint8_t *)(&PressSensorBuffer), sizeof(PressSensorBuffer), 20);

    if((PressSensorBuffer[0]&0x20) == 0)   //Bit5 do status está em 0 significa que a conversão está pronta
    {              
        //printf("Raw Data: %02X %02X %02X %02X %02X %02X\n", PressSensorBuffer[0], PressSensorBuffer[1], PressSensorBuffer[2], PressSensorBuffer[3], PressSensorBuffer[4], PressSensorBuffer[5]);

        uint8_t PressSensorCommand = 0xAC;  //Comando para iniciar conversor ADC
        i2c_master_transmit(i2c_smp3011_handle, (uint8_t *)(&PressSensorCommand), 1, 20);            

        float pressurePercentage = (((uint32_t)PressSensorBuffer[1]<<16)|((uint32_t)PressSensorBuffer[2]<<8)|((uint32_t)PressSensorBuffer[3]));        
        pressurePercentage = (pressurePercentage / 16777215.0f);        
        pressurePercentage -= 0.15f;
        pressurePercentage /= 0.7f;
        pressurePercentage *= 500000.0f;

        float temperaturePercentage = (((uint32_t)PressSensorBuffer[4]<<8)|((uint32_t)PressSensorBuffer[5]));
        temperaturePercentage /= 65535.0f;        
        temperaturePercentage = ((150.0f - (-40.0f))*temperaturePercentage) - 40.0f;


        printf("Pressure: %f  Temperature: %f \n", pressurePercentage, temperaturePercentage);
        
    }
}

```
- Clique no ícone da *Espressif*
- Em seguida, clique em **Build project** e aguarde finalizar a compilação  
- Conecte o Kit na porta USB
- Clique em **ESP-IDF: Select Flash Method**
- No centro da tele selecione a opção **UART**
- Clique em **Select Port to Use** 
- No centro da tele selecione a porta serial correspondente ao Kit, exemplo **COM2**
- Clique em **Select Monitor Port to Use** 
- No centro da tele selecione a porta serial correspondente ao Kit, exemplo **COM2**
- No Kit, mantenha o botão ***BOOT*** pressionado e clique em **Flash Device**
- Assim que o *download* começar, libere o botão ***BOOT***
- Quando finalizar o *download* clique em **Monitor Device**
- Pressione o botão ***EN*** para reiniciar a placa
- O códido do scanner irá executar e irá encontrar o dispositivo ***0x78*** , que é o sensor **SMP3011**  

