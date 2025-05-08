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
    puts("========================================");
    puts("     Base de Datos de Canciones");
    puts("========================================");
    puts("1) Cargar Canciones");
    puts("2) Buscar por ID");
    puts("3) Buscar por genero");
    puts("4) Buscar por artista");
    puts("5) Buscar por tempo");
    puts("6) Agregar a favoritos");
    puts("7) Mostrar favoritos");
    puts("8) Salir");
    puts("========================================");
    fflush(stdout);
}

char **leer_linea_csv(FILE *archivo, char delimitador) 
{
    char **campos = NULL;  //NULL para la lista de campos
    char linea[1024];  //para la línea leída
    int index = 0;
    size_t capacidad = 10;  //capacidad inicial para 10 campos
    campos = malloc(sizeof(char*) * capacidad);  //reserva memoria inicial

    //Lee una línea del archivo
    if (fgets(linea, sizeof(linea), archivo) == NULL) 
    {
        return NULL; //Si no hay más líneas retornamos NULL
    }

    //divide la línea por el delimitador
    char *campo = strtok(linea, &delimitador);
    while (campo != NULL) 
    {
        if (index >= capacidad) 
        {  //Si hemos alcanzado la capacidad, ampliamos el arreglo
            capacidad *= 2;  //doblando la capacidad
            campos = realloc(campos, sizeof(char*) * capacidad);
        }

        campos[index] = malloc(strlen(campo) + 1);
        strcpy(campos[index], campo);
        index++;

        //lee el siguiente campo
        campo = strtok(NULL, &delimitador);
    }
    campos[index] = NULL;//Finaliza el arreglo con NULL
    return campos;
}

void limpiar_string(char* str)
{
    char *inicio = str;
    while (*inicio == ' ' || *inicio == '\t' || *inicio == '\n') inicio++;
    char *final = str + strlen(str) - 1;
    while (final > inicio && (*final == ' ' || *final == '\t' || *final == '\n')) final--;

    memmove(str, inicio, final - inicio + 1);
    str[final - inicio + 1] = '\0';
}


void cargar_canciones(TreeMap *canciones_id, TreeMap *canciones_genero, TreeMap *canciones_artistas, TreeMap *tempo_lentas, TreeMap *tempo_Moderadas, TreeMap *tempo_rapidas)
{
    FILE *archivo = fopen("song_dataset_.csv", "r");
    if (archivo == NULL)
    {
        perror("Error al abrir el archivo");
        return;
    }

    char **campos = leer_linea_csv(archivo, ',');
    if (campos != NULL) 
    {
        for (int i = 0; campos[i] != NULL; i++) free(campos[i]);
        free(campos);
    }

    while ((campos = leer_linea_csv(archivo, ',')) != NULL)
    {
        //contar campos
        int n_campos = 0;
        while (campos[n_campos] != NULL) n_campos++;

        if (n_campos < 21) 
        {
            for (int i = 0; i < n_campos; i++) 
            free(campos[i]);
            free(campos);
            continue;
        }

        musica *cancion = malloc(sizeof(musica));
        if (cancion == NULL) 
        {
            perror("Error al asignar memoria a musica, por favor intente nuevamente");
            exit(EXIT_FAILURE);
        }

        strcpy(cancion->id, campos[0]);
        limpiar_string(cancion->id);

        strcpy(cancion->artists, campos[2]);
        limpiar_string(cancion->artists);

        strcpy(cancion->album_name, campos[3]);
        limpiar_string(cancion->album_name);

        strcpy(cancion->track_name, campos[4]);
        limpiar_string(cancion->track_name);

        cancion->tempo = atof(campos[18]);

        strcpy(cancion->track_genre, campos[20]);
        limpiar_string(cancion->track_genre);

        float *tempoKey = malloc(sizeof(float));
        *tempoKey = cancion->tempo;

        if (cancion->tempo < 80.0) insertTreeMap(tempo_lentas, tempoKey, cancion);
        else if (cancion->tempo <= 120.0) insertTreeMap(tempo_Moderadas, tempoKey, cancion);
        else insertTreeMap(tempo_rapidas, tempoKey, cancion);

        //insertar en los otros tres mapas
        insertTreeMap(canciones_id, cancion->id, cancion);
        insertTreeMap(canciones_artistas, cancion->artists, cancion);

        Pair *par_genero = searchTreeMap(canciones_genero, cancion->track_genre);
        List *lista_genero = NULL;
        if (par_genero != NULL)
            lista_genero = (List *)par_genero->value;
        if (lista_genero == NULL)
        {
            lista_genero = createList();
            insertTreeMap(canciones_genero, cancion->track_genre, lista_genero);
        }
        pushBack(lista_genero, cancion);


        for (int i = 0; i < n_campos; i++) free(campos[i]);
        free(campos);
    }

    fclose(archivo);
    printf("Canciones cargadas correctamente.\n");
}

void buscar_por_id(TreeMap *canciones_id)
{
    char id_busqueda[100];
    printf("Ingresa el ID de la cancion: ");
    scanf(" %[^\n]", id_busqueda);
    Pair *par = searchTreeMap(canciones_id, id_busqueda);

    if (par == NULL)
    {
        printf("No se encontro ninguna cancion con el mismo ID.\n");
        return;
    }

    musica *cancion = (musica *)par->value;

    /*imprime la informacion*/
    printf("\n--- Informacion de la cancion ---\n");
    printf("ID: %s\n", cancion->id);
    printf("Artista(s): %s\n", cancion->artists);
    printf("Album: %s\n", cancion->album_name);
    printf("Nombre de la cancion: %s\n", cancion->track_name);
    printf("Genero: %s\n", cancion->track_genre);
    printf("Tempo: %.2f BPM\n\n", cancion->tempo);
}

void buscar_por_genero(TreeMap *canciones_genero)
{
    char genero_buscado[100];
    printf("Ingrese el genero: ");
    scanf(" %[^\n]", genero_buscado);
    getchar(); // Limpiar el buffer de entrada

    Pair *par = searchTreeMap(canciones_genero, genero_buscado);
    if (par == NULL)
    {
        printf("No se encontro ninguna cancion con ese genero.\n");
        return;
    }

    List *lista = (List *)par->value;
    musica *cancion = firstList(lista);

    printf("\n--- Canciones del genero '%s' ---\n", genero_buscado);
    while(cancion != NULL)
    {
        printf("ID: %s\n", cancion->id);
        printf("Artista(s): %s\n", cancion->artists);
        printf("Album: %s\n", cancion->album_name);
        printf("Nombre de la cancion: %s\n", cancion->track_name);
        printf("Tempo: %.2f BPM\n\n", cancion->tempo);
        
        cancion = nextList(lista);
        printf("--------------------------\n");
    }
}

void buscar_por_artista(TreeMap *canciones_artistas)
{
    char artista_buscado[100];
    printf("Ingrese el nombre del artista: ");
    scanf(" %[^\n]", artista_buscado);
    getchar(); // Limpiar el buffer de entrada

    Pair *par = searchTreeMap(canciones_artistas, artista_buscado);
    if (par == NULL)
    {
        printf("No se encontro ninguna cancion de ese artista.\n");
        return;
    }
    musica *cancion = par->value;
    printf("Cancion encontrada: %s - %s\n", cancion->track_name, cancion->album_name);
}

void buscar_por_tempo(TreeMap *lentas, TreeMap *moderadas, TreeMap *rapidas)
{
    printf("Canciones lentas (< 80 BPM):\n");
    for (Pair *par = firstTreeMap(lentas); par != NULL; par = nextTreeMap(lentas))
    {
        musica *cancion = par->value;
        printf("%s - %.2f BPM\n", cancion->track_name, cancion->tempo);
    }
    printf("\n\nCanciones moderadas (80 - 120 BPM):\n");
    for (Pair *par = firstTreeMap(moderadas); par != NULL; par = nextTreeMap(moderadas))
    {
        musica *cancion = par->value;
        printf("%s - %.2f BPM\n", cancion->track_name, cancion->tempo);
    }
    printf("\n\nCanciones rapidas (> 120 BPM):\n");
    for (Pair *par = firstTreeMap(rapidas); par != NULL; par = nextTreeMap(rapidas))
    {
        musica *cancion = par->value;
        printf("%s - %.2f BPM\n", cancion->track_name, cancion->tempo);
    }
}

void agregar_favoritos(TreeMap *favoritos, TreeMap *canciones_id)
{
    char id[100];
    char categoria[100];

    printf("Ingresa el ID de la cancion: ");
    scanf(" %[^\n]", id);
    getchar(); // Limpiar el buffer de entrada

    Pair *par = searchTreeMap(canciones_id, id);
    if (par == NULL)
    {
        printf("No se encontro ninguna cancion con ese ID.\n");
        return;
    }

    musica *cancion = par->value;

    printf("Ingresa la categoria de favoritos: ");
    scanf(" %[^\n]", categoria);
    getchar(); // Limpiar el buffer de entrada

    Pair *par_fav = searchTreeMap(favoritos, categoria);
    List *lista;

    if (par_fav == NULL)
    {
        lista = createList();
        insertTreeMap(favoritos, strdup(categoria), lista);
    }
    else
    {
        lista = (List *)par_fav->value;
    }

    pushBack(lista, cancion);
    printf("Cancion agregada a favoritos en la categoria: %s\n", categoria);
}

void mostrar_favoritos(TreeMap *favoritos)
{
    Pair *par = firstTreeMap(favoritos);
    if (par == NULL)
    {
        printf("No hay canciones en favoritos.\n");
        return;
    }

    while (par != NULL)
    {

        printf("\nCategoria: %s\n", (char*)par->key);
        printf("--------------------------\n");
        List *lista = par->value;

        musica *cancion = firstList(lista);
        while (cancion != NULL)
        {
            printf("ID: %s\n", cancion->id);
            printf("Artista(s): %s\n", cancion->artists);
            printf("Album: %s\n", cancion->album_name);
            printf("Nombre de la cancion: %s\n", cancion->track_name);
            printf("Genero: %s\n", cancion->track_genre);
            printf("Tempo: %.2f BPM\n", cancion->tempo);
            printf("--------------------------\n");

            cancion = nextList(lista);
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

int main()
{
    int opcion;
    TreeMap *canciones_id = createTreeMap(lower_than_str); //mapa de ID
    TreeMap *canciones_genero = createTreeMap(lower_than_str); //mapa de genero
    TreeMap *canciones_artistas = createTreeMap(lower_than_str); //mapa de artistas
    TreeMap *tempo_lentas = createTreeMap(lower_than_float); //mapa de tempo canciones lentas
    TreeMap *tempo_Moderadas = createTreeMap(lower_than_float); //mapa de tempo canciones moderadas
    TreeMap *tempo_rapidas = createTreeMap(lower_than_float); //mapa de tempo canciones rapidas
    TreeMap *favoritos = createTreeMap(lower_than_str); //mapa de favoritos

    do {
        mostrarMenu();
        opcion = leerOpcionValida();
        
        if (opcion == -1) {
            printf("Presione Enter para continuar...");
            while (getchar() != '\n');
            continue;
        }

        switch(opcion) {
            case 1:
                cargar_canciones(canciones_id, canciones_genero, canciones_artistas,
                               tempo_lentas, tempo_Moderadas, tempo_rapidas);
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
                buscar_por_tempo(tempo_lentas, tempo_Moderadas, tempo_rapidas);
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
            while (getchar() != '\n');
        }
        
    } while(opcion != 8);

    return 0;
}