# Utuclient2

C/SDL2/FFMPEG client for Utuputki2.

## Requirements

* SDL2
* libwebsockets
* ffmpeg
* argtable2

```sudo apt-get install libwebsocket-dev libffmpeg-dev libsdl2-dev libargtable2-dev```

## Compiling

1. ```cmake -G "MSYS Makefiles" -DCMAKE_BULD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..```
2. make
3. Profit!

## License

MIT. Please see ```LICENSE``` for details.
