#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define HT_ric_size 251
#define HT_mag_size 251

//Ricettario   -----------------------------------------------
typedef struct ing_type{
    char nome[26];
    u_int32_t quantita;
    struct ing_type* next;
}ing_type;

typedef struct ric_type{
    char nome[26];
    ing_type *ingredienti;
    struct ric_type* next;
}ric_type;

typedef struct {
    ric_type **slots;  //array di puntatori
}hash_table_ric;

u_int32_t hash_ric(char* key){
   u_int32_t len = strlen(key), val = 0;
    for (int i = 0; i < len; ++i)
        val = val + key[i]*19;
    return val % HT_ric_size;
}

hash_table_ric* create_Ricettario(){
    hash_table_ric *table = (hash_table_ric*)malloc(sizeof(hash_table_ric));
    table->slots = malloc(sizeof(ric_type*) * HT_ric_size);
    for (int i = 0; i < HT_ric_size; ++i)
        table->slots[i] = NULL;
    return table;
}

void insert_Ricettario(hash_table_ric* table, ric_type *ricetta){
    ric_type *tmp;
    u_int32_t idx = hash_ric(ricetta->nome);

    //inserimento in testa
    tmp = table->slots[idx];
    table->slots[idx] = ricetta;
    ricetta->next = tmp;

    printf( "aggiunta\n");
}

bool check_Ricettario(hash_table_ric *table, char *nome){
    u_int32_t idx = hash_ric(nome);
    ric_type *cur = table->slots[idx];
    while (cur != NULL){
        if (strcmp(cur->nome, nome) == 0)
            return true; //è gia presente una ricetta con lo stesso nome
        cur = cur->next;
    }
    return false; //non è presente una ricetta col nome indicato
}

//Magazzino   ---------------------------------------------------
typedef struct lotto_type{
    u_int32_t scadenza;
    u_int32_t quantita;
    struct lotto_type* next;
}lotto_type;

typedef struct mat_type{
    char nome[26];
    lotto_type *lotti;
    u_int32_t last_checked;
    u_int32_t quantita_tot;
    struct mat_type *next;
}mat_type;

typedef struct {
    mat_type **slots;  //array di puntatori
}hash_table_mag;

u_int32_t hash_mag(char* key){
    u_int32_t len = strlen(key), val = 0;
    for (int i = 0; i < len; ++i)
        val = val + key[i]*19;
    return val % HT_mag_size;
}

hash_table_mag *create_Magazzino(){
    hash_table_mag *table = (hash_table_mag*)malloc(sizeof(hash_table_mag));
    table->slots = malloc(sizeof(mat_type*) * HT_mag_size);
    for (int i = 0; i < HT_mag_size; ++i)
        table->slots[i] = NULL;
    return table;
}

void insert_Magazzino(hash_table_mag* magazzino, mat_type *nuova_materia){
    mat_type *tmp;
    u_int32_t idx = hash_mag(nuova_materia->nome);

    //inserimento in testa
    tmp = magazzino->slots[idx];
    nuova_materia->next = tmp;
    magazzino->slots[idx] = nuova_materia;
}

//check quantità e scadenze
bool check_quantity_Magazzino(u_int32_t quantita_necessaria, mat_type *materia_prima, u_int32_t tempo ){
    u_int32_t quantita_trovata = 0;
    lotto_type *cur_lotto, *prev_lotto;

    //la materia è stata già controllata in questo quanto di tempo
    if (materia_prima->last_checked == tempo){
        if (materia_prima->quantita_tot >= quantita_necessaria)
            return true;
        else
            return false;
    }
    //la materia non è stata già controllata in questo quanto di tempo
    cur_lotto = materia_prima->lotti;
    prev_lotto = cur_lotto;
    while (cur_lotto != NULL){
        //verifica scadenza lotti
        if (cur_lotto->scadenza <= tempo){
            //lotto scaduto
            if (prev_lotto == cur_lotto){
                materia_prima->lotti = cur_lotto->next;
                free(cur_lotto);
                cur_lotto = materia_prima->lotti;
                prev_lotto = cur_lotto;
            }
            else{
                prev_lotto->next = cur_lotto->next;
                free(cur_lotto);
                cur_lotto = prev_lotto->next;
            }
        }
        else{
            materia_prima->last_checked = tempo;
            quantita_trovata += cur_lotto->quantita;
            prev_lotto = cur_lotto;
            cur_lotto = cur_lotto->next;
        }
    }
    materia_prima->last_checked = tempo;
    materia_prima->quantita_tot = quantita_trovata;
    if (quantita_trovata >= quantita_necessaria)
        return true;
    else
        return false;
}

//Ordini -----------------------------------------------------------
typedef struct ordine_type{
    ric_type *ricetta;
    u_int32_t num_elem;
    u_int32_t peso;
    u_int32_t tempo_di_arrivo;
    struct ordine_type *next;
}ordine_type;

typedef struct{
    ordine_type *start;
    ordine_type *end;
}queue;

queue *create_Queue(){
    queue *coda = (queue*)malloc(sizeof(queue));
    coda->start = NULL;
    coda->end = NULL;
    return coda;
}

void insert_Queue(queue *coda, ordine_type *nuovo_ordine){
    nuovo_ordine->next = NULL;
    if (coda->start == NULL){ //la coda è vuota
        //aggiungi in testa
        coda->start = nuovo_ordine;
        coda->end = coda->start;
        return;
    }
    //altrimenti aggiungi nuovo_ordine in coda
    (coda->end)->next = nuovo_ordine;
    coda->end = nuovo_ordine;
}

void insert_ListaPronti(ordine_type **lista_ordini_pronti, ordine_type *nuovo_ordine){
    ordine_type *cur, *prev;
    nuovo_ordine->next =  NULL;

    //inserimento in lista ordinata
    cur = *lista_ordini_pronti;
    prev = cur;
    while (cur != NULL){
        if (nuovo_ordine->tempo_di_arrivo < cur->tempo_di_arrivo){
            if (prev == cur){
                nuovo_ordine->next = *lista_ordini_pronti;
                *lista_ordini_pronti = nuovo_ordine;
                return;
            }
            else {
                nuovo_ordine->next = cur;
                prev->next = nuovo_ordine;
                return;
            }
        }
        else if(nuovo_ordine->tempo_di_arrivo == cur->tempo_di_arrivo){
            nuovo_ordine->next = cur->next;
            cur->next = nuovo_ordine;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL){
        if (prev == NULL){
            *lista_ordini_pronti = nuovo_ordine;
            return;
        }
        else{
            prev->next = nuovo_ordine;
            return;
        }
    }
}

void make_Order(ordine_type *nuovo_ordine, hash_table_mag *magazzino){
    ric_type *cur_ric;
    ing_type *cur_ing;
    u_int32_t idx;
    mat_type *cur_mat;
    u_int32_t quantita_necessaria;
    lotto_type *cur_lotto, *last_lotto, *succ;
    u_int32_t quantita_trovata, differenza_quantita;
    //ricorda che è già stato effetuato il check quantity
    cur_ric = nuovo_ordine->ricetta;
    cur_ing = cur_ric->ingredienti;
    while (cur_ing != NULL){
        idx = hash_mag(cur_ing->nome);
        cur_mat = magazzino->slots[idx];
        //prev_mat = cur_mat;
        while (cur_mat != NULL){
            if (strcmp(cur_mat->nome, cur_ing->nome) == 0)
                break;
            //prev_mat = cur_mat;
            cur_mat = cur_mat->next;
        }
        //cur_mat punta alla materia nel magazzino
        quantita_necessaria = cur_ing->quantita * nuovo_ordine->num_elem;
        cur_mat->quantita_tot -= quantita_necessaria;
        quantita_trovata = 0;
        cur_lotto = cur_mat->lotti;
        while (cur_lotto != NULL && quantita_trovata < quantita_necessaria){
            quantita_trovata += cur_lotto->quantita;
            if (quantita_trovata >= quantita_necessaria){
                differenza_quantita = quantita_trovata - quantita_necessaria;
                //esempio,  trovati: 100g + 100g = 200g
                //          necessari: 130g / 200g
                //          differenza: 70g / 0g   --> nel secondo lotto rimangono 70g / 0g
                //importante!!!   se rimangono 0g devo eliminare il lotto
                cur_lotto->quantita = differenza_quantita;
                break;
            }
            cur_lotto = cur_lotto->next;
        }

        if (quantita_trovata >= quantita_necessaria){
            last_lotto = cur_lotto;
            if (last_lotto->quantita == 0) //se rimangono 0g devo eliminare il lotto
                last_lotto = last_lotto->next;
            cur_lotto = cur_mat->lotti;
            cur_mat->lotti = last_lotto;
            while (cur_lotto != last_lotto){
                succ = cur_lotto->next;
                free(cur_lotto);
                cur_lotto = succ;
            }
        }
        //verifica se materia non ha più lotti, allora eliminala dal magazzino
        cur_ing = cur_ing->next;
    }
    //ora sono stati cancellati/aggiornati i lotti usati
    //per preparare l'ordine
    //RICORDA: inserisci in coda dei pronti il nuovo ordine!
}

//Spedizione corriere -----------------------------------------------------------
typedef struct{
    u_int32_t periodicita;
    u_int32_t capienza;
}corriere_type;

void spedizione_Corriere(corriere_type corriere, ordine_type **lista_ordini_pronti){
    u_int32_t peso = 0, cont = 0;
    ordine_type *cur_ordine, *last_ordine;
    ordine_type **array;
    int i, j;
    ordine_type *cur;

    cur_ordine = *lista_ordini_pronti;
    if (cur_ordine == NULL){
        printf( "camioncino vuoto\n");
        return;
    }
    while (cur_ordine != NULL){
        peso += cur_ordine->peso;
        if (peso <= corriere.capienza){ //l'ordine viene selezionato per essere caricato
            cont++;
            cur_ordine = cur_ordine->next;
        }
        else{
            break;
        }
    }
    //ora cur_ordine punta all'ordine DOPO l'ultimo che viene selezionato
    last_ordine = cur_ordine;
    cur_ordine = *lista_ordini_pronti;
    array = malloc(sizeof(ordine_type*) * cont);
    for (i = 0; i < cont; i++, cur_ordine = cur_ordine->next) {
        array[i] = cur_ordine;
    }
    *lista_ordini_pronti = last_ordine;

    //ordinare per peso l'array in ordine DECRESCENTE
    //Insertion Sort
    for (i=1; i < cont; i++) {
        //key = array[i]->peso;
        cur = array[i];
        j = i-1;
        while ( (j >= 0) && (cur->peso > array[j]->peso)) {
            //swap elementi
            array[j+1] = array[j];
            j--;
        }
        array[j+1] = cur;
    }

    for (i = 0; i < cont; ++i) {

        printf( "%d %s %d\n", array[i]->tempo_di_arrivo, array[i]->ricetta->nome,
                array[i]->num_elem);
        free(array[i]);
    }
    free(array);
}

//Comandi  --------------------------------------------------------------
void fun_aggiungi_ricetta(hash_table_ric *ricettario){
    char nome_ricetta[26], nome_ingrediente[26], c = 'a'; //input nome nuovo igrediente
    u_int32_t input_quantita; //quantita input nuovo ingrediente
    ric_type *nuova_ricetta;
    ing_type *nuovo_ingrediente;
    //nome ricetta
    if(scanf("%s", nome_ricetta));
    //verifica se la ricetta è già presente
    bool presente = check_Ricettario(ricettario, nome_ricetta);
    if (presente == true){
        while (c != '\n')
            c = fgetc(stdin);

        printf( "ignorato\n");
        return;
    }
    // !presente
    //crea nuova ricetta
    nuova_ricetta = (ric_type*) malloc(sizeof(ric_type));
    strncpy(nuova_ricetta->nome, nome_ricetta, 26);
    ing_type **prev = &(nuova_ricetta->ingredienti); //prev punta al puntatore ingredienti di nuova_ricetta
    while (c != '\n'){
        if(scanf("%s", nome_ingrediente));
        nuovo_ingrediente = (ing_type*) malloc(sizeof(ing_type));
        strncpy(nuovo_ingrediente->nome, nome_ingrediente, 26);
        if(scanf("%d", &input_quantita));
        nuovo_ingrediente->quantita = input_quantita;
        nuovo_ingrediente->next = NULL;
        *prev = nuovo_ingrediente; //ciò che è puntato da prev viene sostituito con l'indirizzo di nuovo_ingrediente
        prev = &(nuovo_ingrediente->next); //prev punta al campo next di nuovo_ingrediente
        c = fgetc(stdin); //legge carattere dopo quantità
    }
    //inserisci nel ricettario
    insert_Ricettario(ricettario, nuova_ricetta);
}

void fun_rimuovi_ricetta(hash_table_ric *ricettario, queue *coda_ordini_attesa,
                         ordine_type **lista_ordini_pronti){
    char nome_ricetta[26], c = 'a';
    u_int32_t idx;
    ric_type *cur, *prev;
    ing_type *cur_ing, *succ;
    ordine_type *cur_ordine_coda;

    if(scanf("%s", nome_ricetta));
    while (c != '\n')
        c = fgetc(stdin);

    //verifica se c'è un ordine in attesa relativo alla ricetta da eliminare
    cur_ordine_coda = coda_ordini_attesa->start;
    while (cur_ordine_coda != NULL){
        if (strcmp(nome_ricetta, (cur_ordine_coda->ricetta)->nome) == 0){

            printf( "ordini in sospeso\n");
            return;
        }
        cur_ordine_coda = cur_ordine_coda->next;
    }
    //verifica se c'è un ordine pronto relativo alla ricetta da eliminare
    cur_ordine_coda = *lista_ordini_pronti;
    while (cur_ordine_coda != NULL){
        if (strcmp(nome_ricetta, (cur_ordine_coda->ricetta)->nome) == 0){

            printf( "ordini in sospeso\n");
            return;
        }
        cur_ordine_coda = cur_ordine_coda->next;
    }



    //non ci sono ordini in attesa o pronti con la ricetta da rimuovere
    idx = hash_ric(nome_ricetta);
    cur = ricettario->slots[idx];
    prev = cur;
    while (cur != NULL){
        if (strcmp(cur->nome, nome_ricetta) == 0){ //la ricetta da eliminare è stata trovata
            //free lista ingredienti
            cur_ing = cur->ingredienti;
            while (cur_ing != NULL){
                succ = cur_ing->next;
                free(cur_ing);
                cur_ing = succ;
            }
            //free ricetta
            if (prev == cur){
                ricettario->slots[idx] = cur->next;
                free(cur);

                printf( "rimossa\n");
                return;
            }
            else{
                prev->next = cur->next;
                free(cur);

                printf( "rimossa\n");
                return;
            }
        }
        prev = cur;
        cur = cur->next;
    }
    //la ricetta non è presente

    printf( "non presente\n");
}

void fun_rifornimento(hash_table_mag *magazzino, queue *coda_ordini_attesa,
                      ordine_type **lista_ordini_pronti, u_int32_t tempo ){

    char nome_materia[26], c = 'a'; //input nome materia
    u_int32_t input_quantita, input_scadenza, idx; //quantita input nuovo ingrediente
    mat_type *nuova_materia, *cur_mat;
    lotto_type *nuovo_lotto, *cur_lotto, *prev_lotto;
    bool trovato = false; //flag se trovi già la materia nell'hash table
    //dichiariazioni x verifica ordini in attesa
    ordine_type *cur_ordine_coda, *prev_ordine_coda;
    ric_type *cur_ric;
    ing_type *cur_ing;
    u_int32_t quantita_necessaria, peso_parziale, peso_totale;
    bool presente, q_sufficiente;

    //rifornimento
    while (c != '\n'){
        //nome materia
        if(scanf("%s", nome_materia));
        //quantita
        if(scanf("%d", &input_quantita));
        //scadenza
        if(scanf("%d", &input_scadenza));
        c = fgetc(stdin);

        //inserimento materie/lotti nel magazzino
        idx = hash_mag(nome_materia);
        cur_mat = magazzino->slots[idx];
        trovato = false;
        while (cur_mat != NULL){
            if (strcmp(nome_materia, cur_mat->nome) == 0) {
                trovato = true;
                break;
            }
            cur_mat = cur_mat->next;
        }

        if (trovato == false){ //nel Magazzino non è già presente la materia
            nuova_materia = (mat_type*)malloc(sizeof(mat_type));
            nuovo_lotto = (lotto_type*)malloc(sizeof(lotto_type));
            strncpy(nuova_materia->nome, nome_materia, 26);
            nuova_materia->next = NULL;
            nuovo_lotto->scadenza = input_scadenza;
            nuovo_lotto->quantita = input_quantita;
            nuovo_lotto->next = NULL;
            nuova_materia->lotti = nuovo_lotto;
            insert_Magazzino(magazzino, nuova_materia);
            //printf( "%d: rifornimento di %s %d g %d\n", tempo, nuova_materia->nome,nuovo_lotto->quantita, nuovo_lotto->scadenza);
        }
        else{ //nel Magazzino è gia presente la materia
            cur_lotto = cur_mat->lotti; //cur_mat punta alla materia cercata nella HT
            prev_lotto = cur_lotto;
            //inserimento in lista ORDINATA in ordine crescente di scadenza
            while (cur_lotto != NULL){
                if (input_scadenza < cur_lotto->scadenza){
                    nuovo_lotto = (lotto_type*)malloc(sizeof(lotto_type));
                    nuovo_lotto->scadenza = input_scadenza;
                    nuovo_lotto->quantita = input_quantita;
                    if (prev_lotto == cur_lotto){ //inserimento in testa
                        nuovo_lotto->next = cur_lotto;
                        cur_mat->lotti = nuovo_lotto;
                        cur_lotto = cur_mat->lotti;
                        prev_lotto = cur_lotto;
                        break;
                    }
                    else{ //inserimento in  mezzo
                        prev_lotto->next = nuovo_lotto;
                        nuovo_lotto->next = cur_lotto;
                        break;
                    }
                }
                else if (input_scadenza > cur_lotto->scadenza){
                    prev_lotto = cur_lotto;
                    cur_lotto = cur_lotto->next;
                }
                else if (input_scadenza == cur_lotto->scadenza){
                    cur_lotto->quantita += input_quantita;
                    break;
                }
            }
            if(cur_lotto == NULL){
                if (prev_lotto == NULL){//la materia non ha lotti (scaduti)
                    nuovo_lotto = (lotto_type*)malloc(sizeof(lotto_type));
                    nuovo_lotto->scadenza = input_scadenza;
                    nuovo_lotto->quantita = input_quantita;
                    nuovo_lotto->next = NULL;
                    cur_mat->lotti = nuovo_lotto;
                }
                else{//il lotto ha scadenza maggiore di tutti, inserimento in coda
                    nuovo_lotto = (lotto_type*)malloc(sizeof(lotto_type));
                    nuovo_lotto->scadenza = input_scadenza;
                    nuovo_lotto->quantita = input_quantita;
                    nuovo_lotto->next = NULL;
                    prev_lotto->next = nuovo_lotto;
                }
            }
            //printf( "%d: rifornimento di %s %d g %d\n", tempo, cur_mat->nome,nuovo_lotto->quantita, nuovo_lotto->scadenza);
        }
    }

    printf( "rifornito\n");

    //verifica se possibile completare ordini in attesa
    cur_ordine_coda = coda_ordini_attesa->start;
    prev_ordine_coda = cur_ordine_coda;
    while (cur_ordine_coda != NULL){
        cur_ric = cur_ordine_coda->ricetta;
        cur_ing = cur_ric->ingredienti;
        peso_parziale = 0;
        peso_totale = 0;
        presente = false;
        q_sufficiente = false;
        while (cur_ing != NULL){ //verifica per ciascun ingrediente
            idx = hash_mag(cur_ing->nome);
            cur_mat = magazzino->slots[idx];
            presente = false;
            while (cur_mat != NULL){
                if (strcmp(cur_mat->nome, cur_ing->nome) == 0){
                    presente = true;
                    break;
                }
                cur_mat = cur_mat->next;
            }
            if (presente == false)//l'ingrediente richiesto non è presente nel magazzino, prova next ordine
                break;
            // l'ingrediente richiesto è presente nel magazzino
            quantita_necessaria = cur_ing->quantita * cur_ordine_coda->num_elem;
            //check per vedere se c'è sufficiente mat prima dell'ing. nel magazzino
            q_sufficiente = check_quantity_Magazzino(quantita_necessaria, cur_mat, tempo);
            if(q_sufficiente == false)//la quantita di mat non è suff, prova next ordine
                break;
            //la quantita nel magazzino è sufficiente per questo ingrediente
            peso_parziale += quantita_necessaria;
            //vai avanti con la verifica del prossimo ingrediente con relativa materia prima
            cur_ing = cur_ing->next;
        }
        if (presente == true && q_sufficiente == true){
            //prepara ordine
            peso_totale = peso_parziale;
            cur_ordine_coda->peso = peso_totale;
            make_Order(cur_ordine_coda, magazzino);

            //rimuovi ordine dalla coda ordini in attesa e aggiungi in quella dei pronti
            if (cur_ordine_coda == prev_ordine_coda){
                coda_ordini_attesa->start = cur_ordine_coda->next;
                //verifica se la coda è vuota, in tal caso modifica campo end
                if (coda_ordini_attesa->start == NULL)
                    coda_ordini_attesa->end = NULL;
                insert_ListaPronti(lista_ordini_pronti, cur_ordine_coda);
                //printf( "(%s pronto)\n", cur_ordine_coda->ricetta->nome);
                cur_ordine_coda = coda_ordini_attesa->start;
                prev_ordine_coda = cur_ordine_coda;
            }
            else {
                prev_ordine_coda->next = cur_ordine_coda->next;
                //verifica se cur è l'ultimo della coda, in tal caso modifica campo end
                if (cur_ordine_coda->next == NULL)
                    coda_ordini_attesa->end = prev_ordine_coda;
                insert_ListaPronti(lista_ordini_pronti, cur_ordine_coda);
                //printf( "(%s pronto)\n", cur_ordine_coda->ricetta->nome);
                cur_ordine_coda = prev_ordine_coda->next;
                //prev_ordine_coda rimane lo stesso
            }
        }
        else{
            prev_ordine_coda = cur_ordine_coda;
            cur_ordine_coda = cur_ordine_coda->next;
        }
    }
}

void fun_ordine(hash_table_ric *ricettario, hash_table_mag *magazzino, queue *coda_ordini_attesa,
                ordine_type **lista_ordini_pronti, u_int32_t tempo){
    char nome_ricetta[26], c = 'a';
    u_int32_t num_elem_ordinati = 0, idx, quantita_necessaria, peso_parziale = 0, peso_totale = 0;
    ric_type *cur_ric;
    ing_type *cur_ing;
    mat_type *cur_mat;
    ordine_type *nuovo_ordine;
    bool presente = false, q_sufficiente = false;

    if(scanf("%s", nome_ricetta));
    if(scanf("%d", &num_elem_ordinati));
    while (c != '\n')
        c = fgetc(stdin);
    //verifica presenza ricetta
    idx = hash_ric(nome_ricetta);
    cur_ric = ricettario->slots[idx];
    while (cur_ric != NULL){
        if (strcmp(cur_ric->nome, nome_ricetta) == 0) {
            presente = true; //è gia presente una ricetta col nome indicato
            break;
        }
        cur_ric = cur_ric->next;
    }
    if (presente == false) { //non è presente una ricetta col nome indicato
        
        printf( "rifiutato\n");
        return;
    }
    //è presente una ricetta col nome indicato
    //crea nuovo ordine
    nuovo_ordine = (ordine_type *) malloc(sizeof(ordine_type));
    nuovo_ordine->ricetta = cur_ric;
    nuovo_ordine->num_elem = num_elem_ordinati;
    nuovo_ordine->tempo_di_arrivo = tempo;
    nuovo_ordine->next = NULL;

    cur_ing = cur_ric->ingredienti;
    while (cur_ing != NULL){ //verifica per ciascun ingrediente
        idx = hash_mag(cur_ing->nome);
        cur_mat = magazzino->slots[idx];
        presente = false;
        while (cur_mat != NULL){
            if (strcmp(cur_mat->nome, cur_ing->nome) == 0){
                presente = true;
                break;
            }
            cur_mat = cur_mat->next;
        }
        if (presente == false){ //l'ingrediente richiesto non è presente nel magazzino
            //metti in coda d'attesa l'ordine
            insert_Queue(coda_ordini_attesa, nuovo_ordine);
            //printf( "accettato -> (%s in attesa: manca ingrediente)\n", nuovo_ordine->ricetta->nome);
            
            printf( "accettato\n");
            return;
        }
        // l'ingrediente richiesto è presente nel magazzino
        quantita_necessaria = cur_ing->quantita * num_elem_ordinati;
        //check per vedere se c'è sufficiente mat prima dell'ing. nel magazzino
        q_sufficiente = check_quantity_Magazzino(quantita_necessaria, cur_mat, tempo);
        if(q_sufficiente == false){
            //metti in coda d'attesa l'ordine
            insert_Queue(coda_ordini_attesa, nuovo_ordine);
            //printf( "accettato -> (%s in attesa: quantita insufficiente)\n", nuovo_ordine->ricetta->nome);
            
            printf( "accettato\n");
            return;
        }
        //la quantita nel magazzino è sufficiente per questo ingrediente
        //vai avanti con la verifica del prossimo ingrediente con relativa materia prima
        peso_parziale += quantita_necessaria;
        cur_ing = cur_ing->next;
    }
    //uscito dal while significa che ciascun ingrediente della ricetta è stato verificato
    //che abbia sufficiente materia prima nel magazzino
    peso_totale = peso_parziale;
    nuovo_ordine->peso = peso_totale;
    //ora bisogna cancellare i lotti o modificare le quantita dei lotti
    //relativi alle materie prime da usare
    make_Order(nuovo_ordine, magazzino);
    insert_ListaPronti(lista_ordini_pronti, nuovo_ordine);
    //printf( "accettato -> (%s pronto)\n", nuovo_ordine->ricetta->nome);
    
    printf( "accettato\n");
}

//-----------------------------------------------------------

int main() {
    u_int32_t tempo = 0;
    corriere_type corriere;
    char comando[25], c = 'a';

    //crea ricettario -> Hash Table
    hash_table_ric *ricettario = create_Ricettario();
    //crea magazzino -> Hash Table
    hash_table_mag *magazzino = create_Magazzino();
    //crea coda ordini in attesa
    queue *coda_ordini_attesa = create_Queue();
    //crea lista ordini pronti
    ordine_type *lista_ordini_pronti = NULL;


    if(scanf("%d", &(corriere.periodicita)));
    if(scanf("%d", &(corriere.capienza)));
    while (c != '\n')
        c = fgetc(stdin); // legge \n

    if(scanf( "%s", comando));
    while(!feof(stdin)){

        if (tempo != 0 && tempo % corriere.periodicita == 0){
            spedizione_Corriere(corriere, &lista_ordini_pronti);
        }

        if (strcmp(comando, "aggiungi_ricetta") == 0)
            fun_aggiungi_ricetta(ricettario);
        else if (strcmp(comando, "rimuovi_ricetta") == 0)
            fun_rimuovi_ricetta(ricettario, coda_ordini_attesa, &lista_ordini_pronti);
        else if (strcmp(comando, "rifornimento") == 0)
            fun_rifornimento(magazzino, coda_ordini_attesa, &lista_ordini_pronti, tempo);
        else if (strcmp(comando, "ordine") == 0)
            fun_ordine(ricettario, magazzino, coda_ordini_attesa, &lista_ordini_pronti, tempo);
        else
            return -1;

        if(scanf("%s", comando));
        tempo++;
    }

    if (feof(stdin)){

        if (tempo % corriere.periodicita == 0)
            spedizione_Corriere(corriere, &lista_ordini_pronti);
    }
    return 0;
}
