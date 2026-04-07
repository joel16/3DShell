# Arquitectura del Código - 3DShell

## Diagrama de Arquitectura
```
main.cpp
├── Services::Init()
│   ├── Inicialización de FS (SD/NAND)
│   ├── Config::Load()
│   ├── Inicialización gráfica (C3D/C2D)
│   └── Textures::Init()
├── GUI::Loop()
└── Services::Exit()
```

## Módulos Principales

### GUI (gui/)
- `gui.cpp`: Bucle principal de la interfaz
- `filebrowser.cpp`: Navegador de archivos
- `image.cpp`: Vista previa de imágenes
- `options.cpp`: Menús de opciones
- `settings.cpp`: Configuraciones
- `properties.cpp`: Propiedades de archivos
- `delete.cpp`: Funciones de eliminación
- `update.cpp`: Actualizaciones

### Core
- `fs.cpp`: Operaciones del sistema de archivos
- `textures.cpp`: Gestión de texturas
- `config.cpp`: Configuración persistente
- `utils.cpp`: Utilidades generales
- `log.cpp`: Sistema de logging
- `net.cpp`: Funciones de red
- `cia.cpp`: Manejo de archivos CIA
- `archive_helper.cpp`: Extracción de archivos
- `c2d_helper.cpp`: Ayudantes para Citro2D
- `osk.cpp`: Teclado en pantalla
- `touch.cpp`: Manejo de entrada táctil

### Bibliotecas Externas
- `libnsbmp`: Decodificación de imágenes BMP
- `libnsgif`: Decodificación de GIFs

## Flujo de Datos
1. Inicialización → Carga de configuración
2. Bucle GUI → Procesamiento de entrada
3. Operaciones FS → Actualización de estado
4. Renderizado → Pantalla 3DS

## Patrones de Diseño
- **Singleton**: Para servicios globales (FS, Config)
- **Helper Classes**: Funciones utilitarias agrupadas
- **State Machine**: En el bucle principal GUI

## Consideraciones de Rendimiento
- Buffers de texto pre-asignados
- Gestión manual de memoria
- Renderizado optimizado para hardware 3DS limitado