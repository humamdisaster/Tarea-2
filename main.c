#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "treemap.h"
#include "list.h"

typedef struct 
{
    char id[100];
    char artists[100];
    char album_name[300];
    char track_name[100];
    float tempo;
    char track_genre[100];
} musica;

int lower_than_str(void* key1, void* key2){
    return strcmp((char*)key1, (char*)key2) < 0;
}

int lower_than_float(void* key1, void* key2){
    return *(float*)key1 < *(float*)key2;
}

void mostrarMenu() {
    printf("\n========================================\n");
    printf("     Base de Datos de Canciones\n");
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
    while (getchar() != '\n');
}

void normalizar_string(char* str) {
    // Eliminar espacios al inicio y final
    char *inicio = str;
    while (*inicio == ' ' || *inicio == '\t' || *inicio == '\n') inicio++;
    
    char *final = str + strlen(str) - 1;
    while (final > inicio && (*final == ' ' || *final == '\t' || *final == '\n')) final--;

    memmove(str, inicio, final - inicio + 1);
    str[final - inicio + 1] = '\0';

    // Convertir a minúsculas
    for (char *p = str; *p; ++p) *p = tolower(*p);
}

void cargar_canciones(TreeMap *canciones_id, TreeMap *canciones_genero, 
    TreeMap *canciones_artistas, TreeMap *tempo_lentas, 
    TreeMap *tempo_moderadas, TreeMap *tempo_rapidas) {
    char ruta[256];
    printf("Ingrese la ruta del archivo CSV: ");
    scanf(" %255[^\n]", ruta);
    limpiar_buffer();

    FILE *archivo = fopen(ruta, "r");
    if (!archivo) {
        printf("Error al abrir el archivo %s\n", ruta);
        return;
    }

    char linea[1024];
    fgets(linea, sizeof(linea), archivo); // Leer cabecera

    int contador = 0;
    while (fgets(linea, sizeof(linea), archivo)) {
        char *tokens[21];
        int campo = 0;
        char *token = strtok(linea, ",");

        while (token != NULL && campo < 21) {
            tokens[campo++] = token;
            token = strtok(NULL, ",");
        }

        musica *cancion = malloc(sizeof(musica));
        if (!cancion) {
            printf("Error asignando memoria\n");
            continue;
        }
        memset(cancion, 0, sizeof(musica));

        // Asignar campos
        if (campo > 0) strncpy(cancion->id, tokens[0], sizeof(cancion->id)-1);
        if (campo > 2) strncpy(cancion->artists, tokens[2], sizeof(cancion->artists)-1);
        if (campo > 3) strncpy(cancion->album_name, tokens[3], sizeof(cancion->album_name)-1);
        if (campo > 4) strncpy(cancion->track_name, tokens[4], sizeof(cancion->track_name)-1);
        if (campo > 18) cancion->tempo = atof(tokens[18]);
        if (campo > 20) strncpy(cancion->track_genre, tokens[20], sizeof(cancion->track_genre)-1);

        // Normalizar strings
        normalizar_string(cancion->id);
        normalizar_string(cancion->artists);
        normalizar_string(cancion->track_genre);

        // Insertar en mapa por ID
        insertTreeMap(canciones_id, strdup(cancion->id), cancion);

        // Insertar en mapa por género
        Pair *par_genero = searchTreeMap(canciones_genero, cancion->track_genre);
        List *lista_genero = par_genero ? par_genero->value : createList();
        if (!par_genero) {
            insertTreeMap(canciones_genero, strdup(cancion->track_genre), lista_genero);
        }
        pushBack(lista_genero, cancion);

        // Insertar en mapa por artista
        Pair *par_artista = searchTreeMap(canciones_artistas, cancion->artists);
        List *lista_artista = par_artista ? par_artista->value : createList();
        if (!par_artista) {
            insertTreeMap(canciones_artistas, strdup(cancion->artists), lista_artista);
        }
        pushBack(lista_artista, cancion);

        // Insertar en mapa por tempo
        TreeMap *destino = NULL;
        if (cancion->tempo < 80) destino = tempo_lentas;
        else if (cancion->tempo < 120) destino = tempo_moderadas;
        else destino = tempo_rapidas;

        float *tempo_key = malloc(sizeof(float));
        *tempo_key = cancion->tempo;
        insertTreeMap(destino, tempo_key, cancion);

        contador++;
    }

    fclose(archivo);
    printf("Canciones cargadas correctamente. Total: %d\n", contador);
}

void buscar_por_id(TreeMap *canciones_id) {
    char id_busqueda[100];
    printf("Ingresa el ID de la cancion: ");
    scanf(" %99[^\n]", id_busqueda);
    limpiar_buffer();

    normalizar_string(id_busqueda);

    Pair *par = searchTreeMap(canciones_id, id_busqueda);

    if (par == NULL) {
        printf("No se encontro ninguna cancion con el ID '%s'.\n", id_busqueda);
        return;
    }

    musica *cancion = par->value;

    printf("\n--- Informacion de la cancion ---\n");
    printf("ID: %s\n", cancion->id);
    printf("Artista(s): %s\n", cancion->artists);
    printf("Album: %s\n", cancion->album_name);
    printf("Nombre de la cancion: %s\n", cancion->track_name);
    printf("Genero: %s\n", cancion->track_genre);
    printf("Tempo: %.2f BPM\n\n", cancion->tempo);
}

void buscar_por_genero(TreeMap *canciones_genero) {
    if (canciones_genero == NULL) {
        printf("Error: El mapa de generos no esta inicializado.\n");
        return;
    }

    char genero_buscado[100];
    printf("Ingrese el genero de la cancion: ");
    scanf(" %99[^\n]", genero_buscado);
    limpiar_buffer();

    normalizar_string(genero_buscado);

    Pair *par = searchTreeMap(canciones_genero, genero_buscado);
    
    if (par == NULL) {
        printf("\nNo se encontraron canciones del genero '%s'\n", genero_buscado);
        
        // Mostrar generos disponibles
        printf("\nGeneros disponibles:\n");
        Pair *genero_actual = firstTreeMap(canciones_genero);
        int count = 0;
        while (genero_actual != NULL && count < 10) {
            printf("- %s\n", (char*)genero_actual->key);
            genero_actual = nextTreeMap(canciones_genero);
            count++;
        }
        return;
    }

    List *lista = (List *)par->value;
    musica *cancion = firstList(lista);
    
    if (cancion == NULL) {
        printf("\nEl genero '%s' no contiene canciones.\n", genero_buscado);
        return;
    }

    printf("\n--- Canciones del genero '%s' ---\n", genero_buscado);
    int contador = 0;
    
    while (cancion != NULL) {
        printf("\n%d. %s - %s\n", ++contador, cancion->artists, cancion->track_name);
        printf("   Album: %s\n", cancion->album_name);
        printf("   ID: %s | Tempo: %.2f BPM\n", cancion->id, cancion->tempo);
        printf("   -------------------------");
        
        cancion = nextList(lista);
    }
    printf("\nTotal: %d canciones\n", contador);
}

void buscar_por_artista(TreeMap *canciones_artistas) {
    if (canciones_artistas == NULL) {
        printf("Error: Mapa de artistas no inicializado\n");
        return;
    }

    char artista_buscado[100];
    printf("Ingrese el nombre del artista: ");
    scanf(" %99[^\n]", artista_buscado);
    limpiar_buffer();

    normalizar_string(artista_buscado);

    Pair *par = searchTreeMap(canciones_artistas, artista_buscado);
    
    if (par == NULL) {
        printf("\nNo se encontraron canciones del artista '%s'\n", artista_buscado);
        return;
    }

    List *lista = (List *)par->value;
    musica *cancion = firstList(lista);
    
    if (cancion == NULL) {
        printf("\nEl artista '%s' no tiene canciones registradas.\n", artista_buscado);
        return;
    }

    printf("\n--- Canciones del artista '%s' ---\n", artista_buscado);
    int contador = 0;
    
    while (cancion != NULL) {
        printf("\n%d. %s\n", ++contador, cancion->track_name);
        printf("   Album: %s\n", cancion->album_name);
        printf("   Genero: %s\n", cancion->track_genre);
        printf("   Tempo: %.2f BPM\n", cancion->tempo);
        printf("   ID: %s\n", cancion->id);
        printf("   -------------------------");
        
        cancion = nextList(lista);
    }
    printf("\nTotal: %d canciones\n", contador);
}

void buscar_por_tempo(TreeMap *lentas, TreeMap *moderadas, TreeMap *rapidas) {
    int opcion;
    
    printf("\nSeleccione el rango de tempo:\n");
    printf("1) Lentas (menos de 80 BPM)\n");
    printf("2) Moderadas (80-120 BPM)\n");
    printf("3) Rapidas (mas de 120 BPM)\n");
    printf("Opcion: ");
    
    if (scanf("%d", &opcion) != 1) {
        printf("Entrada no válida.\n");
        limpiar_buffer();
        return;
    }
    limpiar_buffer();
    
    TreeMap *mapa = NULL;
    char *rango = "";
    
    if (opcion == 1) {
        mapa = lentas;
        rango = "Lentas (menos de 80 BPM)";
    } else if (opcion == 2) {
        mapa = moderadas;
        rango = "Moderadas (80-120 BPM)";
    } else if (opcion == 3) {
        mapa = rapidas;
        rango = "Rápidas (más de 120 BPM)";
    } else {
        printf("Opcion no valida.\n");
        return;
    }

    if (mapa == NULL) {
        printf("Error: El mapa de tempos no está inicializado.\n");
        return;
    }

    Pair *par = firstTreeMap(mapa);
    if (par == NULL) {
        printf("No hay canciones en este rango de tempo.\n");
        return;
    }

    printf("\n--- Canciones %s ---\n", rango);
    int contador = 0;
    
    while (par != NULL) {
        musica *cancion = (musica *)par->value;
        
        printf("\n%d. %s - %s\n", ++contador, cancion->artists, cancion->track_name);
        printf("   Album: %s\n", cancion->album_name);
        printf("   Genero: %s\n", cancion->track_genre);
        printf("   Tempo: %.2f BPM\n", cancion->tempo);
        printf("   ID: %s\n", cancion->id);
        printf("   -------------------------");
        
        par = nextTreeMap(mapa);
    }
    printf("\nTotal: %d canciones\n", contador);
}

void agregar_favoritos(TreeMap *favoritos, TreeMap *canciones_id) {
    if (favoritos == NULL || canciones_id == NULL) {
        printf("Error: Los mapas no estan inicializados.\n");
        return;
    }

    char id[100];
    char categoria[100];

    printf("Ingresa el ID de la cancion: ");
    if (scanf(" %99[^\n]", id) != 1) {
        printf("Error al leer el ID.\n");
        limpiar_buffer();
        return;
    }
    limpiar_buffer();

    printf("Ingresa la categoria de favoritos: ");
    if (scanf(" %99[^\n]", categoria) != 1) {
        printf("Error al leer la categoria.\n");
        limpiar_buffer();
        return;
    }
    limpiar_buffer();

    normalizar_string(id);
    normalizar_string(categoria);

    Pair *par = searchTreeMap(canciones_id, id);
    if (par == NULL) {
        printf("No se encontro ninguna cancion con el ID '%s'.\n", id);
        return;
    }

    musica *cancion = (musica *)par->value;
    Pair *par_fav = searchTreeMap(favoritos, categoria);
    List *lista = NULL;

    if (par_fav == NULL) {
        lista = createList();
        if (lista == NULL) {
            printf("Error al crear la lista de favoritos.\n");
            return;
        }
        char *categoria_dup = strdup(categoria);
        if (categoria_dup == NULL) {
            printf("Error al duplicar la categoria.\n");
            free(lista);
            return;
        }
        insertTreeMap(favoritos, categoria_dup, lista);
    } else {
        lista = (List *)par_fav->value;
    }

    // Crear copia de la canción
    musica *copia_cancion = malloc(sizeof(musica));
    if (copia_cancion == NULL) {
        printf("Error al crear copia de la cancion.\n");
        return;
    }
    memcpy(copia_cancion, cancion, sizeof(musica));
    
    pushBack(lista, copia_cancion);
    printf("Cancion agregada a favoritos en la categoria: %s\n", categoria);
}

void mostrar_favoritos(TreeMap *favoritos) {
    if (favoritos == NULL) {
        printf("No hay canciones en favoritos.\n");
        return;
    }

    Pair *par = firstTreeMap(favoritos);
    if (par == NULL) {
        printf("No hay canciones en favoritos.\n");
        return;
    }

    while (par != NULL) {
        printf("\nCategoria: %s\n", (char*)par->key);
        printf("--------------------------\n");
        
        List *lista = (List *)par->value;
        musica *cancion = firstList(lista);
        
        if (cancion == NULL) {
            printf("Esta categoria no tiene canciones.\n");
        } else {
            int contador = 0;
            while (cancion != NULL) {
                printf("\n%d. %s - %s\n", ++contador, cancion->artists, cancion->track_name);
                printf("   Album: %s\n", cancion->album_name);
                printf("   Genero: %s\n", cancion->track_genre);
                printf("   Tempo: %.2f BPM\n", cancion->tempo);
                printf("   ID: %s\n", cancion->id);
                printf("   -------------------------");
                
                cancion = nextList(lista);
            }
            printf("\nTotal: %d canciones\n", contador);
        }
        
        par = nextTreeMap(favoritos);
    }
}

int leerOpcionValida() {
    char input[10];
    printf("\nIngrese la opcion (1-8): ");
    if (fgets(input, sizeof(input), stdin)) {
        int opcion = atoi(input);
        if (opcion >= 1 && opcion <= 8) 
            return opcion;
    }
    printf("Opcion no valida.\n");
    return -1;
}

int main() {
    int opcion;
    TreeMap *canciones_id = createTreeMap(lower_than_str);
    TreeMap *canciones_genero = createTreeMap(lower_than_str);
    TreeMap *canciones_artistas = createTreeMap(lower_than_str);
    TreeMap *tempo_lentas = createTreeMap(lower_than_float);
    TreeMap *tempo_moderadas = createTreeMap(lower_than_float);
    TreeMap *tempo_rapidas = createTreeMap(lower_than_float);
    TreeMap *favoritos = createTreeMap(lower_than_str);

    do {
        mostrarMenu();
        opcion = leerOpcionValida();
        
        if (opcion == -1) {
            printf("Presione Enter para continuar...");
            limpiar_buffer();
            continue;
        }

        switch(opcion) {
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
        }
        
        if (opcion != 8) {
            printf("\nPresione Enter para continuar...");
            limpiar_buffer();
        }
        
    } while(opcion != 8);

    return 0;
}