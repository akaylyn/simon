ctags -f cpp.tags `find . -name "*.cpp" -o -name "*.h"`
ctags -f ino.tags --langmap=c++:.ino `find . -name "*.ino"`
cat cpp.tags ino.tags > tags
sort tags -o tags
rm -f *.tags

