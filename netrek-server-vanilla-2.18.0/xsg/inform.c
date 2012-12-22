/*
 * inform.c
 */
#include "copyright.h"

#include <stdio.h>
#include <math.h>
#include <signal.h>
#include "Wlib.h"
#include "defs.h"
#include "xsg_defs.h"
#include "struct.h"
#include "localdata.h"
#include "ltd_stats.h"

/* Display information about the nearest object to mouse */

/*
** When the player asks for info, this routine finds the object
** nearest the mouse, either player or planet, and pop up a window
** with the desired information in it.
** 
** We intentionally provide less information than is actually
** available.  Keeps the fog of war up.
**
** There is a different sized window for each type player/planet
** and we take care to keep it from extending beyond the main
** window boundaries.
*/

static char *classes[NUM_TYPES] = {
    "SC", "DD", "CA", "BB", "AS", "SB",
#ifdef SGALAXY
    "GA",
#endif
    "??"
   };

inform(ww, x, y, key)
W_Window ww;
int x, y;
char key;
{
    char buf[BUFSIZ];
    int line = 0;
    register struct player *j;
    register struct planet *k;
    int mx, my;
    struct obtype *gettarget(), *target;
    int windowWidth, windowHeight;

    mx=x;
    my=y;
    infomapped = 1;
    if (key == 'i') {
	target = gettarget(ww, x, y, TARG_PLAYER|TARG_PLANET);
    } else {
	target = gettarget(ww, x, y, TARG_PLAYER|TARG_SELF);
    }
    if(!target) return;

    /* This is pretty lame.  We make a graphics window for the info window
     *  so we can accurately space the thing to barely fit into the galactic
     *  map or whatever.
     */

    windowWidth=W_WindowWidth(ww);
    windowHeight=W_WindowHeight(ww);
    if (target->o_type == PLAYERTYPE) {
	/* Too close to the edge? */
	if (mx + 23 * W_Textwidth + 2 > windowWidth)
	    mx = windowWidth - 23 * W_Textwidth - 2;
	if (my + 10 * W_Textheight + 2 > windowHeight)
	    my = windowHeight - 10 * W_Textheight - 2;

	infow = W_MakeWindow("info", mx, my, 23*W_Textwidth, 10*W_Textheight, 
	    ww, 2,foreColor);
	W_MapWindow(infow);
	j = &players[target->o_num];
	(void) sprintf(buf, "%s (%c%c):", j->p_name, teamlet[j->p_team], shipnos[j->p_no]);
	W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf), shipFont(j));
	if (key == 'i') {
	    (void) sprintf(buf, "Login   %-s", j->p_login);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf), W_RegularFont);
	    (void) sprintf(buf, "Display %-s", j->p_monitor);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf), W_RegularFont);
	    (void) sprintf(buf, "Speed   %-d", j->p_speed);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf), W_RegularFont);

	    (void) sprintf(buf, "kills   %-4.2f", j->p_kills);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf), W_RegularFont);
	    (void) sprintf(buf, "S-Class %-s", classes[j->p_ship.s_type]);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf), W_RegularFont);
	    
	    if (j->p_swar & me->p_team)
		W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), "WAR", 3, W_RegularFont);
	    else if (j->p_hostile & me->p_team)
		W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), "HOSTILE", 7, W_RegularFont);
	    else
		W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), "PEACEFUL", 8, W_RegularFont);

	    line++;
	    (void) sprintf(buf, "Shld: %3d%%  Dam: %3d%%",
		(100*j->p_shield)/j->p_ship.s_maxshield,
		(100*j->p_damage)/j->p_ship.s_maxdamage);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf), W_BoldFont);
	    (void) sprintf(buf, "Fuel: %3d%%  Armies:%2d",
		(100*j->p_fuel)/j->p_ship.s_maxfuel, j->p_armies);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf), W_BoldFont);

	} else {	/* New information window! */
	    strcpy(buf, "        Rating Total");
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf),
		W_RegularFont);
#ifndef LTD_STATS
	    sprintf(buf, "Bombing: %5.2f  %5d", 
		bombingRating(j),
		j->p_stats.st_armsbomb + j->p_stats.st_tarmsbomb);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf),
		W_RegularFont);
	    sprintf(buf, "Planets: %5.2f  %5d",
		planetRating(j),
		j->p_stats.st_planets + j->p_stats.st_tplanets);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf),
		W_RegularFont);
	    sprintf(buf, "Offense: %5.2f  %5d",
		offenseRating(j),
		j->p_stats.st_kills + j->p_stats.st_tkills);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf),
		W_RegularFont);
	    sprintf(buf, "Defense: %5.2f  %5d", 
		defenseRating(j),
		j->p_stats.st_losses + j->p_stats.st_tlosses);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf),
		W_RegularFont);
	    if (j->p_ship.s_type == STARBASE) {
		sprintf(buf, "  Maxkills: %6.2f", j->p_stats.st_sbmaxkills);
	    } else {
		sprintf(buf, "  Maxkills: %6.2f", j->p_stats.st_maxkills);
	    }
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf),
		W_RegularFont);
	    sprintf(buf, "  Hours:    %6.2f", 
		(float) j->p_stats.st_tticks / 36000.0);
	    W_WriteText(infow, W_Textwidth, W_Textheight*line++, playerColor(j), buf, strlen(buf),
		W_RegularFont);
#endif /* LTD_STATS */
	}    
    }
    else {  /* Planet */
	/* Too close to the edge? */
	if (mx + 20 * W_Textwidth + 2 > windowWidth)
	    mx = windowWidth - 25 * W_Textwidth - 2;
	if (my + 3 * W_Textheight + 2 > windowHeight)
	    my = windowHeight - 3 * W_Textheight - 2;

	infow = W_MakeWindow("info",mx,my,W_Textwidth*25,W_Textheight*3,ww,
	    2,foreColor);
	W_MapWindow(infow);
	k = &planets[target->o_num];
	(void) sprintf(buf, "%s (%c)", k->pl_name, teamlet[k->pl_owner]);
	W_WriteText(infow, W_Textwidth, W_Textheight*line++, planetColor(k), buf, strlen(buf),
	    planetFont(k));
	(void) sprintf(buf, "Armies %d", k->pl_armies);
	W_WriteText(infow, W_Textwidth, W_Textheight*line++, planetColor(k), buf, strlen(buf),
	    W_RegularFont);
	(void) sprintf(buf, "%s %s %s %c%c%c%c",
	    (k->pl_flags & PLREPAIR ? "REPAIR" : "      "),
	    (k->pl_flags & PLFUEL ? "FUEL" : "    "),
	    (k->pl_flags & PLAGRI ? "AGRI" : "    "),
	    (k->pl_info & FED ? 'F' : ' '),
	    (k->pl_info & ROM ? 'R' : ' '),
	    (k->pl_info & KLI ? 'K' : ' '),
	    (k->pl_info & ORI ? 'O' : ' '));
	W_WriteText(infow, W_Textwidth, W_Textheight*line++, planetColor(k), buf, strlen(buf),
	    W_RegularFont);
    }
}


destroyInfo()
{
    W_DestroyWindow(infow);
    infomapped = 0;
}

