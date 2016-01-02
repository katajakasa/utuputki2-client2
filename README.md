# Utuclient2

C/SDL2/FFMPEG client for Utuputki2.

## Requirements

* SDL2
* libwebsockets
* ffmpeg
* OpenAL
* argtable2

### Debian / Ubuntu

```
sudo apt-get install libwebsockets-dev libsdl2-dev libargtable2-dev libopenal-dev \
libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavresample-dev \
libavutil-dev libswresample-dev libswscale-dev libpostproc-dev
```

### MSYS2 64bit

For 32bit installation, just change the package names a bit .

```
pacman -S mingw-w64-x86_64-libwebsockets mingw-w64-x86_64-SDL2 mingw-w64-x86_64-openal mingw-w64-x86_64-ffmpeg
```

libargtable2 you will have to install manually. Eg:

```
wget http://prdownloads.sourceforge.net/argtable/argtable2-13.tar.gz
tar xvf argtable2-13.tar.gz
cd argtable2-13/
./configure --prefix=/usr/local --build=x86_64-w64-mingw32
make -j
make install
```

## Compiling

1. ```cmake -G "MSYS Makefiles" -DCMAKE_BULD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..```
2. ```make```
3. ```sudo make install```
4. Profit!

## License

MIT. Please see ```LICENSE``` for details.
