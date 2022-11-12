ep2: ep2.cpp makefile
	g++ -g -fsanitize=leak -o ep2 ep2.cpp
ep2.cpp: /media/shared/ep2/ep2.cpp
	cp /media/shared/ep2/ep2.cpp ep2.cpp
makefile: /media/shared/ep2/devmakefile/makefile
	cp /media/shared/ep2/devmakefile/makefile makefile
run: ep2 
	./ep2
update:
	cp -r /media/shared/ep2/* ~/build/ep2/
clean:
	rm -f ~/build/ep2/ep2
	rm -f ~/build/ep2/ep2.cpp
prune:
	rm ~/build/ep2/*
	cp /media/shared/ep2/devmakefile/makefile ~/build/ep2/makefile
