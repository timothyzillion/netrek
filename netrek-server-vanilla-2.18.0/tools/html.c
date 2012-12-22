#include <stdio.h>
#include <varargs.h>
#include <sys/types.h>
#include <sys/file.h>
#include <pwd.h>
#include <stdlib.h>

#include "defs.h"
#include INC_FCNTL
#include "data.h"
#include "struct.h"

/* There is a time bias towards people with at least 4 hours of play... */
#define TIMEBIAS 4

int daysold = 28;

extern double sqrt(double);

void printout();
int             out = 1;        /* stdout for non-socket connections */

int numplayers = 0;
double sumoffense = 0.0;
double sumdefense = 0.0;
double sumplanets = 0.0;
double sumbombing = 0.0;
double sum2offense = 0.0;
double sum2defense = 0.0;
double sum2planets = 0.0;
double sum2bombing = 0.0;
double avgoffense = 0.0;
double avgdefense = 0.0;
double avgplanets = 0.0;
double avgbombing = 0.0;
double stdoffense = 0.0;
double stddefense = 0.0;
double stdplanets = 0.0;
double stdbombing = 0.0;

struct statentry play_entry;
struct rawdesc *output=NULL;
char mode;
int showall;
struct rawdesc {
  struct rawdesc *next;
  char name[NAME_LEN];
  int ttime;
  int rank;
  float bombing, planets, offense, defense;
  int nbomb, nplan, nkill, ndie;
  int sbkills, sblosses, sbtime;
  float sbmaxkills;
  LONG lastlogin;
#ifdef GENO_COUNT
  int genos;
#endif

}

addNewPlayer(name, pl, mode)
     char *name;
     struct player *pl;
     char mode;
{
  struct rawdesc *temp;
  struct rawdesc **temp2;

  temp=(struct rawdesc *) malloc(sizeof(struct rawdesc));
  strcpy(temp->name, name);
  temp->lastlogin=pl->p_stats.st_lastlogin;
  temp->ttime=pl->p_stats.st_tticks / 10;
  temp->rank=pl->p_stats.st_rank;
  temp->bombing=bombingRating(pl);
  temp->planets=planetRating(pl);
  temp->offense=offenseRating(pl);
  temp->defense=defenseRating(pl);
  temp->nbomb=pl->p_stats.st_tarmsbomb;
  temp->nplan=pl->p_stats.st_tplanets;
  temp->nkill=pl->p_stats.st_tkills;
  temp->ndie=pl->p_stats.st_tlosses;
  temp->sbtime=pl->p_stats.st_sbticks / 10;
  temp->sbkills=pl->p_stats.st_sbkills;
  temp->sblosses=pl->p_stats.st_sblosses;
  temp->sbmaxkills=pl->p_stats.st_sbmaxkills;
#ifdef GENO_COUNT
  temp->genos=pl->p_stats.st_genos;
#endif
  temp2 = &output;

  if (mode=='n') cumestats(temp);

  for (;;) {
    if (((*temp2)==NULL) || ordered(temp, *temp2, mode)) {
      temp->next=(*temp2);
      (*temp2) = temp;
      break;
    }
    temp2= &((*temp2)->next);
  }
}

ordered(num1, num2, mode)
     struct rawdesc *num1, *num2;
     char mode;
{
  float temp, temp2;

  switch(mode) {
  case 'o':
    temp =num1->ttime*(num1->offense)/(num1->ttime/90 + 40 * TIMEBIAS);
    temp2=num2->ttime*(num2->offense)/(num2->ttime/90 + 40 * TIMEBIAS);

    if (temp >= temp2) {
      return(1);
    } else {
      return(0);
    }

  case 'd':
    temp =(float) num1->ttime/(num1->ndie + num1->ttime/90 + 40 * TIMEBIAS);
    temp2=(float) num2->ttime/(num2->ndie + num2->ttime/90 + 40 * TIMEBIAS);

    if (temp >= temp2) {
      return(1);
    } else {
      return(0);
    }
  case 'b':
    temp =num1->ttime*(num1->bombing)/(num1->ttime/90 + 40 * TIMEBIAS);
    temp2=num2->ttime*(num2->bombing)/(num2->ttime/90 + 40 * TIMEBIAS);

    if (temp >= temp2) {
      return(1);
    } else {
      return(0);
    }
  case 'p':
    temp =num1->ttime*(num1->planets)/(num1->ttime/90 + 40 * TIMEBIAS);
    temp2=num2->ttime*(num2->planets)/(num2->ttime/90 + 40 * TIMEBIAS);

    if (temp >= temp2) {
      return(1);
    } else {
      return(0);
    }
  case 't':
    temp =num1->ttime*(num1->planets+num1->bombing+num1->offense)/(num1->ttime/90 + 10 * TIMEBIAS);
    temp2=num2->ttime*(num2->planets+num1->bombing+num1->offense)/(num2->ttime/90 + 10 * TIMEBIAS);

    if (temp >= temp2) {
      return(1);
    } else {
      return(0);
    }
  case 'h':
    temp =num1->ttime;
    temp2=num2->ttime;

    if (temp >= temp2) {
      return(1);
    } else {
      return(0);
    }
#ifdef GENO_COUNT
  case 'g':
    if (num1->genos>=num2->genos) {
      if (num1->genos==num2->genos) {
	temp =num1->ttime*(num1->planets+num1->bombing+num1->offense);
	temp2=num2->ttime*(num2->planets+num2->bombing+num2->offense);
	if (temp>=temp2) {
	  return(1);
	} else {
	  return(0);
	}
      }
      return(1);    
    } else {
      return(0);
    }
#endif
  case 'n':
  case 'a':
#ifdef GENO_COUNT
  case 'G':
#endif
    if (num1->rank > num2->rank) return(1);
    if (num2->rank > num1->rank) return(0);

    /* 4/2/89  Rank by DI within class.
     */
  case 'D':
    temp =num1->ttime*(num1->planets+num1->bombing+num1->offense);
    temp2=num2->ttime*(num2->planets+num2->bombing+num2->offense);

    if (temp >= temp2) {
      return(1);
    } else {
      return(0);
    }
  case 's':
    if (num1->sblosses*10 < num1->sbkills) {
      temp =((float) num1->sbkills/10.0 - (float) num1->sblosses) *
	((float) num1->sbkills * 3600.0 / (float) num1->sbtime) / 90.0;
    } else {
      temp =((float) num1->sbkills/10.0 - (float) num1->sblosses) /
	((float) num1->sbkills * 3600.0 / (float) num1->sbtime) * 90.0;
    }

    if (num2->sblosses*10 < num2->sbkills) {
      temp2 =((float) num2->sbkills/10.0 - (float) num2->sblosses) *
	((float) num2->sbkills * 3600.0 / (float) num2->sbtime) / 90.0;
    } else {
      temp2 =((float) num2->sbkills/10.0 - (float) num2->sblosses) /
	((float) num2->sbkills * 3600.0 / (float) num2->sbtime) * 90.0;
    }

    if (temp >= temp2) {
      return(1);
    } else {
      return(0);
    }
  default:
    return(1);
  }
}
	    
main(argc, argv)
     int argc;
     char **argv;
{
  int fd;
  static struct player j;
  struct rawdesc *temp;
  int i,c;
  int max_player=-1; /* -1 is all players */
  double totalTime=0;
  extern char *optarg;
  extern int optind;
  char namebuf[NAME_LEN+1];
  
  getpath();
  status=(struct status *) malloc(sizeof(struct status));

  showall=0; output=NULL;
  mode='a';
  
  while ((c = getopt(argc, argv, "aodpbtsGnrTODhgi:")) != EOF) {
    switch (c) {
      case 'i':
        max_player = atoi(optarg);
        break;
      case 'a':
      case 'o':
      case 'd':
      case 'p':
      case 'b':
      case 't':
      case 's':
      case 'G':
      case 'n':
      case 'r':
      case 'T':
      case 'O':
      case 'D':
      case 'h':
      case 'g':
        mode = (char)c;
        break;
    }
  }

/*
  if (argc>1){
    mode= *argv[1];

    if (argv[1][1]=='a') {
      showall=1;
    }
  }
*/
    if (mode == 'a') {
      showall = 1;
    }

/*
  if (mode=='O') {
    if (argc>2) daysold=atoi(argv[2]);
    if (argc > 3) out = 0;
  }
  else if (argc > 2) out = 0;
*/

  fd = open(Global, O_RDONLY, 0777);
  if (fd < 0) {
    printout("Cannot open the global file!\n");
    exit(0);
  }
  read(fd, (char *) status, sizeof(struct status));
  close(fd);
  fd = open(PlayerFile, O_RDONLY, 0777);
  if (fd < 0) {
    perror(PlayerFile);
    exit(1);
  }
  printout("<html>\n");
  printout("<body TEXT=\"#FF0000\" BGCOLOR=\"#000000\" LINK=\"#FFFF00\" VLINK=\"#FFFFFF\" ALINK=\"#FFFF00\">\n");
 switch(mode) {
  case 'O':
  case 'o':
  case 'r':
  case 'b':
  case 'p':
  case 'd':
  case 'a':
  case 'n':
  case 't':
    printout("<table border=1 cellpadding=1>\n");
    printout("<tr>\n");
    printout("<th align=left colspan=2 nowrap> The Continuum Netrek Stats\n");
    printout("<th>Hours\n");
    printout("<th colspan=2>Planets\n");
    printout("<th colspan=2>Bombing\n");
    printout("<th colspan=2>Offense\n");
    printout("<th colspan=2>Defense<br>\n");
    printout("<tr>\n");
    printout("<td colspan=2 align=left><b>Totals<b>\n");
    printout("<td align=right> %7.2f\n", status->time/36000.0);
    printout("<td align=right colspan=2> %11d\n", status->planets);
    printout("<td align=right colspan=2> %11d\n", status->armsbomb);
    printout("<td align=right colspan=2> %11d\n", status->kills);
    printout("<td align=right colspan=2> %11d<br>\n", status->losses);
    printout("<tr>\n");    
    break;
#ifdef GENO_COUNT
  case 'g':
    printout("<table border=1 cellpadding=1>\n");
    printout("<tr>\n");
    printout("<th align=left colspan=2 nowrap> The Continuum Netrek Stats\n");
    printout("<th>Hours\n");
    printout("<th>Rating\n");
    printout("<th>DI\n");
    printout("<th>Genos\n");
    printout("<tr>");
    break;
  case 'G':
    printout("<table border=1 cellpadding=1>\n");
    printout("<tr>\n");
    printout("<th align=left colspan=2 nowrap> The Continuum Netrek Stats\n");
    printout("<th>Hours\n");
    printout("<th colspan=2>Planets\n");
    printout("<th colspan=2>Bombing\n");
    printout("<th colspan=2>Offense\n");
    printout("<th colspan=2>Defense\n");
    printout("<th>Genos\n");
    printout("<tr>\n");
    printout("<td colspan=2 align=left><b>Totals<b>\n");
    printout("<td align=right> %7.2f\n", status->time/36000.0);
    printout("<td align=right colspan=2> %11d\n", status->planets);
    printout("<td align=right colspan=2> %11d\n", status->armsbomb);
    printout("<td align=right colspan=2> %11d\n", status->kills);
    printout("<td align=right colspan=2> %11d<br>\n", status->losses);
    printout("<td> N/A\n");
    printout("<tr>\n");    
    break;
#endif
  case 'D':
  case 'h':
    printout("<table border=1 cellpadding=1>\n");
    printout("<tr>\n");
    printout("<th align=left colspan=2 nowrap> The Continuum Netrek Stats\n");
    printout("<th>Hours\n");
    printout("<th>Ratings\n");
    printout("<th>DI\n");
    printout("<th>Last Login\n");
    printout("<tr>\n");
    break;
  case 's':
    printout("<table border=1 cellpadding=1>\n");
    printout("<tr>\n");
    printout("<th align=left colspan=2 nowrap> The Continuum Netrek Stats\n");
    printout("<th>Hours\n");
    printout("<th>Kills\n");
    printout("<th>Losses\n");
    printout("<th>Ratio\n");
    printout("<th>Maxkills\n");
    printout("<th>Kills/hr\n");
    printout("<th>Rating\n");
    printout("<tr>\n");
    break;
  case 'T':
    totalTime=status->timeprod/10;
    break;
  default:
    usage();
    break;
  case 'A':
    printout("%10ld %10d %10d %10d %10d %10ld\n",
	     status->time,
	     status->planets,
	     status->armsbomb,
	     status->kills,
	     status->losses, 
	     status->timeprod);
    break;
  }

  i= -1;
  while (read(fd, (char *) &play_entry, sizeof(struct statentry)) == sizeof(struct statentry)) {
    i++;
    j.p_stats = play_entry.stats;
    if (j.p_stats.st_tticks<1) j.p_stats.st_tticks=1;
    if (play_entry.stats.st_tticks<1) play_entry.stats.st_tticks=1;
    fixkeymap(play_entry.stats.st_keymap);
    switch (mode) {
    case 'o':
    case 'b':
    case 'p':
    case 'd':
    case 'a':
    case 'n':
    case 't':
    case 'D':
    case 'h':
#ifdef GENO_COUNT
    case 'G':
    case 'g':
#endif
      /* Throw away entries with less than 1 hour play time */
      if (play_entry.stats.st_tticks < 36000) continue;
    case 's':
      /* Throw away entries unused for over 4 weeks */
      if (!showall && (mode!='n') && time(NULL) > play_entry.stats.st_lastlogin + 2419200) continue;
      /* Throw away SB's with less than 1 hour play time */
      if (mode=='s' && play_entry.stats.st_sbticks < 36000) continue;
#ifdef GENO_COUNT
      /* Don't list people with 0 genos */
      if (mode=='g' && play_entry.stats.st_genos == 0) continue;
#endif
      addNewPlayer(play_entry.name, &(j), mode);
      break;
    case 'O':
      if (play_entry.stats.st_lastlogin + daysold*86400 > time(NULL)) break;
    case 'r':
	printout("<td align=right> %3d\n", i);
	printout("<td align=left nowrap> %-16.16s\n", play_entry.name);
	printout("<td align=right> %6.2f\n", play_entry.stats.st_tticks / 36000.0);
	printout("<td align=right> %5d    <td> %5.2f\n",
		 play_entry.stats.st_tplanets, planetRating(&j)); 
	printout("<td align=right> %5d    <td> %5.2f\n",
		 play_entry.stats.st_tarmsbomb, bombingRating(&j));
	printout("<td align=right> %5d    <td> %5.2f\n",
		 play_entry.stats.st_tkills, offenseRating(&j)); 
	printout("<td align=right> %5d    <td> %5.2f\n",
		 play_entry.stats.st_tlosses, defenseRating(&j)); 
	printout("<tr>\n");
      break;
    case 'T':
      totalTime += play_entry.stats.st_ticks / 10 +
	play_entry.stats.st_sbticks / 10;
      break;
    default:
    case 'A':
      strcpy(namebuf, play_entry.name);
      strcat(namebuf, "_");
#ifdef GENO_COUNT
      printf("%-16.16s %-16.16s %-96.96s %1d %9.2lf %7d %7d %7d %7d %7d %7d %7d %7d %7d %7d %7d %7d %7d %9.2lf %9ld %7d %7d\n",
#else
	     printout("%-16.16s %-16.16s %-96.96s %1d %9.2lf %7d %7d %7d %7d %7d %7d %7d %7d %7d %7d %7d %7d %7d %9.2lf %9ld %7d\n",
#endif
		      namebuf,
		      play_entry.password,
		      play_entry.stats.st_keymap,
		      play_entry.stats.st_rank,
		      play_entry.stats.st_maxkills,
		      play_entry.stats.st_kills,
		      play_entry.stats.st_losses,
		      play_entry.stats.st_armsbomb,
		      play_entry.stats.st_planets,
		      play_entry.stats.st_ticks,
		      play_entry.stats.st_tkills,
		      play_entry.stats.st_tlosses,
		      play_entry.stats.st_tarmsbomb,
		      play_entry.stats.st_tplanets,
		      play_entry.stats.st_tticks,
		      play_entry.stats.st_sbkills,
		      play_entry.stats.st_sblosses,
		      play_entry.stats.st_sbticks,
		      play_entry.stats.st_sbmaxkills,
		      play_entry.stats.st_lastlogin,
#ifdef GENO_COUNT
		      play_entry.stats.st_flags,
		      play_entry.stats.st_genos);
#else
	     play_entry.stats.st_flags);
#endif
      break;
    }
  }
  close(fd);
  if (mode=='T') {
    printout("Total play time for this game has been at least %ld seconds\n",
	     totalTime);
  }
  if (mode=='o' || mode=='b' || mode=='p' || mode=='d' || mode=='a' || mode=='s' || mode=='D' || mode=='h' || mode=='t' || mode=='n' || mode=='G' || mode=='g') {
    int lastrank= -1;
    temp=output;
    i=0;
    /* Calculate std., averages */
    if (mode=='n') {
      avgoffense = sumoffense / (double) numplayers;
      avgdefense = sumdefense / (double) numplayers;
      avgplanets = sumplanets / (double) numplayers;
      avgbombing = sumbombing / (double) numplayers;
      stdoffense = sqrt(sum2offense / (double) numplayers - avgoffense*avgoffense);
      stddefense = sqrt(sum2defense / (double) numplayers - avgdefense*avgdefense);
      stdplanets = sqrt(sum2planets / (double) numplayers - avgplanets*avgplanets);
      stdbombing = sqrt(sum2bombing / (double) numplayers -avgbombing*avgbombing);
      printout("<td align=left colspan=3><b>Average</b>\n");
      printout("<td align=right colspan=2> %4.2f\n", avgplanets);
      printout("<td align=right colspan=2> %4.2f\n", avgbombing);
      printout("<td align=right colspan=2> %4.2f\n", avgoffense);
      printout("<td align=right colspan=2> %4.2f\n", avgdefense);
      printout("<tr>");
      printout("<td align=left colspan=3><b>Std. Dev.</b>\n");
      printout("<td align=right colspan=2> %4.2f\n", stdplanets);
      printout("<td align=right colspan=2> %4.2f\n", stdbombing);
      printout("<td align=right colspan=2> %4.2f\n", stdoffense);
      printout("<td align=right colspan=2> %4.2f\n", stddefense);
      printout("<tr>");
    }
    while (temp!=NULL) {
      if (((mode == 'a') || (mode == 'n') || mode=='G') && lastrank != temp->rank) {
	lastrank = temp->rank;
	printout("<th>%s\n", ranks[lastrank].name);
	printout("<th>Name\n");
	printout("<th>Hours\n");
	printout("<th colspan=2>Planets\n");
	printout("<th colspan=2>Bombing\n");
	printout("<th colspan=2>Offense\n");
	printout("<th colspan=2>Defense\n");
	if (mode == 'G') {
	  printout("<th> Genos\n");
	}
	printout("<tr>\n");
      }
      if (i == max_player) {
	break;
      }	
      i++;
      switch(mode) {
      case 'o':
      case 'b':
      case 'p':
      case 'd':
      case 'a':
      default:
	printout("<td align=right> %3d\n", i);
	printout("<td align=left nowrap> %-16.16s\n", temp->name);
	printout("<td align=right> %6.2f\n", temp->ttime/3600.0);
	printout("<td align=right> %5d    <td> %5.2f\n", temp->nplan, temp->planets);
	printout("<td align=right> %5d    <td> %5.2f\n", temp->nbomb, temp->bombing);
	printout("<td align=right> %5d    <td> %5.2f\n", temp->nkill, temp->offense);
	printout("<td align=right> %5d    <td> %5.2f\n", temp->ndie, temp->defense);
	printout("<tr>\n");

	break;
#ifdef GENO_COUNT
      case 'g':
	printout("<td align=right> %3d\n", i);
	printout("<td align=left nowrap> %-16.16s\n", temp->name);
	printout("<td align=right> %6.2f\n", temp->ttime/3600.0);
	printout("<td align=right> %7.2f\n",
		 temp->bombing+temp->offense+temp->planets);   
	printout("<td align=right> %8.2f\n",
		 (temp->bombing+temp->offense+temp->planets)*temp->ttime/3600.0); 
	printout("<td align=right> %3d\n", temp->genos);
	printout("<tr>\n");
	break;
      case 'G':
	printout("<td align=right> %3d\n", i);
	printout("<td align=left nowrap> %-16.16s\n", temp->name);
	printout("<td align=right> %6.2f\n", temp->ttime/3600.0);
	printout("<td align=right> %5d    <td> %5.2f\n", temp->nplan, temp->planets);
	printout("<td align=right> %5d    <td> %5.2f\n", temp->nbomb, temp->bombing);
	printout("<td align=right> %5d    <td> %5.2f\n", temp->nkill, temp->offense);
	printout("<td align=right> %5d    <td> %5.2f\n", temp->ndie, temp->defense);
	printout("<td align=right> %3d\n", temp->genos);
	printout("<tr>\n");
	break;
#endif
      case 'n':
	printout("<td align=right> %3d\n", i);
	printout("<td align=left nowrap> %-16.16s\n", temp->name);
	printout("<td align=right> %6.2f\n", temp->ttime/3600.0);
	printout("<td align=right> %5.2f <td> (%5.2f)\n", temp->planets,
		 (temp->planets-avgplanets)/stdplanets);	
	printout("<td align=right> %5.2f <td> (%5.2f)\n", temp->bombing,
		 (temp->bombing-avgbombing)/stdbombing);	
	printout("<td align=right> %5.2f <td> (%5.2f)\n", temp->offense,
		 (temp->offense-avgoffense)/stdoffense);
	printout("<td align=right> %5.2f <td> (%5.2f)\n", temp->defense,
		 (temp->defense-avgdefense)/stddefense); 
	printout("<tr>\n");
	break;
      case 'D':
      case 'h':
	printout("<td align=right> %3d\n", i);
	printout("<td align=left nowrap> %-16.16s\n", temp->name);
	printout("<td align=right> %6.2f\n", temp->ttime/3600.0);
	printout("<td align=right> %6.2f\n",
		 temp->bombing+temp->offense+temp->planets); 
	printout("<td align=right> %7.2f\n",
		 (temp->bombing+temp->offense+temp->planets)*temp->ttime/3600.0); 
	printout("<td align=right> %s\n", ctime(&(temp->lastlogin)));
	printout("<tr>\n");
	break;
      case 's':
	printout("<td align=right> %3d\n", i);
	printout("<td align=left nowrap> %-16.16s\n", temp->name);
	printout("<td align=right> %6.2f\n", temp->sbtime / 3600.0); 
	printout("<td align=right> %7d\n", temp->sbkills);
	printout("<td align=right> %8d\n", temp->sblosses);
	printout("<td align=right> %7.2f\n", (temp->sblosses==0) ? (float)
		 temp->sbkills : (float) temp->sbkills/temp->sblosses); 
	printout("<td align=right> %9.2f\n", temp->sbmaxkills);
	printout("<td align=right> %9.2f\n", (float) temp->sbkills * 3600.0 /
		 (float) temp->sbtime); 
	printout("<td align=right> %6.2f\n", (temp->sblosses*10 < temp->sbkills) ?
		 ((float) temp->sbkills/10.0 - (float) temp->sblosses) *
		 ((float) temp->sbkills * 3600.0 / (float) temp->sbtime) / 90.0 :
		 ((float) temp->sbkills/10.0 - (float) temp->sblosses) /
		 ((float) temp->sbkills * 3600.0 / (float) temp->sbtime) *
		 90.0);  
	printout("<tr>\n");
	break;
      }
      temp=temp->next;
    }
  }
  printout("</table>\n");
  printout("</html>\n");
  return 1;		/* satisfy lint */
}

fixkeymap(s)
     char *s;
{
  int i;

  for (i=0; i<95; i++) {
    if ((s[i]==0) || (s[i] == 10)) { /* Pig fix (no ^J) 7/8/92 TC */
      s[i]=i+32;
    }
  }
  s[95]=0;
}

cumestats(new)
     struct rawdesc *new;
{
  numplayers++;
  sumoffense += (double) new->offense;
  sumdefense += (double) new->defense;
  sumplanets += (double) new->planets;
  sumbombing += (double) new->bombing;
  sum2offense += (double) new->offense*new->offense;
  sum2defense += (double) new->defense*new->defense;
  sum2planets += (double) new->planets*new->planets;
  sum2bombing += (double) new->bombing*new->bombing;
  return;
}

usage()
{
  printout("usage: scores [mode]\n");
  printout("Modes:\n");
  printout("a - print best players in order\n");
  printout("o - list players with best offense\n");
  printout("d - list players with best defense\n");
  printout("p - list players with best planet rating\n");
  printout("b - list players with best bombing rating\n");
  printout("t - list players with best total ratings\n");
  printout("s - list best starbase players\n");
#ifdef GENO_COUNT
  printout("G - same as a, but show genocides too\n");
#endif
  printout("  Note: for modes [aodpbs], players with less than 1 hour\n");
  printout("    or who haven't played for four weeks are omitted\n");
  printout("    Add 'a' to the letter to include players older than 4\n");
  printout("    weeks (eg, scores sa to get all base players)\n");
  printout("n - print best players in order, showing std. deviations\n");
  printout("r - list all players, hours, and ratings (no order)\n");
  printout("A - list entire database in ascii form (for use with newscores)\n");
  printout("T - print rough number of seconds of play time\n");
  printout("O - print players who haven't logged in for 4 weeks\n");
  printout("D - list players by DI\n");
  printout("h - list players by hours\n");
#ifdef GENO_COUNT
  printout("g - list players by genos\n");
#endif
  printout("none: the default is scores a\n");
  exit(0);
}

void
printout(va_alist)

     va_dcl
{
  va_list      ap;
  char         *format;

  va_start(ap);
  format = va_arg(ap, char *);

  if(out == 1){
    vfprintf(stdout, format, ap);
  }
  else {
    char      buf[512];
    vsprintf(buf, format, ap);
    write(out, buf, strlen(buf));
  }
  va_end(args);
}
