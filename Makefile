HttpServer.o:
	g++ -c -g HttpServer.cpp

Main.o:
	g++ -c -g Main.cpp

Objects: HttpServer.o Main.o

HttpServer: Objects
	g++ -o HttpServer *.o

Rebuild: Clean HttpServer

Run: HttpServer
	./HttpServer

Clean:
	rm *.o	
	rm HttpServer