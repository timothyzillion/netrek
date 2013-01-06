#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>
#include <netinet/in.h>

/* netrek specific */
#include "config.h"
#include "defs.h"
#include "Wlib.h"
#include "struct.h"

static int listenFd=-1;
static int peerFd=-1;
static struct sockaddr_in myAddr;
static struct sockaddr_in peerAddr;

static short borgPort=10000;
static int borgAcceptCommands=0;


void
borg_init(void)
{
  int fd=-1;

  if (listenFd != -1) {
    fprintf(stderr, "borg_init(): reinit ? closing listener\n");
    close(listenFd);
    listenFd = -1;
  }

  if (peerFd != -1) {
    fprintf(stderr, "borg_init(): reinit ? closing peer\n");
    close(peerFd);
    peerFd = -1;
  }

  fd = socket(PF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    fprintf(stderr, "borg_init(): cannot create socket (%s)\n", strerror(errno));
    return;
  }

  memset(&myAddr, 0, sizeof(struct sockaddr_in));
  myAddr.sin_family = AF_INET;
  myAddr.sin_port = htons(borgPort);
  if (bind(fd, (struct sockaddr *)&myAddr, sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr, "borg_init(): bind failed (%s)\n", strerror(errno));
    close(fd);
    return;
  }

  if (listen(fd, 5) < 0) {
    fprintf(stderr, "borg_init(): failed to create new listener (%s)\n", strerror(errno));
    close(fd);
    return;
  }

  listenFd = fd;
  fprintf(stderr,"borg_init(): listening on port %d\n", borgPort);
  return;
}

void
borg_toggle(void)
{
  borgAcceptCommands = borgAcceptCommands ? 0 : 1;
}

/*
 * Pass in a bunch of callbacks for various commands -- right now focus is on
 * dogfighting / auto-dodge. Shooting would be easier, probably.
 */
void
borg_update(struct player *me, struct torp *torps, struct plasmatorp *plasma,
            void (*set_heading)(unsigned char),
            void (*set_speed)(int),
            void (*shield_up)(void),
            void (*shield_down)(void),
            void (*repair_start)(void),
            void (*repair_stop)(void))
{
  int ret;
  fd_set readSet, writeSet;
  struct timeval tv;

  if (peerFd == -1) {
    socklen_t sockLen;
    /* See if someone has connected. */
    FD_ZERO(&readSet);
    FD_SET(listenFd, &readSet);
    memset(&tv, 0, sizeof(tv));
    ret = select(listenFd + 1, &readSet, NULL, NULL, &tv);
    if (ret <= 0 || !FD_ISSET(listenFd, &readSet)) {
      return;
    }
    sockLen = sizeof(peerAddr);
    peerFd = accept(listenFd, (struct sockaddr *)&peerAddr, &sockLen);
    if (peerFd == -1) {
      fprintf(stderr, "borg_update(): accept problem (%s)\n", strerror(errno));
      return;
    }

    fprintf(stderr, "borg_update(): client connected\n");
  }

  FD_ZERO(&readSet);
  FD_SET(peerFd, &readSet);
  FD_ZERO(&writeSet);
  FD_SET(peerFd, &writeSet);
  memset(&tv, 0, sizeof(tv));
  ret = select(peerFd + 1, &readSet, &writeSet, NULL, &tv);
  if (ret <= 0)  {
    return;
  }

  if (FD_ISSET(peerFd, &writeSet)) {
    /* send the updated game state. */
  }

  if (borgAcceptCommands && FD_ISSET(peerFd, &readSet)) {
    /* read commands */
  }
}
