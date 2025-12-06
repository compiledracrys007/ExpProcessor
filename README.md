Step 1: Clone the repo
```shell
git clone git@github.com:compiledracrys007/ExpProcessor.git
```

Step 2: Build from source
```shell
mkdir build
cd build
cmake ..
cmake --build .
```

Step 3: Run tests
 - Set environment variables
```shell
 export ROOT_DIR=<project-root>
```

 - Run test to simulate experimental processor
```shell
 $ROOT_DIR/build/test/test_epu
```
