
if [[ `uname` -eq 'Darwin' ]]; then

  gcc -g -DDEBUG_CPU -DLOG_USE_COLOR -DLOG_LEVEL="LOG_ERROR | LOG_DEBUG | LOG_PARSER | LOG_NES" -DCPU_FREQ="(1789773)" -I/usr/local/include src/main.c src/cpu.c src/utils.c src/parser.c src/sdl.c src/log.c src/nes.c src/listing.c src/clock.c -lSDL2-2.0.0 -lpthread -o nes && ./nes $@


else

gcc -DDEBUG_CPU -DLOG_USE_COLOR -DLOG_LEVEL="LOG_CPU | LOG_BUS" -DCPU_FREQ="1000" -I/mnt/nfs/homes/pleroux/Documents/sdl2-build/include src/main.c src/cpu.c src/utils.c src/parser.c src/sdl.c src/log.c src/listing.c src/clock.c -L/mnt/nfs/homes/pleroux/Documents/sdl2-build/lib -lSDL2 -lpthread -o nes && ./nes $@

fi
