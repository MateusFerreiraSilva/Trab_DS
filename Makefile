StringUtils.o:	
	g++ -c -g -std=c++17 StringUtils.cpp 

ThreadPool.o:	
	g++ -c -g -std=c++17 ThreadPool.cpp 

HttpServer.o:
	g++ -c -g -std=c++17 HttpServer.cpp

Main.o:
	g++ -c -g -std=c++17 Main.cpp

Objects: StringUtils.o ThreadPool.o HttpServer.o Main.o

HttpServer: Objects
	g++ -o HttpServer *.o -lpthread

Rebuild: Clean HttpServer

Run: HttpServer
	./HttpServer

Clean:
	rm *.o	
	rm HttpServer