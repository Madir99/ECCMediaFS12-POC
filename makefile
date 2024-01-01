make: bin
	g++ ./src/mkfs.cpp ./src/common/*.cpp -Wall -Werror -pedantic -o ./bin/mkfs
	g++ ./src/ls.cpp ./src/common/*.cpp -Wall -Werror -pedantic -o ./bin/ls
	g++ ./src/write.cpp ./src/common/*.cpp -Wall -Werror -pedantic -o ./bin/write
	g++ ./src/rm.cpp ./src/common/*.cpp -Wall -Werror -pedantic -o ./bin/rm
	g++ ./src/read.cpp ./src/common/*.cpp -Wall -Werror -pedantic -o ./bin/read
bin:
	mkdir bin