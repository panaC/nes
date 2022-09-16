gcc -DDEBUG_CPU src/main.c src/cpu.c src/test.c src/utils.c src/debug.c src/parser.c -o nes && ./nes $@
