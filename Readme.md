
# Sysy-Compiler

### Run Test on Docker:

```shell
docker run -it --rm -v $(pwd):/root/compiler-mirror maxxing/compiler-dev \
autotest -riscv -s lv9 /root/compiler-mirror/
```


```shell
docker run -it --rm -v $(pwd):/root/compiler maxxing/compiler-dev bash

cd compiler
#autotest -w wd compiler 2>&1 | tee compiler/out.txt


clang test.S -c -o test.o -target riscv32-unknown-linux-elf -march=rv32im -mabi=ilp32 
ld.lld test.o -L$CDE_LIBRARY_PATH/riscv32 -lsysy -o test
qemu-riscv32-static test; echo $?

```

