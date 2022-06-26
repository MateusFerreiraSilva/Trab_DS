CC = g++
# -Wall
CFLAGS  = -c -g -std=c++17

All:	
	$(CC) $(CFLAGS) *.cpp 

HttpServer: All
	$(CC) -o HttpServer *.o -lpthread

Rebuild: Clean HttpServer

Run: HttpServer
	./HttpServer

Clean:
	rm *.o	
	rm HttpServer