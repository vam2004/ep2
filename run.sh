#! /bin/sh
# build with debugging options enabled
make ep2
# proxies arguments to program
if [ -d build ]; then
	echo "running ./build/ep2"
	./build/ep2 "$@"
else
	echo "running ./ep2"
	./ep2 "$@"
fi
