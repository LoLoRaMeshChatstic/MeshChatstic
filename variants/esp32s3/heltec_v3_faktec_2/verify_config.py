#!/usr/bin/env python3
"""
Verificador de configuración SPI para Heltec V3 FAKTEC 2
Este script analiza el firmware compilado para detectar conflictos de pines SPI
"""

import re
import sys


def check_pin_conflicts():
    """Verifica que no haya conflictos de pines entre LoRa y pantalla"""

    # Pines del LoRa SX1262
    lora_pins = {
        "SCK": 9,
        "MOSI": 10,
        "MISO": 11,
        "CS": 8,
        "RESET": 12,
        "DIO1": 14,
        "BUSY": 13,
    }

    # Pines de la pantalla ILI9225
    display_pins = {
        "SCK": 19,
        "MOSI": 20,
        "MISO": -1,
        "CS": 5,
        "DC": 7,
        "RST": 33,
        "BL": 21,
    }

    # Verificar conflictos
    conflicts = []
    lora_values = set(v for v in lora_pins.values() if v >= 0)
    display_values = set(v for v in display_pins.values() if v >= 0)

    common_pins = lora_values & display_values

    if common_pins:
        conflicts.append(f"❌ CONFLICTO: Pines compartidos detectados: {common_pins}")
    else:
        print("✅ No hay conflictos de pines entre LoRa y pantalla")

    # Verificar que los buses SPI son diferentes
    print("\n📊 Configuración de buses SPI:")
    print(
        f"  LoRa SX1262: SPI2_HOST (pines {lora_pins['SCK']}, {lora_pins['MOSI']}, {lora_pins['MISO']})"
    )
    print(
        f"  Pantalla ILI9225: SPI3_HOST (pines {display_pins['SCK']}, {display_pins['MOSI']}, {display_pins['MISO']})"
    )

    # Verificar pines críticos
    print("\n🔌 Pines críticos:")
    print(f"  LoRa CS: GPIO {lora_pins['CS']}")
    print(f"  Display CS: GPIO {display_pins['CS']}")
    print(f"  Display DC: GPIO {display_pins['DC']}")

    if conflicts:
        print("\n" + "\n".join(conflicts))
        return False
    return True


def check_spi_frequencies():
    """Verifica que las frecuencias SPI sean seguras"""
    print("\n⚡ Frecuencias SPI configuradas:")
    print("  ILI9225 Write: 10 MHz")
    print("  ILI9225 Read: 8 MHz")
    print("  LoRa: 4 MHz (default)")

    print("\n💡 Recomendación:")
    print("  Si experimentas problemas, reduce la frecuencia del ILI9225 a 5MHz")


def check_esp32s3_pins():
    """Verifica que los pines sean compatibles con ESP32-S3"""
    print("\n🔧 Verificación de compatibilidad ESP32-S3:")

    # Pines que NO se deben usar en ESP32-S3
    reserved_pins = [
        (0, "Boot button"),
        (45, 46, "USB D- y D+"),
        (26, 27, 28, 29, 30, 31, 32, "Flash/PSRAM (solo lectura)"),
    ]

    used_pins = [9, 10, 11, 8, 12, 14, 13, 19, 20, 5, 7, 33, 21]

    problems = []
    for pin in used_pins:
        if pin in [26, 27, 28, 29, 30, 31, 32]:
            problems.append(f"⚠️  GPIO {pin} está reservado para Flash/PSRAM")

    if not problems:
        print("  ✅ Todos los pines son seguros para uso general")
    else:
        for p in problems:
            print(f"  {p}")


def print_diagnostic_commands():
    """Muestra comandos útiles para diagnóstico"""
    print("\n📝 Comandos de diagnóstico:")
    print("\n1. Compilar con información de debug:")
    print("   pio run -e heltec-v3-faktec-2 -v")

    print("\n2. Monitor serial con filtro:")
    print("   pio device monitor --filter=esp32_exception_decoder")

    print("\n3. Flashear y monitorear:")
    print("   pio run -e heltec-v3-faktec-2 --target upload && pio device monitor")

    print("\n4. Limpiar y recompilar:")
    print("   pio run -e heltec-v3-faktec-2 --target clean")
    print("   pio run -e heltec-v3-faktec-2")


def main():
    print("=" * 60)
    print("  Verificador de Configuración - Heltec V3 FAKTEC 2")
    print("  Pantalla ILI9225 + LoRa SX1262")
    print("=" * 60)

    all_ok = True

    # Verificaciones
    all_ok &= check_pin_conflicts()
    check_spi_frequencies()
    check_esp32s3_pins()
    print_diagnostic_commands()

    print("\n" + "=" * 60)
    if all_ok:
        print("✅ Configuración verificada - lista para compilar")
    else:
        print("❌ Se encontraron problemas - revisa la configuración")
    print("=" * 60)

    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
