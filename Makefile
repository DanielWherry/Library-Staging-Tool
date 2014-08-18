libstaging.o: staging.c
	cc -o libstaging.o staging.c -c
libStaging.exe: libstaging.o
	cc -o libStaging.exe libstaging.o
	cp libStaging.exe $(MEMBERWORK)/stf007/LibStaging/
