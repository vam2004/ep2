build/ep2: build_dir ep2.cpp
	g++ -g -Wall -fsanitize=leak -o build/ep2 ep2.cpp
build_dir:
	mkdir -p build
run: build/ep2
	./build/ep2
test: build_dir
	cp -i tests/*.txt build/
clean:
	rm -f build/*.txt
	rm -f build/ep2
