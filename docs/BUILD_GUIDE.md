# Guía de Construcción - 3DShell

## Requisitos del Sistema
- macOS/Linux/Windows con devkitPro instalado
- devkitARM toolchain
- ctrulib, Citro3D, Citro2D
- makerom y bannertool en PATH

## Instalación de Dependencias
```bash
# Instalar devkitPro
sudo dkp-pacman -Syu 3ds-dev --noconfirm --needed
sudo dkp-pacman -Syu 3ds-curl --noconfirm --force
sudo dkp-pacman -Syu 3ds-libarchive 3ds-jansson 3ds-libjpeg-turbo 3ds-libpng --noconfirm
```

## Proceso de Construcción
1. Clonar el repositorio con submódulos:
   ```bash
   git clone --recursive https://github.com/joel16/3DShell.git
   cd 3DShell
   ```

2. Construir el binario:
   ```bash
   make
   ```

3. Los archivos de salida estarán en el directorio raíz:
   - `3DShell.3dsx`: Archivo ejecutable para homebrew
   - `3DShell.smdh`: Metadatos de la aplicación

## Configuración del Makefile
- TARGET: Nombre del ejecutable
- SOURCES: Directorios con código fuente
- INCLUDES: Directorios con headers
- ROMFS: Directorio para recursos embebidos
- VERSION: Control de versiones (5.1.0)

## Solución de Problemas
- Verificar que DEVKITARM esté configurado
- Asegurar que todas las dependencias estén instaladas
- Revisar logs de compilación para errores específicos