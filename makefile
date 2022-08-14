LIBS = `curl-config --libs`
OPTS = `curl-config --cflags`

4chaner: 4chaner.o
	gcc -o 4chaner 4chaner.o $(OPTS) $(LIBS) -lstdc++
mycurlapp.o: 4chaner.cpp 
	gcc -c 4chaner.cpp  

clean:
	rm -f 4chaner.o 4chaner
