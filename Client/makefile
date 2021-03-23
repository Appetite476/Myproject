SRC = $(wildcard *.cpp) $(wildcard src/*.cpp)
tarfile=main
all:$(tarfile)
main:$(SRC)
	g++ -o $@ $^ -g -I./inc/ -lssl -lcrypto -w

.PHONY:clean all

clean:
	@rm $(tarfile)
	


