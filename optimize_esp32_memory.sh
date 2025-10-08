#!/bin/bash

# Script para optimizar memoria en dispositivos ESP32 originales
# Añade MESHTASTIC_EXCLUDE_CHAT_HISTORY=1 a los build_flags

echo "🔧 Iniciando optimización de memoria para dispositivos ESP32..."

# Lista de dispositivos ESP32 originales que pueden tener problemas de memoria
esp32_devices=(
	"variants/esp32/tlora_v2_1_18/platformio.ini"
	"variants/esp32/tlora_v1/platformio.ini"
	"variants/esp32/m5stack_core/platformio.ini"
	"variants/esp32/tbeam_v07/platformio.ini"
	"variants/esp32/heltec_wsl_v2.1/platformio.ini"
)

optimized_count=0

for device_path in "${esp32_devices[@]}"; do
	if [[ -f $device_path ]]; then
		# Verificar si ya tiene la optimización
		if grep -q "MESHTASTIC_EXCLUDE_CHAT_HISTORY=1" "$device_path"; then
			echo "⚠️  $device_path ya tiene optimización"
		else
			# Añadir la optimización después de la última línea de build_flags
			echo "📝 Optimizando $device_path..."

			# Usar sed para añadir la optimización
			sed -i '/build_flags = /,/^[[:space:]]*-D/ {
                /^[[:space:]]*-D/ a\
  -D MESHTASTIC_EXCLUDE_CHAT_HISTORY=1
                t added
                b
                :added
                n
                /^[[:space:]]*-D/ b added
            }' "$device_path"

			((optimized_count++))
			echo "✅ Añadida optimización a $device_path"
		fi
	else
		echo "❌ No encontrado: $device_path"
	fi
done

echo ""
echo "🎉 Optimización completada: $optimized_count dispositivos ESP32 actualizados"
echo "💡 Los dispositivos optimizados ahora tendrán memoria reducida pero mejor compatibilidad"
