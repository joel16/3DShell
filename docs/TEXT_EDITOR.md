# Editor de Texto - 3DShell

## Descripción
El editor de texto integrado permite abrir y editar archivos de texto plano directamente desde 3DShell. Está diseñado como un editor simple similar a Notepad, optimizado para la interfaz de Nintendo 3DS.

## Archivos Soportados
El editor se activa automáticamente al seleccionar archivos con las siguientes extensiones:
- `.txt` - Archivos de texto plano
- `.log` - Archivos de registro
- `.json` - Archivos JSON
- `.xml` - Archivos XML
- `.ini` - Archivos de configuración INI
- `.cfg` - Archivos de configuración
- `.md` - Archivos Markdown
- `.cpp`, `.c` - Código fuente C/C++
- `.h`, `.hpp` - Headers C/C++
- `.py` - Scripts Python
- `.js` - Scripts JavaScript
- `.html` - Páginas HTML
- `.css` - Hojas de estilo CSS

## Interfaz del Editor

### Modo Vista (View)
- **Pantalla superior**: Muestra el contenido del archivo
- **Pantalla inferior**: Muestra los botones de acción (Nuevo archivo, Nueva carpeta, Propiedades, Renombrar)
- **Barra de título**: Nombre del archivo (con "*" si hay cambios sin guardar)
- **Barra de estado**: Posición del cursor (línea y columna) y modo actual
- **Controles**: A:Editar, X:Guardar, B:Volver

### Modo Edición (Edit)
- **Pantalla superior**: Editor de texto con cursor visible
- **Pantalla inferior**: Teclado virtual personalizado con múltiples layouts
- **Cursor visible**: Parpadea en la posición actual
- **Navegación**: D-pad para mover el cursor
- **Controles:** SELECT:Abrir teclado virtual, A:Guardar, B:Cancelar

## Funcionalidades

### Navegación
- **Flechas direccionales**: Mover cursor por línea/columna
- **Scroll automático**: La vista se desplaza cuando el cursor se acerca al borde

### Edición
- **Teclado virtual personalizado**: 
  - Layout QWERTY con letras minúsculas
  - Layout con letras mayúsculas (presionar ⇧)
  - Layout numérico y símbolos (presionar 123)
  - Espacio, retroceso (⌫) y enter (⏎)
- **Inserción**: El texto se inserta en la posición del cursor
- **Borrado**: Retroceso elimina caracteres hacia atrás
- **Nueva línea**: Enter crea una nueva línea

### Archivo
- **Carga automática**: Al abrir un archivo, se carga su contenido
- **Guardado manual**: Presionar X en modo vista o A en modo edición
- **Indicador de cambios**: "*" en el título cuando hay modificaciones sin guardar

## Teclado Virtual

### Layouts Disponibles
1. **QWERTY Minúsculas**: Letras minúsculas estándar
2. **QWERTY Mayúsculas**: Letras mayúsculas (accesible con ⇧)
3. **Numérico/Símbolos**: Números y símbolos comunes (accesible con 123)
4. **Símbolos Extendidos**: Símbolos adicionales (accesible con #+=)

### Controles del Teclado
- **Táctil**: Tocar las teclas para insertar caracteres
- **Retroceso (⌫)**: Elimina el último carácter
- **Espacio**: Inserta un espacio
- **Enter (⏎)**: Confirma la entrada y cierra el teclado
- **⇧**: Cambia a mayúsculas
- **⇩**: Cambia a minúsculas
- **123**: Cambia a layout numérico
- **ABC**: Cambia a layout alfabético

### Características
- Diseño optimizado para pantalla táctil de 3DS
- Feedback visual al presionar teclas
- Cambio automático entre layouts según el contexto
- Inserción directa en la posición del cursor del editor

## Limitaciones
- **Líneas máximas**: 1000 líneas por archivo
- **Longitud de línea**: Limitada a 50 caracteres visibles
- **Codificación**: Solo UTF-8 básico
- **Funciones avanzadas**: Sin búsqueda/reemplazo, deshacer/rehacer

## Arquitectura del Código

### Archivos
- `source/notepad/notepad.h` - Declaraciones del editor
- `source/notepad/notepad.cpp` - Implementación completa del editor
- `source/osk/virtual_keyboard.h` - Declaraciones del teclado virtual
- `source/osk/virtual_keyboard.cpp` - Implementación del teclado virtual
- `include/gui.h` - Estructuras NotepadData y KeyboardData
- `source/gui/gui.cpp` - Integración en el bucle principal
- `source/gui/filebrowser.cpp` - Detección de archivos de texto

### Estados
- `MENU_STATE_NOTEPAD` - Estado principal del editor
- `NOTEPAD_VIEW` - Modo de solo lectura
- `NOTEPAD_EDIT` - Modo de edición

### Estructura de Datos
```cpp
struct NotepadData {
    std::string filename;      // Nombre del archivo
    std::string filepath;      // Ruta completa
    std::vector<std::string> lines;  // Contenido por líneas
    int cursor_line;           // Línea actual del cursor
    int cursor_col;            // Columna actual del cursor
    int scroll_offset;         // Desplazamiento vertical
    NotepadState state;        // Estado actual
    bool modified;             // Indicador de cambios
};
```

## Uso en el Código
Para integrar nuevos tipos de archivos o modificar el comportamiento:

1. **Agregar extensiones**: Modificar `IsTextFile()` en `notepad.cpp`
2. **Cambiar límites**: Ajustar constantes como `MAX_LINES`
3. **Agregar funciones**: Extender `Control()` para nuevos atajos
4. **Modificar UI**: Cambiar `Display()` para nueva apariencia

## Consideraciones de Rendimiento
- **Memoria limitada**: Optimizado para archivos pequeños/medianos
- **Renderizado eficiente**: Solo líneas visibles se dibujan
- **Entrada controlada**: Teclado virtual evita sobrecarga

## Futuras Mejoras
- Soporte para más codificaciones
- Funciones de búsqueda y reemplazo
- Deshacer/rehacer básico
- Resaltado de sintaxis para código
- Soporte para archivos más grandes