cmake --build .;
./main build "../tests/testforllvm.cpplox";
lli output.ll;