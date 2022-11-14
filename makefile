build/ep2: ep2.cpp
	mkdir -p build
	g++ -g -fsanitize=leak -o build/ep2 ep2.cpp
run: build/ep2
	./build/ep2
test:
	cp -i tests/*.txt build/
clean:
	rm -f build/*.txt
	rm -f build/ep2
