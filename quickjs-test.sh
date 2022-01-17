echo "compiling..."
g++ es.cc -std=c++17 -O2 -I$(PWD) -o bin/es
echo "run test_builtin"
bin/es test/quickjs/test_builtin.js
echo "run test_closure"
bin/es test/quickjs/test_closure.js
echo "run test_language"
bin/es test/quickjs/test_language.js
echo "run test_loop"
bin/es test/quickjs/test_loop.js
