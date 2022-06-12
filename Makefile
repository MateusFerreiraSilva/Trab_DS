HttpServer.o:
	g++ -c -g HttpServer.cpp

Main.o:
	g++ -c -g Main.cpp

Objects: HttpServer.o Main.o

HttpServer: Objects
	g++ -o HttpServer *.o

run: HttpServer
	./HttpServer

clean:
	rm *.o	
	rm HttpServer