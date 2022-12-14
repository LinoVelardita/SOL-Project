#include "serverAPI.h"
#include <time.h>


int socketfd = 0, myerrno;
char __sockname[UNIX_PATH_MAX];

extern int foundP;

int readNFiles(int n, char * dirname){

    if(dirname == NULL){
        myerrno = E_BAD_RQ; //E_BAD_RQ = 145
        return -1;
    }

    char pth[MAX_PATH], pthToSave[MAX_PATH];
    
    int reqType = READ_N_F, resp = 0;

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }
    
    if(writen(socketfd, &n, sizeof(int)) != sizeof(int)){  // scrivo il numero di file
        myerrno = errno;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }
    if(resp != 1){ // c'è stato un errore (l'unica risposta corretta è 1)
        myerrno = resp;
        return -1;
    }

    FILE* outFile; int size; void * buf;
    while(1){

        if(readn(socketfd, &size, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la size, se è -1 ho finito
        if(size == -1){
            break;
        }
        
        if(readn(socketfd, pth, MAX_PATH) != MAX_PATH){ myerrno = errno; return -1;} // leggo il path

        ASSERT_EXIT(buf = malloc(size), == NULL); // alloco il buffer
        
        if(readn(socketfd, buf, size) != size){ myerrno = errno; free(buf); return -1;} // leggo il contenuto
        
        strcpy(pthToSave, dirname);
        strcat(pthToSave, pth + onlyName(pth) );
        if (( outFile = fopen(pthToSave, "wb")) == NULL){
            myerrno = errno;
            return -1;
        }
        
        if(fwrite(buf, 1, size, outFile) != size){  // scrivo contenuto su file
            myerrno = errno;
            free(buf);
            fclose(outFile);
            return -1;
        }

        free(buf);
        fclose(outFile);
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta finale 
        myerrno = errno;
        return -1;
    }

    if(resp != 0){
        myerrno = resp;
        return -1;
    }
    else{
        return 0;
    }


}

int readFile(char * pathname, void ** buf, size_t * size){


    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }
    
    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = READ_F, resp = 0;

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(writen(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp != 1){
        myerrno = resp;
        return -1;
    }

    int i_size;
    char * i_buf;

    if(readn(socketfd, &i_size, sizeof(int)) != sizeof(int)){  // leggo la size
        myerrno = errno;
        return -1;
    }

    ASSERT_EXIT(i_buf = malloc(i_size), == NULL); // alloco il buffer

    if(readn(socketfd, i_buf, i_size) != i_size){  // leggo il contenuto
        free(i_buf);
        myerrno = errno;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        free(i_buf);
        myerrno = errno;
        return -1;
    }

    if(resp != 0){
        free(i_buf);
        myerrno = resp;
        return -1;
    }
    else {	//se è andato tutto bene...
        *buf = i_buf;
        *size = i_size;
        return 0;
    }
}

int removeFile(char * pathname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;//not connected
        return -1;
    }

    if(pathname == NULL){ myerrno = E_INV_PTH; return -1;}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = REMOVE_F, resp = 0;

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(writen(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int closeFile(char * pathname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;//not connected
        return -1;
    }

    if(pathname == NULL){
		myerrno = E_INV_PTH;
		return -1;
	}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = CLOSE_F, resp = 0;

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(writen(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int unlockFile(char * pathname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;//not connected
        return -1;
    }

    if(pathname == NULL){
		myerrno = E_INV_PTH;//invalid path (null)
		return -1;
	}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = UNLOCK_F, resp = 0;

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(writen(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int lockFile(char * pathname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;
        return -1;
    }

    if(pathname == NULL){
		myerrno = E_INV_PTH; return -1;
	}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = LOCK_F, resp = 0;

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(writen(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int appendToFile(char * pathname, void * buf, size_t size, char * dirname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;//not connected
        return -1;
    }
    
    if(pathname == NULL){
		myerrno = E_INV_PTH;//invalid path(null)
		return -1;
	}

    if(buf == NULL || size < 1){
		myerrno = E_BAD_RQ;//bad request
		return -1;
	}

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = APPEND_T_F, resp = 0;

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(writen(socketfd, &size, sizeof(int)) != sizeof(int)){  // scrivo la size
        myerrno = errno;
        return -1;
    }
    
    if(writen(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }
    
    if(writen(socketfd, buf, size) != size){  // scrivo il buffer
        myerrno = errno;
        return -1;
    }
 
    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){  // non devo fare altro
        return 0;
    }

    if(resp != 1){  // devo ricevere file
        myerrno = resp;
        return -1;
    }

    // lettura file di risposta
    
    if(resp == 1){ // devo leggere N file: size path cont
        FILE* outFile;
        char pthToSave[MAX_PATH];
        while(1){

            if(readn(socketfd, &size, sizeof(int)) != sizeof(int)){ // leggo la size, se è -1 ho finito
				myerrno = errno;
				return -1;
			}
            if(size == -1){
                break;
            }
            
            if(readn(socketfd, pth, MAX_PATH) != MAX_PATH){ myerrno = errno; return -1;} // leggo il path

            ASSERT_EXIT(buf = malloc(size), == NULL); // alloco il buffer
            
            if(readn(socketfd, buf, size) != size){ myerrno = errno; free(buf); return -1;} // leggo il contenuto

            if(dirname != NULL){

                strcpy(pthToSave, dirname);
                strcat(pthToSave, pth + onlyName(pth) );
                if (( outFile = fopen(pthToSave, "wb")) == NULL){
                    myerrno = errno;
                    return -1;
                }
                if (( outFile = fopen(pthToSave, "wb")) == NULL){
                    myerrno = errno;
                    return -1;
                }
                
                if(fwrite(buf, 1, size, outFile) != size){  // scrivo contenuto su file
                    myerrno = errno;
                    free(buf);
                    fclose(outFile);
                    return -1;
                }
                fclose(outFile);
            }
            free(buf);
            
        }
    }
    
    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;}

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }
    
}

int writeFile(char* pathname, char * dirname){

    if(socketfd == 0){
        myerrno = E_NOT_CON;//not connected
        return -1;
    }

    if(pathname == NULL){
        myerrno = E_INV_PTH;
        return -1;
    }

    char pth[MAX_PATH], *buf;
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = WRITE_F, resp = 0;
    FILE * inFile;

    if((inFile = fopen(pth, "rb")) == NULL){
        myerrno = errno;
        fprintf(stderr,"impossibile aprire file\n");
        return -1;
    }
 
    // leggo il file
    int size = 0;

    fseek(inFile, 0, SEEK_END);//setto il file pointer alla fine del file
    size = ftell(inFile); //ftell ritorna la posizione corrente del file pointer
    fseek(inFile, 0, SEEK_SET);//rimetto il file pointer all'inizio del file
    ASSERT_EXIT(buf = malloc(size), == NULL); //alloco buff
    if( fread(buf, 1, size, inFile) != size){
        fprintf(stderr, "impossibile leggere il file\n");
        myerrno = errno;
        free(buf);
        return -1;
    }
    
    fclose(inFile);

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }
    if(writen(socketfd, &size, sizeof(int)) != sizeof(int)){  // scrivo la size
        myerrno = errno;
        return -1;
    }
    if(writen(socketfd, pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }
    if(writen(socketfd, buf, size) != size){  // scrivo il contenuto
        myerrno = errno;
        return -1;
    }
    free(buf);

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }
    if(resp == 0){
        return 0;
    }

    if(resp != 1){
        myerrno = resp;
        return -1;
    }
    // lettura file di risposta
    
    if(resp == 1){ // devo leggere N file: size path cont
        FILE* outFile; char pthToSave[MAX_PATH];
        while(1){

            if(readn(socketfd, &size, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la size, se è -1 ho finito
            if(size == -1){
                break;
            }
            
            if(readn(socketfd, pth, MAX_PATH) != MAX_PATH){ myerrno = errno; return -1;} // leggo il path

            ASSERT_EXIT(buf = malloc(size), == NULL); // alloco il buffer
            
            if(readn(socketfd, buf, size) != size){ myerrno = errno; free(buf); return -1;} // leggo il contenuto

            if(dirname != NULL){
                strcpy(pthToSave, dirname);
                strcat(pthToSave, pth + onlyName(pth) );
                
                if (( outFile = fopen(pthToSave, "wb")) == NULL){
                    myerrno = errno;
                    return -1;
                }
                if (( outFile = fopen(pthToSave, "wb")) == NULL){
                    myerrno = errno;
                    return -1;
                }
                
                if(fwrite(buf, 1, size, outFile) != size){  // scrivo contenuto su file
                    myerrno = errno;
                    free(buf);
                    fclose(outFile);
                    return -1;
                }
                fclose(outFile);
            }

            free(buf);
            

        }
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else{
        myerrno = resp;
        return -1;
    }

}

int openFile(const char* pathname, int flags){

    if(socketfd == 0){
        myerrno = E_NOT_CON; //not connected
        return -1;
    }
    
    if(flags < 0 || flags > 3) {	//1 -> O_CREATE, 2 -> O_LOCK
        myerrno = E_INV_FLG; //invalid flag
        return -1;
    }

    char pth[MAX_PATH];
    strncpy(pth, pathname, MAX_PATH);
    pth[MAX_PATH-1] = '\0';
    int reqType = OPEN_F; //OPEN_F = 1
    int resp = 0;

    if(pth == NULL){
        myerrno = E_INV_PTH; //invalid path error
        return -1;
    }

    if(writen(socketfd, &reqType, sizeof(int)) != sizeof(int)){  // scrivo il tipo di richesta
        myerrno = errno;
        return -1;
    }

    if(writen(socketfd, &pth, MAX_PATH) != MAX_PATH){  // scrivo il path
        myerrno = errno;
        return -1;
    }

    if(writen(socketfd, &flags, sizeof(int)) != sizeof(int)){  // scrivo i flag
        myerrno = errno;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){  // leggo la risposta
        myerrno = errno;
        return -1;
    }

    if(resp == 0){
        return 0;
    }
    else if (resp != 1){
        myerrno = resp;
        return -1;
    }

    // c'è un file da salvare

    int size; void * buf;
    if(readn(socketfd, &size, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la size
    if(readn(socketfd, pth, MAX_PATH) != MAX_PATH){ myerrno = errno; return -1;} // leggo il path

    ASSERT_EXIT(buf = malloc(size), == NULL); // alloco il buffer
            
    if(readn(socketfd, buf, size) != size){ myerrno = errno; free(buf); return -1;} // leggo il contenuto

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la risposta di fine file

    if(resp != -1){     // è successo qualcosa di strano
        myerrno = E_BAD_RQ;
        return -1;
    }

    if(readn(socketfd, &resp, sizeof(int)) != sizeof(int)){ myerrno = errno; return -1;} // leggo la risposta finale 

    if(resp != 0){     // è successo qualcosa di strano
        myerrno = resp;
        return -1;
    }
    else{
        return 0;
    }
}


int openConnection(const char* sockname , int msec, const struct timespec abstime){
    
    if (sockname == NULL || strnlen(sockname, UNIX_PATH_MAX) >= UNIX_PATH_MAX) {
        myerrno = E_INV_SCK; //invalid socket name
        return -1;
    }
    strcpy(__sockname, sockname); //__sockname -> globale
    struct sockaddr_un server_addr;
    socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(socketfd == -1) {
        myerrno = errno;
        return -1;
    }
    if (memset(&server_addr, 0, sizeof(server_addr)) == NULL) return -1;

    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, sockname);

    struct timespec toWait, _abstime = abstime;
    ms2ts(&toWait, msec);

    while(_abstime.tv_nsec > 0 || _abstime.tv_sec > 0){
        if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
            perror("Tentativo di connessione fallito");
        }
        else{
            return 0;
        }
        nanosleep(&toWait, NULL);
        timespec_diff(&_abstime, &toWait, &_abstime);
    }

    myerrno = errno;
    return -1;

    return 0;

}
void timespec_diff(struct timespec *a, struct timespec *b, struct timespec *result) {
    result->tv_sec  = a->tv_sec  - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        --result->tv_sec;
        result->tv_nsec += 1000000000L;
    }
}

void ms2ts(struct timespec *ts, unsigned long ms){
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = (ms % 1000) * 1000000;
}

void msleep(int msec){
    struct timespec toWait;
    ms2ts(&toWait, msec);
    nanosleep(&toWait, NULL);
}


int closeConnection(const char* sockname){

    if(socketfd == 0){
        myerrno = E_NOT_CON; //error not connected
        return -1;
    }

    if (sockname == NULL || (strncmp(__sockname, sockname, UNIX_PATH_MAX) != 0)) {
        myerrno = E_INV_SCK; //invalid socket name
        return -1;
    }
    close(socketfd);
    socketfd = 0;
    return 0;
}

//ritorna la posizione dell'ultimo carattere '/', 0 altrimenti
int onlyName(char * str){ 
    int n = 0, last = 0;
    while(n < strlen(str)){
        if(str[n] == '/') last = n;
        n++;
    }
    return last;
}

int readn(int fd, void *ptr, size_t n) {  
    size_t   nleft;//byte rimanenti da leggere
	//uso nread perchè read mi ritorna un ssize_t
	//nread conterrà il numero di byte letti (success)
    ssize_t  nread;

    nleft = n;
    while (nleft > 0) {
        if((nread = read(fd, ptr, nleft)) < 0) {
        	if (nleft == n) return -1; /* error, return -1 */
        	else break;
    	}
		else if (nread == 0) break; // EOF
    
    	nleft -= nread;
    	ptr   = (void*)((char*)ptr + nread);
    }
    return(n - nleft); // return >= 0
}

int writen(int fd, void *ptr, size_t n) {  
    size_t   nleft;//byte rimanenti
	//uso nwritten perchè write mi ritorna un ssize_t
	//nwritten conterrà il numero di byte scritti(in caso di successo)
	//-1 altrimenti
    ssize_t  nwritten;	//range -1..MAX_INT 

    nleft = n;//inizializzo nleft al massimo numero di byte che posso leggere
    while (nleft > 0) { //fin quando ho qualcosa da leggere...
        if((nwritten = write(fd, ptr, nleft)) < 0) { //la write mi ha restituito -1
        	if (nleft == n) return -1; /* error, return -1 */
        	else break;
        }
		else if (nwritten == 0) break; //EOF
        nleft -= nwritten;
		//incremento la posizione del puntatore sul buffer
        ptr = (void*)((char*)ptr + nwritten);
    }
    return(n - nleft); // return >= 0
}


//my error
void myperror(const char * str){
    if(myerrno >= 140){
        fprintf(stderr, "Error in \"%s\": ", str);

        switch (myerrno)
        {
        case E_INV_FLG:
            fprintf(stderr, "Invalid flags\n");
            break;
        case E_INV_PTH:
            fprintf(stderr, "Invalid path\n");
            break;
        case E_LOCK:
            fprintf(stderr, "File locked\n");
            break;
        case E_NOT_EX:
            fprintf(stderr, "File not exists\n");
            break;
        case E_ALR_EX:
            fprintf(stderr, "File already exists\n");
            break;
        case E_BAD_RQ:
            fprintf(stderr, "Invalid request\n");
            break;
        case E_ALR_LK:
            fprintf(stderr, "File already locked\n");
            break;
        case E_NO_SPACE:
            fprintf(stderr, "No enough space (in server memory)\n");
            break;
        case E_NOT_OPN:
            fprintf(stderr, "File not open\n");
            break;
        case E_INV_SCK:
            fprintf(stderr, "Invalid socket path\n");
            break;
        case E_NOT_CON:
            fprintf(stderr, "Not connected\n");
            break;
        default:
            fprintf(stderr, "Unkown error\n");
            break;
        }
        
    }
    else{
        perror(str);
    }
}
