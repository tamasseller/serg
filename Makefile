all: test-mlz test-rans test-serg serg

test-rans: TestRans.cpp *.h
	g++ -O0 -g3 $< -o $@

test-mlz: TestMLZ.cpp *.h
	g++ -O0 -g3 $< -o $@

test-serg: TestSerg.cpp *.h
	g++ -O0 -g3 $< -o $@

serg: Serg.cpp *.h
	g++ -O0 -g3 $< -o $@
