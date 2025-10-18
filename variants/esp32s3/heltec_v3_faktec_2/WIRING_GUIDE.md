# Guía de Soldadura - Pantalla ILI9225 para Heltec V3 FAKTEC 2

## 📋 Pines de la Pantalla ILI9225

Tu módulo tiene estos pines (de izquierda a derecha):

```
┌─────────────────────────────────────┐
│  VCC  GND  NC  NC  LED  CLK  SDI   │
│  RES REST  CS                      │
└─────────────────────────────────────┘
```

## 🔌 Tabla de Conexiones

| Pin Pantalla | Función      | →   | Heltec V3 GPIO | Cable Sugerido |
| ------------ | ------------ | --- | -------------- | -------------- |
| **VCC**      | Alimentación | →   | **3.3V**       | Rojo           |
| **GND**      | Tierra       | →   | **GND**        | Negro          |
| **NC**       | No conectar  | →   | ---            | ---            |
| **NC**       | No conectar  | →   | ---            | ---            |
| **LED**      | Backlight    | →   | **GPIO 21**    | Blanco         |
| **CLK**      | SPI Clock    | →   | **GPIO 19**    | Amarillo       |
| **SDI**      | SPI MOSI     | →   | **GPIO 20**    | Naranja        |
| **RES**      | Reset        | →   | **GPIO 33**    | Azul           |
| **REST**     | DC/RS        | →   | **GPIO 34**    | Verde          |
| **CS**       | Chip Select  | →   | **GPIO 5**     | Violeta        |

## 🎨 Diagrama Visual

```
    PANTALLA ILI9225                      HELTEC V3
    ════════════════                      ═════════

    VCC (Pin 1)    ────────[ROJO]────────>  3.3V

    GND (Pin 2)    ────────[NEGRO]───────>  GND

    NC  (Pin 3)    ─────────────────────>  (no conectar)

    NC  (Pin 4)    ─────────────────────>  (no conectar)

    LED (Pin 5)    ────────[BLANCO]──────>  GPIO 21 ⚡ PWM

    CLK (Pin 6)    ────────[AMARILLO]────>  GPIO 19 (SPI SCK)

    SDI (Pin 7)    ────────[NARANJA]─────>  GPIO 20 (SPI MOSI)

    RES (Pin 8)    ────────[AZUL]────────>  GPIO 33 (Reset)

    REST (Pin 9)   ────────[VERDE]───────>  GPIO 34 (DC/RS)

    CS  (Pin 10)   ────────[VIOLETA]─────>  GPIO 5  (CS)
```

## ⚠️ Notas Importantes

### Alimentación

- ✅ **Usar 3.3V** - NO conectar a 5V
- ✅ La pantalla tiene regulador integrado pero funciona mejor a 3.3V
- ✅ Conectar GND primero antes de VCC

### Pines NC (No Conectar)

- ❌ **No soldar** los pines marcados como NC
- Son para el lector SD integrado (no lo usamos)

### Backlight (LED)

- GPIO 21 controla el brillo por PWM
- Si no conectas LED, la pantalla funcionará pero sin luz de fondo
- Puedes conectarlo después si lo necesitas

### Orden de Soldadura Recomendado

1. **Primero**: GND y VCC (alimentación)
2. **Segundo**: CS, CLK, SDI (SPI básico)
3. **Tercero**: RES, REST (control)
4. **Último**: LED (backlight - opcional al principio)

## 🔍 Verificación de Conexiones

Antes de encender, verifica con multímetro:

```
✓ Continuidad VCC pantalla → 3.3V Heltec
✓ Continuidad GND pantalla → GND Heltec
✓ NO hay cortocircuito entre VCC y GND
✓ Cada pin GPIO va a su destino correcto
```

## 🚀 Primera Prueba

Una vez soldado y flasheado el firmware:

1. Conectar Heltec V3 por USB
2. La pantalla debería mostrar el logo de Meshtastic
3. El backlight debería encenderse (GPIO 21 HIGH)
4. Si no ves nada, ajusta contraste o verifica VCC

## 🛠️ Troubleshooting

### Pantalla en blanco

- Verifica GPIO 21 (backlight) está conectado
- Mide voltaje en pin VCC de pantalla (debe ser ~3.3V)
- Revisa conexión RES (reset)

### Pantalla con pixeles aleatorios

- Revisa conexiones SPI (CLK, SDI)
- Verifica CS está conectado
- Reduce frecuencia SPI en platformio.ini

### Pantalla con colores invertidos

- Normal en primera prueba
- Se ajusta en configuración (TFT_INVERT)

## 📸 Foto de Referencia de Pines

```
Vista FRONTAL de la pantalla (mirando el display):

Pines abajo a la izquierda →

[1] VCC  ●────── 3.3V (ROJO)
[2] GND  ●────── GND (NEGRO)
[3] NC   ●
[4] NC   ●
[5] LED  ●────── GPIO 21 (BLANCO)
[6] CLK  ●────── GPIO 19 (AMARILLO)
[7] SDI  ●────── GPIO 20 (NARANJA)
[8] RES  ●────── GPIO 33 (AZUL)
[9] REST ●────── GPIO 34 (VERDE)
[10] CS  ●────── GPIO 5 (VIOLETA)
```

## ✅ Checklist Final

- [ ] VCC → 3.3V (ROJO)
- [ ] GND → GND (NEGRO)
- [ ] LED → GPIO 21 (BLANCO)
- [ ] CLK → GPIO 19 (AMARILLO)
- [ ] SDI → GPIO 20 (NARANJA)
- [ ] RES → GPIO 33 (AZUL)
- [ ] REST → GPIO 34 (VERDE)
- [ ] CS → GPIO 5 (VIOLETA)
- [ ] Firmware flasheado (heltec-v3-faktec-2)
- [ ] No hay cortocircuitos
- [ ] Cables bien soldados

¡Listo para probar! 🎉
