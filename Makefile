all: hang

hang: src/*
	$(CC) src/hang.c src/tinymt32.c -o hang.exe -std=c99

clean:
	rm -f hang.exe
