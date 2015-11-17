/* main.c
*  Tomas
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>


#ifndef TEST
int main(void) {

	puts("Starting service");


	return EXIT_SUCCESS;
}


#else // ifndef TEST

int main(void) {
	puts("ALL OK");
	return EXIT_SUCCESS;
}

#endif
