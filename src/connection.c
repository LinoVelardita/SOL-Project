#include "connection.h"

int updatemax(fd_set set, int fdmax) {
    for(int i=(fdmax);i>=0;--i)
	if (FD_ISSET(i, &set)) return i;
    ASSERT_EXIT("error in updatemax", == NULL);
    return -1;
}

//inizializzo il server
int init_server(char * sck_name){

    ASSERT_EXIT(strnlen(sck_name, UNIX_PATH_MAX) >= UNIX_PATH_MAX,  != 0);

    sck_name[strnlen(sck_name, UNIX_PATH_MAX-1)] = '\0';

    int socket_fd;
    ASSERT_EXIT(socket_fd = socket(AF_UNIX, SOCK_STREAM, 0), == -1);

    struct sockaddr_un server_addr;
    ASSERT_EXIT(memset(&server_addr, 0, sizeof(struct sockaddr_un)), == NULL);

    server_addr.sun_family = AF_UNIX;
    ASSERT_EXIT(strncpy(server_addr.sun_path, sck_name, UNIX_PATH_MAX-1),  == NULL);

    ASSERT_EXIT(bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)), != 0);
    ASSERT_EXIT(listen(socket_fd, MAX_CONNECTION_QUEUE), != 0);
    return socket_fd; 
}

int find_max(int a, int b , int c, int d, int e){
    int max = 0;
    if(a>max) max = a;
    if(b>max) max = b;
    if(c>max) max = c;
    if(d>max) max = d;
    if(e>max) max = e;
    return max;

}
