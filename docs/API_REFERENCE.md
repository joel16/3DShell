# Referencia de Funciones - 3DShell

## Espacio de Nombres Services
- `Init()`: Inicializa todos los servicios del sistema
- `Exit()`: Libera recursos y cierra servicios

## Módulo FS (fs.h/cpp)
- `OpenArchive()`: Abre un archivo de sistema
- `CloseArchive()`: Cierra un archivo
- `GetDirContents()`: Lista contenidos de directorio
- `CreateDir()`: Crea directorio
- `RenameFile()`: Renombra archivo/directorio
- `DeleteFile()`: Elimina archivo/directorio
- `CopyFile()`: Copia archivo
- `MoveFile()`: Mueve archivo

## Módulo GUI (gui.h/cpp)
- `Loop()`: Bucle principal de la interfaz
- `RenderTop()`: Renderiza pantalla superior
- `RenderBottom()`: Renderiza pantalla inferior
- `HandleInput()`: Procesa entrada del usuario

## Módulo Textures (textures.h/cpp)
- `Init()`: Inicializa sistema de texturas
- `Exit()`: Libera texturas
- `LoadImageFile()`: Carga imagen desde archivo
- `FreeImage()`: Libera memoria de imagen

## Módulo Config (config.h/cpp)
- `Load()`: Carga configuración desde archivo
- `Save()`: Guarda configuración
- `Get()`: Obtiene valor de configuración
- `Set()`: Establece valor de configuración

## Módulo Utils (utils.h/cpp)
- `FormatBytes()`: Formatea tamaño en bytes
- `GetFileExtension()`: Obtiene extensión de archivo
- `IsImage()`: Verifica si es archivo de imagen
- `GetTimeString()`: Obtiene cadena de tiempo

## Módulo Log (log.h/cpp)
- `Open()`: Abre archivo de log
- `Write()`: Escribe en log
- `Error()`: Registra error
- `Close()`: Cierra log

## Módulo Archive Helper (archive_helper.h/cpp)
- `ExtractArchive()`: Extrae archivo comprimido
- `GetArchiveType()`: Determina tipo de archivo

## Módulo C2D Helper (c2d_helper.h/cpp)
Nota: Este módulo está implementado sobre Citro2D (referido como c2D) y expone funciones de ayuda para renderizado 2D.
- `DrawRect()`: Dibuja rectángulo
- `DrawText()`: Dibuja texto
- `LoadFont()`: Carga fuente

## Tipos de Datos Principales
- `FS_Archive`: Estructura de archivo del sistema
Render targets are managed internally; the public API exposes C2D helpers and opaque types.
- `C2D_TextBuf`: Buffer de texto
- `Texture`: Estructura de textura personalizada