#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void mostrarMenu()
{
    puts("========================================");
    puts("     Base de Datos de Canciones");
    puts("========================================");
    puts("1) Cargar Canciones");
    puts("2) Buscar por ID");
    puts("3) Buscar por genero");
    puts("4) Buscar por artista");
    puts("5) Buscar por tempo");
    puts("8) Salir");
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
            perror("Error al asignar memoria a musica");
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
        insertTreeMap(canciones_genero, cancion->track_genre, cancion);

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

int main()
{
    char opcion;
    TreeMap *canciones_id = createTreeMap(lower_than_str); //mapa de ID
    TreeMap *canciones_genero = createTreeMap(lower_than_str); //mapa de genero
    TreeMap *canciones_artistas = createTreeMap(lower_than_str); //mapa de artistas
    TreeMap *tempo_lentas = createTreeMap(lower_than_float);
    TreeMap *tempo_Moderadas = createTreeMap(lower_than_float);
    TreeMap *tempo_rapidas = createTreeMap(lower_than_float);

    do
    {
        mostrarMenu();
        printf("Ingrese la opcion: ");
        scanf(" %c", &opcion);

        switch(opcion) 
        {
            case '1':
                system("cls");
                cargar_canciones(canciones_id, canciones_genero, canciones_artistas, tempo_lentas, tempo_Moderadas, tempo_rapidas);
                break;
            case '2':
                buscar_por_id(canciones_id);
                break;
            default:
                printf("Saliendo...\n");
                break;
        }
    } while(opcion != '8');

    return 0;
}
