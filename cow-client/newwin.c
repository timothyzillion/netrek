
/* newwin.c
 *
 * $Log: newwin.c,v $
 * Revision 1.6  2006/09/19 10:20:39  quozl
 * ut06 full screen, det circle, quit on motd, add icon, add desktop file
 *
 * Revision 1.5  2006/05/16 06:16:35  quozl
 * add PLCORE
 *
 * Revision 1.4  2002/06/22 04:43:24  tanner
 * Clean up of SDL code. #ifdef'd out functions not needed in SDL.
 *
 * Revision 1.3  1999/08/05 16:46:32  siegl
 * remove several defines (BRMH, RABBITEARS, NEWDASHBOARD2)
 *
 * Revision 1.2  1999/03/05 23:06:38  carlos
 * In the Makefile, updated the KEYGOD address
 *
 * In all else, (cowmain.c, main.c newwin.c parsemeta.{c,h} x11window.c
 * added UDP metaserver query functionality as proposed by James Cameron
 * and implemented by James Cameron and Carlos Villalpando.
 *
 * Revision 1.1.1.1  1998/11/01 17:24:10  siegl
 * COW 3.0 initial revision
 * */
#include "config.h"
#include "copyright.h"

#include <stdio.h>
#include INC_STDLIB
#include <math.h>
#include <signal.h>
#include <sys/types.h>

#include <time.h>
#include INC_SYS_TIME
#include INC_SYS_SELECT

#include "Wlib.h"
#include "defs.h"
#include "struct.h"
#include "data.h"
#include "playerlist.h"
#include "bitmaps.h"
#include "moobitmaps.h"
#include "rabbitbitmaps.h"
#include "parsemeta.h"

#ifdef TNG_FED_BITMAPS
#include "tngbitmaps.h"
#endif

#ifdef VARY_HULL
#include "hullbitmaps.h"
#endif

#include "oldbitmaps.h"
#include "packets.h"
#include "spopt.h"

extern char cbugs[];
extern int metaHeight;   /* height of metaserver window */

static int line = 0;
int     MaxMotdLine = 0;

/* if a motd line from the server is this, the client will junk all motd *
 * data it currently has.  New data may be received */
#define MOTDCLEARLINE  "\033\030CLEAR_MOTD\000"

#define SIZEOF(a)       (sizeof (a) / sizeof (*(a)))

#define BOXSIDE         (TWINSIDE / 5)
#define TILESIDE        16
#define MESSAGESIZE     20

#define PLISTSIZE       254			 /* (MENU_PAD * 2 + *
						  * (MAXPLAYERS + 3) * *
						  * W_Textheight) */
#define STATSIZE        (MESSAGESIZE * 2 + BORDER)
#define YOFF            0

#define stipple_width 16
#define stipple_height 16
static char stipple_bits[] =
{
  0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08,
  0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80,
  0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08,
  0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80};

/* ATM: extra stuff for those who don't like my visible tractors */
#define tract_width 5
#define tract_height 5
static char tract_bits[] =
{
  0x1f, 0x04, 0x04, 0x04, 0x04};

#define press_width 5
#define press_height 5
static char press_bits[] =
{
  0x0f, 0x11, 0x0f, 0x01, 0x01};

/* Event Handlers. */
extern void drawIcon(void), redrawTstats(void), planetlist(void);
extern void ranklist(void), fillhelp(void), fillmacro(void);
extern void redrawLMeter(void), redrawPStats(void), redrawStats(void);

extern void nsaction(W_Event * data);
extern void optionaction(W_Event * data);
extern void udpaction(W_Event * data), waraction(W_Event * data);

#if defined(SOUND) && !defined(HAVE_SDL)
extern void soundaction(W_Event * data);

#endif

/* Other function declarations */
extern int smessage(char ichar);



static void
        handleMessageWindowKeyDown(W_Event * event)
{
  smessage(event->key);
}

static void
        handleMessageWindowButton(W_Event * event)
{

#ifndef WIN32
  if (event->key == W_MBUTTON)
    pastebuffer();				 /* xcutbuffer stuff --RW */
#endif
}

newwin(char *hostmon, char *progname)
{
  int     i;
  int main_width = TWINSIDE + GWINSIDE + BORDER;
  int main_height = GWINSIDE + 2 * BORDER + PLISTSIZE;

  W_Initialize(hostmon);

  if (booleanDefault("FullScreen", 1)) {
    main_width = MAX(main_width, 1024);
    main_height = MAX(main_height, 768);
  }

  baseWin = W_MakeWindow("netrek", 0, YOFF, main_width,
		   main_height, NULL, BORDER, gColor);

  iconWin = W_MakeWindow("netrek_icon", 0, 0, icon_width, icon_height, NULL,
			 BORDER, gColor);
  W_SetWindowExposeHandler(iconWin, drawIcon);

  W_SetIconWindow(baseWin, iconWin);
  w = W_MakeWindow("local", 0, 0, TWINSIDE, TWINSIDE, baseWin,
		   BORDER, foreColor);

  mapw = W_MakeWindow("map", TWINSIDE+BORDER, 0, GWINSIDE, GWINSIDE, baseWin,
		      BORDER, foreColor);

  tstatw = W_MakeWindow("tstat", 0, TWINSIDE+BORDER, TWINSIDE, STATSIZE,
			baseWin, BORDER, foreColor);

#ifdef nodef					 /* 01/18/95 No messages for
						  * * * dashdord [007] */
  W_SetWindowKeyDownHandler(tstatw, handleMessageWindowKeyDown);
#endif

  W_SetWindowExposeHandler(tstatw, redrawTstats);

  warnw = W_MakeWindow("warn", TWINSIDE+BORDER, GWINSIDE+BORDER, GWINSIDE, MESSAGESIZE,
		       baseWin, BORDER, foreColor);
  W_SetWindowKeyDownHandler(warnw, handleMessageWindowKeyDown);

  messagew = W_MakeWindow("message", TWINSIDE+BORDER, GWINSIDE+BORDER+MESSAGESIZE,
			  GWINSIDE, MESSAGESIZE, baseWin, BORDER, foreColor);
  W_SetWindowKeyDownHandler(messagew, handleMessageWindowKeyDown);
  W_SetWindowButtonHandler(messagew, handleMessageWindowButton);

  planetw = W_MakeTextWindow("planet", 10, 10, 57, MAXPLANETS + 3, w, 2);
  W_SetWindowExposeHandler(planetw, planetlist);

  rankw = W_MakeTextWindow("rank", 50, 300, 65, NUMRANKS + 9, w, 2);
  W_SetWindowExposeHandler(rankw, ranklist);

#ifdef SMALL_SCREEN
  playerw = W_MakeTextWindow("player", TWINSIDE,
			     YOFF + GWINSIDE + 2 * BORDER + 2 * MESSAGESIZE,
			     PlistMaxWidth(), MAXPLAYER + 3, baseWin, 2);
#else
  playerw = W_MakeTextWindow("player", 0,
			     YOFF + TWINSIDE + BORDER + STATSIZE + BORDER,
			     PlistMaxWidth(), MAXPLAYER + 3, baseWin, 2);
#endif

  W_SetWindowExposeHandler(playerw, RedrawPlayerList);

  helpWin = W_MakeTextWindow("help", 0, YOFF + TWINSIDE + 2 * BORDER + 2 * MESSAGESIZE,
			     160, 20, NULL, BORDER);
  W_SetWindowExposeHandler(helpWin, fillhelp);

#ifdef META
  metaWin = W_MakeMenu("MetaServer List", 0, 0, 80, metaHeight,
		       NULL, 2);
  W_SetWindowKeyDownHandler(metaWin, metaaction);
  W_SetWindowButtonHandler(metaWin, metaaction);
#endif

#ifdef SMALL_SCREEN
  /* note that wk and phaswerwin are drawn under wi, and wa under wt-- there
   * * just isn't ROOM for them all */
  messwt = W_MakeScrollingWindow("review_team", 0,
				 YOFF + TWINSIDE + BORDER + STATSIZE,
				 80, 1, baseWin, 1);
  messwi = W_MakeScrollingWindow("review_your", 0,
				 YOFF + TWINSIDE + MESSAGESIZE + STATSIZE,
				 80, 1, baseWin, 1);
  messwa = W_MakeScrollingWindow("review_all", 0,
			       YOFF + TWINSIDE + 2 * MESSAGESIZE + STATSIZE,
				 80, 1, baseWin, 1);
  messwk = W_MakeScrollingWindow("review_kill", 0,
			       YOFF + TWINSIDE + 3 * MESSAGESIZE + STATSIZE,
				 35, 1, baseWin, 1);
  phaserwin = W_MakeScrollingWindow("review_phaser", 25 + 35 * W_Textwidth,
			       YOFF + TWINSIDE + 3 * MESSAGESIZE + STATSIZE,
				    40, 1, baseWin, 1);
  reviewWin = W_MakeScrollingWindow("review", 0,
	       YOFF + TWINSIDE + BORDER + STATSIZE, 80, 2, baseWin, BORDER);
#else
  messwa = W_MakeScrollingWindow("review_all", TWINSIDE + BORDER,
   YOFF + GWINSIDE + 3 * BORDER + 2 * MESSAGESIZE, 80, 10, baseWin, BORDER);
  messwt = W_MakeScrollingWindow("review_team", TWINSIDE + BORDER,
     YOFF + GWINSIDE + 4 * BORDER + 2 * MESSAGESIZE + 10 * W_Textheight + 8,
				 80, 5, baseWin, BORDER);
  messwi = W_MakeScrollingWindow("review_your", TWINSIDE + BORDER,
    YOFF + GWINSIDE + 5 * BORDER + 2 * MESSAGESIZE + 15 * W_Textheight + 16,
				 80, 4, baseWin, BORDER);
  messwk = W_MakeScrollingWindow("review_kill", TWINSIDE + BORDER,
    YOFF + GWINSIDE + 6 * BORDER + 2 * MESSAGESIZE + 19 * W_Textheight + 24,
				 80, 6, baseWin, BORDER);
  phaserwin = W_MakeScrollingWindow("review_phaser", TWINSIDE + BORDER,
    YOFF + GWINSIDE + 3 * BORDER + 2 * MESSAGESIZE + 15 * W_Textheight + 16,
				    80, 4, baseWin, BORDER);
  reviewWin = W_MakeScrollingWindow("review", TWINSIDE + BORDER + 1,
	      YOFF + GWINSIDE + BORDER + STATSIZE, 81, 21, baseWin, BORDER-1);
#endif

  W_SetWindowKeyDownHandler(messwa, handleMessageWindowKeyDown);
  W_SetWindowKeyDownHandler(messwt, handleMessageWindowKeyDown);
  W_SetWindowKeyDownHandler(messwi, handleMessageWindowKeyDown);
  W_SetWindowKeyDownHandler(reviewWin, handleMessageWindowKeyDown);

  netstatWin = W_MakeMenu("netstat", TWINSIDE + 10, -BORDER + 10, 40,
			  NETSTAT_NUMFIELDS, NULL, BORDER);
  W_SetWindowKeyDownHandler(netstatWin, nsaction);
  W_SetWindowButtonHandler(netstatWin, nsaction);

  lMeter = W_MakeWindow("lagMeter", 800, 4, lMeterWidth(), lMeterHeight(),
			NULL, BORDER, foreColor);
  W_SetWindowExposeHandler(lMeter, redrawLMeter);

  pStats = W_MakeWindow("pingStats", 500, 4, pStatsWidth(), pStatsHeight(),
			NULL, 1, foreColor);
  W_SetWindowExposeHandler(pStats, redrawPStats);

  udpWin = W_MakeMenu("UDP", TWINSIDE + 10, -BORDER + 10, 40, UDP_NUMOPTS,
		      NULL, 2);
  W_SetWindowButtonHandler(udpWin, udpaction);

#ifdef SHORT_PACKETS
  spWin = W_MakeMenu("network", TWINSIDE + 10, -BORDER + 10, 40, SPK_NUMFIELDS,
		     NULL, 2);
  W_SetWindowKeyDownHandler(spWin, spaction);
  W_SetWindowButtonHandler(spWin, spaction);
#endif

#ifdef SOUND
#if defined(HAVE_SDL)
  soundWin = W_MakeMenu("sound", TWINSIDE + 20, -BORDER + 10, 40, 1, NULL, 2);
#else
  soundWin = W_MakeMenu("sound", TWINSIDE + 20, -BORDER + 10, 30,
			MESSAGE_SOUND + 4, NULL, 2);
  W_SetWindowKeyDownHandler(soundWin, soundaction);
  W_SetWindowButtonHandler(soundWin, soundaction);
  W_DefineArrowCursor(soundWin);
#endif
#endif

#ifdef TOOLS
  toolsWin = W_MakeScrollingWindow("tools", TWINSIDE + BORDER, BORDER,
				   80, TOOLSWINLEN, NULL, BORDER);
  W_DefineTrekCursor(toolsWin);
#endif

#ifdef XTREKRC_HELP
  defWin = W_MakeTextWindow("xtrekrc_help", 1, 100, 174, 41, NULL, BORDER);
#endif

#ifdef DOC_WIN
  docwin = W_MakeWindow("DocWin", 0, 181, 500, 500, 0, 2,
			foreColor);
  xtrekrcwin = W_MakeWindow("xtrekrcWin", 0, 200, 500, 500, 0, 2,
			    foreColor);
#endif

  teamWin[0] = W_MakeWindow(teamshort[1 << 0],  -1, 400, BOXSIDE, BOXSIDE, w, 1, foreColor);
  teamWin[1] = W_MakeWindow(teamshort[1 << 1],  -1,  -1, BOXSIDE, BOXSIDE, w, 1, foreColor);
  teamWin[2] = W_MakeWindow(teamshort[1 << 2], 400,  -1, BOXSIDE, BOXSIDE, w, 1, foreColor);
  teamWin[3] = W_MakeWindow(teamshort[1 << 3], 400, 400, BOXSIDE, BOXSIDE, w, 1, foreColor);
  qwin = W_MakeWindow("quit", 200, 400, BOXSIDE, BOXSIDE, w, 1,
		      foreColor);

#ifdef ARMY_SLIDER
  statwin = W_MakeWindow("stats", 422, 13, 160, 110, NULL, 5, foreColor);
#else
  statwin = W_MakeWindow("stats", 422, 13, 160, 95, NULL, 5, foreColor);
#endif /* ARMY_SLIDER */

  W_SetWindowExposeHandler(statwin, redrawStats);

  scanwin = W_MakeWindow("scanner", 422, 13, 160, 120, baseWin, 5, foreColor);
  W_DefineArrowCursor(netstatWin);
  W_DefineArrowCursor(lMeter);
  W_DefineTrekCursor(baseWin);
  W_DefineLocalcursor(w);
  W_DefineMapcursor(mapw);
  W_DefineTrekCursor(pStats);
  W_DefineTextCursor(warnw);
  W_DefineTrekCursor(messwt);
  W_DefineTrekCursor(messwi);
  W_DefineTrekCursor(helpWin);

#ifdef META
  W_DefineArrowCursor(metaWin);
#endif

  W_DefineTrekCursor(reviewWin);
  W_DefineTrekCursor(messwk);
  W_DefineTrekCursor(phaserwin);
  W_DefineTrekCursor(playerw);
  W_DefineTrekCursor(rankw);
  W_DefineTrekCursor(statwin);
  W_DefineTrekCursor(iconWin);
  W_DefineTextCursor(messagew);
  W_DefineTrekCursor(tstatw);
  W_DefineWarningCursor(qwin);
  W_DefineTrekCursor(scanwin);
  W_DefineArrowCursor(udpWin);

#ifdef SHORT_PACKETS
  W_DefineArrowCursor(spWin);
#endif

  W_DefineFedCursor(teamWin[0]);
  W_DefineRomCursor(teamWin[1]);
  W_DefineKliCursor(teamWin[2]);
  W_DefineOriCursor(teamWin[3]);

#define WARHEIGHT 2
#define WARWIDTH 20
#define WARBORDER 2

  war = W_MakeMenu("war", TWINSIDE + 10, -BORDER + 10, WARWIDTH, 6, baseWin,
		   WARBORDER);
  W_SetWindowButtonHandler(war, waraction);

  W_DefineArrowCursor(war);

  getResources(progname);
  savebitmaps();
}

mapAll(void)
{
  initinput();
  W_MapWindow(mapw);
  W_MapWindow(tstatw);
  W_MapWindow(warnw);

#ifdef XTRA_MESSAGE_UI
  /* Grr. checkMapped() defaults to off - not nice */
  if (booleanDefault("message.mapped", 1))
#endif

    W_MapWindow(messagew);
  W_MapWindow(w);
  W_MapWindow(baseWin);
  W_FullScreenBegin();
  /* since we aren't mapping windows that have root as parent in x11window.c
   * * * (since that messes up the TransientFor feature) we have to map them
   * * * here. (If already mapped, W_MapWindow returns) */

  if (checkMapped("planet"))
    W_MapWindow(planetw);
  if (checkMapped("rank"))
    W_MapWindow(rankw);
  if (checkMapped("help"))
    W_MapWindow(helpWin);

#ifdef META
  if (checkMapped("MetaServer List"))
    metawindow();
#endif

  if (checkMapped("review_all"))
    W_MapWindow(messwa);
  if (checkMapped("review_team"))
    W_MapWindow(messwt);
  if (checkMapped("review_your"))
    W_MapWindow(messwi);
  if (checkMapped("review_kill"))
    W_MapWindow(messwk);
  if (checkMapped("netstat"))
    nswindow(netstatWin);
  if (checkMapped("lagMeter"))
    W_MapWindow(lMeter);
  if (checkMapped("pingStats"))
    W_MapWindow(pStats);
  if (checkMapped("review_phaser"))
    {
      W_MapWindow(phaserwin);
      phaserWindow = 1;
    }
  if (checkMappedPref("player", 1))
    W_MapWindow(playerw);
  if (checkMappedPref("review", 1))
    W_MapWindow(reviewWin);
  if (checkMapped("UDP"))
    udpwindow(udpWin);

#ifdef SHORT_PACKETS
  if (checkMapped("network"))
    spwindow();
#endif

}

savebitmaps(void)
{
  register int i;

  for (i = 0; i < VIEWS; i++)
    {

#ifdef TNG_FED_BITMAPS
      tng_fed_bitmaps[SCOUT][i] =
	  W_StoreBitmap(fed_scout_width, fed_scout_height,
			tng_fed_scout_bits[i], w);
      tng_fed_bitmaps[DESTROYER][i] =
	  W_StoreBitmap(fed_destroyer_width, fed_destroyer_height,
			tng_fed_destroyer_bits[i], w);
      tng_fed_bitmaps[CRUISER][i] =
	  W_StoreBitmap(fed_cruiser_width, fed_cruiser_height,
			tng_fed_cruiser_bits[i], w);
      tng_fed_bitmaps[BATTLESHIP][i] =
	  W_StoreBitmap(fed_battleship_width, fed_battleship_height,
			tng_fed_battleship_bits[i], w);
      tng_fed_bitmaps[ASSAULT][i] =
	  W_StoreBitmap(fed_assault_width, fed_assault_height,
			tng_fed_assault_bits[i], w);
      tng_fed_bitmaps[STARBASE][i] =
	  W_StoreBitmap(fed_starbase_width, fed_starbase_height,
			fed_starbase_bits[i], w);
      tng_fed_bitmaps[SGALAXY][i] =
	  W_StoreBitmap(fed_galaxy_width, fed_galaxy_width,
			fed_galaxy_bits[i], w);
      tng_fed_bitmaps[ATT][i] =
	  W_StoreBitmap(fed_cruiser_width, fed_cruiser_height,
			fed_cruiser_bits[i], w);
#endif

      ROMVLVS_bitmap[i] =
	  W_StoreBitmap(rom_cruiser_width, rom_cruiser_height,
			ROMVLVS_bits[i], w);
      noinfoplanet = W_StoreBitmap(planet_width, planet_height, noinfo_bits, w);
      fed_bitmaps[SCOUT][i] =
	  W_StoreBitmap(fed_scout_width, fed_scout_height,
			fed_scout_bits[i], w);
      fed_bitmaps[DESTROYER][i] =
	  W_StoreBitmap(fed_destroyer_width, fed_destroyer_height,
			fed_destroyer_bits[i], w);
      fed_bitmaps[CRUISER][i] =
	  W_StoreBitmap(fed_cruiser_width, fed_cruiser_height,
			fed_cruiser_bits[i], w);
      fed_bitmaps[BATTLESHIP][i] =
	  W_StoreBitmap(fed_battleship_width, fed_battleship_height,
			fed_battleship_bits[i], w);
      fed_bitmaps[ASSAULT][i] =
	  W_StoreBitmap(fed_assault_width, fed_assault_height,
			fed_assault_bits[i], w);
      fed_bitmaps[STARBASE][i] =
	  W_StoreBitmap(fed_starbase_width, fed_starbase_height,
			fed_starbase_bits[i], w);
      fed_bitmaps[SGALAXY][i] =
	  W_StoreBitmap(fed_galaxy_width, fed_galaxy_width,
			fed_galaxy_bits[i], w);
      fed_bitmaps[ATT][i] =
	  W_StoreBitmap(fed_cruiser_width, fed_cruiser_height,
			fed_cruiser_bits[i], w);

      kli_bitmaps[SCOUT][i] =
	  W_StoreBitmap(kli_scout_width, kli_scout_height,
			kli_scout_bits[i], w);
      kli_bitmaps[DESTROYER][i] =
	  W_StoreBitmap(kli_destroyer_width, kli_destroyer_height,
			kli_destroyer_bits[i], w);
      kli_bitmaps[CRUISER][i] =
	  W_StoreBitmap(kli_cruiser_width, kli_cruiser_height,
			kli_cruiser_bits[i], w);
      kli_bitmaps[BATTLESHIP][i] =
	  W_StoreBitmap(kli_battleship_width, kli_battleship_height,
			kli_battleship_bits[i], w);
      kli_bitmaps[ASSAULT][i] =
	  W_StoreBitmap(kli_assault_width, kli_assault_height,
			kli_assault_bits[i], w);
      kli_bitmaps[STARBASE][i] =
	  W_StoreBitmap(kli_starbase_width, kli_starbase_height,
			kli_starbase_bits[i], w);
      kli_bitmaps[SGALAXY][i] =
	  W_StoreBitmap(kli_galaxy_width, kli_galaxy_width,
			kli_galaxy_bits[i], w);
      kli_bitmaps[ATT][i] =
	  W_StoreBitmap(kli_cruiser_width, kli_cruiser_height,
			kli_cruiser_bits[i], w);

      rom_bitmaps[SCOUT][i] =
	  W_StoreBitmap(rom_scout_width, rom_scout_height,
			rom_scout_bits[i], w);
      rom_bitmaps[DESTROYER][i] =
	  W_StoreBitmap(rom_destroyer_width, rom_destroyer_height,
			rom_destroyer_bits[i], w);
      rom_bitmaps[CRUISER][i] =
	  W_StoreBitmap(rom_cruiser_width, rom_cruiser_height,
			rom_cruiser_bits[i], w);
      rom_bitmaps[BATTLESHIP][i] =
	  W_StoreBitmap(rom_battleship_width, rom_battleship_height,
			rom_battleship_bits[i], w);
      rom_bitmaps[ASSAULT][i] =
	  W_StoreBitmap(rom_assault_width, rom_assault_height,
			rom_assault_bits[i], w);
      rom_bitmaps[STARBASE][i] =
	  W_StoreBitmap(rom_starbase_width, rom_starbase_height,
			rom_starbase_bits[i], w);
      rom_bitmaps[SGALAXY][i] =
	  W_StoreBitmap(rom_galaxy_width, rom_galaxy_width,
			rom_galaxy_bits[i], w);
      rom_bitmaps[ATT][i] =
	  W_StoreBitmap(rom_cruiser_width, rom_cruiser_height,
			rom_cruiser_bits[i], w);

      ori_bitmaps[SCOUT][i] =
	  W_StoreBitmap(ori_scout_width, ori_scout_height,
			ori_scout_bits[i], w);
      ori_bitmaps[DESTROYER][i] =
	  W_StoreBitmap(ori_destroyer_width, ori_destroyer_height,
			ori_destroyer_bits[i], w);
      ori_bitmaps[CRUISER][i] =
	  W_StoreBitmap(ori_cruiser_width, ori_cruiser_height,
			ori_cruiser_bits[i], w);
      ori_bitmaps[BATTLESHIP][i] =
	  W_StoreBitmap(ori_battleship_width, ori_battleship_height,
			ori_battleship_bits[i], w);
      ori_bitmaps[ASSAULT][i] =
	  W_StoreBitmap(ori_assault_width, ori_assault_height,
			ori_assault_bits[i], w);
      ori_bitmaps[STARBASE][i] =
	  W_StoreBitmap(ori_starbase_width, ori_starbase_height,
			ori_starbase_bits[i], w);
      ori_bitmaps[SGALAXY][i] =
	  W_StoreBitmap(ori_galaxy_width, ori_galaxy_width,
			ori_galaxy_bits[i], w);
      ori_bitmaps[ATT][i] =
	  W_StoreBitmap(ori_cruiser_width, ori_cruiser_height,
			ori_cruiser_bits[i], w);

      ind_bitmaps[SCOUT][i] =
	  W_StoreBitmap(ind_scout_width, ind_scout_height,
			ind_scout_bits[i], w);
      ind_bitmaps[DESTROYER][i] =
	  W_StoreBitmap(ind_destroyer_width, ind_destroyer_height,
			ind_destroyer_bits[i], w);
      ind_bitmaps[CRUISER][i] =
	  W_StoreBitmap(ind_cruiser_width, ind_cruiser_height,
			ind_cruiser_bits[i], w);
      ind_bitmaps[BATTLESHIP][i] =
	  W_StoreBitmap(ind_battleship_width, ind_battleship_height,
			ind_battleship_bits[i], w);
      ind_bitmaps[ASSAULT][i] =
	  W_StoreBitmap(ind_assault_width, ind_assault_height,
			ind_assault_bits[i], w);
      ind_bitmaps[STARBASE][i] =
	  W_StoreBitmap(ind_starbase_width, ind_starbase_height,
			ind_starbase_bits[i], w);
      ind_bitmaps[SGALAXY][i] =
	  W_StoreBitmap(ind_galaxy_width, ind_galaxy_height,
			ind_galaxy_bits[i], w);
      ind_bitmaps[ATT][i] =
	  W_StoreBitmap(ind_cruiser_width, ind_cruiser_height,
			ind_cruiser_bits[i], w);
    }

  clockpic = W_StoreBitmap(clock_width, clock_height, clock_bits, qwin);

#ifdef BEEPLITE
  for (i = 0; i < emph_player_seq_frames; i++)
    {
      emph_player_seq[emph_player_seq_frames - (i + 1)] =
	  W_StoreBitmap(emph_player_seq_width, emph_player_seq_height,
			emph_player_seq_bits[i], mapw);
    }

  for (i = 0; i < emph_player_seql_frames; i++)
    {
      emph_player_seql[emph_player_seql_frames - (i + 1)] =
	  W_StoreBitmap(emph_player_seql_width, emph_player_seql_height,
			emph_player_seql_bits[i], w);
    }

  for (i = 0; i < emph_planet_seq_frames; i++)
    {
      emph_planet_seq[emph_planet_seq_frames - (i + 1)] =
	  W_StoreBitmap(emph_planet_seq_width, emph_planet_seq_height,
			emph_planet_seq_bits[i], mapw);
    }
#endif

  for (i = 0; i < 5; i++)
    {
      cloud[i] = W_StoreBitmap(cloud_width, cloud_height, cloud_bits[4 - i], w);
      plasmacloud[i] = W_StoreBitmap(plasmacloud_width,
			    plasmacloud_height, plasmacloud_bits[4 - i], w);
    }
  etorp = W_StoreBitmap(etorp_width, etorp_height, etorp_bits, w);
  mtorp = W_StoreBitmap(mtorp_width, mtorp_height, mtorp_bits, w);
  eplasmatorp =
      W_StoreBitmap(eplasmatorp_width, eplasmatorp_height, eplasmatorp_bits, w);
  mplasmatorp =
      W_StoreBitmap(mplasmatorp_width, mplasmatorp_height, mplasmatorp_bits, w);
  bplanets[0] = W_StoreBitmap(planet_width, planet_height, indplanet_bits, w);
  bplanets[1] = W_StoreBitmap(planet_width, planet_height, fedplanet_bits, w);
  bplanets[2] = W_StoreBitmap(planet_width, planet_height, romplanet_bits, w);
  bplanets[3] = W_StoreBitmap(planet_width, planet_height, kliplanet_bits, w);
  bplanets[4] = W_StoreBitmap(planet_width, planet_height, oriplanet_bits, w);
  bplanets[5] = W_StoreBitmap(planet_width, planet_height, planet_bits, w);
  bplanets[6] = W_StoreBitmap(planet_width, planet_height, myplanet000_bits, w);

  mbplanets[0] = W_StoreBitmap(mplanet_width, mplanet_height, indmplanet_bits, mapw);
  mbplanets[1] = W_StoreBitmap(mplanet_width, mplanet_height, fedmplanet_bits, mapw);
  mbplanets[2] = W_StoreBitmap(mplanet_width, mplanet_height, rommplanet_bits, mapw);
  mbplanets[3] = W_StoreBitmap(mplanet_width, mplanet_height, klimplanet_bits, mapw);
  mbplanets[4] = W_StoreBitmap(mplanet_width, mplanet_height, orimplanet_bits, mapw);
  mbplanets[5] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet_bits, mapw);
  bplanets2[0] = bplanets[0];
  mbplanets2[0] = mbplanets[0];
  bplanets2[1] = W_StoreBitmap(planet_width, planet_height, planet001_bits, w);
  bplanets2[2] = W_StoreBitmap(planet_width, planet_height, planet010_bits, w);
  bplanets2[3] = W_StoreBitmap(planet_width, planet_height, planet011_bits, w);
  bplanets2[4] = W_StoreBitmap(planet_width, planet_height, planet100_bits, w);
  bplanets2[5] = W_StoreBitmap(planet_width, planet_height, planet101_bits, w);
  bplanets2[6] = W_StoreBitmap(planet_width, planet_height, planet110_bits, w);
  bplanets2[7] = W_StoreBitmap(planet_width, planet_height, planet111_bits, w);
  mbplanets2[1] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet001_bits, mapw);
  mbplanets2[2] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet010_bits, mapw);
  mbplanets2[3] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet011_bits, mapw);
  mbplanets2[4] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet100_bits, mapw);
  mbplanets2[5] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet101_bits, mapw);
  mbplanets2[6] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet110_bits, mapw);
  mbplanets2[7] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet111_bits, mapw);

  bplanets3[0] = W_StoreBitmap(planet_width, planet_height, myplanet000_bits, w);
  bplanets3[1] = W_StoreBitmap(planet_width, planet_height, myplanet001_bits, w);
  bplanets3[2] = W_StoreBitmap(planet_width, planet_height, myplanet010_bits, w);
  bplanets3[3] = W_StoreBitmap(planet_width, planet_height, myplanet011_bits, w);
  bplanets3[4] = W_StoreBitmap(planet_width, planet_height, myplanet100_bits, w);
  bplanets3[5] = W_StoreBitmap(planet_width, planet_height, myplanet101_bits, w);
  bplanets3[6] = W_StoreBitmap(planet_width, planet_height, myplanet110_bits, w);
  bplanets3[7] = W_StoreBitmap(planet_width, planet_height, myplanet111_bits, w);

  /* <isae> Added this */
  mbplanets3[0] = W_StoreBitmap(mplanet_width, mplanet_height, myindmplanet_bits, mapw);
  mbplanets3[1] = W_StoreBitmap(mplanet_width, mplanet_height, mymplanet001_bits, mapw);
  mbplanets3[2] = W_StoreBitmap(mplanet_width, mplanet_height, mymplanet010_bits, mapw);
  mbplanets3[3] = W_StoreBitmap(mplanet_width, mplanet_height, mymplanet011_bits, mapw);
  mbplanets3[4] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet100_bits, mapw);
  mbplanets3[5] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet101_bits, mapw);
  mbplanets3[6] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet110_bits, mapw);
  mbplanets3[7] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet111_bits, mapw);

  bplanets4[0] = W_StoreBitmap(planet_width, planet_height, rmyplanet000_bits, w);
  bplanets4[1] = W_StoreBitmap(planet_width, planet_height, rmyplanet001_bits, w);
  bplanets4[2] = W_StoreBitmap(planet_width, planet_height, rmyplanet010_bits, w);
  bplanets4[3] = W_StoreBitmap(planet_width, planet_height, rmyplanet011_bits, w);
  bplanets4[4] = W_StoreBitmap(planet_width, planet_height, rmyplanet100_bits, w);
  bplanets4[5] = W_StoreBitmap(planet_width, planet_height, rmyplanet101_bits, w);
  bplanets4[6] = W_StoreBitmap(planet_width, planet_height, rmyplanet110_bits, w);
  bplanets4[7] = W_StoreBitmap(planet_width, planet_height, rmyplanet111_bits, w);

  mbplanets4[0] = W_StoreBitmap(mplanet_width, mplanet_height, rmyindmplanet_bits, mapw);
  mbplanets4[1] = W_StoreBitmap(mplanet_width, mplanet_height, rmymplanet001_bits, mapw);
  mbplanets4[2] = W_StoreBitmap(mplanet_width, mplanet_height, rmymplanet010_bits, mapw);
  mbplanets4[3] = W_StoreBitmap(mplanet_width, mplanet_height, rmymplanet011_bits, mapw);
  mbplanets4[4] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet100_bits, mapw);
  mbplanets4[5] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet101_bits, mapw);
  mbplanets4[6] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet110_bits, mapw);
  mbplanets4[7] = W_StoreBitmap(mplanet_width, mplanet_height, mplanet111_bits, mapw);

  for (i = 0; i < EX_FRAMES; i++)
    {
      expview[i] = W_StoreBitmap(ex_width, ex_height, ex_bits[i], w);
    }
  for (i = 0; i < SBEXPVIEWS; i++)
    {
      sbexpview[i] = W_StoreBitmap(sbexp_width, sbexp_height, sbexp_bits[i], w);
    }

#ifndef VSHIELD_BITMAPS
  shield = W_StoreBitmap(shield_width, shield_height, shield_bits, w);
#else
  for (i = 0; i < SHIELD_FRAMES; i++)
    shield[i] = W_StoreBitmap(shield_width, shield_height, shield_bits[i], w);
#endif

#ifdef VARY_HULL
  for (i = 0; i < HULL_FRAMES; i++)
    hull[i] = W_StoreBitmap(hull_width, hull_height, hull_bits[i], w);
#endif

  cloakicon = W_StoreBitmap(cloak_width, cloak_height, cloak_bits, w);
  icon = W_StoreBitmap(icon_width, icon_height, icon_bits, iconWin);
  tractbits = W_StoreBitmap(tract_width, tract_height, tract_bits, w);
  pressbits = W_StoreBitmap(press_width, press_height, press_bits, w);
}

/* This routine throws up an entry window for the player. */

entrywindow(int *team, int *s_type)
{
  int     typeok = 0, i = 0;
  time_t  startTime;
  W_Event event;
  int     lastplayercount[4];
  int     okayMask, lastOkayMask;
  int     resetting = 0;
  int     tiled = 0;
  time_t  quittime = 60;
  time_t  lasttime = -1;
  time_t  spareTime = 1000;			 /* Allow them an extra 240 * 

						  * 
						  * 
						  * * seconds, as LONG */

  /* as they are active */
  fd_set  mask;

  /* fast quit? - jn */
  if (fastQuit)
    {
      *team = -1;
      return;
    }

  /* The following allows quick choosing of teams */

  lastOkayMask = okayMask = tournMask;

  for (i = 0; i < 4; i++)
    {
      if (okayMask & (1 << i))
	{
	  tiled = 0;
	}
      else
	{
	  tiled = 1;
	}

      if (tiled)
	{
	  W_TileWindow(teamWin[i], stipple);
	}
      else
	{
	  W_UnTileWindow(teamWin[i]);
	}
      W_MapWindow(teamWin[i]);
      lastplayercount[i] = -1;			 /* force redraw first time * 
						  * 
						  * * through */
    }
  W_MapWindow(qwin);

  *team = -1;
  startTime = time(0);
  if (me->p_whydead != KWINNER && me->p_whydead != KGENOCIDE)
    showMotd(w, line);

  run_clock(startTime);
  updatedeath();

  if (remap[me->p_team] == NOBODY)
    RedrawPlayerList();				 /* When you first login */
  else
    UpdatePlayerList();				 /* Otherwise */


  quittime = (time_t) intDefault("autoquit", quittime);		/* allow *
								 * extra */
  /* quit time -RW */

  do
    {
      while (!W_EventsPending())
	{
	  time_t  elapsed;
	  fd_set  rfds;
	  struct timeval tv;

#ifndef HAVE_WIN32
	  W_FullScreen(baseWin);
	  tv.tv_sec = 1;
#else
	  /* Since we don't have a socket to check on Win32 for windowing *
	   * system events, we set the timeout to zero and effectively poll.
	   * * Yes, I could do the correct thing and call *
	   * WaitForMultipleObjects() etc. but I don't feel like it */
	  tv.tv_sec = 0;
#endif

	  tv.tv_usec = 0;
	  FD_ZERO(&rfds);

#ifndef HAVE_WIN32
	  FD_SET(W_Socket(), &rfds);
#endif

	  FD_SET(sock, &rfds);
	  if (udpSock >= 0)
	    FD_SET(udpSock, &rfds);
	  SELECT(32, &rfds, 0, 0, &tv);		 /* hmm,  32 might be too * * 
						  * small */

	  if (FD_ISSET(sock, &rfds) ||
	      (udpSock >= 0 && FD_ISSET(udpSock, &rfds)))
	    {
	      readFromServer(&rfds);
	    }
	  elapsed = time(0) - startTime;
	  if (elapsed > quittime)
	    {
	      printf("Auto-Quit.\n");
	      *team = 4;
	      break;
	    }

#ifndef HAVE_WIN32
	  map();				 /* jmn - update galactic */
#endif

	  if (lasttime != time(0))
	    {
	      run_clock(lasttime);
	      updatedeath();
	      if (W_IsMapped(playerw))
		UpdatePlayerList();
	      showTimeLeft(elapsed, quittime);
	      lasttime = time(0);
	    }

	  okayMask = tournMask;

	  for (i = 0; i < 4; i++)
	    {
	      if ((okayMask ^ lastOkayMask) & (1 << i))
		{
		  if (okayMask & (1 << i))
		    {
		      W_UnTileWindow(teamWin[i]);
		    }
		  else
		    {
		      W_TileWindow(teamWin[i], stipple);
		    }
		  lastplayercount[i] = -1;	 /* force update */
		}
	      redrawTeam(teamWin[i], i, &lastplayercount[i]);
	    }
	  lastOkayMask = okayMask;
	}
      if (*team == 4)
	break;

      if (time(0) - startTime <= spareTime)
	{
	  spareTime -= time(0) - startTime;
	  startTime = time(0);
	}
      else
	{
	  startTime += spareTime;
	  spareTime = 0;
	}
      if (!W_EventsPending())
	continue;
      W_NextEvent(&event);
      typeok = 1;
      switch ((int) event.type)
	{
	case W_EV_KEY:
	  switch (event.key)
	    {
	    case 'q':
	      *team = -1;
	      return;
	    case 's':
	      *s_type = SCOUT;
	      break;
	    case 'd':
	      *s_type = DESTROYER;
	      break;
	    case 'c':
	      *s_type = CRUISER;
	      break;
	    case 'b':
	      *s_type = BATTLESHIP;
	      break;
	    case 'g':
	      *s_type = SGALAXY;
	      break;
	    case 'X':
	      *s_type = ATT;
	      break;
	    case 'a':
	      *s_type = ASSAULT;
	      break;
	    case 'o':
	      *s_type = STARBASE;
	      break;
	    default:
	      typeok = 0;
	      break;
	    }
	  if (event.Window == w)
	    {
	      switch (event.key)
		{
		case 'y':
		  if (resetting)
		    {
		      sendResetStatsReq('Y');
		      warning("OK, your reset request has been sent to the server.");
		      resetting = 0;
		    }
		  break;
		case 'n':
		  if (resetting)
		    {
		      warning("Yeah, WHATever.");
		      resetting = 0;
		    }
		  break;
		case 'R':
		  warning("Please confirm reset request. (y/n)");
		  resetting = 1;
		  break;
		case 'f':			 /* Scroll motd forward */
		  line = line + 28;
		  if (line > MaxMotdLine)
		    {
		      line = line - 28;
		      break;
		    }
		  W_ClearWindow(w);
		  showMotd(w, line);
		  break;
		case 'b':			 /* Scroll motd backward */
		  if (line == 0)
		    break;
		  line = line - 28;
		  if (line < 0)
		    line = 0;
		  W_ClearWindow(w);
		  showMotd(w, line);
		  break;
		case 'F':			 /* Scroll motd forward */
		  line = line + 4;
		  if (line > MaxMotdLine)
		    {
		      line = line - 4;
		      break;
		    }
		  W_ClearWindow(w);
		  showMotd(w, line);
		  break;
		case 'B':			 /* Scroll motd backward */
		  if (line == 0)
		    break;
		  line = line - 4;
		  if (line < 0)
		    line = 0;
		  W_ClearWindow(w);
		  showMotd(w, line);
		  break;
		}
	    }
	  /* No break, we just fall through */
	case W_EV_BUTTON:
	  if (typeok == 0)
	    break;
	  for (i = 0; i < 4; i++)
	    if (event.Window == teamWin[i])
	      {
		*team = i;
		break;
	      }
	  if (event.Window == qwin /* new */  &&
	      event.type == W_EV_BUTTON)
	    {
	      *team = 4;
	      break;
	    }
	  if (*team != -1 && !teamRequest(*team, *s_type))
	    {
	      *team = -1;
	    }
	  break;
	case W_EV_EXPOSE:
	  for (i = 0; i < 4; i++)
	    if (event.Window == teamWin[i])
	      {
		lastplayercount[i] = -1;	 /* force update */
		redrawTeam(teamWin[i], i, &lastplayercount[i]);
		break;
	      }
	  if (event.Window == qwin)
	    {
	      run_clock(lasttime);
	      redrawQuit();
	    }
	  else if (event.Window == tstatw)
	    redrawTstats();
	  else if (event.Window == iconWin)
	    drawIcon();
	  else if (event.Window == w)
	    {
	      run_clock(lasttime);
	      showMotd(w, line);
	    }
	  else if (event.Window == helpWin)
	    fillhelp();

#ifdef NBT
	  else if (event.Window == macroWin)
	    fillmacro();
#endif

	  else if (event.Window == playerw)
	    RedrawPlayerList();
	  else if (event.Window == warnw)
	    W_ClearWindow(warnw);
	  else if (event.Window == messagew)
	    W_ClearWindow(messagew);
	  break;
	}
    }
  while (*team < 0);
  if (event.Window != qwin)
    {
      char    buf[80];

      sprintf(buf, "Welcome aboard %s!", ranks[me->p_stats.st_rank].name);
      warning(buf);
    }

  if (*team == 4)
    {
      *team = -1;
      return;
    }

  for (i = 0; i < 4; i++)
    W_UnmapWindow(teamWin[i]);
  W_UnmapWindow(qwin);
}

/* Attempt to pick specified team & ship */
teamRequest(int team, int ship)
{
  time_t  lastTime;

  pickOk = -1;
  sendTeamReq(team, ship);
  lastTime = time(NULL);
  while (pickOk == -1)
    {
      if (lastTime + 3 < time(NULL))
	{
	  sendTeamReq(team, ship);
	}
      socketPause();
      readFromServer(NULL);
      if (isServerDead())
	{
	  printf("Oh SHIT,  We've been ghostbusted!\n");
	  printf("hope you weren't in a base\n");

#ifdef HAVE_XPM
	  W_GalacticBgd(GHOST_PIX);
#endif

	  /* UDP fail-safe */
	  commMode = commModeReq = COMM_TCP;
	  commSwitchTimeout = 0;
	  if (udpSock >= 0)
	    closeUdpConn();
	  if (udpWin)
	    {
	      udprefresh(UDP_CURRENT);
	      udprefresh(UDP_STATUS);
	    }
	  connectToServer(nextSocket);
	  printf(" We've been resurrected!\n");
	  pickOk = 0;
	  break;
	}
    }
  return (pickOk);
}

numShips(int owner)
{
  int     i, num = 0;
  struct player *p;

  for (i = 0, p = players; i < MAXPLAYER; i++, p++)
    if (p->p_status == PALIVE && p->p_team == owner)
      num++;
  return (num);
}

realNumShips(int owner)
{
  int     i, num = 0;
  struct player *p;

  for (i = 0, p = players; i < MAXPLAYER; i++, p++)
    if (p->p_status != PFREE &&
	p->p_team == owner)
      num++;
  return (num);
}

deadTeam(int owner)

/* The team is dead if it has no planets and cannot coup it's home planet */
{
  int     i, num = 0;
  struct planet *p;

  if (planets[remap[owner] * 10 - 10].pl_couptime == 0)
    return (0);
  for (i = 0, p = planets; i < MAXPLANETS; i++, p++)
    {
      if (p->pl_owner & owner)
	{
	  num++;
	}
    }
  if (num != 0)
    return (0);
  return (1);
}

checkBold(char *line)
/* Determine if that line should be highlighted on sign-on screen */
/* Which is done when it is the players own score being displayed */

{
  char   *s, *t;
  int     i;
  int     end = 0;

  if (strlen(line) < 60)
    return (0);
  s = line + 4;
  t = me->p_name;

  if (me == NULL)
    return (0);

  for (i = 0; i < 16; i++)
    {
      if (!end)
	{
	  if (*t == '\0')
	    end = 1;
	  else if (*t != *s)
	    return (0);
	}
      if (end)
	{
	  if (*s != ' ')
	    return (0);
	}
      s++;
      t++;
    }
  return (1);
}

struct list
{
  char    bold;
  struct list *next;
  char   *data;
};
static struct list *motddata = NULL;		 /* pointer to first bit of * 

						  * 
						  * 
						  * * motddata */
static int first = 1;

showMotd(W_Window motdwin, int atline)
{
  int     i, length, top, center;
  struct list *data;
  int     count;
  char    buf[128];

return;

  sprintf(buf, "---  %s  ---", (char *) query_cowid());
  length = strlen(buf);
  center = TWINSIDE / 2 - (length * W_Textwidth) / 2;
  W_WriteText(motdwin, center, W_Textheight, textColor,
	      buf, length, W_BoldFont);
  sprintf(buf, cbugs);
  length = strlen(buf);
  center = TWINSIDE / 2 - (length * W_Textwidth) / 2;
  W_WriteText(motdwin, center, 3 * W_Textheight, textColor,
	      buf, length, W_RegularFont);

  top = 10;

  if (first)
    {
      first = 0;
      data = motddata;
      while (data != NULL)
	{
	  data->bold = checkBold(data->data);
	  data = data->next;
	}
    }

  data = motddata;
  for (i = 0; i < atline; i++)
    {
      if (data == NULL)
	{
	  atline = 0;
	  data = motddata;
	  break;
	}
      data = data->next;
    }
  count = 28;					 /* Magical # of lines to * * 
						  * display */
  for (i = top; i < 50; i++)
    {
      if (data == NULL)
	break;
      if (!strcmp(data->data, "\t@@@"))		 /* ATM */
	break;
      if (data->bold)
	{
	  W_WriteText(motdwin, 20, i * W_Textheight, textColor, data->data,
		      strlen(data->data), W_BoldFont);
	}
      else
	{
	  W_WriteText(motdwin, 20, i * W_Textheight, textColor, data->data,
		      strlen(data->data), W_RegularFont);
	}
      data = data->next;
      count--;
      if (count <= 0)
	break;
    }

  showValues(data);
}

/* ATM: show the current values of the .sysdef parameters. */
showValues(struct list *data)
{
  int     i;
  static char *msg = "OPTIONS SET WHEN YOU STARTED WERE:";

  /* try to find the start of the info */
  while (1)
    {
      if (data == NULL)
	return;
      if (!strcmp(data->data, STATUS_TOKEN))
	break;
      data = data->next;
    }
  data = data->next;

  W_WriteText(mapw, 20, 14 * W_Textheight, textColor, msg,
	      strlen(msg), W_RegularFont);
  for (i = 16; i < 50; i += 2)
    {
      if (data == NULL)
	break;
      if (data->data[0] == '+')			 /* quick boldface hack */
	W_WriteText(mapw, 20, i * W_Textheight, textColor, data->data + 1,
		    strlen(data->data) - 1, W_BoldFont);
      else
	W_WriteText(mapw, 20, i * W_Textheight, textColor, data->data,
		    strlen(data->data), W_RegularFont);
      data = data->next;
    }
}

newMotdLine(char *line)
{
  static struct list **temp = &motddata;
  static int statmode = 0;			 /* ATM */

  if (!statmode && !strcmp(line, STATUS_TOKEN))
    statmode = 1;
  if (!statmode)
    MaxMotdLine++;				 /* ATM - don't show on left */
  (*temp) = (struct list *) malloc(sizeof(struct list));

  if ((*temp) == NULL)
    {						 /* malloc error checking --
						  * * * 10/30/92 EM */
      printf("Warning:  Couldn't malloc space for a new motd line!");
      return;
    }
  /* Motd clearing code */
  if (strcmp(line, MOTDCLEARLINE) == 0)
    {
      ClearMotd();
      return;
    }

  (*temp)->next = NULL;
  (*temp)->data = malloc(strlen(line) + 1);
  strcpy((*temp)->data, line);
  temp = &((*temp)->next);
}

/* Free the current motdData */
ClearMotd(void)
{
  struct list *temp, *temp2;

  temp = motddata;				 /* start of motd information 
						  * 
						  */
  while (temp != NULL)
    {
      temp2 = temp;
      temp = temp->next;
      free(temp2->data);
      free(temp2);
    }

  first = 1;					 /* so that it'll check bold
						  * * * next time around */
}

/* ARGSUSED */
getResources(char *prog)
{
  getColorDefs();
  getTiles();
}

getTiles(void)
{
  stipple = W_StoreBitmap(stipple_width, stipple_height, stipple_bits, w);
}

redrawTeam(W_Window win, int teamNo, int *lastnum)
{
  char    buf[BUFSIZ];
  static char *teams[] =
  {"Federation", "Romulan", "Klingon", "Orion"};
  int     num = numShips(1 << teamNo);

  /* Only redraw if number of players has changed */
  if (*lastnum == num)
    return;

  W_ClearWindow(win);
  W_WriteText(win, 5, 5, shipCol[teamNo + 1], teams[teamNo],
	      strlen(teams[teamNo]), W_RegularFont);
  (void) sprintf(buf, "%d", num);
  W_MaskText(win, 5, 46, shipCol[teamNo + 1], buf, strlen(buf),
	     W_BigFont);
  *lastnum = num;
}

redrawQuit(void)
{
  W_WriteText(qwin, 5, 5, textColor, "Quit COW  ", 10, W_RegularFont);
}

void    drawIcon(void)
{
  W_WriteBitmap(0, 0, icon, W_White);
}

#define CLOCK_WID       (BOXSIDE * 9 / 10)
#define CLOCK_HEI       (BOXSIDE * 2 / 3)
#define CLOCK_BDR       0
#define CLOCK_X         (BOXSIDE / 2 - CLOCK_WID / 2)
#define CLOCK_Y         (BOXSIDE / 2 - CLOCK_HEI / 2)

#define XPI             3.141592654

showTimeLeft(time_t time, time_t max)
{
  char    buf[BUFSIZ], *cp;
  int     cx, cy, ex, ey, tx, ty;

  if ((max - time) < 10 && time & 1)
    {
      W_Beep();
    }
  /* XFIX */
  W_ClearArea(qwin, CLOCK_X, CLOCK_Y, CLOCK_WID, CLOCK_HEI);

  cx = CLOCK_X + CLOCK_WID / 2;
  cy = CLOCK_Y + (CLOCK_HEI - W_Textheight) / 2;
  ex = cx - clock_width / 2;
  ey = cy - clock_height / 2;
  W_WriteBitmap(ex, ey, clockpic, foreColor);

  ex = cx - clock_width * sin(2 * XPI * time / max) / 2;
  ey = cy - clock_height * cos(2 * XPI * time / max) / 2;
  W_MakeLine(qwin, cx, cy, ex, ey, foreColor);

  sprintf(buf, "%d", max - time);
  tx = cx - W_Textwidth * strlen(buf) / 2;
  ty = cy - W_Textheight / 2;
  W_WriteText(qwin, tx, ty, textColor, buf, strlen(buf), W_RegularFont);

  cp = "Auto Quit";
  tx = CLOCK_X + cx - W_Textwidth * strlen(cp) / 2;
  ty = CLOCK_Y + CLOCK_HEI - W_Textheight;
  W_WriteText(qwin, tx, ty, textColor, cp, strlen(cp), W_RegularFont);
}
