#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "treemap.h" 
#include "list.h"    

typedef struct {
    char id[100];
    List *artists; 
    char album_name[300];
    char track_name[100];
    float tempo;
    char track_genre[100];
} musica;

int lower_than_str(void* key1, void* key2) {
    return strcmp((char*)key1, (char*)key2) < 0;
}

int lower_than_float(void* key1, void* key2) {
    return *(float*)key1 < *(float*)key2;
}

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

void limpiar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void normalizar_string(char* str) {
    if (!str) return;
    //printf("[DEBUG] normalizar_string: Entrada='%s'\n", str); fflush(stdout);
    char *inicio = str;
    while (*inicio && (*inicio == ' ' || *inicio == '\t' || *inicio == '\n')) inicio++;
    
    char *final = str + strlen(str) - 1;
    while (final > inicio && (*final == ' ' || *final == '\t' || *final == '\n')) final--;

    if (inicio > final) { // Cadena vacía o solo espacios
        str[0] = '\0';
    } else {
        memmove(str, inicio, final - inicio + 1);
        str[final - inicio + 1] = '\0'; 
    }
    //printf("[DEBUG] normalizar_string: Despues de trim='%s'\n", str); fflush(stdout);
    for (char *p = str; *p; ++p) *p = tolower((unsigned char)*p);
    //printf("[DEBUG] normalizar_string: Finalizado='%s'\n", str); fflush(stdout);
}

List *split_string(char *str, const char *delim) {
    //printf("[DEBUG] split_string: Entrada str='%s', delim='%s'\n", str ? str : "NULL", delim); fflush(stdout);
    if (!str) {
        //printf("[DEBUG] split_string: str es NULL, retornando NULL\n"); fflush(stdout);
        return NULL;
    }
    List *list = createList(); 
    if (!list) {
        //printf("[DEBUG] split_string: createList falló.\n"); fflush(stdout);
        return NULL;
    }
    //printf("[DEBUG] split_string: Lista creada.\n"); fflush(stdout);
    
    char *copy = strdup(str); 
    if (!copy) {
        //printf("[DEBUG] split_string: strdup(str) falló.\n"); fflush(stdout);
        // Deberías liberar 'list' si createList tuvo éxito pero strdup falló
        // cleanList(list); // o la función de destrucción apropiada
        // free(list); 
        return NULL; 
    }
    //printf("[DEBUG] split_string: Cadena duplicada: '%s'\n", copy); fflush(stdout);
    
    char *token = strtok(copy, delim);
    //printf("[DEBUG] split_string: Primer token='%s'\n", token ? token : "NULL"); fflush(stdout);
    
    while (token != NULL) {
        char *artist_token = strdup(token); 
        if (!artist_token) {
            //printf("[DEBUG] split_string: strdup(token) falló para token '%s'.\n", token); fflush(stdout);
            // Manejo de error: liberar 'copy' y lo que se haya puesto en 'list', luego 'list' misma.
            free(copy);
            // cleanList(list); // y liberar los strdup previos dentro de la lista
            // free(list);
            return NULL; // O devolver la lista parcialmente llena si se prefiere
        }
        //printf("[DEBUG] split_string: Token duplicado: '%s'\n", artist_token); fflush(stdout);
        normalizar_string(artist_token);
        //printf("[DEBUG] split_string: Token normalizado: '%s'\n", artist_token); fflush(stdout);
        pushBack(list, artist_token); 
        //printf("[DEBUG] split_string: Token '%s' agregado a la lista.\n", artist_token); fflush(stdout);
        token = strtok(NULL, delim);
        //printf("[DEBUG] split_string: Siguiente token='%s'\n", token ? token : "NULL"); fflush(stdout);
    }
    
    free(copy); 
    //printf("[DEBUG] split_string: Finalizado, retornando lista.\n"); fflush(stdout);
    return list;
}

void imprimir_cancion(musica *cancion) {
    if (!cancion) {
        printf("[DEBUG] imprimir_cancion: Intento de imprimir canción NULL.\n"); fflush(stdout);
        return;
    }
    //printf("[DEBUG] imprimir_cancion: Imprimiendo canción ID '%s'\n", cancion->id); fflush(stdout);
    
    printf("\nID: %s\n", cancion->id);
    printf("Track: %s\n", cancion->track_name);
    printf("Album: %s\n", cancion->album_name);
    printf("Artistas: ");
    
    if (cancion->artists) {
        char *artist = (char *)firstList(cancion->artists); 
        while (artist != NULL) {
            printf("%s", artist);
            artist = (char *)nextList(cancion->artists); 
            if (artist != NULL) printf(", ");
        }
    } else {
        printf("[No especificado]");
    }
    
    printf("\nGenero: %s\n", cancion->track_genre);
    printf("Tempo: %.2f BPM\n", cancion->tempo);
    printf("---------------------------------------\n");
    //printf("[DEBUG] imprimir_cancion: Fin de impresión para ID '%s'\n", cancion->id); fflush(stdout);
}

void cargar_canciones(TreeMap *canciones_id, TreeMap *canciones_genero, 
                      TreeMap *canciones_artistas, TreeMap *tempo_lentas, 
                      TreeMap *tempo_moderadas, TreeMap *tempo_rapidas) {
    printf("[DEBUG] cargar_canciones: Iniciando carga de canciones.\n"); fflush(stdout);

    char ruta[256];
    printf("Ingrese la ruta del archivo CSV: ");
    if (fgets(ruta, sizeof(ruta), stdin) == NULL) {
        printf("[ERROR] cargar_canciones: Error al leer la ruta con fgets.\n"); fflush(stdout);
        return;
    }
    ruta[strcspn(ruta, "\n")] = '\0'; 
    printf("[DEBUG] cargar_canciones: Ruta ingresada: '%s'\n", ruta); fflush(stdout);

    FILE *archivo = fopen(ruta, "r");
    if (!archivo) {
        perror("[ERROR] cargar_canciones: Error al abrir el archivo"); 
        printf("[DEBUG] cargar_canciones: fopen falló para la ruta: %s\n", ruta); fflush(stdout);
        return;
    }
    printf("[DEBUG] cargar_canciones: Archivo '%s' abierto correctamente.\n", ruta); fflush(stdout);

    char linea[1024];
    printf("[DEBUG] cargar_canciones: Intentando leer cabecera...\n"); fflush(stdout);
    if (!fgets(linea, sizeof(linea), archivo)) { 
        printf("[ERROR] cargar_canciones: Archivo vacío o inválido (no se pudo leer la cabecera).\n"); fflush(stdout);
        fclose(archivo);
        return;
    }
    printf("[DEBUG] cargar_canciones: Cabecera leída (y descartada): %s", linea); fflush(stdout); // linea ya tiene \n

    int contador_lineas_leidas = 0;
    int contador_exitoso = 0;
    printf("[DEBUG] cargar_canciones: Iniciando bucle de lectura de líneas de datos.\n"); fflush(stdout);
    while (fgets(linea, sizeof(linea), archivo)) {
        contador_lineas_leidas++;
        //printf("[DEBUG] cargar_canciones: Línea %d leída: %s", contador_lineas_leidas, linea); fflush(stdout);

        char *linea_copia = strdup(linea); 
        if (!linea_copia) {
            printf("[ERROR] cargar_canciones: strdup(linea) falló para línea %d.\n", contador_lineas_leidas); fflush(stdout);
            continue; 
        }
        //printf("[DEBUG] cargar_canciones: Línea %d duplicada para strtok.\n", contador_lineas_leidas); fflush(stdout);

        char *tokens[21] = {NULL}; 
        int campo = 0;
        char *token = strtok(linea_copia, ",");
        //printf("[DEBUG] cargar_canciones: Línea %d, primer token: '%s'\n", contador_lineas_leidas, token ? token : "NULL"); fflush(stdout);

        while (token && campo < 21) {
            tokens[campo++] = token;
            token = strtok(NULL, ",");
            //if (campo < 21) printf("[DEBUG] cargar_canciones: Línea %d, token %d: '%s'\n", contador_lineas_leidas, campo, tokens[campo-1] ? tokens[campo-1] : "NULL"); fflush(stdout);
        }
        //printf("[DEBUG] cargar_canciones: Línea %d, total campos tokenizados: %d\n", contador_lineas_leidas, campo); fflush(stdout);


        if (campo < 21) { 
            printf("[WARN] cargar_canciones: Línea %d con menos de 21 campos (%d) ignorada: %s\n", contador_lineas_leidas, campo, linea); fflush(stdout);
            free(linea_copia);
            continue;
        }

        //printf("[DEBUG] cargar_canciones: Línea %d, creando struct musica...\n", contador_lineas_leidas); fflush(stdout);
        musica *cancion_actual = (musica*)malloc(sizeof(musica));
        if (!cancion_actual) {
            printf("[ERROR] cargar_canciones: malloc para musica falló en línea %d.\n", contador_lineas_leidas); fflush(stdout);
            free(linea_copia);
            continue;
        }
        memset(cancion_actual, 0, sizeof(musica)); 
        //printf("[DEBUG] cargar_canciones: Línea %d, struct musica creada y vaciada.\n", contador_lineas_leidas); fflush(stdout);

        //printf("[DEBUG] cargar_canciones: Línea %d, asignando ID: '%s'\n", contador_lineas_leidas, tokens[0]); fflush(stdout);
        strncpy(cancion_actual->id, tokens[0], sizeof(cancion_actual->id)-1);
        
        //printf("[DEBUG] cargar_canciones: Línea %d, procesando artistas: '%s'\n", contador_lineas_leidas, tokens[2]); fflush(stdout);
        cancion_actual->artists = split_string(tokens[2], ";"); 
        if (!cancion_actual->artists) {
            printf("[ERROR] cargar_canciones: split_string para artistas falló en línea %d (artistas: '%s'). Saltando canción.\n", contador_lineas_leidas, tokens[2]); fflush(stdout);
            free(cancion_actual);
            free(linea_copia);
            continue;
        }
        //printf("[DEBUG] cargar_canciones: Línea %d, artistas procesados.\n", contador_lineas_leidas); fflush(stdout);

        //printf("[DEBUG] cargar_canciones: Línea %d, asignando album: '%s'\n", contador_lineas_leidas, tokens[3]); fflush(stdout);
        strncpy(cancion_actual->album_name, tokens[3], sizeof(cancion_actual->album_name)-1);
        //printf("[DEBUG] cargar_canciones: Línea %d, asignando track: '%s'\n", contador_lineas_leidas, tokens[4]); fflush(stdout);
        strncpy(cancion_actual->track_name, tokens[4], sizeof(cancion_actual->track_name)-1);
        
        if (tokens[18]) {
            //printf("[DEBUG] cargar_canciones: Línea %d, asignando tempo: '%s'\n", contador_lineas_leidas, tokens[18]); fflush(stdout);
            cancion_actual->tempo = atof(tokens[18]);
        } else {
            //printf("[WARN] cargar_canciones: Línea %d, tempo es NULL. Usando 0.0.\n", contador_lineas_leidas); fflush(stdout);
            cancion_actual->tempo = 0.0f; 
        }
        
        //printf("[DEBUG] cargar_canciones: Línea %d, asignando genero: '%s'\n", contador_lineas_leidas, tokens[20]); fflush(stdout);
        strncpy(cancion_actual->track_genre, tokens[20], sizeof(cancion_actual->track_genre)-1);

        //printf("[DEBUG] cargar_canciones: Línea %d, normalizando ID y género...\n", contador_lineas_leidas); fflush(stdout);
        normalizar_string(cancion_actual->id);
        normalizar_string(cancion_actual->track_genre);
        //printf("[DEBUG] cargar_canciones: Línea %d, ID normalizado: '%s', Género normalizado: '%s'\n", contador_lineas_leidas, cancion_actual->id, cancion_actual->track_genre); fflush(stdout);

        //printf("[DEBUG] cargar_canciones: Línea %d, duplicando ID para clave de TreeMap...\n", contador_lineas_leidas); fflush(stdout);
        char *id_key = strdup(cancion_actual->id);
        if (!id_key) { 
            printf("[ERROR] cargar_canciones: strdup para id_key falló en línea %d.\n", contador_lineas_leidas); fflush(stdout);
            // Liberar cancion_actual->artists y sus elementos, luego cancion_actual
            if (cancion_actual->artists) {
                char* art_to_free = (char*)firstList(cancion_actual->artists);
                while(art_to_free){ free(art_to_free); art_to_free = (char*)nextList(cancion_actual->artists); } // Ojo, nextList podría necesitar resetear el current. Mejor usar pop.
                cleanList(cancion_actual->artists); free(cancion_actual->artists);
            }
            free(cancion_actual); 
            free(linea_copia); 
            continue; 
        }
        //printf("[DEBUG] cargar_canciones: Línea %d, insertando en canciones_id con clave '%s'.\n", contador_lineas_leidas, id_key); fflush(stdout);
        insertTreeMap(canciones_id, id_key, cancion_actual);

        //printf("[DEBUG] cargar_canciones: Línea %d, buscando/insertando en canciones_genero para género '%s'.\n", contador_lineas_leidas, cancion_actual->track_genre); fflush(stdout);
        Pair *par_genero_existente = searchTreeMap(canciones_genero, cancion_actual->track_genre);
        List *lista_canciones_genero;
        if (par_genero_existente) {
            lista_canciones_genero = (List *)par_genero_existente->value; 
            //printf("[DEBUG] cargar_canciones: Línea %d, género '%s' encontrado, usando lista existente.\n", contador_lineas_leidas, cancion_actual->track_genre); fflush(stdout);
        } else {
            //printf("[DEBUG] cargar_canciones: Línea %d, género '%s' NO encontrado, creando nueva lista.\n", contador_lineas_leidas, cancion_actual->track_genre); fflush(stdout);
            lista_canciones_genero = createList(); 
            if (!lista_canciones_genero) { 
                printf("[ERROR] cargar_canciones: createList para lista_canciones_genero falló en línea %d.\n", contador_lineas_leidas); fflush(stdout);
                free(id_key); 
                // Liberar cancion_actual->artists y sus elementos, luego cancion_actual
                if (cancion_actual->artists) { /* ... (código de liberación de artistas) ... */ cleanList(cancion_actual->artists); free(cancion_actual->artists); }
                free(cancion_actual); 
                free(linea_copia); 
                continue; 
            }
            char *genre_key = strdup(cancion_actual->track_genre);
            if (!genre_key) { 
                printf("[ERROR] cargar_canciones: strdup para genre_key falló en línea %d.\n", contador_lineas_leidas); fflush(stdout);
                free(id_key); 
                if (cancion_actual->artists) { /* ... (código de liberación de artistas) ... */ cleanList(cancion_actual->artists); free(cancion_actual->artists); }
                free(cancion_actual); 
                cleanList(lista_canciones_genero); free(lista_canciones_genero);
                free(linea_copia); 
                continue; 
            }
            //printf("[DEBUG] cargar_canciones: Línea %d, insertando nuevo género '%s' en canciones_genero.\n", contador_lineas_leidas, genre_key); fflush(stdout);
            insertTreeMap(canciones_genero, genre_key, lista_canciones_genero);
        }
        //printf("[DEBUG] cargar_canciones: Línea %d, agregando canción a lista de género '%s'.\n", contador_lineas_leidas, cancion_actual->track_genre); fflush(stdout);
        pushBack(lista_canciones_genero, cancion_actual); 

        //printf("[DEBUG] cargar_canciones: Línea %d, procesando inserción por artista.\n", contador_lineas_leidas); fflush(stdout);
        char *artista_iter = (char *)firstList(cancion_actual->artists); 
        while (artista_iter != NULL) {
            //printf("[DEBUG] cargar_canciones: Línea %d, artista actual para TreeMap: '%s'.\n", contador_lineas_leidas, artista_iter); fflush(stdout);
            Pair *par_artista_existente = searchTreeMap(canciones_artistas, artista_iter);
            List *lista_canciones_artista;
            if (par_artista_existente) {
                lista_canciones_artista = (List *)par_artista_existente->value; 
                //printf("[DEBUG] cargar_canciones: Línea %d, artista '%s' encontrado, usando lista existente.\n", contador_lineas_leidas, artista_iter); fflush(stdout);
            } else {
                //printf("[DEBUG] cargar_canciones: Línea %d, artista '%s' NO encontrado, creando nueva lista.\n", contador_lineas_leidas, artista_iter); fflush(stdout);
                lista_canciones_artista = createList(); 
                if (!lista_canciones_artista) { 
                    printf("[ERROR] cargar_canciones: createList para lista_canciones_artista ('%s') falló en línea %d.\n", artista_iter, contador_lineas_leidas); fflush(stdout);
                    /* Podríamos continuar con el siguiente artista o abortar la canción */
                    artista_iter = (char *)nextList(cancion_actual->artists);
                    continue;
                }
                char *artist_key = strdup(artista_iter); 
                if (!artist_key) { 
                    printf("[ERROR] cargar_canciones: strdup para artist_key ('%s') falló en línea %d.\n", artista_iter, contador_lineas_leidas); fflush(stdout);
                    cleanList(lista_canciones_artista); free(lista_canciones_artista);
                    artista_iter = (char *)nextList(cancion_actual->artists);
                    continue;
                }
                //printf("[DEBUG] cargar_canciones: Línea %d, insertando nuevo artista '%s' en canciones_artistas.\n", contador_lineas_leidas, artist_key); fflush(stdout);
                insertTreeMap(canciones_artistas, artist_key, lista_canciones_artista);
            }
            //printf("[DEBUG] cargar_canciones: Línea %d, agregando canción a lista de artista '%s'.\n", contador_lineas_leidas, artista_iter); fflush(stdout);
            pushBack(lista_canciones_artista, cancion_actual);
            artista_iter = (char *)nextList(cancion_actual->artists); 
        }
        
        //printf("[DEBUG] cargar_canciones: Línea %d, determinando TreeMap de tempo para tempo %.2f.\n", contador_lineas_leidas, cancion_actual->tempo); fflush(stdout);
        TreeMap *mapa_tempo_destino = NULL;
        if (cancion_actual->tempo < 80.0f) mapa_tempo_destino = tempo_lentas;
        else if (cancion_actual->tempo <= 120.0f) mapa_tempo_destino = tempo_moderadas;
        else mapa_tempo_destino = tempo_rapidas;

        //printf("[DEBUG] cargar_canciones: Línea %d, creando clave float para tempo...\n", contador_lineas_leidas); fflush(stdout);
        float *tempo_map_key = (float*)malloc(sizeof(float));
        if (!tempo_map_key) { 
            printf("[ERROR] cargar_canciones: malloc para tempo_map_key falló en línea %d.\n", contador_lineas_leidas); fflush(stdout);
            free(linea_copia); 
            /* Liberar todo lo asociado a cancion_actual antes de 'continue' */
            continue; 
        }
        *tempo_map_key = cancion_actual->tempo; 
        
        //printf("[DEBUG] cargar_canciones: Línea %d, insertando en TreeMap de tempo (clave: %.2f).\n", contador_lineas_leidas, *tempo_map_key); fflush(stdout);
        insertTreeMap(mapa_tempo_destino, tempo_map_key, cancion_actual);

        free(linea_copia); 
        contador_exitoso++;
        //printf("[DEBUG] cargar_canciones: Línea %d procesada exitosamente. Total exitoso: %d\n", contador_lineas_leidas, contador_exitoso); fflush(stdout);
    }

    printf("[DEBUG] cargar_canciones: Fin del bucle de lectura de líneas. Total líneas leídas del archivo (sin cabecera): %d\n", contador_lineas_leidas); fflush(stdout);
    fclose(archivo);
    printf("[DEBUG] cargar_canciones: Archivo cerrado.\n"); fflush(stdout);
    printf("Canciones cargadas correctamente. Total: %d\n", contador_exitoso);
    printf("[DEBUG] cargar_canciones: Finalizado.\n"); fflush(stdout);
}

void buscar_por_id(TreeMap *canciones_id) {
    printf("[DEBUG] buscar_por_id: Iniciando.\n"); fflush(stdout);
    char id_busqueda[100];
    printf("Ingresa el ID de la cancion: ");
    if (fgets(id_busqueda, sizeof(id_busqueda), stdin) == NULL) {
        printf("[ERROR] buscar_por_id: Error al leer el ID.\n"); fflush(stdout);
        return;
    }
    id_busqueda[strcspn(id_busqueda, "\n")] = '\0';
    printf("[DEBUG] buscar_por_id: ID leído: '%s'\n", id_busqueda); fflush(stdout);
    normalizar_string(id_busqueda);
    printf("[DEBUG] buscar_por_id: ID normalizado: '%s'\n", id_busqueda); fflush(stdout);

    printf("[DEBUG] buscar_por_id: Buscando en TreeMap...\n"); fflush(stdout);
    Pair *par_encontrado = searchTreeMap(canciones_id, id_busqueda);
    
    if (!par_encontrado) {
        printf("[DEBUG] buscar_por_id: No se encontró el par para el ID '%s'.\n", id_busqueda); fflush(stdout);
        printf("No se encontro ninguna cancion con el ID '%s'\n", id_busqueda);
        return;
    }
    printf("[DEBUG] buscar_por_id: Par encontrado para ID '%s'. Key del par: '%s'\n", id_busqueda, (char*)par_encontrado->key); fflush(stdout);

    musica *cancion = (musica*)par_encontrado->value; 
    if (!cancion) {
        printf("[ERROR] buscar_por_id: Par encontrado, pero el valor (canción) es NULL para ID '%s'.\n", id_busqueda); fflush(stdout);
        return;
    }
    printf("[DEBUG] buscar_por_id: Canción obtenida del par. Imprimiendo...\n"); fflush(stdout);
    imprimir_cancion(cancion);
    printf("[DEBUG] buscar_por_id: Finalizado.\n"); fflush(stdout);
}

void buscar_por_genero(TreeMap *canciones_genero) {
    printf("[DEBUG] buscar_por_genero: Iniciando.\n"); fflush(stdout);
    char genero_buscado[100];
    printf("Ingrese el genero de la cancion: ");
    if (fgets(genero_buscado, sizeof(genero_buscado), stdin) == NULL) {
        printf("[ERROR] buscar_por_genero: Error al leer el género.\n"); fflush(stdout);
        return;
    }
    genero_buscado[strcspn(genero_buscado, "\n")] = '\0';
    normalizar_string(genero_buscado);
    printf("[DEBUG] buscar_por_genero: Género normalizado: '%s'\n", genero_buscado); fflush(stdout);

    Pair *par_genero = searchTreeMap(canciones_genero, genero_buscado);
    if (!par_genero) {
        printf("\nNo se encontraron canciones del genero '%s'\n", genero_buscado);
        printf("[DEBUG] buscar_por_genero: No se encontró el par para el género '%s'.\n", genero_buscado); fflush(stdout);
        
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
    printf("[DEBUG] buscar_por_genero: Par encontrado para género '%s'.\n", genero_buscado); fflush(stdout);

    List *lista_de_canciones = (List *)par_genero->value; 
    if (!lista_de_canciones) {
        printf("[ERROR] buscar_por_genero: Par de género encontrado, pero la lista de canciones es NULL para '%s'.\n", genero_buscado); fflush(stdout);
        return;
    }
    printf("[DEBUG] buscar_por_genero: Lista de canciones obtenida para '%s'.\n", genero_buscado); fflush(stdout);

    musica *cancion_actual_lista = (musica *)firstList(lista_de_canciones); 
    if (!cancion_actual_lista) {
        printf("\nEl genero '%s' no contiene canciones (lista vacía).\n", genero_buscado);
        printf("[DEBUG] buscar_por_genero: La lista para el género '%s' está vacía (firstList es NULL).\n", genero_buscado); fflush(stdout);
        return;
    }
    printf("[DEBUG] buscar_por_genero: Primera canción de la lista obtenida. Iniciando iteración...\n"); fflush(stdout);

    printf("\n--- Canciones del genero '%s' ---\n", genero_buscado);
    int contador_canciones = 0;
    while (cancion_actual_lista) {
        contador_canciones++;
        //printf("[DEBUG] buscar_por_genero: Imprimiendo canción %d de la lista.\n", contador_canciones); fflush(stdout);
        imprimir_cancion(cancion_actual_lista);
        cancion_actual_lista = (musica *)nextList(lista_de_canciones); 
    }
    printf("\nTotal: %d canciones\n", contador_canciones);
    printf("[DEBUG] buscar_por_genero: Finalizado. Total canciones impresas: %d\n", contador_canciones); fflush(stdout);
}

void buscar_por_artista(TreeMap *canciones_artistas) {
    // Similar a buscar_por_genero, puedes agregar printf aquí
    printf("[DEBUG] buscar_por_artista: Iniciando.\n"); fflush(stdout);
    char artista_buscado[100];
    printf("Ingrese el nombre del artista: ");
    if (fgets(artista_buscado, sizeof(artista_buscado), stdin) == NULL) { /* ... */ return; }
    artista_buscado[strcspn(artista_buscado, "\n")] = '\0';
    normalizar_string(artista_buscado);
    printf("[DEBUG] buscar_por_artista: Artista normalizado: '%s'\n", artista_buscado); fflush(stdout);

    Pair *par_artista = searchTreeMap(canciones_artistas, artista_buscado);
    if (!par_artista) {
        printf("\nNo se encontraron canciones del artista '%s'\n", artista_buscado);
        printf("[DEBUG] buscar_por_artista: No se encontró el par para el artista '%s'.\n", artista_buscado); fflush(stdout);
        return;
    }
     printf("[DEBUG] buscar_por_artista: Par encontrado para artista '%s'.\n", artista_buscado); fflush(stdout);


    List *lista_de_canciones = (List *)par_artista->value;
    if (!lista_de_canciones) {
        printf("[ERROR] buscar_por_artista: Par de artista encontrado, pero la lista de canciones es NULL para '%s'.\n", artista_buscado); fflush(stdout);
        return;
    }
    printf("[DEBUG] buscar_por_artista: Lista de canciones obtenida para '%s'.\n", artista_buscado); fflush(stdout);

    musica *cancion_actual_lista = (musica *)firstList(lista_de_canciones); 
    if (!cancion_actual_lista) {
        printf("\nEl artista '%s' no tiene canciones registradas (lista vacía).\n", artista_buscado);
        printf("[DEBUG] buscar_por_artista: La lista para el artista '%s' está vacía.\n", artista_buscado); fflush(stdout);
        return;
    }
    printf("[DEBUG] buscar_por_artista: Primera canción de la lista obtenida. Iniciando iteración...\n"); fflush(stdout);

    printf("\n--- Canciones del artista '%s' ---\n", artista_buscado);
    int contador_canciones = 0;
    while (cancion_actual_lista) {
        contador_canciones++;
        imprimir_cancion(cancion_actual_lista);
        cancion_actual_lista = (musica *)nextList(lista_de_canciones); 
    }
    printf("\nTotal: %d canciones\n", contador_canciones);
    printf("[DEBUG] buscar_por_artista: Finalizado. Total canciones impresas: %d\n", contador_canciones); fflush(stdout);
}

void buscar_por_tempo(TreeMap *lentas, TreeMap *moderadas, TreeMap *rapidas) {
    // Agregar printf aquí si es necesario
    printf("[DEBUG] buscar_por_tempo: Iniciando.\n"); fflush(stdout);
    int opcion_tempo;
    printf("\nSeleccione el rango de tempo:\n");
    printf("1) Lentas (menos de 80 BPM)\n");
    printf("2) Moderadas (80-120 BPM)\n");
    printf("3) Rapidas (mas de 120 BPM)\n");
    printf("Opcion: ");
    
    if (scanf("%d", &opcion_tempo) != 1) { /* ... */ limpiar_buffer(); return; }
    limpiar_buffer(); 
    printf("[DEBUG] buscar_por_tempo: Opción de tempo seleccionada: %d\n", opcion_tempo); fflush(stdout);
    
    TreeMap *mapa_seleccionado = NULL;
    char *rango_descripcion = "";
    
    switch (opcion_tempo) {
        case 1: mapa_seleccionado = lentas; rango_descripcion = "Lentas (menos de 80 BPM)"; break;
        case 2: mapa_seleccionado = moderadas; rango_descripcion = "Moderadas (80-120 BPM)"; break;
        case 3: mapa_seleccionado = rapidas; rango_descripcion = "Rápidas (más de 120 BPM)"; break;
        default: printf("Opcion no valida.\n"); return;
    }
    printf("[DEBUG] buscar_por_tempo: Mapa seleccionado. Buscando primer par...\n"); fflush(stdout);

    Pair *par_mapa_tempo = firstTreeMap(mapa_seleccionado);
    if (!par_mapa_tempo) {
        printf("No hay canciones en este rango de tempo.\n");
        printf("[DEBUG] buscar_por_tempo: No hay canciones en el mapa de tempo seleccionado.\n"); fflush(stdout);
        return;
    }
    printf("[DEBUG] buscar_por_tempo: Primer par encontrado en mapa de tempo. Iterando...\n"); fflush(stdout);

    printf("\n--- Canciones %s ---\n", rango_descripcion);
    int contador_canciones = 0;
    while (par_mapa_tempo) {
        contador_canciones++;
        musica *cancion = (musica *)par_mapa_tempo->value; 
        imprimir_cancion(cancion);
        par_mapa_tempo = nextTreeMap(mapa_seleccionado);
    }
    printf("\nTotal: %d canciones\n", contador_canciones);
    printf("[DEBUG] buscar_por_tempo: Finalizado. Total: %d\n", contador_canciones); fflush(stdout);
}

void agregar_favoritos(TreeMap *favoritos, TreeMap *canciones_id) {
    // Agregar printf aquí si es necesario
    printf("[DEBUG] agregar_favoritos: Iniciando.\n"); fflush(stdout);
    char id_cancion_fav[100], categoria_fav_nombre[100];

    printf("Ingresa el ID de la cancion a agregar a favoritos: ");
    if (fgets(id_cancion_fav, sizeof(id_cancion_fav), stdin) == NULL) { /* ... */ return; }
    id_cancion_fav[strcspn(id_cancion_fav, "\n")] = '\0';
    normalizar_string(id_cancion_fav);
    printf("[DEBUG] agregar_favoritos: ID canción a agregar: '%s'\n", id_cancion_fav); fflush(stdout);


    printf("Ingresa la categoria de favoritos (ej: 'Para entrenar', 'Relajantes'): ");
    if (fgets(categoria_fav_nombre, sizeof(categoria_fav_nombre), stdin) == NULL) { /* ... */ return; }
    categoria_fav_nombre[strcspn(categoria_fav_nombre, "\n")] = '\0';
    normalizar_string(categoria_fav_nombre);
    printf("[DEBUG] agregar_favoritos: Categoría: '%s'\n", categoria_fav_nombre); fflush(stdout);


    Pair *par_cancion_db = searchTreeMap(canciones_id, id_cancion_fav);
    if (!par_cancion_db) {
        printf("No se encontro ninguna cancion con el ID '%s' en la base de datos principal.\n", id_cancion_fav);
        printf("[DEBUG] agregar_favoritos: Canción con ID '%s' no encontrada en DB principal.\n", id_cancion_fav); fflush(stdout);
        return;
    }
    musica *cancion_original_db = (musica *)par_cancion_db->value; 
    printf("[DEBUG] agregar_favoritos: Canción original '%s' encontrada en DB.\n", cancion_original_db->track_name); fflush(stdout);


    Pair *par_categoria_existente_fav = searchTreeMap(favoritos, categoria_fav_nombre);
    List *lista_fav_en_categoria;

    if (par_categoria_existente_fav) {
        lista_fav_en_categoria = (List *)par_categoria_existente_fav->value; 
        printf("[DEBUG] agregar_favoritos: Categoría '%s' existente en favoritos.\n", categoria_fav_nombre); fflush(stdout);
    } else {
        printf("[DEBUG] agregar_favoritos: Categoría '%s' NO existente. Creando nueva lista...\n", categoria_fav_nombre); fflush(stdout);
        lista_fav_en_categoria = createList(); 
        if (!lista_fav_en_categoria) { /* ... */ return; }
        char *categoria_key_fav = strdup(categoria_fav_nombre);
        if (!categoria_key_fav) { /* ... */ return; }
        insertTreeMap(favoritos, categoria_key_fav, lista_fav_en_categoria);
        printf("[DEBUG] agregar_favoritos: Nueva categoría '%s' insertada en TreeMap de favoritos.\n", categoria_fav_nombre); fflush(stdout);
    }

    printf("[DEBUG] agregar_favoritos: Creando copia de la canción para favoritos...\n"); fflush(stdout);
    musica *copia_para_favoritos = (musica *)malloc(sizeof(musica));
    if (!copia_para_favoritos) { /* ... */ return; }
    
    memcpy(copia_para_favoritos, cancion_original_db, sizeof(musica)); 

    copia_para_favoritos->artists = createList(); 
    if (!copia_para_favoritos->artists) { /* ... */ free(copia_para_favoritos); return; }
    
    printf("[DEBUG] agregar_favoritos: Copiando lista de artistas para la canción favorita...\n"); fflush(stdout);
    char *artista_original_iter = (char *)firstList(cancion_original_db->artists); 
    while (artista_original_iter) {
        char *copia_artista_fav = strdup(artista_original_iter);
        if (!copia_artista_fav) { /* ... */ /* liberar memoria */ return; }
        pushBack(copia_para_favoritos->artists, copia_artista_fav);
        artista_original_iter = (char *)nextList(cancion_original_db->artists); 
    }
    printf("[DEBUG] agregar_favoritos: Lista de artistas copiada. Agregando canción a lista de favoritos de la categoría.\n"); fflush(stdout);
    
    pushBack(lista_fav_en_categoria, copia_para_favoritos); 
    printf("Cancion '%s' agregada a favoritos en la categoria: '%s'\n", copia_para_favoritos->track_name, categoria_fav_nombre);
    printf("[DEBUG] agregar_favoritos: Finalizado.\n"); fflush(stdout);
}

void mostrar_favoritos(TreeMap *favoritos) {
    // Agregar printf aquí si es necesario
    printf("[DEBUG] mostrar_favoritos: Iniciando.\n"); fflush(stdout);
    if (!favoritos || !firstTreeMap(favoritos)) { 
        printf("No hay canciones en favoritos.\n");
        printf("[DEBUG] mostrar_favoritos: No hay TreeMap de favoritos o está vacío.\n"); fflush(stdout);
        return;
    }

    Pair *par_categoria_iter = firstTreeMap(favoritos);
    printf("\n--- MIS FAVORITOS ---\n");
    while (par_categoria_iter) {
        printf("\nCategoria: %s\n", (char*)par_categoria_iter->key); 
        printf("--------------------------\n");
        printf("[DEBUG] mostrar_favoritos: Mostrando categoría '%s'.\n", (char*)par_categoria_iter->key); fflush(stdout);
        
        List *lista_canciones_fav_categoria = (List *)par_categoria_iter->value; 
        musica *cancion_fav_iter = (musica *)firstList(lista_canciones_fav_categoria); 
        
        if (!cancion_fav_iter) {
            printf("Esta categoria no tiene canciones.\n");
            printf("[DEBUG] mostrar_favoritos: Categoría '%s' no tiene canciones (lista vacía).\n", (char*)par_categoria_iter->key); fflush(stdout);
        } else {
            int contador_fav = 0;
            while (cancion_fav_iter) {
                contador_fav++;
                imprimir_cancion(cancion_fav_iter); 
                cancion_fav_iter = (musica *)nextList(lista_canciones_fav_categoria); 
            }
            printf("\nTotal en esta categoria: %d canciones\n", contador_fav);
        }
        par_categoria_iter = nextTreeMap(favoritos);
    }
    printf("[DEBUG] mostrar_favoritos: Finalizado.\n"); fflush(stdout);
}

int main() {
    printf("[DEBUG] main: Iniciando programa.\n"); fflush(stdout);

    printf("[DEBUG] main: Creando TreeMaps...\n"); fflush(stdout);
    TreeMap *canciones_id = createTreeMap(lower_than_str);
    TreeMap *canciones_genero = createTreeMap(lower_than_str);
    TreeMap *canciones_artistas = createTreeMap(lower_than_str);
    TreeMap *tempo_lentas = createTreeMap(lower_than_float); 
    TreeMap *tempo_moderadas = createTreeMap(lower_than_float);
    TreeMap *tempo_rapidas = createTreeMap(lower_than_float);
    TreeMap *favoritos = createTreeMap(lower_than_str); 

    if (!canciones_id || !canciones_genero || !canciones_artistas || !tempo_lentas || !tempo_moderadas || !tempo_rapidas || !favoritos) {
        printf("[CRITICAL ERROR] main: No se pudo crear uno o más TreeMaps. Saliendo.\n"); fflush(stdout);
        // Liberar los que sí se crearon
        return 1;
    }
    printf("[DEBUG] main: TreeMaps creados exitosamente.\n"); fflush(stdout);

    int opcion_menu;
    do {
        mostrarMenu();
        printf("\nIngrese la opcion (1-8): ");
        if (scanf("%d", &opcion_menu) != 1) {
            printf("Entrada inválida. Por favor, ingrese un número.\n"); fflush(stdout);
            limpiar_buffer(); 
            opcion_menu = 0; 
            continue;
        }
        limpiar_buffer(); 
        printf("[DEBUG] main: Opción de menú seleccionada: %d\n", opcion_menu); fflush(stdout);

        switch(opcion_menu) {
            case 1:
                printf("[DEBUG] main: Llamando a cargar_canciones...\n"); fflush(stdout);
                cargar_canciones(canciones_id, canciones_genero, canciones_artistas,
                                 tempo_lentas, tempo_moderadas, tempo_rapidas);
                printf("[DEBUG] main: Retorno de cargar_canciones.\n"); fflush(stdout);
                break;
            case 2:
                printf("[DEBUG] main: Llamando a buscar_por_id...\n"); fflush(stdout);
                buscar_por_id(canciones_id);
                printf("[DEBUG] main: Retorno de buscar_por_id.\n"); fflush(stdout);
                break;
            case 3:
                printf("[DEBUG] main: Llamando a buscar_por_genero...\n"); fflush(stdout);
                buscar_por_genero(canciones_genero);
                printf("[DEBUG] main: Retorno de buscar_por_genero.\n"); fflush(stdout);
                break;
            case 4:
                printf("[DEBUG] main: Llamando a buscar_por_artista...\n"); fflush(stdout);
                buscar_por_artista(canciones_artistas);
                printf("[DEBUG] main: Retorno de buscar_por_artista.\n"); fflush(stdout);
                break;
            case 5:
                printf("[DEBUG] main: Llamando a buscar_por_tempo...\n"); fflush(stdout);
                buscar_por_tempo(tempo_lentas, tempo_moderadas, tempo_rapidas);
                printf("[DEBUG] main: Retorno de buscar_por_tempo.\n"); fflush(stdout);
                break;
            case 6:
                printf("[DEBUG] main: Llamando a agregar_favoritos...\n"); fflush(stdout);
                agregar_favoritos(favoritos, canciones_id);
                printf("[DEBUG] main: Retorno de agregar_favoritos.\n"); fflush(stdout);
                break;
            case 7:
                printf("[DEBUG] main: Llamando a mostrar_favoritos...\n"); fflush(stdout);
                mostrar_favoritos(favoritos);
                printf("[DEBUG] main: Retorno de mostrar_favoritos.\n"); fflush(stdout);
                break;
            case 8:
                printf("Saliendo del sistema...\n"); fflush(stdout);
                break;
            default:
                printf("Opción no válida. Intente de nuevo.\n"); fflush(stdout);
        }

        if (opcion_menu != 8) {
            printf("\nPresione Enter para continuar..."); fflush(stdout);
            getchar(); 
        }
    } while(opcion_menu != 8);

    printf("[DEBUG] main: Programa finalizando. Aquí se debería liberar memoria.\n"); fflush(stdout);
    // IMPORTANTE: Implementar la liberación de toda la memoria dinámica.
    // ... (código de liberación de memoria) ...

    printf("Programa terminado.\n"); fflush(stdout);
    return 0;
}