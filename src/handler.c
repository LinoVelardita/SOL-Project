#include "handler.h"


//handler della struct sigaction
void ter_handler(int sig){
    int n;
    if ((sig == SIGINT) || (sig == SIGQUIT)){
        n=2;
        if(write(pipeSigWriting, &n, sizeof(int)) != sizeof(int)){
            perror("write");
        }
        return;
    }
    if( sig == SIGHUP){
        n=1;
        if(write(pipeSigWriting, &n, sizeof(int)) != sizeof(int)){
            perror("write");
        }
        return;
    }

    ASSERT_EXIT(write(1, "unknown signalhandler_installer();", 15), <= 0);
}

void print_handler(int sig){
    ASSERT_EXIT(write(2, "segnale ricevuto dal worker: %d", sig), == 0);
    return;
}

void handler_installer(){

    sigset_t fullmask, handlermask, complementar;
    sigfillset(&fullmask);
    sigemptyset(&handlermask);
    sigfillset(&complementar);
	
	//imposto il nuovo signal mask usando fullmask 
    ASSERT_EXIT(pthread_sigmask(SIG_SETMASK, &fullmask, NULL), != 0);

    sigaddset(&handlermask, SIGINT);
    sigaddset(&handlermask, SIGQUIT);
    sigaddset(&handlermask, SIGHUP);
    sigdelset(&complementar, SIGINT);
    sigdelset(&complementar, SIGQUIT);
    sigdelset(&complementar, SIGHUP);

	//fullmask contiene tutti i segnali
	//handlermask contiene SIGNINT, SIGQUIT, SIGHUP
	//complementar contiene tutti esclusi SIGINT, SIGQUIT, SIGHUP
    
    struct sigaction sa;

	//inizializzo sa (copio il carattere 0 nei campi di "sa")
    ASSERT_EXIT(memset(&sa, 0, sizeof(struct sigaction)), == NULL);
    
    sa.sa_handler = ter_handler;	//handler
    sa.sa_mask = handlermask;	//sigset

    ASSERT_EXIT( sigaction(SIGINT, &sa, NULL), != 0);
    ASSERT_EXIT( sigaction(SIGQUIT, &sa, NULL), != 0);
    ASSERT_EXIT( sigaction(SIGHUP, &sa, NULL), != 0);

    ASSERT_EXIT(pthread_sigmask(SIG_SETMASK, &complementar, NULL), != 0);
}

void worker_handler_installer(){

    sigset_t fullmask;
    sigfillset(&fullmask);
    ASSERT_EXIT(pthread_sigmask(SIG_SETMASK, &fullmask, NULL), != 0);

}

