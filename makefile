CFLAGS = -Wall -Werror -pedantic
make: bin/mkfs bin/ls bin/write bin/rm bin/read bin/fsconv
	rm ./bin/*.o 
bin/common.o: bin
	g++ -c ./src/common/ECC.cpp $(CFLAGS) -o ./bin/ECC.o
	g++ -c ./src/common/ETC.cpp $(CFLAGS) -o ./bin/ETC.o
	ld -relocatable ./bin/ECC.o ./bin/ETC.o -o ./bin/common.o
bin/mkfs: bin/common.o
	g++ -c ./src/mkfs.cpp $(CFLAGS) -o ./bin/mkfs.o
	g++ ./bin/common.o ./bin/mkfs.o -o ./bin/mkfs
bin/ls: bin/common.o
	g++ -c ./src/ls.cpp $(CFLAGS) -o ./bin/ls.o
	g++ ./bin/common.o ./bin/ls.o -o ./bin/ls
bin/write: bin/common.o
	g++ -c ./src/write.cpp $(CFLAGS) -o ./bin/write.o
	g++ ./bin/common.o ./bin/write.o -o ./bin/write
bin/rm: bin/common.o
	g++ -c ./src/rm.cpp $(CFLAGS) -o ./bin/rm.o
	g++ ./bin/common.o ./bin/rm.o -o ./bin/rm
bin/read: bin/common.o
	g++ -c ./src/read.cpp $(CFLAGS) -o ./bin/read.o
	g++ ./bin/common.o ./bin/read.o -o ./bin/read
bin/fsconv: bin/common.o
	g++ -c ./src/fsconv.cpp $(CFLAGS) -o ./bin/fsconv.o
	g++ ./bin/common.o ./bin/fsconv.o -o ./bin/fsconv
bin:
	mkdir bin
