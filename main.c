#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "treemap.h"
#include "list.h"

//Definicion de la estructura de la cancion
typedef struct 
{
    char id[100];
    char artists[100];
    char album_name[300];
    char track_name[100];
    float tempo;
    char track_genre[100];
} musica;

//Funciones de comparacion para los mapas
int lower_than_str(void* key1, void* key2)
{
    return strcmp((char*)key1, (char*)key2) < 0;
}

//Funcion que compara dos floats, para ser usados en el mapa de tempos.
int lower_than_float(void* key1, void* key2)
{
    return *(float*)key1 < *(float*)key2;
}

//Muestra el menu de opciones al usuario.
void mostrarMenu()
{
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

//Repite la lectura, hasta que encuentre el salto de linea.
void limpiar_buffer()
{
    while (getchar() != '\n');
}

//Elimina espacios en el inicio y al final, ademas de convertir las cadenas a minuscula.
void normalizar_string(char* str)
{
    //Eliminar espacios al inicio y final
    char *inicio = str;
    while (*inicio == ' ' || *inicio == '\t' || *inicio == '\n') inicio++;
    
    //Eliminar espacios al final
    char *final = str + strlen(str) - 1;
    while (final > inicio && (*final == ' ' || *final == '\t' || *final == '\n')) final--;
    
    //Ajustar el final de la cadena
    memmove(str, inicio, final - inicio + 1);
    str[final - inicio + 1] = '\0';

    //Convertir a minúsculas
    for (char *p = str; *p; ++p) *p = tolower(*p);
}

/*Funcion que se encarga de cargar las canciones de un archivo, y guarda los datos en cada 
mapa correspondiente, para ser usados en los funciones posteriores*/
void cargar_canciones(TreeMap *canciones_id, TreeMap *canciones_genero, TreeMap *canciones_artistas, TreeMap *tempo_lentas, TreeMap *tempo_moderadas, TreeMap *tempo_rapidas)
{
    char ruta[256];
    printf("Ingrese la ruta del archivo CSV: ");
    scanf(" %255[^\n]", ruta);
    limpiar_buffer(); //Eliminar el salto de línea

    FILE *archivo = fopen(ruta, "r");
    if (!archivo)
    {
        printf("Error al abrir el archivo %s\n", ruta);
        return;
    }

    char linea[1024];
    fgets(linea, sizeof(linea), archivo); //Leer la cabecera

    int contador = 0;
    while (fgets(linea, sizeof(linea), archivo)) //Leer cada línea del archivo
    {
        char *tokens[21]; //Cambiado a 21 para evitar overflow
        int campo = 0; //Inicializar contador de campos
        char *token = strtok(linea, ","); //Dividir la línea por comas

        //Guardar cada token en el arreglo
        while (token != NULL && campo < 21)
        {
            tokens[campo++] = token;
            token = strtok(NULL, ",");
        }
        musica *cancion = malloc(sizeof(musica));
        if (!cancion)
        {
            printf("Error asignando memoria\n");
            continue;
        }
        //Inicializar la estructura
        memset(cancion, 0, sizeof(musica));

        //Asignar campos
        if (campo > 0) strncpy(cancion->id, tokens[0], sizeof(cancion->id)-1);
        if (campo > 2) strncpy(cancion->artists, tokens[2], sizeof(cancion->artists)-1);
        if (campo > 3) strncpy(cancion->album_name, tokens[3], sizeof(cancion->album_name)-1);
        if (campo > 4) strncpy(cancion->track_name, tokens[4], sizeof(cancion->track_name)-1);
        if (campo > 18) cancion->tempo = atof(tokens[18]);
        if (campo > 20) strncpy(cancion->track_genre, tokens[20], sizeof(cancion->track_genre)-1);

        //Normalizar strings
        normalizar_string(cancion->id);
        normalizar_string(cancion->artists);
        normalizar_string(cancion->track_genre);

        //Insertar en mapa por ID
        insertTreeMap(canciones_id, strdup(cancion->id), cancion);

        //Insertar en mapa por género
        Pair *par_genero = searchTreeMap(canciones_genero, cancion->track_genre);
        List *lista_genero = par_genero ? par_genero->value : createList();
        if (!par_genero)
        {
            insertTreeMap(canciones_genero, strdup(cancion->track_genre), lista_genero);
        }
        pushBack(lista_genero, cancion);

        //Insertar en mapa por artista
        Pair *par_artista = searchTreeMap(canciones_artistas, cancion->artists);
        List *lista_artista = par_artista ? par_artista->value : createList();
        if (!par_artista)
        {
            insertTreeMap(canciones_artistas, strdup(cancion->artists), lista_artista);
        }
        pushBack(lista_artista, cancion);

        //Insertar en mapa por tempo
        TreeMap *destino = NULL;
        if (cancion->tempo < 80) destino = tempo_lentas;
        else if (cancion->tempo < 120) destino = tempo_moderadas;
        else destino = tempo_rapidas;

        // Crear una copia del tempo para usar como clave
        float *tempo_key = malloc(sizeof(float));
        *tempo_key = cancion->tempo;
        insertTreeMap(destino, tempo_key, cancion);

        contador++;
    }

    //Cerrar el archivo
    fclose(archivo);
    printf("Canciones cargadas correctamente. Total: %d\n", contador);
}

/*La funcion recibe un ID, y lo busca en el mapa, donde al encontar el ID, muestra la
informacion correspondiente.*/
void buscar_por_id(TreeMap *canciones_id)
{
    char id_busqueda[100];
    printf("Ingresa el ID de la cancion: ");
    scanf(" %99[^\n]", id_busqueda);
    limpiar_buffer(); // Eliminar el salto de línea

    //Normalizar el ID ingresado
    normalizar_string(id_busqueda);

    Pair *par = searchTreeMap(canciones_id, id_busqueda);

    if (par == NULL)
    {
        printf("No se encontro ninguna cancion con el ID '%s'.\n", id_busqueda);
        return;
    }

    musica *cancion = par->value;
    // Mostrar la información de la canción
    printf("\n--- Informacion de la cancion ---\n");
    printf("ID: %s\n", cancion->id);
    printf("Artista(s): %s\n", cancion->artists);
    printf("Album: %s\n", cancion->album_name);
    printf("Nombre de la cancion: %s\n", cancion->track_name);
    printf("Genero: %s\n", cancion->track_genre);
    printf("Tempo: %.2f BPM\n\n", cancion->tempo);
}

/*La funcion recibe un genero, el cual a traves de una lista, muestra la informacion
toda la informacion de las canciones del mismo genero*/
void buscar_por_genero(TreeMap *canciones_genero)
{
    //Si el mapa esta vacio, muestra un mensaje de error, al dar un genero invalido.
    if (canciones_genero == NULL)
    {
        printf("Error: El mapa de generos no esta inicializado.\n");
        return;
    }

    char genero_buscado[100];
    printf("Ingrese el genero de la cancion: ");
    scanf(" %99[^\n]", genero_buscado);
    limpiar_buffer();

    //Normalizar el genero ingresado
    normalizar_string(genero_buscado);

    Pair *par = searchTreeMap(canciones_genero, genero_buscado);
    
    //Si no se encuentra el genero, muestra un mensaje de error
    if (par == NULL)
    {
        printf("\nNo se encontraron canciones del genero '%s'\n", genero_buscado);
        return;
    }

    List *lista = (List *)par->value;
    musica *cancion = firstList(lista);
    
    if (cancion == NULL)
    {
        printf("\nEl genero '%s' no contiene canciones.\n", genero_buscado);
        return;
    }

    printf("\n--- Canciones del genero '%s' ---\n", genero_buscado);
    int contador = 0;
    
    //Muestra hasta que sea igual a NULL
    while (cancion != NULL)
    {
        printf("\n%d. %s - %s\n", ++contador, cancion->artists, cancion->track_name);
        printf("   Album: %s\n", cancion->album_name);
        printf("   ID: %s | Tempo: %.2f BPM\n", cancion->id, cancion->tempo);
        printf("   -------------------------");
        
        cancion = nextList(lista);
    }
    printf("\nTotal: %d canciones\n", contador);
}

/*La funcion recibe un nombre de un artista, y busca que si es valido el nombre, si lo es
muestra cada cancion relacionada con el.*/
void buscar_por_artista(TreeMap *canciones_artistas)
{
    if (canciones_artistas == NULL)
    {
        printf("Error: Mapa de artistas no inicializado\n");
        return;
    }

    char artista_buscado[100];
    printf("Ingrese el nombre del artista: ");
    scanf(" %99[^\n]", artista_buscado);
    limpiar_buffer();
    
    //Normalizar el nombre del artista
    normalizar_string(artista_buscado);

    Pair *par = searchTreeMap(canciones_artistas, artista_buscado);
    
    if (par == NULL)
    {
        printf("\nNo se encontraron canciones del artista '%s'\n", artista_buscado);
        return;
    }

    List *lista = (List *)par->value;
    musica *cancion = firstList(lista);
    
    if (cancion == NULL)
    {
        printf("\nEl artista '%s' no tiene canciones registradas.\n", artista_buscado);
        return;
    }

    printf("\n--- Canciones del artista '%s' ---\n", artista_buscado);
    int contador = 0;
    
    //Muestra canciones hasta que sea igual a NULL
    while (cancion != NULL)
    {
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

void buscar_por_tempo(TreeMap *lentas, TreeMap *moderadas, TreeMap *rapidas)
{
    int opcion;

    printf("\nSeleccione el rango de tempo:\n");
    printf("1) Lentas (menos de 80 BPM)\n");
    printf("2) Moderadas (80-120 BPM)\n");
    printf("3) Rapidas (mas de 120 BPM)\n");
    printf("Opcion: ");
    
    //Leer la opción del usuario
    if (scanf("%d", &opcion) != 1)
    {
        printf("Entrada no válida.\n");
        limpiar_buffer();
        return;
    }
    limpiar_buffer(); //limina el salto de línea
    
    TreeMap *mapa = NULL;
    char *rango = "";
    
    //Seleccionar el mapa correspondiente según la opción
    if (opcion == 1)
    {
        mapa = lentas;
        rango = "Lentas (menos de 80 BPM)";
    } else if (opcion == 2)
    {
        mapa = moderadas;
        rango = "Moderadas (80-120 BPM)";
    } else if (opcion == 3)
    {
        mapa = rapidas;
        rango = "Rápidas (más de 120 BPM)";
    } else
    {
        printf("Opcion no valida.\n");
        return;
    }

    //Verificar si el mapa está inicializado
    if (mapa == NULL)
    {
        printf("Error: El mapa de tempos no está inicializado.\n");
        return;
    }

    //Buscar la primera canción en el rango de tempo
    Pair *par = firstTreeMap(mapa);
    if (par == NULL)
    {
        printf("No hay canciones en este rango de tempo.\n");
        return;
    }

    printf("\n--- Canciones %s ---\n", rango);
    int contador = 0;
    
    //Mostrar las canciones en el rango de tempo seleccionado por el usuario
    while (par != NULL)
    {
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

void agregar_favoritos(TreeMap *favoritos, TreeMap *canciones_id)
{
    //Verificar si los mapas están inicializados
    if (favoritos == NULL || canciones_id == NULL)
    {
        printf("Error: Los mapas no estan inicializados.\n");
        return;
    }

    char id[100];
    char categoria[100];

    //Solicitar ID y categoría al usuario
    printf("Ingresa el ID de la cancion: ");
    if (scanf(" %99[^\n]", id) != 1)
    {
        printf("Error al leer el ID.\n");
        limpiar_buffer();
        return;
    }
    limpiar_buffer(); //Eliminar el salto de línea

    printf("Ingresa la categoria de favoritos: ");
    if (scanf(" %99[^\n]", categoria) != 1) {
        printf("Error al leer la categoria.\n");
        limpiar_buffer();
        return;
    }
    limpiar_buffer(); //Eliminar el salto de línea

    //Normalizar el ID y la categoría
    normalizar_string(id);
    normalizar_string(categoria);

    //Buscar la canción por ID
    Pair *par = searchTreeMap(canciones_id, id);
    if (par == NULL)
    {
        printf("No se encontro ninguna cancion con el ID '%s'.\n", id);
        return;
    }
 
    musica *cancion = (musica *)par->value;
    Pair *par_fav = searchTreeMap(favoritos, categoria);
    List *lista = NULL;

    //Si la categoría no existe, crear una nueva lista
    if (par_fav == NULL)
    {
        lista = createList();
        if (lista == NULL)
        {
            printf("Error al crear la lista de favoritos.\n");
            return;
        }
        //Crear una copia de la categoría para usar como clave
        char *categoria_dup = strdup(categoria);
        if (categoria_dup == NULL)
        {
            printf("Error al duplicar la categoria.\n");
            free(lista);
            return;
        }
        insertTreeMap(favoritos, categoria_dup, lista);
    }
    //Si la categoría ya existe, obtener la lista existente
    else
    {
        lista = (List *)par_fav->value;
    }

    //Crea una copia de la canción
    musica *copia_cancion = malloc(sizeof(musica));
    if (copia_cancion == NULL)
    {
        printf("Error al crear copia de la cancion.\n");
        return;
    }
    //Inicializar la copia
    memcpy(copia_cancion, cancion, sizeof(musica));
    
    //Agregar la cancion a la lista de favoritos
    pushBack(lista, copia_cancion);
    printf("Cancion agregada a favoritos en la categoria: %s\n", categoria);
}

void mostrar_favoritos(TreeMap *favoritos)
{
    //Verificar si el mapa de favoritos está inicializado, si no, imprime mensaje de error
    if (favoritos == NULL)
    {
        printf("No hay canciones en favoritos.\n");
        return;
    }

    //Buscar la primera categoría de favoritos
    Pair *par = firstTreeMap(favoritos);
    //Si no hay categorías, mostrar mensaje
    if (par == NULL)
    {
        printf("No hay canciones en favoritos.\n");
        return;
    }

    //Si hay categorías, mostrar cada una 
    while (par != NULL)
    {
        printf("\nCategoria: %s\n", (char*)par->key);
        printf("--------------------------\n");
        
        List *lista = (List *)par->value;
        musica *cancion = firstList(lista);
        
        //Si la lista está vacía, mostrar mensaje
        if (cancion == NULL)
        {
            printf("Esta categoria no tiene canciones.\n");
        } else
        {
            int contador = 0;
            //Mostrar las canciones en la lista
            while (cancion != NULL)
            {
                printf("\n%d. %s - %s\n", ++contador, cancion->artists, cancion->track_name);
                printf("   Album: %s\n", cancion->album_name);
                printf("   Genero: %s\n", cancion->track_genre);
                printf("   Tempo: %.2f BPM\n", cancion->tempo);
                printf("   ID: %s\n", cancion->id);
                printf("   -------------------------");
                
                //Obtener la siguiente canción
                cancion = nextList(lista);
            }
            printf("\nTotal: %d canciones\n", contador);
        }
        //Buscar la siguiente categoría
        par = nextTreeMap(favoritos);
    }
}

//Asegura que el usuario solo pueda elegir una opcion valida.
int leerOpcionValida()
{
    char input[10];
    printf("\nIngrese la opcion (1-8): ");
    //Verifica que la entrada sea un numero entre 1 y 8
    if (fgets(input, sizeof(input), stdin))
    {
        int opcion = atoi(input);
        if (opcion >= 1 && opcion <= 8) return opcion;
    }
    //Si la entrada no es valida, muestra un mensaje de error
    printf("Opcion no valida.\n");
    return -1;
}

//Función para liberar toda la memoria utilizada
void liberar_memoria(TreeMap *canciones_id, TreeMap *canciones_genero, TreeMap *canciones_artistas, TreeMap *tempo_lentas, TreeMap *tempo_moderadas, TreeMap *tempo_rapidas, TreeMap *favoritos) {
    //Liberar memoria del mapa de canciones por ID
    if (canciones_id != NULL)
    {
        Pair *pair = firstTreeMap(canciones_id);
        while (pair != NULL)
        {
            //Liberar la clave (ID)
            free(pair->key);
            //Liberar la estructura de música
            free(pair->value);
            pair = nextTreeMap(canciones_id);
        }
        //Liberar el árbol mismo
        eraseTreeMap(canciones_id, NULL);
    }

    //Liberar memoria del mapa de canciones por género
    if (canciones_genero != NULL)
    {
        Pair *pair = firstTreeMap(canciones_genero);
        while (pair != NULL)
        {
            //Liberar la clave (género)
            free(pair->key);
            //Liberar la lista de canciones
            List *lista = pair->value;
            cleanList(lista); // Solo limpiamos la lista porque las canciones ya fueron liberadas en canciones_id
            free(lista);
            pair = nextTreeMap(canciones_genero);
        }
        eraseTreeMap(canciones_genero, NULL);
    }

    //Liberar memoria del mapa de canciones por artista
    if (canciones_artistas != NULL)
    {
        Pair *pair = firstTreeMap(canciones_artistas);
        while (pair != NULL)
        {
            //Liberar la clave (artista)
            free(pair->key);
            //Liberar la lista de canciones
            List *lista = pair->value;
            cleanList(lista); //Solo limpiar la lista porque las canciones ya fueron liberadas en canciones_id
            free(lista);
            pair = nextTreeMap(canciones_artistas);
        }
        eraseTreeMap(canciones_artistas, NULL);
    }

    //Liberar memoria de los mapas de tempo (las canciones ya fueron liberadas en canciones_id)
    if (tempo_lentas != NULL)
    {
        Pair *pair = firstTreeMap(tempo_lentas);
        while (pair != NULL)
        {
            //Liberar la clave (tempo)
            free(pair->key);
            pair = nextTreeMap(tempo_lentas);
        }
        eraseTreeMap(tempo_lentas, NULL);
    }

    if (tempo_moderadas != NULL)
    {
        Pair *pair = firstTreeMap(tempo_moderadas);
        while (pair != NULL)
        {
            //Liberar la clave (tempo)
            free(pair->key);
            pair = nextTreeMap(tempo_moderadas);
        }
        eraseTreeMap(tempo_moderadas, NULL);
    }

    if (tempo_rapidas != NULL)
    {
        Pair *pair = firstTreeMap(tempo_rapidas);
        while (pair != NULL)
        {
            //Liberar la clave (tempo)
            free(pair->key);
            pair = nextTreeMap(tempo_rapidas);
        }
        eraseTreeMap(tempo_rapidas, NULL);
    }

    //Liberar memoria del mapa de favoritos
    if (favoritos != NULL)
    {
        Pair *pair = firstTreeMap(favoritos);
        while (pair != NULL)
        {
            //Liberar la clave (categoría)
            free(pair->key);
            //Liberar la lista de canciones favoritas
            List *lista = pair->value;
            musica *cancion = firstList(lista);
            while (cancion != NULL) {
                free(cancion);
                cancion = nextList(lista);
            }
            free(lista);
            pair = nextTreeMap(favoritos);
        }
        eraseTreeMap(favoritos, NULL);
    }
}

//Funcion principal, donde se declaran los mapas, y las llamadas a las difrentes funciones.
int main()
{
    int opcion;
    //Crear los mapas para almacenar las canciones
    TreeMap *canciones_id = createTreeMap(lower_than_str);
    TreeMap *canciones_genero = createTreeMap(lower_than_str);
    TreeMap *canciones_artistas = createTreeMap(lower_than_str);
    TreeMap *tempo_lentas = createTreeMap(lower_than_float);
    TreeMap *tempo_moderadas = createTreeMap(lower_than_float);
    TreeMap *tempo_rapidas = createTreeMap(lower_than_float);
    TreeMap *favoritos = createTreeMap(lower_than_str);

    //Comenzar el bucle del menu
    do
    {
        //Mostrar el menu de opciones
        mostrarMenu();
        opcion = leerOpcionValida();
        
        //Si la opción es -1, significa que el usuario ingresó una opción inválida
        if (opcion == -1)
        {
            //Mostrar mensaje de error y continuar
            printf("Presione Enter para continuar...");
            limpiar_buffer(); // Limpiar el buffer
            continue;
        }

        //Ejecutar la opción seleccionada
        switch(opcion)
        {
            //Cargar canciones desde un archivo CSV
            case 1:
                cargar_canciones(canciones_id, canciones_genero, canciones_artistas, 
                               tempo_lentas, tempo_moderadas, tempo_rapidas);
                break;
            //Buscar por ID
            case 2:
                buscar_por_id(canciones_id);
                break;
            //Buscar por género
            case 3:
                buscar_por_genero(canciones_genero);
                break;
            //Buscar por artista
            case 4:
                buscar_por_artista(canciones_artistas);
                break;
            //Buscar por tempo
            case 5:
                buscar_por_tempo(tempo_lentas, tempo_moderadas, tempo_rapidas);
                break;
            //Agregar a favoritos
            case 6:
                agregar_favoritos(favoritos, canciones_id);
                break;
            //Mostrar favoritos
            case 7:
                mostrar_favoritos(favoritos);
                break;
            //Salir del programa
            case 8:
                printf("Saliendo del sistema...\n");
                //Liberar memoria antes de salir
                liberar_memoria(canciones_id, canciones_genero, canciones_artistas, tempo_lentas, tempo_moderadas, tempo_rapidas, favoritos);
                break;
        }
        //Si la opción no es 8, espera a que el usuario presione Enter para continuar
        if (opcion != 8)
        {
            printf("\nPresione Enter para continuar...");
            limpiar_buffer(); //Limpiar el buffer
        }
    //Asegura que el ciclo no sea infinito, y que si el usuario elige la opcion 8, pueda salir del menu
    } while(opcion != 8);

    //Terminar el programa
    return 0;
}