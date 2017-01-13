## Building for Debian

(Works with other architectures probably.)

Simply build and run from the src directory with the following command. The Makefile is already included.

```
make clean; make && ./socketman --config=/home/simon
```

Bonus points if you're using valgrind:

```
make clean; make && valgrind --leak-check=full ./socketman --config=/home/user/config.json
```
