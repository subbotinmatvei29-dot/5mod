obj-m += netlinkcode.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -Wall -Wextra -o user user.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f user
