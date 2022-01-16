test:

```bash
cmake -S . -B build
cmake --build build
cd build && ctest --verbose; cd ..
```

compiler:
```
g++ es.cc -std=c++17 -O2 -I/Users/zilinzhu/Documents/es/es -o bin/es
```

run
```
bin/es test/quickjs/test_builtin.js
```