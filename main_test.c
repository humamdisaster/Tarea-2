#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "treemap.h" // Asegúrate que treemap.h y treemap.c estén correctos y se enlacen
#include "list.h"    // Asegúrate que list.h y list.c estén correctos y se enlacen

typedef struct {
    char id[100];
    List *artists; // Lista de cadenas (char*) para los artistas
    char album_name[300];
    char track_name[100];
    float tempo;
    char track_genre[100];
} musica;

// Función de comparación para claves de tipo cadena (char*) en el TreeMap
int lower_than_str(void* key1, void* key2) {
    return strcmp((char*)key1, (char*)key2) < 0;
}

// Función de comparación para claves de tipo float en el TreeMap
int lower_than_float(void* key1, void* key2) {
    return *(float*)key1 < *(float*)key2;
}

// Muestra el menú principal de opciones
void mostrarMenu() {
    printf("\n========================================\n");
    printf("      Base de Datos de Canciones\n");
    printf("========================================\n");
    printf("1) Cargar Canciones\n");
    printf("2) Buscar por ID\n");
    printf("3) Buscar por genero\n");
    printf("4) Buscar por artista\n");
    printf("5) Buscar por tempo\n");
    printf("6) Agregar a favoritos\n");
    printf("7) Mostrar favoritos\n");
    printf("8) Salir\n");
    printf("========================================\n");
}

// Limpia el buffer de entrada (stdin)
void limpiar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Normaliza una cadena: elimina espacios al inicio/final y convierte a minúsculas
void normalizar_string(char* str) {
    if (!str) return;
    
    char *inicio = str;
    while (*inicio == ' ' || *inicio == '\t' || *inicio == '\n') inicio++;
    
    char *final = str + strlen(str) - 1;
    while (final > inicio && (*final == ' ' || *final == '\t' || *final == '\n')) final--;

    memmove(str, inicio, final - inicio + 1);
    str[final - inicio + 1] = '\0'; 

    for (char *p = str; *p; ++p) *p = tolower((unsigned char)*p);
}

// Divide una cadena en subcadenas usando un delimitador y las almacena en una lista
List *split_string(char *str, const char *delim) {
    // USA LOS NOMBRES DE list.h
    List *list = createList(); 
    if (!list) return NULL;
    
    char *copy = strdup(str); 
    if (!copy) {
        // cleanList(list); // Si tuvieras una función para destruir la lista y sus nodos pero no los datos
        // free(list); // Si createList hace malloc para List y cleanList no lo libera
        return NULL; 
    }
    char *token = strtok(copy, delim);
    
    while (token != NULL) {
        char *artist_token = strdup(token); 
        if (!artist_token) {
            // Manejar error de memoria
            // Por ejemplo, limpiar los artistas ya agregados a la lista y la lista misma
            // cleanList(list); free(list); // o una función de destrucción más completa
            free(copy);
            return list; // Devolver lista parcialmente llena o NULL
        }
        normalizar_string(artist_token);
        // USA LOS NOMBRES DE list.h
        pushBack(list, artist_token); 
        token = strtok(NULL, delim);
    }
    
    free(copy); 
    return list;
}

// Imprime la información de una canción
void imprimir_cancion(musica *cancion) {
    if (!cancion) return;
    
    printf("\nID: %s\n", cancion->id);
    printf("Track: %s\n", cancion->track_name);
    printf("Album: %s\n", cancion->album_name);
    printf("Artistas: ");
    
    // USA LOS NOMBRES DE list.h
    // El cast a (char *) es correcto si firstList y nextList devuelven void*
    char *artist = (char *)firstList(cancion->artists); 
    while (artist != NULL) {
        printf("%s", artist);
        // USA LOS NOMBRES DE list.h
        artist = (char *)nextList(cancion->artists); 
        if (artist != NULL) printf(", ");
    }
    
    printf("\nGenero: %s\n", cancion->track_genre);
    printf("Tempo: %.2f BPM\n", cancion->tempo);
    printf("---------------------------------------\n");
}

// Carga canciones desde un archivo CSV a los TreeMaps
void cargar_canciones(TreeMap *canciones_id, TreeMap *canciones_genero, 
                      TreeMap *canciones_artistas, TreeMap *tempo_lentas, 
                      TreeMap *tempo_moderadas, TreeMap *tempo_rapidas) {
    char ruta[256];
    printf("Ingrese la ruta del archivo CSV: ");
    if (fgets(ruta, sizeof(ruta), stdin) == NULL) {
        printf("Error al leer la ruta.\n");
        return;
    }
    ruta[strcspn(ruta, "\n")] = '\0'; 

    FILE *archivo = fopen(ruta, "r");
    if (!archivo) {
        perror("Error al abrir el archivo"); 
        printf("Ruta intentada: %s\n", ruta);
        return;
    }

    char linea[1024];
    if (!fgets(linea, sizeof(linea), archivo)) { // Omitir cabecera
        printf("Archivo vacío o inválido (no se pudo leer la cabecera).\n");
        fclose(archivo);
        return;
    }

    int contador_exitoso = 0;
    while (fgets(linea, sizeof(linea), archivo)) {
        char *tokens[21] = {NULL}; 
        int campo = 0;
        char *linea_copia = strdup(linea); 
        if (!linea_copia) {
            printf("Error de memoria al duplicar línea.\n");
            continue; 
        }
        char *token = strtok(linea_copia, ",");

        while (token && campo < 21) {
            tokens[campo++] = token;
            token = strtok(NULL, ",");
        }

        if (campo < 21) { 
            //printf("Advertencia: Línea con menos de 21 campos ignorada: %s\n", linea);
            free(linea_copia);
            continue;
        }

        musica *cancion_actual = (musica*)malloc(sizeof(musica));
        if (!cancion_actual) {
            printf("Error de memoria al crear estructura cancion\n");
            free(linea_copia);
            continue;
        }
        memset(cancion_actual, 0, sizeof(musica)); 

        strncpy(cancion_actual->id, tokens[0], sizeof(cancion_actual->id)-1);
        cancion_actual->artists = split_string(tokens[2], ";"); 
        if (!cancion_actual->artists) {
            printf("Error de memoria al procesar artistas para ID %s. Saltando canción.\n", cancion_actual->id);
            free(cancion_actual);
            free(linea_copia);
            continue;
        }
        strncpy(cancion_actual->album_name, tokens[3], sizeof(cancion_actual->album_name)-1);
        strncpy(cancion_actual->track_name, tokens[4], sizeof(cancion_actual->track_name)-1);
        
        if (tokens[18]) {
            cancion_actual->tempo = atof(tokens[18]);
        } else {
            cancion_actual->tempo = 0.0f; 
        }
        
        strncpy(cancion_actual->track_genre, tokens[20], sizeof(cancion_actual->track_genre)-1);

        normalizar_string(cancion_actual->id);
        normalizar_string(cancion_actual->track_genre);

        char *id_key = strdup(cancion_actual->id);
        if (!id_key) { /* manejo error */ free(cancion_actual->artists); /*limpiar lista artistas*/ free(cancion_actual); free(linea_copia); continue; }
        insertTreeMap(canciones_id, id_key, cancion_actual);

        Pair *par_genero_existente = searchTreeMap(canciones_genero, cancion_actual->track_genre);
        List *lista_canciones_genero;
        if (par_genero_existente) {
            lista_canciones_genero = (List *)par_genero_existente->value; 
        } else {
            // USA LOS NOMBRES DE list.h
            lista_canciones_genero = createList(); 
            if (!lista_canciones_genero) { /* manejo error */ free(id_key); free(cancion_actual->artists); /*limpiar*/ free(cancion_actual); free(linea_copia); continue; }
            char *genre_key = strdup(cancion_actual->track_genre);
            if (!genre_key) { /* manejo error */ free(id_key); free(cancion_actual->artists); /*limpiar*/ free(cancion_actual); free(linea_copia); /*cleanList+free lista_canciones_genero*/ continue; }
            insertTreeMap(canciones_genero, genre_key, lista_canciones_genero);
        }
        // USA LOS NOMBRES DE list.h
        pushBack(lista_canciones_genero, cancion_actual); 

        // USA LOS NOMBRES DE list.h
        char *artista_iter = (char *)firstList(cancion_actual->artists); 
        while (artista_iter != NULL) {
            Pair *par_artista_existente = searchTreeMap(canciones_artistas, artista_iter);
            List *lista_canciones_artista;
            if (par_artista_existente) {
                lista_canciones_artista = (List *)par_artista_existente->value; 
            } else {
                // USA LOS NOMBRES DE list.h
                lista_canciones_artista = createList(); 
                if (!lista_canciones_artista) { /* manejo error */ continue; } // Simplificado, idealmente liberar más
                char *artist_key = strdup(artista_iter); // La clave es el nombre del artista
                if (!artist_key) { /* manejo error */ /*cleanList+free lista_canciones_artista*/ continue; }
                insertTreeMap(canciones_artistas, artist_key, lista_canciones_artista);
            }
            // USA LOS NOMBRES DE list.h
            pushBack(lista_canciones_artista, cancion_actual);
            // USA LOS NOMBRES DE list.h
            artista_iter = (char *)nextList(cancion_actual->artists); 
        }
        
        TreeMap *mapa_tempo_destino = NULL;
        if (cancion_actual->tempo < 80.0f) mapa_tempo_destino = tempo_lentas;
        else if (cancion_actual->tempo <= 120.0f) mapa_tempo_destino = tempo_moderadas;
        else mapa_tempo_destino = tempo_rapidas;

        float *tempo_map_key = (float*)malloc(sizeof(float));
        if (!tempo_map_key) { /* manejo error */ continue; }
        *tempo_map_key = cancion_actual->tempo; 
        
        // Considerar si múltiples canciones pueden tener el mismo tempo exacto.
        // Si es así, el valor en el TreeMap de tempo debería ser una Lista de canciones.
        // Esta implementación simple asume que insertTreeMap maneja colisiones o que son infrecuentes.
        // O, si la clave ya existe, se podría agregar a una lista existente como con géneros/artistas.
        // Por ahora, se inserta directamente la canción.
        insertTreeMap(mapa_tempo_destino, tempo_map_key, cancion_actual);

        free(linea_copia); 
        contador_exitoso++;
    }

    fclose(archivo);
    printf("Canciones cargadas correctamente. Total: %d\n", contador_exitoso);
}

// Busca una canción por su ID e imprime su información
void buscar_por_id(TreeMap *canciones_id) {
    char id_busqueda[100];
    printf("Ingresa el ID de la cancion: ");
    if (fgets(id_busqueda, sizeof(id_busqueda), stdin) == NULL) {
        printf("Error al leer el ID.\n");
        return;
    }
    id_busqueda[strcspn(id_busqueda, "\n")] = '\0';
    normalizar_string(id_busqueda);

    Pair *par_encontrado = searchTreeMap(canciones_id, id_busqueda);
    if (!par_encontrado) {
        printf("No se encontro ninguna cancion con el ID '%s'\n", id_busqueda);
        return;
    }

    musica *cancion = (musica*)par_encontrado->value; 
    imprimir_cancion(cancion);
}

// Busca canciones por género e imprime la lista de canciones encontradas
void buscar_por_genero(TreeMap *canciones_genero) {
    char genero_buscado[100];
    printf("Ingrese el genero de la cancion: ");
    if (fgets(genero_buscado, sizeof(genero_buscado), stdin) == NULL) {
        printf("Error al leer el género.\n");
        return;
    }
    genero_buscado[strcspn(genero_buscado, "\n")] = '\0';
    normalizar_string(genero_buscado);

    Pair *par_genero = searchTreeMap(canciones_genero, genero_buscado);
    if (!par_genero) {
        printf("\nNo se encontraron canciones del genero '%s'\n", genero_buscado);
        
        printf("\nGeneros disponibles (hasta 10):\n");
        Pair *genero_actual_mapa = firstTreeMap(canciones_genero);
        int count = 0;
        while (genero_actual_mapa && count < 10) {
            printf("- %s\n", (char*)genero_actual_mapa->key); 
            genero_actual_mapa = nextTreeMap(canciones_genero);
            count++;
        }
        return;
    }

    List *lista_de_canciones = (List *)par_genero->value; 
    // USA LOS NOMBRES DE list.h
    musica *cancion_actual_lista = (musica *)firstList(lista_de_canciones); 
    if (!cancion_actual_lista) {
        printf("\nEl genero '%s' no contiene canciones (lista vacía).\n", genero_buscado);
        return;
    }

    printf("\n--- Canciones del genero '%s' ---\n", genero_buscado);
    int contador_canciones = 0;
    while (cancion_actual_lista) {
        printf("\n%d.", ++contador_canciones);
        imprimir_cancion(cancion_actual_lista);
        // USA LOS NOMBRES DE list.h
        cancion_actual_lista = (musica *)nextList(lista_de_canciones); 
    }
    printf("\nTotal: %d canciones\n", contador_canciones);
}

// Busca canciones por artista e imprime la lista de canciones encontradas
void buscar_por_artista(TreeMap *canciones_artistas) {
    char artista_buscado[100];
    printf("Ingrese el nombre del artista: ");
    if (fgets(artista_buscado, sizeof(artista_buscado), stdin) == NULL) {
        printf("Error al leer el artista.\n");
        return;
    }
    artista_buscado[strcspn(artista_buscado, "\n")] = '\0';
    normalizar_string(artista_buscado);

    Pair *par_artista = searchTreeMap(canciones_artistas, artista_buscado);
    if (!par_artista) {
        printf("\nNo se encontraron canciones del artista '%s'\n", artista_buscado);
        return;
    }

    List *lista_de_canciones = (List *)par_artista->value; 
    // USA LOS NOMBRES DE list.h
    musica *cancion_actual_lista = (musica *)firstList(lista_de_canciones); 
    if (!cancion_actual_lista) {
        printf("\nEl artista '%s' no tiene canciones registradas (lista vacía).\n", artista_buscado);
        return;
    }

    printf("\n--- Canciones del artista '%s' ---\n", artista_buscado);
    int contador_canciones = 0;
    while (cancion_actual_lista) {
        printf("\n%d.", ++contador_canciones);
        imprimir_cancion(cancion_actual_lista);
        // USA LOS NOMBRES DE list.h
        cancion_actual_lista = (musica *)nextList(lista_de_canciones); 
    }
    printf("\nTotal: %d canciones\n", contador_canciones);
}

// Busca canciones por rango de tempo e imprime las encontradas
void buscar_por_tempo(TreeMap *lentas, TreeMap *moderadas, TreeMap *rapidas) {
    int opcion_tempo;
    printf("\nSeleccione el rango de tempo:\n");
    printf("1) Lentas (menos de 80 BPM)\n");
    printf("2) Moderadas (80-120 BPM)\n");
    printf("3) Rapidas (mas de 120 BPM)\n");
    printf("Opcion: ");
    
    if (scanf("%d", &opcion_tempo) != 1) {
        printf("Entrada no válida.\n");
        limpiar_buffer();
        return;
    }
    limpiar_buffer(); 
    
    TreeMap *mapa_seleccionado = NULL;
    char *rango_descripcion = "";
    
    switch (opcion_tempo) {
        case 1: mapa_seleccionado = lentas; rango_descripcion = "Lentas (menos de 80 BPM)"; break;
        case 2: mapa_seleccionado = moderadas; rango_descripcion = "Moderadas (80-120 BPM)"; break;
        case 3: mapa_seleccionado = rapidas; rango_descripcion = "Rápidas (más de 120 BPM)"; break;
        default: printf("Opcion no valida.\n"); return;
    }

    Pair *par_mapa_tempo = firstTreeMap(mapa_seleccionado);
    if (!par_mapa_tempo) {
        printf("No hay canciones en este rango de tempo.\n");
        return;
    }

    printf("\n--- Canciones %s ---\n", rango_descripcion);
    int contador_canciones = 0;
    while (par_mapa_tempo) {
        musica *cancion = (musica *)par_mapa_tempo->value; 
        printf("\n%d.", ++contador_canciones);
        imprimir_cancion(cancion);
        par_mapa_tempo = nextTreeMap(mapa_seleccionado);
    }
    printf("\nTotal: %d canciones\n", contador_canciones);
}

// Agrega una canción a una categoría de favoritos
void agregar_favoritos(TreeMap *favoritos, TreeMap *canciones_id) {
    char id_cancion_fav[100], categoria_fav_nombre[100];

    printf("Ingresa el ID de la cancion a agregar a favoritos: ");
    if (fgets(id_cancion_fav, sizeof(id_cancion_fav), stdin) == NULL) { printf("Error al leer el ID.\n"); return; }
    id_cancion_fav[strcspn(id_cancion_fav, "\n")] = '\0';
    normalizar_string(id_cancion_fav);

    printf("Ingresa la categoria de favoritos (ej: 'Para entrenar', 'Relajantes'): ");
    if (fgets(categoria_fav_nombre, sizeof(categoria_fav_nombre), stdin) == NULL) { printf("Error al leer la categoria.\n"); return; }
    categoria_fav_nombre[strcspn(categoria_fav_nombre, "\n")] = '\0';
    normalizar_string(categoria_fav_nombre);

    Pair *par_cancion_db = searchTreeMap(canciones_id, id_cancion_fav);
    if (!par_cancion_db) {
        printf("No se encontro ninguna cancion con el ID '%s' en la base de datos principal.\n", id_cancion_fav);
        return;
    }
    musica *cancion_original_db = (musica *)par_cancion_db->value; 

    Pair *par_categoria_existente_fav = searchTreeMap(favoritos, categoria_fav_nombre);
    List *lista_fav_en_categoria;

    if (par_categoria_existente_fav) {
        lista_fav_en_categoria = (List *)par_categoria_existente_fav->value; 
    } else {
        // USA LOS NOMBRES DE list.h
        lista_fav_en_categoria = createList(); 
        if (!lista_fav_en_categoria) { printf("Error de memoria creando lista de favoritos.\n"); return; }
        char *categoria_key_fav = strdup(categoria_fav_nombre);
        if (!categoria_key_fav) { /* error memoria */ /*cleanList+free lista_fav_en_categoria*/ return; }
        insertTreeMap(favoritos, categoria_key_fav, lista_fav_en_categoria);
    }

    musica *copia_para_favoritos = (musica *)malloc(sizeof(musica));
    if (!copia_para_favoritos) { printf("Error de memoria al copiar cancion para favoritos.\n"); return; }
    
    memcpy(copia_para_favoritos, cancion_original_db, sizeof(musica)); 

    // USA LOS NOMBRES DE list.h
    copia_para_favoritos->artists = createList(); 
    if (!copia_para_favoritos->artists) { 
        printf("Error de memoria creando lista de artistas para copia en favoritos.\n"); 
        free(copia_para_favoritos); 
        return; 
    }
    
    // USA LOS NOMBRES DE list.h
    char *artista_original_iter = (char *)firstList(cancion_original_db->artists); 
    while (artista_original_iter) {
        char *copia_artista_fav = strdup(artista_original_iter);
        if (!copia_artista_fav) { /* error memoria */ free(copia_para_favoritos); /*cleanList+free copia_para_favoritos->artists*/ return; }
        // USA LOS NOMBRES DE list.h
        pushBack(copia_para_favoritos->artists, copia_artista_fav);
        // USA LOS NOMBRES DE list.h
        artista_original_iter = (char *)nextList(cancion_original_db->artists); 
    }
    
    // USA LOS NOMBRES DE list.h
    pushBack(lista_fav_en_categoria, copia_para_favoritos); 
    printf("Cancion '%s' agregada a favoritos en la categoria: '%s'\n", copia_para_favoritos->track_name, categoria_fav_nombre);
}

// Muestra todas las canciones en las categorías de favoritos
void mostrar_favoritos(TreeMap *favoritos) {
    if (!favoritos || !firstTreeMap(favoritos)) { 
        printf("No hay canciones en favoritos.\n");
        return;
    }

    Pair *par_categoria_iter = firstTreeMap(favoritos);
    printf("\n--- MIS FAVORITOS ---\n");
    while (par_categoria_iter) {
        printf("\nCategoria: %s\n", (char*)par_categoria_iter->key); 
        printf("--------------------------\n");
        
        List *lista_canciones_fav_categoria = (List *)par_categoria_iter->value; 
        // USA LOS NOMBRES DE list.h
        musica *cancion_fav_iter = (musica *)firstList(lista_canciones_fav_categoria); 
        
        if (!cancion_fav_iter) {
            printf("Esta categoria no tiene canciones.\n");
        } else {
            int contador_fav = 0;
            while (cancion_fav_iter) {
                printf("\n%d.", ++contador_fav);
                imprimir_cancion(cancion_fav_iter); 
                // USA LOS NOMBRES DE list.h
                cancion_fav_iter = (musica *)nextList(lista_canciones_fav_categoria); 
            }
            printf("\nTotal en esta categoria: %d canciones\n", contador_fav);
        }
        par_categoria_iter = nextTreeMap(favoritos);
    }
}

// Función principal
int main() {
    TreeMap *canciones_id = createTreeMap(lower_than_str);
    TreeMap *canciones_genero = createTreeMap(lower_than_str);
    TreeMap *canciones_artistas = createTreeMap(lower_than_str);
    TreeMap *tempo_lentas = createTreeMap(lower_than_float); 
    TreeMap *tempo_moderadas = createTreeMap(lower_than_float);
    TreeMap *tempo_rapidas = createTreeMap(lower_than_float);
    TreeMap *favoritos = createTreeMap(lower_than_str); 

    if (!canciones_id || !canciones_genero || !canciones_artistas || !tempo_lentas || !tempo_moderadas || !tempo_rapidas || !favoritos) {
        printf("Error crítico: No se pudo crear uno o más TreeMaps. Saliendo.\n");
        // Aquí deberías liberar los TreeMaps que sí se crearon antes de salir.
        // if(canciones_id) eraseTreeMap(canciones_id, NULL); // O una función destroyTreeMap si existe
        return 1;
    }

    int opcion_menu;
    do {
        mostrarMenu();
        printf("\nIngrese la opcion (1-8): ");
        if (scanf("%d", &opcion_menu) != 1) {
            printf("Entrada inválida. Por favor, ingrese un número.\n");
            limpiar_buffer(); 
            opcion_menu = 0; 
            continue;
        }
        limpiar_buffer(); 

        switch(opcion_menu) {
            case 1:
                cargar_canciones(canciones_id, canciones_genero, canciones_artistas,
                                 tempo_lentas, tempo_moderadas, tempo_rapidas);
                break;
            case 2:
                buscar_por_id(canciones_id);
                break;
            case 3:
                buscar_por_genero(canciones_genero);
                break;
            case 4:
                buscar_por_artista(canciones_artistas);
                break;
            case 5:
                buscar_por_tempo(tempo_lentas, tempo_moderadas, tempo_rapidas);
                break;
            case 6:
                agregar_favoritos(favoritos, canciones_id);
                break;
            case 7:
                mostrar_favoritos(favoritos);
                break;
            case 8:
                printf("Saliendo del sistema...\n");
                break;
            default:
                printf("Opción no válida. Intente de nuevo.\n");
        }

        if (opcion_menu != 8) {
            printf("\nPresione Enter para continuar...");
            getchar(); 
        }
    } while(opcion_menu != 8);

    // IMPORTANTE: Liberar toda la memoria dinámica.
    // Esto es complejo y requiere funciones para destruir TreeMaps y Listas,
    // incluyendo todos los datos que almacenan (structs musica, strings duplicados, etc.).
    // Por ejemplo:
    // 1. Iterar sobre canciones_id: para cada canción, liberar su lista de artistas (strings y la lista misma), luego liberar la struct musica. Liberar la clave del TreeMap (id_key). Finalmente, destruir el TreeMap canciones_id.
    // 2. Iterar sobre canciones_genero: para cada género, liberar la List* (pero NO las canciones, ya que son punteros a las mismas de canciones_id). Liberar la clave del TreeMap (genre_key). Destruir el TreeMap.
    // 3. Similar para canciones_artistas.
    // 4. Iterar sobre los TreeMaps de tempo: liberar las claves float* y los punteros a musica (NO las canciones en sí si son las mismas que en canciones_id). Destruir los TreeMaps.
    // 5. Iterar sobre favoritos: para cada categoría, liberar las copias de las canciones (incluyendo sus listas de artistas copiadas) y las listas. Liberar las claves de categoría. Destruir el TreeMap.
    // Esta es una tarea no trivial y depende de cómo estén implementadas tus funciones de destrucción en list.c y treemap.c.

    printf("Programa terminado.\n");
    return 0;
}

