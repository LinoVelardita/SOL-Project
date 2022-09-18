#include "serverAPI.h"
#include <sys/stat.h> //libreria che contiene la structure 'stat'
#include <sys/types.h>
#include <dirent.h>	//Per streaming dati di directory
#include <time.h>
#include <stdarg.h>

#define HELP "-h for help"

//extern int socketfd, err_no;
extern char __sockname[UNIX_PATH_MAX];	//nome socket AF_UNIX

char ret_dir[MAX_PATH] = "";	//cartella di ritorno per le capacity misses (opzione -D)
char read_dir[MAX_PATH] = "";	//cartella dove scrivo i file letti DAL server  (opzione -d)
int delay = 0, foundp = 0, r;
//delay -> valore dell'opzione -t
//foundp -> valore dell'opzione -p
//r -> variabile in cui memorizzo i risultati delle richieste inviate al server


//funzione associata all'opzione -p
//se -p non è presente -> non stampa (return)
void logs(char * format, ... ){
    if(foundp == 0) return;
    va_list arglist;
    va_start(arglist, format);
    vfprintf(stderr, format, arglist);
    fprintf(stderr, "\n");
    va_end(arglist);
}

int is_dot(const char dir[]) {
  int l = strlen(dir);
  
  if ( (l>0 && dir[l-1] == '.') ) return 1;
  return 0;
}

//funzione associata all'opzione -w dirname[,n=k]
void cmd_ls(const char nomedir[], int * n) {
    struct stat statbuf;
    
	//int stat(char* path, struct stat* buf)
	//legge i dati sul file path e li copia su buf,
	//ritorna 0 in caso di successo (controllo che sia una directory)
    ASSERT_EXIT(stat(nomedir,&statbuf),!= 0);

    DIR * dir;
    
    if ((dir=opendir(nomedir)) == NULL) {
        perror("opendir");
        printf("Error opening directory %s\n", nomedir);
        return;
    } 
    
	//tiene traccia del serial number e del nome
    struct dirent *file;
    
    while(((errno=0, file = readdir(dir)) != NULL) && *n != 0 ) {
        struct stat statbuf;
        char filename[MAX_PATH]; 
        int len1 = strlen(nomedir);
        int len2 = strlen(file->d_name);
        if ((len1+len2+2)>MAX_PATH) {
            fprintf(stderr, "ERROR: MAXFILENAME too small\n");
            exit(EXIT_FAILURE);
        }	    
        strncpy(filename,nomedir, MAX_PATH-1);//copio nomedir in filename
        strncat(filename,"/", MAX_PATH-1); //concateno a '/'
        strncat(filename,file->d_name, MAX_PATH-1); //concateno alla sottocartella o files
        
        if (stat(filename, &statbuf)==-1) {
            perror("by executing the stat");
            printf("Error in the file %s\n", filename);
            return;
        }
		//se è una cartella faccio la ricorsione
        if(S_ISDIR(statbuf.st_mode)) {	//visita ricorsiva
            if ( !is_dot(filename) ) //is_dot -> riga 28
                cmd_ls(filename, n); //cmd_ls -> riga 34
        } 
		//ramo else -> è un file
        else {

            (*n)--;
			//PIE -> server.api riga 73
            PIE(openFile(filename, O_CREATE | O_LOCK));   // creo il file nel server
            if(strcmp(ret_dir, "") == 0){ // cartella di ritorno non specificata
                PIE(r = writeFile(filename, NULL));
                logs("Writing the %s file to the server without saving the evicted files, result: %d", filename, r);
            }
            else{
                PIE(r = writeFile(filename, ret_dir));
                logs("Writing the %s file to the server and saving the evicted files in %s, outcome: %d", filename, ret_dir, r);
            }
        }
        
    }
    if (errno != 0) perror("read_dir");
    closedir(dir);
    
}
int is_comand(char * str){
	//Deve essere nel formato "-A" (trattino e una lettera)
    if((str != NULL) && (str[0] == '-') && ((str + 1) != NULL)  && (str[2] == '\0') ){
        return 1;
    }
    return 0;
}
char to_comand(char * str){
	//Ritorno solo la lettera (elimino '-')
    return str[1];
}


void remove_file(char * str){

    char file[MAX_PATH];
    int pos =0;

    while(1){
        
        if(str[pos] == '\0'){ //sono arrivato all'ultimo file
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = removeFile(file));
            logs("Removed the %s file with result %d", file, r);
            return;
        }
        else if(str[pos] == ','){ //ci sono altri file 
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = removeFile(file));
            logs("Removed the %s file with result %d", file, r);
            str = str + pos + 1;
            pos = 0;
        }
        else{
            pos++;
        }
    }

}
void lock_file(char * str){

    char file[MAX_PATH];
    int pos = 0;

    while(1){
        
        if(str[pos] == '\0'){	//sono arrivato all'ultimo file
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = lockFile(file));
            logs("Lock of the %s file with result %d", file, r);
            return;
        }
        else if(str[pos] == ','){	//ci sono altri file
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = lockFile(file));
            logs("Lock of the %s file with result %d", file, r);
            str = str + pos + 1;
            pos = 0;
        }
        else{
            pos++;
        }
    }

}
void unlock_file(char * str){

    char file[MAX_PATH];
    int pos = 0;

    while(1){
        
        if(str[pos] == '\0'){ //sono arrivato all'ultimo file
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = unlockFile(file));
            logs("Unlock of the %s file with result %d", file, r);
            return;
        }
        else if(str[pos] == ','){ //ci sono altri file
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, 0));
            PIE(r = unlockFile(file));
            logs("Unlock of the %s file with result %d", file, r);
            str = str + pos + 1;
            pos = 0;
        }
        else{
            pos++;
        }
    }

}
void read_file(char * str){
    
    char returnPath[MAX_PATH], file[MAX_PATH];
    int pos =0;

    while(1){
        void * buf = NULL; size_t size; FILE * fileptr;
        int r;
        
        if(str[pos] == '\0'){
            strncpy(file, str, pos +1);
            
            PIE(r = readFile(file, &buf, &size));
            logs("Reading of %s file of %d bytes with result %d", file,size, r);
            if(r == -1) {return;}

            if(strcmp(read_dir, "") != 0){ //cartella in cui scrivere i file letti specificata 
                strcpy(returnPath, read_dir);
				//onlyName -> serverAPI.c, riga 705
				//file + onlyName(file) -> nuovo path del file
                strcat(returnPath, (file+onlyName(file) + 1));
                ASSERT_EXIT( fileptr = fopen(returnPath, "wb"), == NULL);
                ASSERT_EXIT(fwrite(buf, 1, size, fileptr), != size);
                logs("Saving the %s file in the %s", file + onlyName(file) +1, read_dir);
                free(buf);
            }

            return;
        }
        else if(str[pos] == ','){	//c'è almeno un altro file da leggere
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            
            PIE(r = readFile(file, &buf, &size));
            logs("Reading of %s file of %d bytes with result %d", file,size, r);

            if(strcmp(read_dir, "") != 0){ //cartella in cui scrivere i file letti specificata
                strcpy(returnPath, read_dir);
				//file + onlyName(file) -> nuovo path del file
                strcat(returnPath, (file+onlyName(file) + 1));
                ASSERT_EXIT( fileptr = fopen(returnPath, "wb"), == NULL);
                ASSERT_EXIT(fwrite(buf, 1, size, fileptr), != size);
                logs("Saving the %s file in %s", file + onlyName(file) +1, read_dir);
                free(buf);
            }

            str = str + pos + 1;	//metto "str" sulla prima lettera dell'argomento successivo
            pos = 0;
        }
        else{
            pos++;
        }
    }
}

void read_n_file(char * str){

    int n;
    if(str == NULL || is_comand(str)) n=0; // devo leggere tutti i file (non ho passato n come argomento di -R)
    else{
        str += 2;//sposto str sul valore k passato come argomento (n=k)
        n = atoi(str);
    }

    if(strcmp(read_dir, "") == 0){
        fprintf(stderr, "read_dir not initialised\n");
        return;
    }

    PIE(r = readNFiles(n, read_dir));
    logs("Reading %d files with result %d", n, r);

}

void send_file(char str[]){ 

    char file[MAX_PATH];
    int pos =0;

    while(1){
        
        if(str[pos] == '\0'){ //sono arrivato all'ultimo file
            strncpy(file, str, pos +1);
            PIE(r = openFile(file, O_CREATE | O_LOCK));
            logs("Opening of file %s with flag %d, result %d", file, O_CREATE | O_LOCK, r);
            if(strcmp(ret_dir, "") == 0){ // cartella di ritorno non specificata
                PIE(r = writeFile(file, NULL));
                logs("Writing the %s file to the server without saving the evicted files, result: %d", file, r);
            }
            else{
                PIE(r = writeFile(file, ret_dir));
                logs("Writing the %s file to the server and saving the evicted files in %s, result: %d", file, ret_dir, r);
            }
            
            return;
        }
        else if(str[pos] == ','){ //ci sono altri file
            str[pos] = '\0';
            strncpy(file, str, pos +1);
            PIE(openFile(file, O_CREATE | O_LOCK));
            if(strcmp(ret_dir, "") == 0){ // cartella di ritorno non specificata
                PIE(r = writeFile(file, NULL));
                logs("Writing the %s file to the server without saving the evicted files, result: %d", file, r);
            }
            else{
                PIE(r = writeFile(file, ret_dir));
                logs("Writing the %s file to the server and saving the evicted files in %s, result: %d", file, ret_dir, r);
            }
            str = str + pos + 1;	//sposto str sull'argomento successivo alla virgola
            pos = 0;
        }
        else{
            pos++;
        }
    }

}

void send_dir(char * str){

    if(strlen(str) > MAX_PATH -1){printf("error"); return;}

    int n = 0, virpos = 0;
    char dirname[MAX_PATH]; 
    
	//ciclo per arrivare alla fine o ad una virgola
    while (str[virpos] != '\0' && str[virpos] != ','){
        virpos++;
    }
	//'virpos' mi permette di prendere solo l'argomento dell'opzione -w
	//copio 'virpos' caratteri di str nella stringa dirname
    strncpy(dirname, str, virpos);
    dirname[virpos] = '\0'; 
    if(str[virpos] == ','){	//se sono arrivato ad una virgola
        str = str + virpos + 3;	// -w,n=k -> metto str su 'k'  
        if(str != NULL){ 
            n = atoi(str);
        }
        else{
            printf("error in number of files");
			printf(HELP);
            return;
        }
    }

    if (n<=0) n=-1;

	//controllo che 'dirname' sia una directory
    struct stat statbuf;
    if (stat(dirname,&statbuf)!= 0 || !S_ISDIR(statbuf.st_mode)){
	    fprintf(stderr, " \"%s\" not a directory\n", dirname);
		printf(HELP);
        return;
    }    

    cmd_ls(dirname, &n); //cmd_ls -> riga 40

}

int main(int argc, char ** argv){
    if(argc == 1){	//Se non passo argomenti al client -> chiudo
        return 0;
    }

    int foundf = 0, i = 1, foundW = 0, foundD = 0, foundr = 0, foundw = 0, foundd = 0, foundR =0;
    char comand;

    while (i < argc){	//scorro argv salvandomi in delle variabili le opzioni passate da terminale
        if(is_comand(argv[i])){		//
            comand = to_comand(argv[i]);
            switch (comand){	//CONTROLLO DEI COMANDI PASSATI DA TERMINALE
            case 'h':
				printf("\n => usage: %s\n   -h <help> -f <filename> -w <dirname[,n=0]>\n   -W <file1[,file2]> -r <file1,[,file2] -R <int>\n   -d <dirname> -t <time> -l <file1[,file2]>\n   -u <file1[,file2]> -c <file1[,file2]> -p\n", argv[0]);
                return 0;
            case 'p':
                foundp++;
                break;
            case 'f':
                foundf++;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-f option not used correctly\n");
                    return 0;
                }
                break;
            case 'w':
                foundw = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-w option not used correctly\n");
                    return 0;
                }
                break;
            case 'W':
                foundW = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-W option not used correctly\n");
                    return 0;
                }
                break;
            case 'D':
                foundD = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-D option not used correctly\n");
                    return 0;
                }
                break;
            case 'r':
                foundr = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-r option not used correctly\n");
                    return 0;
                }
                break;
            case 'd':
                foundd = 1;
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-d option not used correctly\n");
                    return 0;
                }
                break;
            case 'R':
                foundR = 1;
                break;
            case 'l':
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-l option not used correctly\n");
                    return 0;
                }
                break;
            case 'u':
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-u option not used correctly\n");
                    return 0;
                }
                break;
            case 'c':
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-c option not used correctly\n");
                    return 0;
                }
                break;
            case 't':
                if( argv[i+1] == NULL || is_comand(argv[i+1])){
                    fprintf(stderr, "-t option not used correctly\n");
                    return 0;
                }
                delay = atoi(argv[i+1]);
                break;
            }
        }
        i++;
    }

	//Controllo che le opzioni vengano usate correttamente
    if(foundD == 1 && (foundW + foundw) <= 0){	//-D deve essere usato insieme a -w o -W
        fprintf(stderr, "Option -D not used correctly\n");
        return 0;
    }
    if(foundd == 1 && (foundR + foundr) <= 0){	//-d deve essere usato insieme a -r o -R
        fprintf(stderr, "Option -d not used correctly\n");
        return 0;
    }
    if(foundp > 1){
        fprintf(stderr, "Option -p not used correctly\n");
        return 0;
    }
    if(foundf != 1){
        fprintf(stderr, "Option -f not used correctly\n");
        return 0;
    }
    if(delay < 0){
        fprintf(stderr, "Option -t not used correctly\n");	//il delay non può essere negativo
        return 0;
    }
    
    i = 1;

	//nel secondo ciclo eseguo le opzioni
    while (i < argc){
        
        comand = to_comand(argv[i]);
        struct timespec abstime;
        switch (comand){
        case 'f':
            
            abstime.tv_sec = 10; //aspetto che il server si avvii
            abstime.tv_nsec = 0; 
            PIE(openConnection(argv[i+1], 500, abstime));   // ogni 500 ms  //argv[i+1] nome del socket AF_UNIX
            logs("Open a connection with socket: %s", argv[i+1]);
            i++; //Devo saltare il parametro di "-f"
            msleep(delay); //il delay passato con l'opzione -t
            break;
		case 'D':
            strcpy(ret_dir, argv[i+1]);	//specifico la cartella di ritorno nella variabile globale "ret_dir"
            logs("Cache save folder updated in: %s", ret_dir);
            i++;
            break;
        case 'w':
            send_dir(argv[i+1]);	//riga 313
            i++;
            msleep(delay);
            break;
        case 'W':
            send_file(argv[i+1]);	//riga 269
            i++;
            msleep(delay);
            break;
        case 'd':
            strcpy(read_dir, argv[i+1]); //specifico la cartella in cui salvare i file letti con -r o -R
            logs("Reading folder updated in: %s", read_dir);
            break;
		case 'r':
            read_file(argv[i+1]); //riga 200
            i++;
            msleep(delay);
            break;
        case 'R':
            read_n_file(argv[i+1]); //riga 251
            msleep(delay);
            break;
        case 'l':
            lock_file(argv[i+1]);	//riga 142
            i++;
            msleep(delay);
            break;
        case 'u':
            unlock_file(argv[i+1]);	//riga 171
            i++;
            msleep(delay);
            break;
        case 'c':
            remove_file(argv[i+1]);	//riga 113
            i++;
            msleep(delay);
            break;
        }   
            
        i++;
    }

	//chiudo la connessione con il server
    logs("Close the connection with the server and terminate");
    PIE(closeConnection(__sockname));
    
    return 0;

}









