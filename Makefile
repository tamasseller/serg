all: test/test serg

test/test: test/*.cpp lib/* test/*.h
	g++ -O0 -g3 test/*.cpp -o $@ -Ilib

serg: util/Serg.cpp lib/* 
	g++ -O3 $< -o $@ -Ilib
