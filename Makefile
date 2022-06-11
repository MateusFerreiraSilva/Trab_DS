### CORE ###

bitarray.o:
	g++ -c -g ./src/bitarray.cpp
	mv bitarray.o ./resources

sample_pointers.o:
	g++ -c -g ./src/sample_pointers.cpp
	mv sample_pointers.o ./resources

combination.o:
	g++ -c -g ./src/combination.cpp
	mv combination.o ./resources

compressed_bitvector.o:
	g++ -c -g ./src/compressed_bitvector.cpp
	mv compressed_bitvector.o ./resources

wavelet_tree.o:
	g++ -c -g ./src/wavelet_tree.cpp
	mv wavelet_tree.o ./resources

csa.o:
	g++ -c -g ./src/csa.cpp
	mv csa.o ./resources

main.o:
	g++ -c -g ./src/main.cpp
	mv main.o ./resources

core: bitarray.o sample_pointers.o combination.o compressed_bitvector.o wavelet_tree.o csa.o main.o

### END CORE ###

### TESTS ###

sample_pointers_test.o:
	g++ -c -g ./src/structures_test/sample_pointers_test.cpp
	mv sample_pointers_test.o ./resources

compressed_bitvector_test.o:
	g++ -c -g ./src/structures_test/compressed_bitvector_test.cpp
	mv compressed_bitvector_test.o ./resources

wavelet_tree_test.o:
	g++ -c -g ./src/structures_test/wavelet_tree_test.cpp
	mv wavelet_tree_test.o ./resources

csa_test.o:
	g++ -c -g ./src/structures_test/csa_test.cpp
	mv csa_test.o ./resources

tests: sample_pointers_test.o compressed_bitvector_test.o wavelet_tree_test.o csa_test.o 

### END TESTS ###

all: core tests

build-main: main
	g++ -o prog ./resources/*.o

build-tests: tests
	g++ -o prog ./resources/*.o

build: all
	g++ -o prog ./resources/*.o

run: build
	./prog

clean:
	rm ./resources/*.o	
	rm prog