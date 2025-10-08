#!/bin/bash

# Script para aplicar optimización de memoria a todos los dispositivos nRF52840
# que no tienen MESHTASTIC_EXCLUDE_CHAT_HISTORY

echo "Aplicando optimización de memoria a dispositivos nRF52840..."

# Lista de dispositivos que necesitan optimización
devices=(
	"/workspaces/firmware/variants/nrf52840/gat562_mesh_trial_tracker/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/meshlink/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/wio-sdk-wm1110/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/seeed_wio_tracker_L1/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/Dongle_nRF52840-pca10059-v1/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/t-echo-lite/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/wio-tracker-wm1110/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/TWC_mesh_v4/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/MS24SF1/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/meshlink_eink/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/diy/WashTastic/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/diy/seeed_xiao_nrf52840_e22/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/diy/xiao_ble/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/diy/seeed-xiao-nrf52840-wio-sx1262/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/rak4631_epaper/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/ME25LS01-4Y10TD_e-ink/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/rak_wismeshtap/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/feather_diy/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/rak_wismeshtag/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/seeed_xiao_nrf52840_kit/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/canaryone/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/rak2560/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/rak4631_epaper_onrxtx/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/rak4631_eth_gw/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/MakePython_nRF52840_eink/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/ELECROW-ThinkNode-M1/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/wio-t1000-s/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/heltec_mesh_solar/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/nano-g2-ultra/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/rak4631_nomadstar_meteor_pro/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/tracker-t1000-e/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/seeed_wio_tracker_L1_eink/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/ME25LS01-4Y10TD/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/heltec_mesh_pocket/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/seeed_solar_node/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/heltec_mesh_node_t114-inkhud/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/MakePython_nRF52840_oled/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/monteops_hw1/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/meshtiny/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/t-echo/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/heltec_mesh_node_t114/platformio.ini"
	"/workspaces/firmware/variants/nrf52840/r1-neo/platformio.ini"
)

count=0
for file in "${devices[@]}"; do
	if [ -f "$file" ]; then
		# Buscar la línea con build_flags y agregar la optimización
		if grep -q "build_flags" "$file"; then
			# Verificar si ya tiene la optimización
			if ! grep -q "MESHTASTIC_EXCLUDE_CHAT_HISTORY" "$file"; then
				# Agregar la optimización después de la línea build_flags
				sed -i '/build_flags/a\  -D MESHTASTIC_EXCLUDE_CHAT_HISTORY=1' "$file"
				echo "✅ Optimizado: $(basename $(dirname $file))"
				((count++))
			else
				echo "⚠️  Ya optimizado: $(basename $(dirname $file))"
			fi
		else
			echo "❌ No se encontró build_flags en: $(basename $(dirname $file))"
		fi
	else
		echo "❌ Archivo no encontrado: $file"
	fi
done

echo ""
echo "🎉 Optimización completada: $count dispositivos nRF52840 actualizados"
echo "✅ Todos los dispositivos nRF52840 ahora tienen optimización de memoria"
