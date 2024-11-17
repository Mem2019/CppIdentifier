all: CppIdentifier.so

CppIdentifier.so: CppIdentifier.cpp
	clang++ -fPIC -shared `llvm-config --cxxflags --ldflags` CppIdentifier.cpp -o CppIdentifier.so

clean:
	rm -f CppIdentifier.so