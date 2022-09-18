
flags = -g -O3 -Wall -I ./includes -pedantic #-Werror

obj = ./obj/server.o  ./obj/datastructures.o ./obj/connection.o ./obj/handler.o ./obj/filemanager.o
src = ./src/server.c ./src/datastructures.c ./src/connection.c ./src/handler.c ./src/filemanager.c
includes = ./includes/connection.h ./includes/handler.h ./includes/datastructures.h ./includes/utils.h ./includes/filemanager.h

objpath = ./obj/
srcpath = ./src/
libpath = ./lib/

.PHONY: clean cleanall test1 test2 test3

#all

all: ./server ./client

#linking

./server: $(obj)
	gcc $(obj) -o $@ -pthread

./client: $(objpath)client.o $(libpath)libServerApi.a
	gcc $< -o $@ -L $(libpath) -lServerApi

#obj file for server

$(objpath)server.o: $(srcpath)server.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)datastructures.o: $(srcpath)datastructures.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)filemanager.o: $(srcpath)filemanager.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)connection.o: $(srcpath)connection.c $(includes)
	gcc $< $(flags) -c -o $@

$(objpath)handler.o: $(srcpath)handler.c $(includes)
	gcc $< $(flags) -c -o $@ -pthread


#obj file for client

$(objpath)client.o: $(srcpath)client.c $(includes)
	gcc $< $(flags) -c -o $@

$(libpath)libServerApi.a: $(objpath)serverAPI.o
	ar rvs $(libpath)libServerApi.a $(objpath)serverAPI.o

$(objpath)serverAPI.o: $(srcpath)serverAPI.c $(includes)
	gcc $< $(flags) -c -o $@


cleanall	:
	./clean.sh


test1	: ./server ./client
	mkdir -p tests/fileLetti
	mkdir -p tests/fileEvict
	chmod +x tests/test1.sh
	./tests/test1.sh

test2	: ./server ./client
	mkdir -p tests/fileLetti
	mkdir -p tests/fileEvict
	chmod +x tests/test2.sh
	./tests/test2.sh

test3	: ./server ./client
	mkdir -p tests/fileLetti
	mkdir -p tests/fileEvict
	chmod +x tests/test3.sh
	chmod +x tests/test3support.sh
	./tests/test3.sh
