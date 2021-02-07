/* 
 * Rogue
 *
 * Advanced Rogue
 * Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka and AT&T
 * All rights reserved.
 *
 * Based on "Rogue: Exploring the Dungeons of Doom"
 * Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "curses.h"
#if defined(_WIN32)
#undef MOUSE_MOVED
#include <Windows.h>
#include <Lmcons.h>
#include <process.h>
#elif defined(__DJGPP__)
#include <process.h>
#else
#include <pwd.h>
#include <sys/utsname.h>
extern char *getpass(char *password);
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include "mach_dep.h"
#include "network.h"
#include "rogue.h"

#ifdef CHECKTIME
static int num_checks;		/* times we've gone over in checkout() */
#endif

/*
 * fruits that you get at startup
 */
static char *funfruit[] = {
	"candleberry",	"caprifig",	"dewberry",	"elderberry",
	"gooseberry",	"guanabana",	"hagberry",	"ilama",
	"imbu",		"jaboticaba",	"jujube",	"litchi",	
	"mombin",	"pitanga",	"prickly pear", "rambutan",
	"sapodilla",	"soursop",	"sweetsop",	"whortleberry",
	"jellybean",	"apple",	"strawberry",	"blueberry",
	"peach",	"banana"
};
#define NFRUIT (sizeof(funfruit) / sizeof (char *))

main(argc, argv, envp)
char **argv;
char **envp;
{
    register char *env;
    char *xcrypt();
    int lowtime;
    time_t now;
    char *roguehome();
    char *roguedir = roguehome();

    md_dobinaryio();

    /*
     * get home and options from environment
     */

    strncpy(home,md_gethomedir(),LINELEN);

    /* Get default save file */
    strcpy(file_name, home);
    strcat(file_name, "arogue58.sav");

    /* Get default score file */
    strcpy(score_file, roguedir);

    if (*score_file)
        strcat(score_file,"/");

    strcat(score_file, "arogue58.scr");

    if ((env = getenv("ROGUEOPTS")) != NULL)
	parse_opts(env);

    if (whoami[0] == '\0')
        strucpy(whoami, md_getusername(), strlen(md_getusername()));

    if (env == NULL || fruit[0] == '\0') {
	md_srand((long)(getpid()+time(0)));
	strcpy(fruit, funfruit[rnd(NFRUIT)]);
    }

    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	waswizard = TRUE;
	score(0, SCOREIT, 0);
	exit(0);
    }

#ifdef NUMNET
    /*
     * Check for a network update
     */
    if (argc == 2 && strcmp(argv[1], "-u") == 0) {
	unsigned long netread();
	int errcheck, errors = 0;
	unsigned long amount;
	short monster;

	/* Read in the amount and monster values to pass to score */
	amount = netread(&errcheck, sizeof(unsigned long), stdin);
	if (errcheck) errors++;

	monster = (short) netread(&errcheck, sizeof(short), stdin);
	if (errcheck) errors++;

	/* Now do the update if there were no errors */
	if (errors) exit(1);
	else {
	    score(amount, UPDATE, monster);
	    exit(0);
	}
    }
#endif

#ifdef WIZARD
    /*
     * Check to see if he is a wizard
     */
    if (argc >= 2 && argv[1][0] == '\0')
	if (strcmp(PASSWD, xcrypt(md_getpass("Wizard's password: "), "Si")) == 0)
	{
	    printf("Hail Mighty Wizard\n");
	    wizard = TRUE;
	    argv++;
	    argc--;
	}
#endif

#if MAXLOAD|MAXUSERS
    if (too_much() && !wizard && !author())
    {
	printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	printf("Try again later.  Meanwhile, why not enjoy a%s %s?\n",
	    vowelstr(fruit), fruit);
	exit(1);
    }
#endif
    if (argc == 2)
	if (!restore(argv[1], envp)) /* Note: restore will never return */
	    exit(1);
    lowtime = (int) time(&now);
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + getpid());
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d\n", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...\n", whoami);
    fflush(stdout);
    seed = dnum;
    md_srand(seed);

    init_things();			/* Set up probabilities of things */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    md_init();
    initscr();				/* Start up cursor package */
    init_names();			/* Set up names of scrolls */
    init_misc();			/* Set up miscellaneous magic */
    if (LINES < 24 || COLS < 80) {
	printf("\nERROR: screen size to small for rogue\n");
	byebye(-1);
    }

    if ((whoami == NULL) || (*whoami == '\0') || (strcmp(whoami,"dosuser")==0))
    {
        echo();
        mvaddstr(23,2,"Rogue's Name? ");
        wgetnstr(stdscr,whoami,LINELEN);
        noecho();
    }

    if ((whoami == NULL) || (*whoami == '\0'))
        strcpy(whoami,"Rodney");

    setup();
    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);

    init_player();			/* Roll up the rogue */
    waswizard = wizard;
    new_level(NORMLEV);			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    daemon(doctor, &player, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    daemon(stomach, 0, AFTER);
    daemon(runners, 0, AFTER);
    if (player.t_ctype == C_THIEF)
	daemon(trap_look, 0, AFTER);

    /* Choose a quest item */
    quest_item = rnd(MAXRELIC);
    msg("You have been quested to retrieve the %s....",
	rel_magic[quest_item].mi_name);
    mpos = 0;
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */
void
endit(int a)
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

fatal(s)
char *s;
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    draw(stdscr);
    endwin();
    printf("\n");	/* So the curser doesn't stop at the end of the line */
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */

rnd(range)
register int range;
{
    return(md_rand(range));
}

/*
 * roll:
 *	roll a number of dice
 */

roll(number, sides)
register int number, sides;
{
    register int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}
# ifdef SIGTSTP
/*
 * handle stop and start signals
 */
void
tstp(int a)
{
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
    crmode();
    noecho();
    clearok(curscr, TRUE);
    touchwin(cw);
    draw(cw);
    md_flushinp();
}
# endif

setup()
{
#ifdef CHECKTIME
    int  checkout();
#endif

#ifndef DUMP
#ifdef SIGHUP
    signal(SIGHUP, auto_save);
#endif
    signal(SIGILL, bugkill);
#ifdef SIGTRAP
    signal(SIGTRAP, bugkill);
#endif
#ifdef SIGIOT
    signal(SIGIOT, bugkill);
#endif
#if 0
    signal(SIGEMT, bugkill);
    signal(SIGFPE, bugkill);
    signal(SIGBUS, bugkill);
    signal(SIGSEGV, bugkill);
    signal(SIGSYS, bugkill);
    signal(SIGPIPE, bugkill);
#endif
    signal(SIGTERM, auto_save);
#endif

    signal(SIGINT, quit);
#ifndef DUMP
#ifdef SIGQUIT
    signal(SIGQUIT, endit);
#endif
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, tstp);
#endif
#ifdef CHECKTIME
    if (!author())
    {
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
	num_checks = 0;
    }
#endif
    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

playit()
{
    register char *opts;


    /*
     * parse environment declaration of options
     */
    if ((opts = getenv("ROGUEOPTS")) != NULL)
	parse_opts(opts);


    player.t_oldpos = hero;
    oldrp = roomin(&hero);
    after = TRUE;
    while (playing)
	command();			/* Command execution */
    endit(0);
}

#if MAXLOAD|MAXUSERS
/*
 * see if the system is being used too much for this game
 */
too_much()
{
#ifdef MAXLOAD
	double avec[3];
#endif

#ifdef MAXLOAD
	loadav(avec);
	return (avec[2] > (MAXLOAD / 10.0));
#else
	return (ucount() > MAXUSERS);
#endif
}
#endif

/*
 * author:
 *	See if a user is an author of the program
 */
author()
{
	switch (md_getuid()) {
#if AUTHOR
		case AUTHOR:
#endif
		case 0:
			return TRUE;
		default:
			return FALSE;
	}
}


#ifdef CHECKTIME
checkout()
{
	static char *msgs[] = {
	"The system is too loaded for games. Please leave in %d minutes",
	"Please save your game.  You have %d minutes",
	"This is your last chance. You had better leave in %d minutes",
	};
	int checktime;

	signal(SIGALRM, checkout);
	if (!holiday() && !author()) {
	    wclear(cw);
	    mvwaddstr(cw, LINES / 2, 0,
		"Game time is over. Your game is being saved.\n\n");
	    draw(cw);
	    auto_save();		/* NO RETURN */
	}
	if (too_much())	{
	    if (num_checks >= 3)
		fatal("You didn't listen, so now you are DEAD !!\n");
	    checktime = CHECKTIME / (num_checks + 1);
		chmsg(msgs[num_checks++], checktime);
		alarm(checktime * 60);
	}
	else {
	    if (num_checks) {
		chmsg("The load has dropped. You have a reprieve.");
		num_checks = 0;
	    }
	    alarm(CHECKTIME * 60);
	}
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
chmsg(fmt, arg)
char *fmt;
int arg;
{
	if (in_shell) {
		printf(fmt, arg);
		putchar('\n');
		fflush(stdout);
	}
	else
		msg(fmt, arg);
}
#endif

#ifdef LOADAV

#include <nlist.h>

struct nlist avenrun =
{
	"_avenrun"
};

loadav(avg)
reg double *avg;
{
	reg int kmem;

	if ((kmem = open("/dev/kmem", 0)) < 0)
		goto bad;
	nlist(NAMELIST, &avenrun);
	if (avenrun.n_type == 0) {
bad:
		avg[0] = avg[1] = avg[2] = 0.0;
		return;
	}
	lseek(kmem, (long) avenrun.n_value, 0);
	read(kmem, avg, 3 * sizeof (double));
}
#endif

#ifdef UCOUNT
/*
 * ucount:
 *	Count the number of people on the system
 */
#include <sys/types.h>
#include <utmp.h>
struct utmp buf;
ucount()
{
	reg struct utmp *up;
	reg FILE *utmp;
	reg int count;

	if ((utmp = fopen(UTMP, "r")) == NULL)
	    return 0;

	up = &buf;
	count = 0;
	while (fread(up, 1, sizeof (*up), utmp) > 0)
		if (buf.ut_type == USER_PROCESS)
			count++;
	fclose(utmp);
	return count;
}
#endif

/*
 * holiday:
 *	Returns TRUE when it is a good time to play rogue
 */
holiday()
{
	time_t now;
	struct tm *localtime();
	reg struct tm *ntime;

	time(&now);			/* get the current time */
	ntime = localtime(&now);
	if(ntime->tm_wday == 0 || ntime->tm_wday == 6)
		return TRUE;		/* OK on Sat & Sun */
	if(ntime->tm_hour < 8 || ntime->tm_hour >= 17)
		return TRUE;		/* OK before 8AM & after 5PM */
	if(ntime->tm_yday <= 7 || ntime->tm_yday >= 350)
		return TRUE;		/* OK during Christmas */
#if 0 /* not for now */
	if (access("/usr/tmp/.ryes",0) == 0)
	    return TRUE;		/* if author permission */
#endif

	return FALSE;			/* All other times are bad */
}
int
directory_exists(char *dirname)
{
    struct stat sb;

    if (stat(dirname, &sb) == 0) /* path exists */
        return (sb.st_mode & S_IFDIR);

    return(0);
}

char *
roguehome()
{
    static char path[1024];
    char *end,*home;

    if ( (home = getenv("ROGUEHOME")) != NULL)
    {
        if (*home)
        {
            strncpy(path, home, PATH_MAX - 20);

            end = &path[strlen(path)-1];


            while( (end >= path) && ((*end == '/') || (*end == '\\')))
                *end-- = '\0';

            if (directory_exists(path))
                return(path);
        }
    }

    if (directory_exists("/var/games/roguelike"))
        return("/var/games/roguelike");
    if (directory_exists("/var/lib/roguelike"))
        return("/var/lib/roguelike");
    if (directory_exists("/var/roguelike"))
        return("/var/roguelike");
    if (directory_exists("/usr/games/lib"))
        return("/usr/games/lib");
    if (directory_exists("/games/roguelik"))
        return("/games/roguelik");

    return("");
}



/* Windows Compatibility Stubs */

/****
 Machine Dependent Functions

 md_getuid()
 md_memused()
 md_getusername()
 md_gethostname()
 md_gethomedir()
 md_getroguedir()
 md_getshell()
 md_shellescape()
 md_getpass()
 md_crypt()
 md_htons()
 md_nstoh()
 md_unlink()
 md_isdir()
 md_ntohl()
 md_htonl()

****/


void
md_dobinaryio()
{
#if defined(__DJGPP__) || defined(_WIN32)
    _fmode = _O_BINARY;
#endif
}

int
md_rand(range)
register int range;
{
#if defined(_WIN32)
    return(range == 0 ? 0 : rand() % range);
#else
    return(range == 0 ? 0 : (lrand48() % range) & 0x7fffffff);
#endif
}

int
md_srand(seed)
register int seed;
{
#if defined(_WIN32)
    srand(seed);
#else
    srand48(seed);
#endif
}

void 
md_flushinp()
{
    /* ioctl(0,TIOCFLUSH) */
    /* ioctl(_tty_ch,TCFLSH,0) */
    flushinp();
}

int
md_getuid()
{
#ifdef _WIN32
    return(42);
#else
    return(getuid());
#endif
}

long
md_memused()
{
#ifdef _WIN32
    MEMORYSTATUS stat;

    GlobalMemoryStatus(&stat);

    return((long)stat.dwTotalPageFile);
#else
    return( sbrk(0) );
#endif
}

char *
md_getusername()
{
    static char login[80];
    char *l = NULL;
#ifdef _WIN32
    LPSTR mybuffer;
    DWORD size = UNLEN + 1;
    TCHAR buffer[UNLEN + 1];

    mybuffer = buffer;
    GetUserName(mybuffer,&size);
    l = mybuffer;
#endif
#if !defined(_WIN32) && !defined(DJGPP)
    struct passwd *pw;

    pw = getpwuid(getuid());

    l = pw->pw_name;
#endif

    if ((l == NULL) || (*l == '\0'))
        if ( (l = getenv("USERNAME")) == NULL )
            if ( (l = getenv("LOGNAME")) == NULL )
                if ( (l = getenv("USER")) == NULL )
                    l = "nobody";

    strncpy(login,l,80);
    login[79] = 0;

    return(login);
}

char *
md_gethomedir()
{
    static char homedir[PATH_MAX];
    char *h = NULL;
    size_t len;
#if defined(_WIN32) || defined(DJGPP)
    char slash = '\\'; 
#else
    char slash = '/';
    struct passwd *pw;
    pw = getpwuid(getuid());

    h = pw->pw_dir;

    if (strcmp(h,"/") == 0)
        h = NULL;
#endif
    homedir[0] = 0;

    if ( (h == NULL) || (*h == '\0') )
        if ( (h = getenv("HOME")) == NULL ) 
            if ( (h = getenv("HOMEDRIVE")) == NULL) 
                h = "";
            else
            {
                strncpy(homedir,h,PATH_MAX-1);
                homedir[PATH_MAX-1] = 0;
      
                if ( (h = getenv("HOMEPATH")) == NULL)
                    h = "";
            }


    len = strlen(homedir);
    strncat(homedir,h,PATH_MAX-len-1);
    len = strlen(homedir);

    if ((len > 0) && (homedir[len-1] != slash)) {
        homedir[len] = slash;
        homedir[len+1] = 0;
    }

    return(homedir);
}

char *
md_getshell()
{
    static char shell[PATH_MAX];
    char *s = NULL;
#ifdef _WIN32
    char *def = "C:\\WINDOWS\\SYSTEM32\\CMD.EXE";
#elif defined(__DJGPP__)
    char *def = "C:\\COMMAND.COM";
#else
    char *def = "/bin/sh";
    struct passwd *pw;
    pw = getpwuid(getuid());
    s = pw->pw_shell;
#endif
    if ((s == NULL) || (*s == '\0'))
        if ( (s = getenv("COMSPEC")) == NULL) 
            if ( (s = getenv("SHELL")) == NULL) 
                if ( (s = getenv("SystemRoot")) == NULL)
                    s = def;

    strncpy(shell,s,PATH_MAX);
    shell[PATH_MAX-1] = 0;

    return(shell);
}

char *
md_gethostname()
{
    static char nodename[80];
    char *n = NULL;
#if !defined(_WIN32) && !defined(__DJGPP__)
    struct utsname ourname;

    if (uname(&ourname) == 0)
        n = ourname.nodename;
#endif
    if ((n == NULL) || (*n == '\0'))
        if ( (n = getenv("COMPUTERNAME")) == NULL)
            if ( (n = getenv("HOSTNAME")) == NULL)
                n = "localhost";

    strncpy(nodename, n, 80);
    nodename[79] = 0;

    return(nodename);
}

int
md_shellescape()
{
    int pid;
    int ret_status;
    char *sh;

    sh = md_getshell();

#if defined(_WIN32) 
    return ( _spawnl(_P_WAIT,md_getshell(),"shell",NULL,0) );
#elif defined(__DJGPP__)
    return ( spawnl(P_WAIT,md_getshell(),"shell",NULL,0) );
#else
    while((pid = fork()) < 0)
        sleep(1);

    if (pid == 0) /* Shell Process */
    {
        /*
         * Set back to original user, just in case
         */
        setuid(getuid());
        setgid(getgid());
        execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
        perror("No shelly");
        _exit(-1);
    }
    else /* Application */
    {
        void (*int_value)(),
            (*quit_value)();

        int_value = signal(SIGINT, SIG_IGN);
#ifdef SIGQUIT
        quit_value = signal(SIGQUIT, SIG_IGN);
#endif
        while (wait(&ret_status) != pid)
            continue;
        signal(SIGINT, int_value);
#ifdef SIGQUIT
        signal(SIGQUIT, int_value);
#endif
    }

    return(ret_status);
#endif
}

int
md_erasechar()
{
/*
    return(_tty.sg_erase);
    return(_tty.c_cc[VERASE]);
*/
    return(erasechar());
}

int
md_killchar()
{
/*
    return(_tty.sg_kill);
    return(_tty.c_cc[VKILL]);
*/
    return(killchar());
}

long
md_ntohl(netlong)
long netlong;
{
    return( ntohl(netlong) );
}

long
md_htonl(netlong)
long netlong;
{
    return(htonl(netlong));
}

void
md_init()
{
#ifdef __INTERIX
    char *term;

    term = getenv("TERM");

    if (term == NULL)
        setenv("TERM","interix");
#endif
}

void
md_exit()
{
}

    

char *
md_getpass(prompt)
char *prompt;
{
#ifdef _WIN32
    static char password_buffer[9];
    char *p = password_buffer;
    int c, count = 0;
    int max_length = 9;

    fflush(stdout);
    /* If we can't prompt, abort */
    if (fputs(prompt, stderr) < 0)
    {
        *p = '\0';
        return NULL;
    }

    while (1)
    {
        /* Get a character with no echo */
        c = _getch();

        /* Exit on interrupt (^c or ^break) */
        if (c == '\003' || c == 0x100)
            exit(1);

        /* Terminate on end of line or file (^j, ^m, ^d, ^z) */
        if (c == '\r' || c == '\n' || c == '\004' || c == '\032')
            break;

        /* Back up on backspace */
        if (c == '\b')
        {
            if (count)
                count--;
            else if (p > password_buffer)
	        p--;
            continue;
        }

        /* Ignore DOS extended characters */
        if ((c & 0xff) != c)
            continue;

        /* Add to password if it isn't full */
        if (p < password_buffer + max_length - 1)
            *p++ = c;
        else
            count++;
    }
   *p = '\0';

   fputc('\n', stderr);

   return password_buffer;
#else
   return( getpass(prompt) );
#endif
}

