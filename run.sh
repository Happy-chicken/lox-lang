cmake --build .;
printf "%s\n" "-------------------------LLVM IR-------------------------"
./main build "../tests/testforllvm.cpplox";
printf "\n%s\n" "-------------------lli interpret LLVM IR------------------"
lli output.ll;
printf "\n%s\n" "------------------------interpret-------------------------"
./main run "../tests/testforllvm.cpplox";