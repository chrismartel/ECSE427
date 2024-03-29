#include "sut.h"
#include <stdio.h>

void hello1() {
    int i;
    char sbuf[128];
    for (i = 0; i < 100; i++) {
	sprintf(buf, "Hello world!, message from SUT-One i = %d \n", i);
	sut_write(buf);
	sut_yield();
    }
    sut_exit();
}

void hello2() {
    int i;
    for (i = 0; i < 100; i++) {
	printf("Hello world!, this is SUT-Two \n");
	sut_yield();
    }
    sut_exit();
}

int main() {
    sut_init();
    sut_create(hello1);
    sut_create(hello2);
    sut_shutdown();
}
