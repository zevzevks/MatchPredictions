MAINPATH=./main
INCPATH=./main/inc
OBJPATH=./obj
LIBPATH=./lib
CFLAGS=-g -Wall
MINPATH=./lib/minizip-master

install: getobj app putobj 

app:	main.o
	gcc  $(CFLAGS) -o App main.o

main.o: $(MAINPATH)/main.c $(INCPATH)/config.h
	gcc  $(CFLAGS) -I$(INCPATH) $(MAINPATH)/main.c -c 

buildall:
	@echo "****** Invoking tstlib/makefile"\
	cd lib;cd minizip-master; cmake . -DMZ_BUILD_TEST=ON ; cmake --build .\
	@echo "****** Invoking application/makefile"\
	cd application; $(MAKE)\


getobj:
	-mv $(OBJPATH)/*.o . 2>/dev/null

putobj:
	-mv *.o $(OBJPATH) 2>/dev/null

clean:
	-rm -f App
	-rm -f $(OBJPATH)/*.o
