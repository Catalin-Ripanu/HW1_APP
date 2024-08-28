build_serial:
	g++ kmeans_serial.cpp -o kmeans_serial
	# i686-w64-mingw32-g++ -g kmeans_serial.cpp -o kmeans_serial

build_pthreads:
	g++ kmeans_pthreads.cpp -o kmeans_pthreads -lpthread
	# x86_64-w64-mingw32-g++ -g kmeans_pthreads.cpp -o kmeans_pthreads -lpthread
	# i686-w64-mingw32-g++ -g kmeans_pthreads.cpp -o kmeans_pthreads -lpthread

build_openmp:
	g++ kmeans_openmp.cpp -o kmeans_openmp -fopenmp
	# i686-w64-mingw32-g++ -g kmeans_openmp.cpp -o kmeans_openmp -fopenmp

cross_test:
	i686-w64-mingw32-g++ -g -o test test.cpp -lpthread

clean:
	rm -f kmeans_serial kmeans_pthreads kmeans_openmp *.exe data/*.out

.PHONY: clean