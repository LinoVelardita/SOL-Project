#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdarg.h>

#include "utils.h"
#include "server.h"
#include "datastructures.h"
#include "filemanager.h"
#include "handler.h"
#include "connection.h"

int pipeSigWriting; // from myhandler.h -> write-end della pipefd2
FILE * logger; // from myutil.h

typedef struct{
    SharedQueue_t * q;
    int pip_w_fd;
    FileSystem_t * fs;
} Wargs;

void logs(char * format, ... ){
    va_list arglist;
    va_start(arglist, format);
    vfprintf(logger, format, arglist);
    fprintf(logger, "\n");
    fflush(logger);
    va_end(arglist);
}


//funzione per leggere e salvare i parametri di configurazione
int parse(char* configpath,  char** socket_name, int* max_file_number, int* max_storage_size, int* threads_number,char** logspath){

    FILE * fptr;
    ASSERT_EXIT( fptr = fopen(configpath, "r"), ==  NULL);

	//variabili che puntano alla memoria allocata con malloc
    char * key, * val;

	//BUFFER_DIM = 64 (server.h)
    ASSERT_EXIT(key = malloc(BUFFER_DIM), == NULL);
    ASSERT_EXIT(val = malloc(BUFFER_DIM), == NULL);

	//memset riempie i primi "BUFFER_DIM" bytes di key con '/0'
	//e ritorna il puntatore di key
    while(memset(key, '\0', BUFFER_DIM) != NULL){
        if(fscanf(fptr, "%[^=]", key) != 1)
            break;
        fgetc(fptr); //"elimina" il carattere '='
        key[strnlen(key, BUFFER_DIM)] = '\0'; //aggiungo '/0' alla fine di key
        ASSERT_EXIT(memset(val, '\0', BUFFER_DIM), == NULL); //inizializzo val
        
        if (fscanf(fptr, "%[^\n]", val) != 1 ) 
            break;
        fgetc(fptr);//"elimino" il carattere '\n' (o EOF)
        val[strnlen(val, BUFFER_DIM)] = '\0';

		//parsing...        

        // socket name
        if (strncmp(key, SOCKET_NAME, strnlen(SOCKET_NAME, BUFFER_DIM)) == 0){
            ASSERT_EXIT(*socket_name = malloc(BUFFER_DIM), == NULL);
            ASSERT_EXIT(memset(*socket_name, '\0', BUFFER_DIM), == NULL);
            ASSERT_EXIT(strncpy(*socket_name, val, BUFFER_DIM), == NULL);
        }
        // max file number
        else if (strncmp(key, MAX_FILES, strnlen(MAX_FILES, BUFFER_DIM)) == 0){
            *max_file_number = atoi(val);
        }
        // max storage size
        else if (strncmp(key, MAX_STORAGE, strnlen(MAX_STORAGE, BUFFER_DIM)) == 0){
            *max_storage_size = atoi(val);
        }
        // threads number
        else if (strncmp(key, THREAD_NUMBERS, strnlen(THREAD_NUMBERS, BUFFER_DIM)) == 0){ 
            *threads_number = atoi(val);
        }
        // logger path
        else if (strncmp(key, LOG_PATH, strnlen(LOG_PATH, BUFFER_DIM)) == 0){  
            ASSERT_EXIT(*logspath = malloc(BUFFER_DIM), == NULL);
            ASSERT_EXIT(memset(*logspath, '\0', BUFFER_DIM), == NULL);
            ASSERT_EXIT(strncpy(*logspath, val, BUFFER_DIM), == NULL);
        }
        else {
            printf("Config Error: Parameter not recognized found!\n");
            return -1;
        }
    }

	//uscita con successo da while
    if(feof(fptr)){ 
		//free alloc
        free(key);
        free(val);
        fclose(fptr);
        return 0;
    }
	//errore nell'uscita (non sono arrivato alla fine del file)
    else{
        perror("Config Error: reading file");
        exit(-1);
    }
}



void * worker(void * args){
    
    worker_handler_installer();

    SharedQueue_t * q = ((Wargs *)(args))->q;
    int pip_w_fd = ((Wargs *)(args))->pip_w_fd;
    FileSystem_t * fs = ((Wargs *)(args))->fs;
    
    while(1){
        int client_fd = pop(q); 
        if(client_fd == -1) 
            return 0; 
        
        int requestType;

        if(read(client_fd, &requestType, sizeof(int)) <= 0 ){	//-1 -> errore, 0 -> fine del file
            ASSERT_EXIT(fs_request_manager(fs, client_fd, CLOSE_ALL), != 0);	//eseguo la CLOSE_ALL
            client_fd*=-1;	//metto il client_fd negativo (ho eseguito la richiesta)
            ASSERT_EXIT(write(pip_w_fd, &client_fd, sizeof(int)), != sizeof(int));	//scrivo sulla pipe
            continue;
        }

        int result = fs_request_manager(fs, client_fd, requestType);	//eseguo la richiesta del client
        
        if (result == 0){	//è andato tutto bene
            ASSERT_EXIT(write(pip_w_fd, &client_fd, sizeof(int)), != sizeof(int));	//scrivo sulla pipe
        }
        else {
            ASSERT_EXIT(fs_request_manager(fs, client_fd, CLOSE_ALL), != 0);	//eseguo la CLOSE_ALL
            client_fd*=-1; 
            ASSERT_EXIT(write(pip_w_fd, &client_fd, sizeof(int)), != sizeof(int));	//scrivo sulla pipe
        }
    }
}



int main(int argc, char ** argv){

	//configurazione segnali
    handler_installer();	//handler.c 

    if (argc < 2) ASSERT_EXIT("ERRORE PASSAGGIO DEI PARAMETRI",);
    
	//variabili usate per il parsing
    char* sck_name, *logs_path; 
    int max_num_file, max_dim_storage, num_thread_worker;

	//configurazione parametri
    parse(argv[1], &sck_name,  &max_num_file,  &max_dim_storage,  &num_thread_worker, &logs_path);
    ASSERT_EXIT(logger = fopen(logs_path, "w"), == NULL);

    logs("Configuration:\n \
            \tSocket name:%s\n\
            \tMax stockage dimension: %d\n\
            \tMax file number: %d\n\
            \tNumber of parallel threads: %d\n\
            \tLog file path: %s\n", \
            sck_name, max_dim_storage, max_num_file, num_thread_worker, logs_path
    );
    
    SharedQueue_t * ready_clients = SharedQueue();

	//pipefd[], pipefd2[] -> file descriptor della pipe
    int pipefd[2], pip_r_fd, pip_w_fd;
    int pipefd2[2], pipeSigReading;

    ASSERT_EXIT( pipe(pipefd), != 0);
    ASSERT_EXIT( pipe(pipefd2), != 0);

    pip_r_fd = pipefd[0];	//read-end della pipefd
    pip_w_fd = pipefd[1];	//write-end della pipefd
    pipeSigReading = pipefd2[0];	//read-end della pipefd2
    pipeSigWriting = pipefd2[1];   //write-end della pipefd2 (globale)
    
    FileSystem_t * fs = init_FileSystem(max_num_file, max_dim_storage);	//filemanager.c, riga 731

    pthread_t * tidArr;	//array di thread worker
    ASSERT_EXIT( tidArr = malloc(sizeof(pthread_t) * num_thread_worker), == NULL);
    
    Wargs args;
    args.pip_w_fd = pip_w_fd;
    args.q = ready_clients;
    args.fs = fs;

	//inizializzo i thread worker
    for(int i=0; i<num_thread_worker; ++i){
        pthread_t ret;
	    ASSERT_EXIT(pthread_create(&ret, NULL, worker, (void*)&args), != 0);
        tidArr[i] = ret;
    }

    int socket_fd = init_server(sck_name);//connection.c
    int active_clts = 0;
    fd_set tmpset, set;
    FD_ZERO(&tmpset);	//rimuovo tutti i file descriptor da tmpset
    FD_ZERO(&set);	//rimuovo tutti i file descriptor da set

	//aggiungo i fd socket_fd, pipeSigReading e pip_r_fd a set
    FD_SET(socket_fd, &set);
    FD_SET(pipeSigReading, &set);
    FD_SET(pip_r_fd, &set);

	//uso fd_max per scorrere tutti i file descriptor
    int fd_max = find_max(socket_fd, pip_r_fd, pip_w_fd, pipeSigReading, pipeSigWriting);
    fd_max++;
    int endMode=0;	//variabile di terminazione (SIGINT/SIGQUIT/SIGHUP)

    logs("Server Ready");

    while(endMode==0 || active_clts > 0){ 
        tmpset = set;        
        if( select(fd_max + 1, &tmpset, NULL, NULL, NULL) == -1){ 
            logs("Signal %d recieved", errno); 
        }
        //dopo che ho chiamato la select...
		//controllo che pipeSigReading sia ancora in tmpset
        if(FD_ISSET(pipeSigReading, &tmpset)){
			//lettura
            ASSERT_EXIT(read(pipeSigReading, &endMode, sizeof(int)), != sizeof(int));
            
            if(endMode == 1) {	//endmode==1 -> chiudo il server e servo i clienti rimanenti (SIGHUP)
                FD_CLR(socket_fd, &set); 
                fd_max = updatemax(set, fd_max);
                logs("Starting slow termination");
            }
            else if(endMode == 2){	//se endmode==2 -> chiudo il server e non gestisco i clienti rimanenti (SIGNINT o SIGQUIT)
                for(int i=0; i<fd_max+1; i++){ 
                    if(FD_ISSET(i, &set) && (i != pip_r_fd) && (i != socket_fd) && (i != pipeSigReading)){
                        close(i);
                        FD_CLR(i, &set);
                    }
                }
				//rimuovo socket_fd da set
                FD_CLR(socket_fd, &set);
                fd_max = updatemax(set, fd_max);
                active_clts = 0; //numero clienti da servire = 0 -> perchè ho ricevuto il SIGINT/SIGQUIT
                logs("Starting fast termination");
            }
            continue;
        }

        for (int i = 0; i< fd_max +1; i++){
            if(FD_ISSET(i, &tmpset)){ 
                if( i == socket_fd){   //ho un nuovo client
                    int newConnFd;
                    ASSERT_EXIT(newConnFd = accept(socket_fd, NULL, NULL), == -1);
                    FD_SET(newConnFd, &set);
                    active_clts++;
                    if(newConnFd > fd_max) fd_max = newConnFd;
                    logs("New client connection %d", newConnFd);
                }
                else if(i == pip_r_fd){ //ho una nuova richiesta da un client connesso
                    int conn_fd;
                    ASSERT_EXIT(read(pip_r_fd, &conn_fd, sizeof(int)), != sizeof(int));
                    
                    if ((endMode == 0) || (endMode == 1)){  
                        if(conn_fd < 0){
                            active_clts--;
                            conn_fd*=-1;   
                            close(conn_fd);
                        }
                        else{  
                            FD_SET(conn_fd, &set);
                            if(conn_fd > fd_max) fd_max = conn_fd;
                        }
                    }

                    else{  
                        if(conn_fd < 0){
                            active_clts--;   
                            close(-1 * conn_fd);
                        }
                        else{ 
                            active_clts--;   
                            close(conn_fd);                        
                        }
                    } 

                }
                else{ 
                    if (endMode != 2){ //client servito (posso mandare la risposta)
                        push(ready_clients, i); 
                        FD_CLR(i, &set);
                        if (i == fd_max) fd_max = updatemax(set, fd_max);
                    }
                    else{  
                        close(i);
                        FD_CLR(i, &set);
                        if (i == fd_max) fd_max = updatemax(set, fd_max);
                        active_clts--;
                    }
                }
            }//if(FD_ISSET(i, &tmpset)
        }//for
    }//while

    logs("Exit server cycle");
    stats(fs); 

    for (int i=0; i<num_thread_worker; i++){
        push(ready_clients, -1);
    }

	//termino i thread worker
    for (int i= 0; i< num_thread_worker; i++){
        ASSERT_EXIT(pthread_join(tidArr[i], NULL), == -1);
    }
        
	//chiudo le pipe
    close(pip_r_fd);
    close(pip_w_fd);
    close(pipeSigReading);
    close(pipeSigWriting);
    remove(sck_name);
    free(sck_name);
    
    free(logs_path);
    free(tidArr);
    
    free(ready_clients->set);

	//libero i mutex
    pthread_mutex_destroy(&ready_clients->lock);
    pthread_cond_destroy(&ready_clients->is_full);
    pthread_cond_destroy(&ready_clients->is_empty);
    free(ready_clients);
    deinit_FileSystem(fs);
    logs("Server Terminated");
    fclose(logger);

    return 0;
}



