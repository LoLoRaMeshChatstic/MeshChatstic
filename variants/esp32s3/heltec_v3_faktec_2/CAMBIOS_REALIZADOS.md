# Resumen de Cambios - Solución Critical Fault 3

## Fecha: 15 de octubre de 2025

## Variante: Heltec V3 FAKTEC 2 con ILI9225

---

## 🔴 PROBLEMA IDENTIFICADO

**Critical Fault 3** al añadir soporte para pantalla ILI9225 con el módulo LoRa SX1262.

### Causas Raíz:

1. ⚠️ Configuración incorrecta de `bus_shared = true` cuando los buses están separados
2. ⚠️ Falta de sincronización adecuada entre inicialización de pantalla y radio
3. ⚠️ Posible conflicto de recursos DMA entre ambos dispositivos SPI
4. ⚠️ Falta de delays en la inicialización de la pantalla

---

## ✅ CAMBIOS REALIZADOS

### 1. **platformio.ini** - Configuración de Bus Separado

**Cambio:**

```ini
-D ILI9225_BUS_SHARED=false
```

**Razón:**
Los buses SPI están físicamente separados (LoRa en SPI2, Pantalla en SPI3), por lo que NO deben configurarse como compartidos.

---

### 2. **TFTDisplay.cpp** - Configuración de Bus Compartido Condicional

**Antes:**

```cpp
cfg.bus_shared = true;  // Bus is shared with LoRa radio (different CS pins)
```

**Después:**

```cpp
#ifdef ILI9225_BUS_SHARED
    cfg.bus_shared = ILI9225_BUS_SHARED;
#else
    cfg.bus_shared = false;  // Default: separate SPI bus
#endif
```

**Razón:**
Permite configurar dinámicamente si el bus está compartido o no, haciendo el código más flexible y evitando conflictos.

---

### 3. **TFTDisplay.cpp** - Mejoras en Inicialización

**Cambios añadidos:**

```cpp
// Logs detallados de configuración
LOG_INFO("ILI9225 SPI_HOST value: %d (SPI2_HOST=1, SPI3_HOST=2)", ILI9225_SPI_HOST);
LOG_INFO("ILI9225 pins - SCK:%d MOSI:%d MISO:%d CS:%d DC:%d RST:%d", ...);

// Delay para estabilización de pines
delay(50);

// Logs de progreso de inicialización
LOG_INFO("Initializing TFT display...");
tft->init();
LOG_INFO("TFT init completed successfully");
```

**Razón:**

- Proporciona información de depuración detallada
- El delay de 50ms asegura que los pines estén estables antes de la inicialización
- Facilita identificar en qué punto ocurre el fallo

---

### 4. **variant.h** - Documentación de Pines

**Cambio:**
Añadidos comentarios extensivos documentando:

- Bus SPI usado por cada dispositivo
- Función de cada pin
- Advertencias sobre no compartir pines

**Razón:**
Prevenir errores futuros y facilitar el mantenimiento del código.

---

## 🧪 CÓMO PROBAR

### Paso 1: Compilar el Firmware

```bash
cd "c:\Users\MiniPC\Desktop\mhsaez mesh\firmware"
python variants/esp32s3/heltec_v3_faktec_2/verify_config.py
pio run -e heltec-v3-faktec-2
```

### Paso 2: Flashear el Dispositivo

```bash
pio run -e heltec-v3-faktec-2 --target upload
```

### Paso 3: Monitorear el Serial

```bash
pio device monitor --filter=esp32_exception_decoder
```

### Qué Buscar en los Logs:

```
✅ CORRECTO:
[INFO] ILI9225 SPI_HOST value: 2 (SPI2_HOST=1, SPI3_HOST=2)
[INFO] ILI9225 pins - SCK:19 MOSI:20 MISO:-1 CS:5 DC:7 RST:33
[INFO] Initializing TFT display...
[INFO] TFT init completed successfully
[INFO] SX1262 init success

❌ ERROR:
Guru Meditation Error: Core 0 panic'ed (StoreProhibited)
```

---

## 🔧 SI EL PROBLEMA PERSISTE

### Opción 1: Reducir Frecuencias SPI

Edita `platformio.ini`:

```ini
-D SPI_FREQUENCY=5000000      # En lugar de 10000000
-D SPI_READ_FREQUENCY=4000000 # En lugar de 8000000
```

### Opción 2: Desactivar DMA Temporalmente

Edita `src/graphics/TFTDisplay.cpp` línea ~143:

```cpp
cfg.dma_channel = SPI_DMA_CH_DISABLE;  // En lugar de SPI_DMA_CH_AUTO
```

### Opción 3: Aumentar Stack Size

Edita `platformio.ini`:

```ini
board_build.arduino.memory_type = qio_opi
build_flags =
  ...
  -D ARDUINO_LOOP_STACK_SIZE=16384  # Aumentar de 8192
```

### Opción 4: Inicialización Secuencial Estricta

Asegurar que el radio se inicialice ANTES que la pantalla.

---

## 📊 VERIFICACIÓN DE HARDWARE

### Checklist de Conexiones:

#### Pantalla ILI9225:

- [ ] VCC → 3.3V (NO usar VEXT)
- [ ] GND → GND (conexión sólida)
- [ ] SCK → GPIO 19
- [ ] MOSI → GPIO 20
- [ ] CS → GPIO 5
- [ ] DC → GPIO 7
- [ ] RST → GPIO 33
- [ ] BL → GPIO 21 (backlight)

#### LoRa SX1262 (interno Heltec):

- [ ] Antena conectada correctamente
- [ ] No modificar pines GPIO 8, 9, 10, 11, 12, 13, 14

### Verificaciones Eléctricas:

- [ ] Condensador 100nF cerca del VCC de la pantalla
- [ ] Cables cortos (<15cm) para SPI
- [ ] Sin cables sueltos o mal soldados
- [ ] Multímetro: 3.3V estable en VCC de la pantalla

---

## 📈 RECURSOS DE DIAGNÓSTICO

### Scripts Incluidos:

1. **verify_config.py**: Verifica configuración de pines y buses
2. **TROUBLESHOOTING.md**: Guía completa de resolución de problemas

### Comandos Útiles:

```bash
# Ver tamaño de firmware compilado
pio run -e heltec-v3-faktec-2 --target size

# Limpiar completamente y recompilar
pio run -e heltec-v3-faktec-2 --target clean
rm -rf .pio/build/heltec-v3-faktec-2
pio run -e heltec-v3-faktec-2

# Decodificar excepciones en tiempo real
pio device monitor --filter esp32_exception_decoder --baud 115200
```

---

## 🎯 PRÓXIMOS PASOS

1. **Compilar** con los cambios realizados
2. **Flashear** el dispositivo
3. **Monitorear** el serial y capturar logs completos
4. **Verificar** hardware si el problema persiste
5. **Reportar** resultados con:
   - Logs seriales completos
   - Backtrace si hay fault
   - Fotos del hardware si es necesario

---

## 📞 INFORMACIÓN ADICIONAL

**Documentos Relacionados:**

- `TROUBLESHOOTING.md` - Guía completa de depuración
- `WIRING_GUIDE.md` - Guía de soldadura y conexiones
- `README.md` - Información general de la variante

**Configuración del Sistema:**

- ESP32-S3 @ 240MHz
- Flash: 8MB
- PSRAM: 2MB (QSPI)
- Buses SPI: 2 (independientes)

---

**Autor:** GitHub Copilot  
**Fecha:** 2025-10-15  
**Versión:** 1.0 - Initial troubleshooting for Critical Fault 3
