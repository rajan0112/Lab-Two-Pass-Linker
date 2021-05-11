linker: two_pass_linker.cpp
	g++ -std=c++11 -o two_pass_linker two_pass_linker.cpp

clean:
	rm -f two_pass_linker *~

