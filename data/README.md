# Carpeta data para LittleFS

Esta carpeta se usa para subir archivos al filesystem LittleFS del ESP32.

El comando `pio run -t uploadfs` formateará y subirá el contenido de esta carpeta.

Actualmente vacía - LittleFS se inicializará y formateará automáticamente con FORMAT_LITTLEFS_IF_FAILED=1.
