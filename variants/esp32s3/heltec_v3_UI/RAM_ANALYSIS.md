# Análisis de Requisitos de RAM para Device-UI en Heltec V3

## Hardware Actual

- **MCU**: ESP32-S3FN8 (sin PSRAM)
- **SRAM Total**: 512 KB
- **SRAM Disponible**: 320 KB (192 KB reservados para sistema/bootloader)
- **SRAM Usable**: ~248 KB (después de restar overhead del sistema FreeRTOS)
- **Flash**: 8MB
- **Display**: ILI9225 176x220 SPI TFT (38,720 pixels)

## Configuración Actual

- **RAM_SIZE**: 48 KB (49,152 bytes)
- **LV_MEM_SIZE**: 32 KB (32,768 bytes)
- **Draw Buffer LVGL**: 14,520 bytes (calculado automáticamente)
- **Free Heap al Boot**: 179,216 bytes (~175 KB)
- **Free Heap después de init**: 88,200 bytes (~86 KB)

## Requisitos de Memoria de Device-UI

### 1. **RAM_SIZE** (Buffer estático de LVGL)

El `RAM_SIZE` define cuántos **KB** de memoria se asignan para el **buffer de dibujo principal** de LVGL.

**Fórmula aproximada**:

```
RAM_SIZE (bytes) = RAM_SIZE_KB * 1024
```

**Ejemplos de otros dispositivos**:
| Dispositivo | Resolución | PSRAM | RAM_SIZE (KB) | Notas |
|------------|-----------|-------|---------------|-------|
| T-Deck | 320x240 | ✅ Sí | 5 | Con PSRAM puede usar valores pequeños |
| Mesh-Tab | 320x240 | ❌ No | 1 | Muy comprimido, probablemente con problemas |
| PiComputer-S3 | 320x240 | ❌ No | 1.56 | Display similar al nuestro |
| TLora-Pager | 320x240 | ✅ Sí | 5 | Con PSRAM |
| Heltec V3 UI | 176x220 | ❌ No | 48 | **Nuestra config actual** |

**Nota importante**: Los dispositivos con PSRAM pueden usar RAM_SIZE pequeños porque LVGL puede almacenar buffers en PSRAM externa. Sin PSRAM, necesitamos más SRAM interna.

### 2. **LV_MEM_SIZE** (Heap interno de LVGL)

El `LV_MEM_SIZE` define la **memoria heap interna** que LVGL usa para asignar dinámicamente objetos (widgets, estilos, animaciones, etc.).

**Valor actual**: 32 KB (32,768 bytes)

**Problema**: Este heap es **SEPARADO** del heap del sistema ESP32. LVGL administra su propia memoria internamente. Si este valor es muy pequeño, LVGL no puede crear widgets complejos.

### 3. **Draw Buffer** (Buffer de renderizado)

LVGL calcula automáticamente el tamaño del draw buffer basándose en:

```
draw_buffer_size = (LGFX_SCREEN_WIDTH * líneas_por_buffer * bytes_por_pixel)
```

Para nuestra configuración:

- Width: 176 pixels
- Color depth: 16-bit (2 bytes por pixel)
- Líneas estimadas: ~41 líneas

```
14,520 bytes ≈ 176 * 41 * 2
```

**Este buffer se asigna del HEAP del sistema**, no del LV_MEM_SIZE.

## Distribución de Memoria Actual

### Al Boot (antes de inicializar LVGL):

```
Total heap: 215,760 bytes
Free heap: 179,216 bytes (~175 KB disponible)
Used: 36,544 bytes (sistema base, LoRa, GPS, etc.)
```

### Después de inicializar LVGL (pero antes de crear widgets):

```
Free heap: 88,200 bytes (~86 KB)
```

**Consumo de LVGL**:

```
179,216 - 88,200 = 91,016 bytes (~89 KB)
```

**Desglose estimado**:

- RAM_SIZE buffer: 49,152 bytes (48 KB)
- Draw buffer: 14,520 bytes (~14 KB)
- LV_MEM_SIZE pool: 32,768 bytes (32 KB) ❌ **NO SE CUENTA AQUÍ**
- Overhead de LVGL: ~5 KB

**IMPORTANTE**: El `LV_MEM_SIZE` (32 KB) es un **pool interno de LVGL** que se asigna del heap del sistema. Por lo tanto:

```
Total usado por LVGL = 49,152 + 14,520 + 32,768 + 5,000 ≈ 101 KB
```

Pero solo vemos **91 KB** consumidos. Esto sugiere que:

1. El `LV_MEM_SIZE` no se asigna completamente al inicio
2. O LVGL usa un allocator lazy (asigna bajo demanda)

### Durante `init screens...` (FALLA AQUÍ):

LVGL intenta crear los widgets de la UI (pantallas, botones, labels, imágenes, etc.) usando:

1. **LV_MEM_SIZE heap** (32 KB) para objetos LVGL
2. **Heap del sistema** (88 KB libres) para asignaciones grandes

**Error observado**: `lv_realloc: couldn't reallocate memory`

Esto indica que LVGL se quedó sin memoria en su pool interno (LV_MEM_SIZE=32 KB) O el heap del sistema (88 KB) no tiene suficiente espacio contiguo.

## Cálculo de Requisitos Mínimos

### Opción 1: Aumentar LV_MEM_SIZE (recomendado)

El problema principal es que **LV_MEM_SIZE=32 KB es insuficiente** para device-ui con todas sus pantallas y widgets.

**Configuraciones recomendadas**:

| LV_MEM_SIZE | RAM_SIZE | Draw Buffer | Total LVGL | Free Heap | Viable?             |
| ----------- | -------- | ----------- | ---------- | --------- | ------------------- |
| 32 KB       | 48 KB    | 14 KB       | ~94 KB     | ~86 KB    | ❌ **Falla**        |
| 48 KB       | 48 KB    | 14 KB       | ~110 KB    | ~70 KB    | ⚠️ Puede funcionar  |
| 64 KB       | 32 KB    | 14 KB       | ~110 KB    | ~70 KB    | ✅ **Mejor opción** |
| 64 KB       | 48 KB    | 14 KB       | ~126 KB    | ~54 KB    | ⚠️ Muy ajustado     |

**Recomendación**:

```ini
-D LV_MEM_SIZE=65536  ; 64 KB para heap interno de LVGL
-D RAM_SIZE=32        ; 32 KB para buffer de dibujo (suficiente para 176x220)
```

Esto libera 16 KB del RAM_SIZE buffer (que es estático) y lo mueve al LV_MEM_SIZE (que LVGL puede administrar mejor).

### Opción 2: Reducir complejidad de Device-UI

Si aumentar LV_MEM_SIZE no funciona, necesitamos:

1. **Deshabilitar pantallas** que no usas (Map, Debug, etc.)
2. **Reducir número de widgets** en pantallas principales
3. **Usar imágenes más pequeñas** (comprimir assets)
4. **Deshabilitar más features**: Grid layouts, themes, etc.

### Opción 3: Revertir a BaseUI (faktec-2)

El baseui (faktec-2) usa **mucho menos RAM** porque:

- No usa LVGL (usa OLEDDisplay simple)
- No carga assets en memoria
- Renderiza directamente al display

**Uso de RAM estimado**: ~20-30 KB total

## Próximos Pasos

### 1. Probar con LV_MEM_SIZE=64 KB

```ini
-D LV_MEM_SIZE=65536
-D RAM_SIZE=32
```

Compilar y verificar si el sistema pasa de `init screens...`

### 2. Si aún falla, aumentar más

```ini
-D LV_MEM_SIZE=81920  ; 80 KB
-D RAM_SIZE=24        ; 24 KB
```

### 3. Monitorear memoria durante ejecución

Agregar logs para ver cuánto heap usa LVGL:

```cpp
LOG_DEBUG("LVGL mem used: %d / %d", lv_mem_get_size() - lv_mem_get_free_size(), lv_mem_get_size());
LOG_DEBUG("Free heap: %d", ESP.getFreeHeap());
```

### 4. Si todo falla

Considera:

- Excluir módulos adicionales (Telemetry, NeighborInfo, SerialModule)
- Revertir a faktec-2 baseui
- Esperar a que device-ui optimice uso de memoria

## Conclusión

**El problema NO es el RAM_SIZE (48 KB es suficiente)**. El problema es el **LV_MEM_SIZE (32 KB es insuficiente)** para que device-ui cree todas sus pantallas y widgets.

**Solución recomendada**: Aumentar `LV_MEM_SIZE` a 64-80 KB y opcionalmente reducir `RAM_SIZE` a 24-32 KB para compensar.

El límite teórico máximo que podemos usar:

```
Total disponible: ~248 KB
Sistema base: ~40 KB
LoRa/GPS/Periféricos: ~30 KB
Disponible para LVGL: ~178 KB

Distribución óptima:
- LV_MEM_SIZE: 80 KB (heap interno LVGL)
- RAM_SIZE: 32 KB (buffer de dibujo)
- Draw buffer: 14 KB (auto-calculado)
- Free heap restante: ~52 KB (para sistema)
```

Esta configuración debería permitir que device-ui funcione correctamente sin PSRAM.
