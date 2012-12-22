/* reserved.c
 *
 * Kevin P. Smith   7/3/89
 */
#include "copyright2.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "defs.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "packets.h"
#include "struct.h"
#include "data.h"

void makeReservedPacket(struct reserved_spacket *packet)
{
    int i;

    for (i=0; i<16; i++) {
	packet->data[i]= (char) (random() % 256);
    }
    packet->type = SP_RESERVED;
}

/* A "random" table we use to hose our data. */

static u_char swap[16][16] = {
    { 4,13, 9, 7, 5, 8,14, 2,10,11, 6, 3,12,15, 0, 1},
    { 0, 4,12, 5,15, 2, 7, 3,14, 6, 1,10, 8,13,11, 9},
    { 9, 5, 4, 2, 3, 6,10,12, 1, 7,15,14,11, 0,13, 8},
    { 5, 8,15, 4, 6, 9, 1,12,10, 3,13,11, 2, 7, 0,14},
    {15,13, 1,14, 4, 0, 8,12,11, 9, 2,10, 3, 6, 7, 5},
    { 6, 7, 9,11,14, 1, 4,13, 2,12, 8,15,10, 0, 5, 3},
    {11, 0,10,14, 7,12, 2, 4,15,13, 3, 9, 1, 5, 8, 6},
    { 9, 8, 2,10,12,14, 3, 7, 4, 0,11, 1, 5,15, 6,13},
    {15,12, 3, 0,10, 8,13, 9,14, 2, 4, 5, 7, 6, 1,11},
    {14, 3, 0,11, 2,10,15,12,13, 1, 5, 4, 6, 8, 7, 9},
    { 1, 2, 7, 3, 8, 0,10,14, 6, 5,13,12, 4, 9,11,15},
    { 2, 1,11, 9, 7, 3,12, 6, 5, 8, 0,10,14, 4,15,13},
    { 0,12,15, 1,13, 7, 6, 5, 3,14,11, 8, 9,10, 2, 4},
    {10, 9, 0, 6, 1,14, 5,12, 7, 4,15,13, 8, 2, 3,11},
    { 3,11, 6,12,10, 5,14, 1, 9,15, 7, 2,13, 8, 4, 0},
    {12, 6, 5, 8,14, 4,13,15,10, 0,11, 9, 3, 1, 7, 2}
};

void encryptReservedPacket(struct reserved_spacket *spacket,
                           struct reserved_cpacket *cpacket,
                           char *server, int pno)
{
    struct hostent *hp;
    struct in_addr address;
    U_LONG netaddr;
    u_char mixin1, mixin2, mixin3, mixin4, mixin5;
    int i,j,k;
    char buf[16];
    u_char *s;

    memcpy(cpacket->data, spacket->data, 16);
    memcpy(cpacket->resp, spacket->data, 16);
    cpacket->type=CP_RESERVED;

    if ((address.s_addr = inet_addr(server)) == -1) {
	if ((hp = gethostbyname(server)) == NULL) {
	    ERROR(1,("I don't know any %s!\n", server));
	} else {
	    address.s_addr = *(LONG *) hp->h_addr;
	}
    }

    netaddr=htonl(address.s_addr);
    mixin1 = (u_char) ((U_LONG) (netaddr >> 24) & 0xff);
    mixin2 = pno;
    mixin3 = (u_char) ((U_LONG) (netaddr >> 16) & 0xff);
    mixin4 = (u_char) ((U_LONG) (netaddr >> 8) & 0xff);
    mixin5 = (u_char) ((U_LONG) (netaddr & 0xff));

    j=0;
    for (i=0; i<23; i++) {
	j=(j+cpacket->resp[j]) & 15;
	s=swap[j];
	cpacket->resp[s[i % 7]] ^= mixin1;
	cpacket->resp[s[i % 5]] ^= mixin2;
	cpacket->resp[s[i % 13]] ^= mixin3;
	cpacket->resp[s[i % 3]] ^= mixin4;
	cpacket->resp[s[i % 11]] ^= mixin5;
	for (k=0; k<16; k++) {
	    buf[k]=cpacket->resp[s[k]];
	}
	memcpy(cpacket->resp, buf, 16);
    }
}


