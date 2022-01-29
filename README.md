# es

A JavaScript interpreter from scratch, supporting ES5 syntax.

## Compile and Run

Clone and compile:

```
$ git clone git@github.com:zhuzilin/es.git
$ cd es
$ g++ es.cc -std=c++17 -O2 -I$(PWD) -o bin/es
```

Run:

```
$ echo "console.log('hello world!')" > hello_world.js
$ bin/es hello_world.js 
"hello world!"
```

## Test

Use `test/*.cc`:

```bash
cmake -S . -B build
cmake --build build
cd build && ctest; cd ..
```

Run the quickjs test:

```
sh quickjs-test.sh 
```

The quickjs tests pass if there is no output.

## Acknowledgement

I've learned a lot from [Constellation/iv](https://github.com/Constellation/iv) and thanks a lot for 
[Annotated ECMAScript 5.1](https://es5.github.io/#x15.3.5.3).
