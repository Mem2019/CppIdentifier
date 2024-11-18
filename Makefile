all: CppIdentifier.so

CppIdentifier.so: CppIdentifier.cpp
	clang++ -O2 -fPIC -shared `llvm-config --cxxflags --ldflags` CppIdentifier.cpp -o CppIdentifier.so

clean:
	rm -f CppIdentifier.so
