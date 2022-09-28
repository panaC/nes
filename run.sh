gcc -DDEBUG_CPU  -I/usr/local/include src/main.c src/cpu.c src/test.c src/utils.c src/debug.c src/parser.c src/sdl.c src/snake.c src/bus.c -lSDL2-2.0.0 -o nes && ./nes $@
