SRC=$(wildcard *.cpp) $(wildcard src/*.cpp)
LIB=/usr/local/openssl/lib
INC1=/usr/include/mysql/
INC2=./inc/
DLL=-lssl -lcrypto -lmysqlpp -levent 
tarfile=main
all:$(tarfile)
main:$(SRC)
	g++ -o $@ $^ -g -I$(INC1) -I$(INC2) -L$(LIB) $(DLL) -w

.PHONY:clean all

clean:
	@rm $(tarfile)
	


