# Base de Datos de Canciones
# Hecho por Hans Gonzalez y Anyara Guajardo - Paralelo 2

Este programa en permite gestionar una base de datos de canciones a partir de un archivo CSV. Utiliza estructuras como `TreeMap` y `List` para organizar canciones por ID, género, artista y tempo. Además, permite agregar canciones favoritas a categorías personalizadas.

## Funcionalidades Implementadas

1. **Cargar canciones** desde un archivo CSV.
2. **Buscar canción por ID** y mostrar su información completa.
3. **Buscar canciones por género**, listando todas las canciones del género ingresado.
4. **Buscar canciones por artista**, listando todas sus canciones disponibles.
5. **Buscar canciones por tempo**, clasificadas en:
   - Lentas (menos de 80 BPM)
   - Moderadas (80 a 120 BPM)
   - Rápidas (más de 120 BPM)
6. **Agregar canciones a una lista de favoritos**, categorizadas por nombre.
7. **Mostrar canciones favoritas**
8. **Salir del programa**.

## Funcionalidades con errores conocidos

## Requisitos del sistema
- Archivos fuente del proyecto:
  - `main.c`
  - `treemap.c`, `treemap.h`
  - `list.c`, `list.h`
- Archivo CSV song_dataset_.csv

## Cómo ejecutar el programa

1. Compila el programa con:
   ```bash
   gcc -o canciones main.c treemap.c list.c
2. Ejecutar el programa con ./canciones


# Ejemplo de uso
========================================
     Base de Datos de Canciones
========================================
1) Cargar Canciones
2) Buscar por ID
3) Buscar por genero
4) Buscar por artista
5) Buscar por tempo
6) Agregar a favoritos
7) Mostrar favoritos
8) Salir
========================================

Ingrese una opción (1-8): 1
Ingrese la ruta del archivo CSV: song_dataset_.csv
Canciones cargadas correctamente. Total: 114001

Ingrese la opcion (1-8): 2
Ingresa el ID de la cancion: 71922

--- Informacion de la cancion ---
ID: 71922
Artista(s): linkin park
Album: Hybrid Theory (20th Anniversary Edition)
Nombre de la cancion: In the End
Genero: metal
Tempo: 105.19 BPM


Presione Enter para continuar...


** Problemas conocidos
- Liberación de memoria: Ocasionalmente ocurre un segmentation fault al salir del programa.


## Contribuciones

### Hans Gonzalez
- **Agregó estructuras de datos (TDAs):** Implementó y depuró archivos como `list.c`, que inicialmente presentaban errores críticos de funcionamiento.
- **Función `mostrarMenu`:** Desarrolló el menú principal que permite al usuario interactuar con el programa de forma ordenada.
- **Función `cargar_canciones`:** Encargado de leer el archivo CSV y poblar las estructuras de datos con la información de cada canción.
- **Primera versión de la función `main`:** Implementó una versión inicial de la función principal, incompleta debido a la falta de otras funciones en ese momento.
- **Función de búsqueda por ID:** Permite encontrar una canción específica utilizando su identificador único.
- **Primera versión de la búsqueda por género:** Inició la función de búsqueda por género, aunque presentó fallas debido a errores en la implementación de los TDAs.
- **Revisión y depuración del código:** Añadió mensajes de depuración (`printf`) para identificar y resolver errores lógicos y de flujo.
- **Añadió comentarios explicativos a la versión final del programa**

### Anyara Guajardo
- **Corrección de `list.c`:** Solucionó errores clave en el archivo de listas, permitiendo que el resto del programa funcionara correctamente.
- **Última versión de la función `main`:** Integró todas las funcionalidades en una versión final y completamente funcional del flujo principal del programa.
- **Última versión de la búsqueda por género:** Depuró y finalizó correctamente la búsqueda de canciones por género.
- **Función de búsqueda por artista:** Añadió funcionalidad para listar todas las canciones correspondientes a un artista específico.
- **Función de búsqueda por tempo:** Clasificó canciones como lentas, moderadas o rápidas según el valor de `tempo`.
- **Testeo del programa completo:** Verificó el correcto funcionamiento de todas las funcionalidades implementadas, asegurando que no existan errores en tiempo de ejecución.
- **Función `liberar_memoria`:** Añadió la función para liberar memoria antes de salir del programa.
- **Añadió comentarios explicativos a la versión final del programa**
- **Creó el archivo README.md:** Creó y redactó el README.md
