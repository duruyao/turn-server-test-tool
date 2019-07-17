execute:
	g++ multiple_test.cpp -o test

install:
	pip3 install numpy
	pip3 install matplotlib

server:
	sudo ../turnserver -v -L 192.168.22.72 -m 500 -a -f -r 192.168.22.72 -c server_conf/turnserver.conf

peer: 
	../turnutils_peer -v -p 3479 -L 127.0.0.1 -L ::1 -L 0.0.0.0

client:
	./test -t 3 -m 10 -n 5 -I 5 -e 192.168.17.8 -A 192.168.22.72
	
