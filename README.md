# SW10
Master Thesis project

To build the project, run the following commands
- java -jar thirdparty/antlr/antlr-4.7.2-complete.jar -Dlanguage=Cpp -no-listener -visitor -o thirdparty/antlr/antlr4-runtime Small.g4
- mkdir build
- cmake CMakeLists.txt -B build
- cd build && make
