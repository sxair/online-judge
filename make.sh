g++ -O3 -o judge -DDEBUG judge.cpp support.cpp -lmysqlclient
g++ -O3 -o judge-main judge.cpp support.cpp -lmysqlclient
g++ -O3 -o run-client -DDEBUG run.cpp support.cpp provider.cpp solve.cpp compare.cpp -lmysqlclient
