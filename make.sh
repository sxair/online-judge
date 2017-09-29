gcc -O3 -o judge -DDEBUG judge.cpp support.cpp -lmysqlclient
gcc -O3 -o run-cilet -DDEBUG run.cpp support.cpp from.cpp solve.cpp compare.cpp -lmysqlclient
