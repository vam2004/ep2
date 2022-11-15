~/build/ep2/ep2: ep2.cpp 
	g++ -g -fsanitize=leak -o ~/build/ep2/ep2 ep2.cpp
ep2.cpp: /media/shared/ep2/ep2.cpp
	cp /media/shared/ep2/ep2.cpp ~/build/ep2/ep2.cpp
makefile: /media/shared/ep2/devmakefile/makefile
	cp /media/shared/ep2/devmakefile/makefile ~/build/ep2/makefile
copy_test:
	cp /media/shared/ep2/tests/* ~/build/ep2/
run: ~/build/ep2/ep2
	~/build/ep2/ep2
update: clean makefile ep2.cpp copy_test
clean:
	rm -f ~/build/ep2/build/ep2
	rm -f ~/build/ep2/ep2.cpp
prune:
	rm -r ~/build/ep2/*
	cp /media/shared/ep2/devmakefile/makefile ~/build/ep2/makefile
mirror: prune
	cp -r /media/shared/ep2/* ~/build/ep2/
	cat /media/shared/ep2/devmakefile/makefile.extra >> ~/build/ep2/makefile
