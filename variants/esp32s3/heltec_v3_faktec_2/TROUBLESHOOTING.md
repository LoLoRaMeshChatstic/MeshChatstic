# Troubleshooting - Critical Fault 3 en Heltec V3 FAKTEC 2

## Problema: Critical Fault 3 al agregar soporte ILI9225

### Causa Probable

El **Critical Fault 3** (también conocido como `StoreProhibited` o `LoadProhibited`) es típicamente causado por:

1. **Conflicto de buses SPI**: Acceso simultáneo al mismo bus SPI desde el módulo LoRa y la pantalla
2. **Corrupción de memoria**: Punteros NULL o acceso a memoria no inicializada
3. **Timing incorrecto**: Inicialización de dispositivos SPI sin delays suficientes

### Análisis de la Configuración

#### LoRa SX1262 (Bus SPI2_HOST - por defecto)

```
SCK  = GPIO 9
MOSI = GPIO 10
MISO = GPIO 11
CS   = GPIO 8
```

#### Pantalla ILI9225 (Bus SPI3_HOST - separado)

```
SCK  = GPIO 19
MOSI = GPIO 20
MISO = -1 (no usado)
CS   = GPIO 5
DC   = GPIO 7
RST  = GPIO 33
BL   = GPIO 21
```

**✅ CORRECTO**: Los buses están físicamente separados, lo cual es bueno.

## Cambios Realizados para Solucionar

### 1. Configuración de Bus No Compartido

**Archivo**: `platformio.ini`

```ini
-D ILI9225_BUS_SHARED=false
```

Esto asegura que LovyanGFX no intente compartir el bus con otros dispositivos.

### 2. Sincronización con SPILock

**Archivo**: `TFTDisplay.cpp`

- Añadidos logs detallados durante la inicialización
- Delay de 50ms antes de inicializar la pantalla
- Uso correcto de `concurrency::LockGuard` para proteger el acceso SPI

### 3. Información de Diagnóstico Mejorada

Los logs ahora muestran:

- Valor del SPI_HOST usado
- Todos los pines configurados
- Estado de cada paso de inicialización

## Cómo Depurar

### 1. Verificar Logs Seriales

Busca estos mensajes durante el arranque:

```
[INFO] Do TFT init
[INFO] ILI9225 SPI_HOST value: 2 (SPI2_HOST=1, SPI3_HOST=2)
[INFO] ILI9225 pins - SCK:19 MOSI:20 MISO:-1 CS:5 DC:7 RST:33
[INFO] Initializing TFT display...
[INFO] TFT init completed successfully
```

### 2. Si Ocurre Critical Fault 3

Captura el **backtrace completo** del serial monitor. Ejemplo:

```
Guru Meditation Error: Core 0 panic'ed (StoreProhibited). Exception was unhandled.
Core 0 register dump:
PC      : 0x4200abcd  PS      : 0x00060330  A0      : 0x82012345
...
Backtrace: 0x4200abcd:0x3fceb210 0x42012342:0x3fceb230 ...
```

Usa el Exception Decoder:

```bash
python bin/exception_decoder.py <backtrace>
```

### 3. Pruebas Incrementales

#### Paso 1: Desactivar Pantalla Temporalmente

En `platformio.ini`, comenta:

```ini
; -D USE_ILI9225
```

Compila y verifica que el LoRa funciona correctamente.

#### Paso 2: Activar Solo Inicialización de Pantalla

Reactiva `-D USE_ILI9225` y verifica que la pantalla se detecta pero no la uses aún.

#### Paso 3: Activación Completa

Una vez que ambos funcionen por separado, activa todo.

### 4. Verificaciones de Hardware

#### A. Alimentación

- Verifica que la pantalla ILI9225 tenga su propia alimentación de 3.3V
- **NO conectes** VCC de la pantalla al pin VEXT del Heltec (está para el LoRa)
- Usa un pin de 3.3V dedicado o el regulador del Heltec

#### B. Conexiones de Tierra

- **CRÍTICO**: Asegúrate de que GND de la pantalla esté conectado al GND del Heltec
- Una mala conexión de tierra puede causar critical faults

#### C. Resistencias Pull-up/Pull-down

- Si usas cables largos (>10cm), considera añadir resistencias de 10kΩ pull-up en:
  - CS (GPIO 5)
  - DC (GPIO 7)

#### D. Condensadores de Desacople

- Añade un condensador de 100nF cerca del pin VCC de la pantalla
- Esto reduce ruido en la línea de alimentación

## Soluciones Adicionales

### Si el Problema Persiste

#### Opción 1: Reducir Frecuencia SPI

En `platformio.ini`:

```ini
-D SPI_FREQUENCY=5000000    ; Reducir de 10MHz a 5MHz
-D SPI_READ_FREQUENCY=4000000
```

#### Opción 2: Desactivar DMA Temporalmente

En `TFTDisplay.cpp`, línea ~143:

```cpp
cfg.dma_channel = SPI_DMA_CH_DISABLE;  // En lugar de SPI_DMA_CH_AUTO
```

#### Opción 3: Usar el Mismo Bus SPI (Avanzado)

Si las opciones anteriores no funcionan, puedes intentar compartir el bus:

En `platformio.ini`:

```ini
-D ILI9225_SPI_HOST=SPI2_HOST  ; Usar mismo bus que LoRa
-D ILI9225_SCK=9               ; Compartir pines SPI con LoRa
-D ILI9225_MOSI=10
-D ILI9225_MISO=11
-D ILI9225_CS=5                ; CS diferente es OBLIGATORIO
```

**⚠️ ADVERTENCIA**: Requiere configurar `bus_shared=true` y puede degradar rendimiento.

## Monitoreo de Recursos

### Verificar Uso de Memoria

Añade en tu código de setup:

```cpp
LOG_INFO("Heap libre antes de TFT: %d", ESP.getFreeHeap());
LOG_INFO("PSRAM libre antes de TFT: %d", ESP.getFreePsram());
```

Si la memoria libre es <80KB, puede haber problemas de memoria que causen faults.

### Verificar Stack

Critical Fault 3 también puede ocurrir por stack overflow. Aumenta el tamaño del stack si es necesario.

## Checklist de Verificación

- [ ] Logs seriales capturados con detalles completos
- [ ] LoRa funciona correctamente sin la pantalla
- [ ] Conexiones de hardware verificadas (GND, VCC, todos los pines)
- [ ] Condensadores de desacople instalados
- [ ] Frecuencias SPI reducidas para prueba
- [ ] Backtrace decodificado si ocurre fault
- [ ] Memoria heap >80KB disponible

## Información de Versiones

**Firmware Base**: Meshtastic (versión actual)
**LovyanGFX**: v1.1.16
**ESP32-S3**: Heltec WiFi LoRa 32 V3
**Pantalla**: ILI9225 2" 176x220 SPI

## Contacto y Soporte

Si el problema persiste después de seguir estos pasos:

1. Captura el backtrace completo
2. Captura los logs seriales completos desde el arranque
3. Toma fotos de las conexiones de hardware
4. Documenta todos los cambios realizados

---

**Última actualización**: 2025-10-15
**Versión**: FAKTEC 2 - ILI9225 Troubleshooting Guide
