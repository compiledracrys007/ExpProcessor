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

Step 3: Run individual test
 - Set environment variables
```shell
 export ROOT_DIR=<project-root>
```

 - Run test to simulate experimental processor
```shell
 $ROOT_DIR/build/test/Target/EPU/BasicTest/test_epu_basic
```


Step 3: Run test suite
 - Set environment variables
```shell
 export ROOT_DIR=<project-root>
```

 - Run test suite
```shell
 cd $ROOT_DIR/test
 source run_tests.sh
```