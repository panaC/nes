
exit 0;

mkdir -p /usr/local/include/SDL2
mkdir -p /usr/local/lib
ln -s $HOME/Documents/lib/SDL/build/arm64/build/.libs/libSDL2-2.0.0.dylib /usr/local/lib/
ln -sf $HOME/Documents/lib/SDL/include/*.h /usr/local/include/SDL2
