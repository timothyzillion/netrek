/*
 * stats.c
 */
#include "copyright.h"

#include <stdio.h>
#include "Wlib.h"
#include "defs.h"
#include "xsg_defs.h"
#include "struct.h"
#include "localdata.h"

#define	MIN(a,b)	(((a) < (b)) ? (a) : (b))

#define	BX_OFF()	((textWidth + 1) * W_Textwidth + S_IBORDER)
#define	BY_OFF(line)	((line) * (W_Textheight + S_IBORDER) + S_IBORDER)
#define	TX_OFF(len)	((textWidth - len) * W_Textwidth + S_IBORDER)
#define	TY_OFF(line)	BY_OFF(line)

#define STAT_WIDTH		160
#define STAT_HEIGHT		BY_OFF(NUM_SLIDERS)
#define STAT_BORDER		2
#define S_IBORDER		5
#define STAT_X			422
#define STAT_Y			13

#define SL_WID			\
	(STAT_WIDTH - 2 * S_IBORDER - (textWidth + 1) * W_Textwidth)
#define SL_HEI			(W_Textheight)

#define NUM_ELS(a)		(sizeof (a) / sizeof (*(a)))
#define NUM_SLIDERS		NUM_ELS(sliders)

typedef struct slider {
	char	*label;
	int	min, max;
	int	low_red, high_red;
	int	label_length;
	int	diff;
	int	*var;
	int	lastVal;
} SLIDER;

typedef struct record {
	int	*data;
	int	last_value;
} RECORD;

static SLIDER	sliders[] = {
	{ "Shields",		0,	100,	20,	100	},
	{ "Damage",		0,	100,	0,	0	},
	{ "Fuel",		0,	10000,	2000,	10000	},
	{ "Warp",		0,	9,	0,	9	},
	{ "Weapon Temp",	0,	1200,	0,	800	},
	{ "Engine Temp",	0,	1200,	0,	800	},
};

static int		textWidth = 0;
static int 		initialized = 0;
static int lastplayerlock = -1;

initStats()
{
    int	i;
    char	*str;
    struct player *pp;

    pp = &players[me->p_playerl];

    calibrate_stats();
    sliders[0].var = &(pp->p_shield);
    sliders[1].var = &(pp->p_damage);
    sliders[2].var = &(pp->p_fuel);
    sliders[3].var = &(pp->p_speed);
    sliders[4].var = &(pp->p_wtemp);
    sliders[5].var = &(pp->p_etemp);
    /* minor kludge: during playback 'players' changes each time 
       meaning we have to reinit these variables each frame */
    if (initialized) return;
    initialized = 1;
    for (i = 0; i < NUM_SLIDERS; i++) {
	sliders[i].label_length = strlen(sliders[i].label);
	textWidth = MAX(textWidth, sliders[i].label_length);
	sliders[i].diff = sliders[i].max - sliders[i].min;
	sliders[i].lastVal=0;
    }
}

redrawStats()
{
    int	i;

    W_ClearWindow(statwin);
    initialized = 0;
    initStats();
    for (i=0; i<NUM_SLIDERS; i++) {
	sliders[i].lastVal=0;
    }
    for (i = 0; i < NUM_SLIDERS; i++) {
	W_WriteText(statwin, TX_OFF(sliders[i].label_length), TY_OFF(i),
	    textColor, sliders[i].label, sliders[i].label_length,
	    W_RegularFont);
	box(0, BX_OFF() - 1, BY_OFF(i) - 1, SL_WID+2, SL_HEI+2, borderColor);
	sliders[i].lastVal=0;
    }
}

updateStats()
{
    int	i, value, diff, old_x, new_x;
    W_Color color;
    SLIDER	*s;
    if (lastplayerlock != me->p_playerl)
	redrawStats();
    else
	initStats();
    lastplayerlock = me->p_playerl;

    for (i = 0; i < NUM_SLIDERS; i++) {
	s = &sliders[i];
	value= *(s->var);
	if (value < s->min)
	    value = s->min;
	else if (value > s->max)
	    value = s->max;
	if (value == s->lastVal)
	    continue;
	diff = value - s->lastVal;
	if (diff < 0) {
	    old_x = s->lastVal * SL_WID / s->diff;
	    new_x = value * SL_WID / s->diff;
	    box(1, BX_OFF()+new_x, BY_OFF(i), old_x-new_x, SL_HEI, backColor);

	    if (s->lastVal >= s->low_red && value < s->low_red) 
		box(1, BX_OFF(), BY_OFF(i), new_x, SL_HEI, warningColor);
	    else if (s->lastVal > s->high_red && value <= s->high_red) 
		box(1, BX_OFF(), BY_OFF(i), new_x, SL_HEI, myColor);
	} else {
	    if (value < s->low_red)
		color = warningColor;
	    else if (value > s->high_red) {
		color = warningColor;
		if (s->lastVal <= s->high_red)
		    s->lastVal = 0;
	    } else {
		color = myColor;
		if (s->lastVal < s->low_red)
		    s->lastVal = 0;
	    }
	    old_x = s->lastVal * SL_WID / s->diff;
	    new_x = value * SL_WID / s->diff;
	    box(1, BX_OFF() + old_x, BY_OFF(i), new_x - old_x, SL_HEI, color);
	}
	s->lastVal=value;
    }
}

box(filled, x, y, wid, hei, color)
int filled, x, y, wid, hei;
W_Color color;
{
    if (wid==0) return;

    if (filled) {
	W_FillArea(statwin, x, y, wid+1, hei+1, color);
	return;
    }

    W_MakeLine(statwin,x,y,x+wid,y,color);
    W_MakeLine(statwin,x+wid,y,x+wid,y+hei,color);
    W_MakeLine(statwin,x+wid,y+hei,x,y+hei,color);
    W_MakeLine(statwin,x,y+hei,x,y,color);
}


calibrate_stats()
{
    register int i;
    struct player *pp;
    
    pp = &players[me->p_playerl];

    sliders[0].max = pp->p_ship.s_maxshield;
    sliders[0].low_red = .20 * ((double) sliders[0].max);
    sliders[0].high_red = sliders[0].max;

    sliders[1].max = pp->p_ship.s_maxdamage;

    sliders[2].max = pp->p_ship.s_maxfuel;
    sliders[2].low_red = .20 * ((double) sliders[2].max);
    sliders[2].high_red = sliders[2].max;

    sliders[3].max = pp->p_ship.s_maxspeed;
    sliders[3].high_red = sliders[3].max;

    sliders[4].max = 1.2 * ((double) pp->p_ship.s_maxwpntemp);
    sliders[4].high_red = .667 * ((double) sliders[4].max);

    sliders[5].max = 1.2 * ((double) pp->p_ship.s_maxegntemp);
    sliders[5].high_red = .667 * ((double) sliders[5].max);

    for (i=0; i<NUM_SLIDERS; i++) 
	sliders[i].diff = sliders[i].max - sliders[i].min;

}
