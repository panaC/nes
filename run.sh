gcc -DDEBUG_CPU -DLOG_USE_COLOR -DLOG_LEVEL="LOG_CPU | LOG_BUS" -DCPU_FREQ="1000" -I/mnt/nfs/homes/pleroux/Documents/sdl2-build/include src/main.c src/cpu.c src/test.c src/utils.c src/debug.c src/parser.c src/sdl.c src/snake.c src/bus.c src/log.c -L/mnt/nfs/homes/pleroux/Documents/sdl2-build/lib -lSDL2 -o nes && ./nes $@
