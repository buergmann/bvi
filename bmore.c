/* BMORE - binary more
 *
 * 1990-01-31  V 1.0.0
 * 1990-09-04  V 1.1.0
 * 2000-05-31  V 1.3.0 beta
 * 2000-10-18  V 1.3.0 final
 * 2002-01-16  V 1.3.1
 * 2004-01-09  V 1.3.2
 * 2013-08-23  V 1.4.0
 * 2019-01-22  V 1.4.1
 * 2023-03-06  V 1.4.2
 *
 * Copyright 1990-2023 by Gerhard Buergmann
 * gerhard@puon.at
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * See file COPYING for information on distribution conditions.
 */


#include <sys/types.h>

#ifdef HAVE_LOCALE_H
#	include <locale.h>
#endif

#if defined(__MSDOS__) && !defined(DJGPP)
#	define PRINTF	cprintf
#else
#	define PRINTF	printf
#ifndef HELPFILE
#  ifdef DJGPP
#	define HELPFILE "/dev/env/DJDIR/lib/bmore.help"
#  else
#	define HELPFILE "/usr/local/lib/bmore.help"
#  endif
#endif
#endif

#include "bmore.h"

char	*copyright  = "GPL (C) 1990-2022 by Gerhard Buergmann";

int		maxx, maxy;
int		mymaxx = 0, mymaxy = 0;
char	*name = NULL;
char	sstring[MAXCMD] = "";	/* string for search */
char	estring[MAXCMD] = "";	/* string for shell escape */
char	string[MAXCMD];
FILE	*curr_file = NULL, *help_file;
int		AnzAdd;
long	precount = -1;	/* number preceding command */

char	**files;		/* list of input files */
int		numfiles;		/* number of input files */
int		file_nr = 0;	/* number of current input file */

int		arrnum = 0;
char	numarr[64];		/* string for collecting number */
char	addr_form[15];

int		ascii_flag = 0;
int		dup_print_flag = 0;
int		c_flag = 0, d_flag = 0, r_flag = 0;
int		exval = 0;
int		init_search = 0;
char	buffer1[MAXCMD], buffer2[MAXCMD];
int		out_len;
int		corr = 0, do_header = 0, to_print;
off_t	init_byte = 0;
off_t	last_search = 0;
off_t	screen_home, filesize;
off_t	bytepos, oldpos;
int		prompt = 1;
char	helppath[MAXCMD];

static	char	progname[10];
static	char	cmdbuf[2 * MAXCMD];
static	int		cnt = 0;
static	int		icnt = 0;
static	int		smode;

/* char	search_pat[BUFFER];	*/  /* / or ? command */
char	bmore_search_pat[BUFFER];   /* / or ? command */
char	*emptyclass = "Empty byte class '[]' or '[^]'";


/* -a   ASCII mode
 * -d   beginners mode
 * -c	clear before displaying
 * -i   ignore case
 * -n	number of lines/screen
 * -r	display reverse video (highest bit set)
 * -w   width of screen
 */
void
usage()
{
	fprintf(stderr, "Usage: %s [-acdir] [-lines] [+linenum | +/pattern] name1 name2 ...\n", progname);
	exit(1);
}


int
main(argc, argv)
	int argc;
	char *argv[];
{
	int		ch, ch1;
	int		colon = 0, last_ch = 0;
	long	last_pre = 0;
	int		lflag, repeat;
	long	count;
	int		i, n = 1;
	int		d_line, r_line, z_line;
	char	*poi;


#if defined(__MSDOS__) && !defined(DJGPP)
	strcpy(helppath, argv[0]);
	poi = strrchr(helppath, '\\');
	*poi = '\0';
	strcat(helppath, "\\BMORE.HLP");
#else
	strncpy(helppath, HELPFILE, MAXCMD - 1);
#endif

#ifdef HAVE_LOCALE_H
	setlocale(LC_ALL, "");
#endif

	poi = strrchr(argv[0], DELIM);
 
	if (poi) strncpy(progname, ++poi, 9);
	else strncpy(progname, argv[0], 9);
	strtok(progname, ".");

	while (n < argc) {
		switch (argv[n][0]) {
		case '-':
			if (argv[n][1] >= '0' && argv[n][1] <= '9') {
				sscanf(&argv[n][1], "%dx%d", &mymaxy, &mymaxx);
			} else if (argv[n][1] == 'n') {
				if (argv[n+1] == NULL || argv[n+1][0] == '-') {
					usage();
				} else {
					sscanf(&argv[++n][0], "%d", &mymaxy);
				}
			} else if (argv[n][1] == 'w') {
				if (argv[n+1] == NULL || argv[n+1][0] == '-') {
					usage();
				} else {
					sscanf(&argv[++n][0], "%d", &mymaxx);
				}
			} else {
				i = 1;
				while (argv[n][i] != '\0') {
					switch (argv[n][i]) {
					case 'a':	ascii_flag++;
								break;
					case 'c':	c_flag++;
								break;
					case 'd':	d_flag++;
								break;
					case 'i':	ignore_case++;
								break;
					case 'r':	r_flag++;
								break;
					default:		
								usage();
					}
					i++;
				}
			}
			n++;
			break;
		case '+':			/* +cmd */
			if (argv[n][1] == '/' || argv[n][1] == '\\') {
				init_search = argv[n][1];
				strcpy(sstring, &argv[n][2]);
			} else {
				init_byte = strtoll(argv[n] + 1, NULL, 0);
			}
			n++;
			break;
		default:			/* must be a file name */
			name = strdup(argv[n]);
			files = &(argv[n]);
			numfiles = argc - n;
			n = argc;
			break;
		}
	}
	initterm();
	set_tty();
	maxy -= 2;
	if (mymaxy) {
		maxy = mymaxy;
	}
	z_line = maxy;
	d_line = maxy / 2;
	r_line = 1;

	if (numfiles == 0) {
		curr_file = stdin;
		if (isatty(fileno(stdin)) != 0) {
			reset_tty();
			usage();
		}
	} else {
		file_nr = 1;
		while (open_file(name)) {	/* looking for the first existing file */
			do_next(1);
		}
		if (exval) {
			/* We dont't have one! */
			reset_tty();
			exit(exval);
		} else {
			fseeko(curr_file, init_byte, SEEK_SET);
			bytepos += init_byte;
		}
	}
	screen_home = bytepos;

	AnzAdd = 10;
	strcpy(addr_form,  "%08lX  ");

	if (ascii_flag)
		out_len = ((maxx - AnzAdd - 1) / 4) * 4;
	else
		out_len = ((maxx - AnzAdd - 1) / 16) * 4;
	if (mymaxx) {
		out_len = mymaxx;
	}

	if (init_search)
		bmsearch(init_search);

	if (no_tty) {
		int	fileloop;

		for (fileloop = 0; fileloop < numfiles; fileloop++) {
			while(!printout(1));
			do_next(1);
			open_file(name);
		}
		if (curr_file) fclose(curr_file);
		reset_tty();
		exit(exval);
	}
	if (!exval) {
		if (printout(maxy)) {
			do_next(1);
		}
	}
	signal(SIGINT, sig);
	signal(SIGQUIT, sig);
	/* main loop */
	do {
		to_print = 0;
		dup_print_flag = 0;
		if (prompt) {
			if (prompt == 2) {
				while (open_file(name)) {
					do_next(1);
				}
			}
			highlight();
			PRINTF("--More--");
			if (prompt == 2) {
				PRINTF("(Next file: %s)", name);
			} else if (!no_intty && filesize) {
				PRINTF("(%d%%)", (int)((bytepos * 100) / filesize));
			}

			if (d_flag) PRINTF("[Press space to continue, 'q' to quit]");
			normal();
			fflush(stdout);
		}
		ch = vgetc();
		/*
		if (prompt == 2) {
			open_file(name);
		}
		*/
		prompt = 1;
		PRINTF("\r");
		while (ch >= '0' && ch <= '9') {
			numarr[arrnum++] = ch;
			ch = vgetc();
		}
		numarr[arrnum] = '\0';
		if (arrnum != 0) precount = strtol(numarr, (char **)NULL, 10);
			else precount = -1;
		lflag = arrnum = 0;

		if (ch == '.') {
			precount = last_pre;
			ch = last_ch;
			repeat = 1;
		} else {
			last_pre = precount;
			last_ch = ch;
			if (ch == ':') colon = vgetc();
			repeat = 0;
		}

		switch (ch) {
		case ' ':	/*  Display next k lines of text [current screen size] */
					dup_print_flag = 1;
					if (precount > 0) to_print = precount;
						else to_print = maxy;
					break;
		case 'z':	/* Display next k lines of bytes [current screen size]* */
					dup_print_flag = 1;
					if (precount > 0) z_line = precount;
					to_print = z_line;
					break;
		case '\r':
		case '\n':	/* Display next k lines of text [current screen size]* */
					dup_print_flag = 1;
					if (precount > 0) r_line = precount;
					to_print = r_line;
					break;
		case 'q':
		case 'Q':
					cleartoeol();
					fclose(curr_file);
					reset_tty();
					exit(exval);
		case ':' :	
					switch (colon) {
					case 'f':
						prompt = 0;
						if (!no_intty)
							PRINTF("\"%s\" line %lu", name,
								(unsigned long)(bytepos - out_len));
						else
							PRINTF("[Not a file] line %lu",
								(unsigned long)(bytepos - out_len));
						fflush(stdout);
						break;
					case 'n':
						if (precount < 1) precount = 1;
						do_next(precount);
						PRINTF("\r");
						cleartoeol();
						PRINTF("\n...Skipping to file %s\r\n\r\n", name);
						prompt = 2;
						break;
					case 'p':
						if (precount < 1) precount = 1;
						do_next(-precount);
						PRINTF("\r");
						cleartoeol();
						PRINTF("\n...Skipping back to file %s\r\n\r\n", name);
						prompt = 2;
						break;
					case 'q':
						cleartoeol();
						fclose(curr_file);
						reset_tty();
						exit(exval);
						break;
					case '!':
						if (!no_intty) {
							cleartoeol();
							if (rdline(colon, estring)) break;
							doshell(estring);
							PRINTF("------------------------\r\n");
							break;
						}
					default:
						bmbeep();
					}
					break;
		case '!':
			if (!no_intty) {
				cleartoeol();
				if (rdline(ch, estring)) break;
				doshell(estring);
				PRINTF("------------------------\r\n");
				break;
			}
		case 'd':	/* Scroll k lines [current scroll size, initially 11]* */
		case BVICTRL('D'):
					if (precount > 0) d_line = precount;
					to_print = d_line;
					break;
		case BVICTRL('L'):   	/*** REDRAW SCREEN ***/
					if (no_intty) {
						bmbeep();
					} else {
						clearscreen();
						to_print = maxy + 1;
						fseeko(curr_file, screen_home, SEEK_SET);
						bytepos = screen_home;
					}
					break;
		case 'b':		/* Skip backwards k screenfuls of text [1] */
		case BVICTRL('B'):
					if (no_intty) {
						bmbeep();
					} else {
						if (precount < 1) precount = 1;
						PRINTF("...back %ld page", precount);
						if (precount > 1) {
							PRINTF("s\r\n");
						} else {
							PRINTF("\r\n");
						}
						screen_home -= (maxy + 1) * out_len;
						if (screen_home < 0) screen_home = 0;
						fseeko(curr_file, screen_home, SEEK_SET);
						bytepos = screen_home;
						to_print = maxy + 1;
					}
					break;
		case 'f':		/* Skip forward k screenfuls of bytes [1] */
		case 's':		/* Skip forward k lines of bytes [1] */
					if (precount < 1) precount = 1;
					if (ch == 'f') {
						count = maxy * precount;
					} else {
						count = precount;
					}
					putchar('\r');
					cleartoeol();
					PRINTF("\n...skipping %ld line", count);
					if (count > 1) {
						PRINTF("s\r\n\r\n");
					} else {
						PRINTF("\r\n\r\n");
					}
					screen_home += (count + maxy) * out_len;
					fseeko(curr_file, screen_home, SEEK_SET);
					bytepos = screen_home;
					to_print = maxy;
					break;
		case '\\':  
					if (ascii_flag) {
						bmbeep();
						break;
					}
		case '/':	/**** Search String ****/
					if (!repeat) {
						cleartoeol();
						if (rdline(ch, sstring)) break;
					}
		case 'n': 		/**** Search Next ****/
		case 'N':   
					bmsearch(ch);
					/*
					to_print--;
					*/
					break;
		case '\'':   
					if (no_intty) {
						bmbeep();
					} else {
						bytepos = last_search;
						fseeko(curr_file, bytepos, SEEK_SET);
						screen_home = bytepos;
						to_print = maxy;
						PRINTF("\r");
						cleartoeol();
						PRINTF("\n\r\n***Back***\r\n\r\n");
					}
					break;
		case '=':
					prompt = 0;
					cleartoeol();
					PRINTF("%lX hex  %lu dec", (unsigned long)bytepos,
						(unsigned long)bytepos);
					fflush(stdout);
					break;
		case '?':
		case 'h':
					if ((help_file = fopen(helppath, "r")) == NULL) {
						emsg("Can't open help file");
						break;
					}
					while ((ch1 = getc(help_file)) != EOF)
					    putchar(ch1);
					fclose(help_file);
					to_print = 0;
					break;
		case 'w':
		case 'v':
					if (!no_intty) {
						cleartoeol();
						if (ch == 'v') {
							sprintf(string, "bvi +%lu %s", 
								(unsigned long)(screen_home + 
								(maxy + 1) / 2 * out_len), name);
						} else {
							if (precount < 1) precount = bytepos - screen_home;
							sprintf(string, "bvi -b %lu -s %lu %s",
								(unsigned long)screen_home, 
								(unsigned long)precount, name);
						}
						doshell(string);
						to_print = maxy + 1;
						break;
					}
		default :
					if (d_flag) {
						emsg("[Press 'h' for instructions.]");
					} else {
						bmbeep();
					}
					break;
		}
		if (to_print) {
			if (printout(to_print)) {
				do_next(1);
			}
		}
	} while (1);
}


int
rdline(ch, sstring)
	int		ch;
	char	*sstring;
{
	int		i = 0;
	int		ch1 = 0;
	char	bstring[MAXCMD];

	if (ch == '!') {
		strcpy(bstring, sstring);
		sstring[0] = '\0';
	}
	putchar(ch);
	fflush(stdout);

	while (i < MAXCMD) {
		ch1 = vgetc();
		if (ch1 == '\n' || ch1 == '\r' || ch1 == ESC) {
			break;
		} else if (ch1 == 8) {
			if (i) {
				sstring[--i] = '\0';
				PRINTF("\r%c%s", ch, sstring);
				cleartoeol();
			} else {
				ch1 = ESC;
				break;
			}
		} else if (ch1 == '!' && i == 0) {
			if (bstring[0] == '\0') {
				emsg("No previous command");
				return 1;
			}
			putchar(ch1);
			PRINTF("\r%c%s", ch, bstring);
			strcat(sstring, bstring);
			i = strlen(sstring);
		} else {
			putchar(ch1);
			sstring[i++] = ch1;
		}
		fflush(stdout);
	}
	if (ch1 == ESC) {
		putchar('\r');
		cleartoeol();
		return 1;
	}
	if (i) sstring[i] = '\0';
	return 0;
}


void
do_next(n)
	int	n;
{
	if (numfiles) {
		if (n == 1 && file_nr == numfiles) {
			if (curr_file) fclose(curr_file);
			reset_tty();
			exit(exval);
		}
		if ((file_nr + n) > numfiles)
			file_nr = numfiles;
		else if ((file_nr + n) < 1)
			file_nr = 1;
		else
			file_nr += n;
		prompt = 2;
		free(name);
		name = strdup(*(files + file_nr - 1));
	} else {
		if (curr_file) fclose(curr_file);
		reset_tty();
		exit(exval);
	}
}


int
open_file(name)
	char *name;
{
	struct	stat	buf;

	if (stat(name, &buf) > -1) {
		filesize = buf.st_size;
	}
	if (curr_file) fclose(curr_file);
	if (numfiles > 1) do_header = 1;
	if ((curr_file = fopen(name, "rb")) == NULL) {
		perror(name);
		exval = 1;
		return 1;
	}
	exval = 0;
	bytepos = screen_home = 0;
	return 0;
}


void
putline(buf, num)
	char	*buf;
	int		num;
{
	int			print_pos;
	unsigned	char	ch;

	PRINTF(addr_form, (unsigned long)bytepos);

	// Hex section
	if (!ascii_flag) {
		for (print_pos = 0; print_pos < num; print_pos++) {
			ch = buf[print_pos];
		    PRINTF("%02X ", ch);
		}
		for (; print_pos < out_len; print_pos++) {
		    PRINTF("   ");
		}
		PRINTF(" ");
	}
	
	// ASCII section
	for (print_pos = 0; print_pos < num; print_pos++) {
		++bytepos;
		ch = buf[print_pos];
		if ((ch > 31) && (ch < 127)) {
		    PRINTF("%c", ch);
		} else {
			if (r_flag) {
				if ((ch & 128) && ((ch > 159) && (ch < 255))) {
					if (!no_tty) highlight();
		    		PRINTF("%c", ch & 127);
					if (!no_tty) normal();
				} else {
		    		PRINTF(".");
				}
			} else {
		    	PRINTF(".");
			}
		}
	}

	// Fill last line
	for (; print_pos < out_len; print_pos++) {
		++bytepos;
		PRINTF(" ");
	}
	if (no_tty) PRINTF("\n");
	else PRINTF("\r\n");
}


int
printout(lns)
	int lns;
{
	int			c, num;
	int			doub = 0;
	static		int		flag;
	
	if (c_flag) {
		clearscreen();
	}
	if (do_header) {
		if (no_tty) {
			PRINTF("::::::::::::::\n%s\n::::::::::::::\n", name);
		} else {
			PRINTF("\r");
			cleartoeol();
			PRINTF("::::::::::::::\r\n%s\r\n::::::::::::::\r\n", name);
		}
		do_header = 0;
		corr = 2;
	}
	if (corr && (lns > maxy - 2)) lns -= corr;
	corr = 0;
	do {
		for (num = 0; num < out_len; num++) {
			if ((c = nextchar()) == -1) break;
			buffer1[num] = c;
		}
		if (!num) return 1;
		if (memcmp(buffer1, buffer2, num) || !bytepos || !dup_print_flag) {
			memcpy(buffer2, buffer1, num);
			putline(buffer2, num);
			if (!no_tty) flag = TRUE;
			lns--;
		} else {
			if (flag) {
				cleartoeol();
				PRINTF("*\r\n");
				lns--;
			} else {
				doub++;
			}
			flag = FALSE;
			bytepos += num;
		}
		if (lns == 0) {
			screen_home = bytepos - ((maxy + 1 + doub) * out_len);
			if (screen_home < 0) screen_home = 0;
			return 0;
		}
		dup_print_flag = 1;
	} while(num);
	return 1;
}


int
nextchar()
{
	if (cnt == 0) return fgetc(curr_file);
	cnt--;
	return cmdbuf[icnt++] & 0xff;
}


void
pushback(n, where)
	int	n;
	char	*where;
{
	if (cnt) memmove(cmdbuf + n, cmdbuf, n);
	memcpy(cmdbuf, where, n);
	icnt = 0;
	cnt += n;
}




/* Return:
 * -1	EOF
 *  0	not found at current position
 *  1   found
 */
int
bmregexec(scan)
	char	*scan;
{
	char	*act;
	int		count, test;
	int		l;
	char	act_pat[MAXCMD];  /* found pattern */

	act = act_pat;
	l = 0;
	while (*scan != 0) {
		if ((test = nextchar()) == -1) return -1;
		*act++ = test;
		if (++l == MAXCMD) {
			pushback(l, act_pat);
			return 0;
		}
		if (ignore_case && smode == ASCII)	test = toupper(test);
		switch (*scan++) {
		case ONE:	/* exactly one character */
				count = *scan++;
				if (count == 1) {
					if (test != *scan) {
						bytepos++;
						if (l > 1) pushback(--l, act_pat + 1);
						return 0;
					}
					scan++;
				} else if (count > 1) {
					if (sbracket(test, scan, count)) {
						bytepos++;
						if (l > 1) pushback(--l, act_pat + 1);
						return 0;
					}
					scan += count;
				}
				break;
		case STAR:  /* zero or more characters */
				count = *scan++;
				if (count == 1) {	/* only one character, 0 - n times */
					while (test == *scan) {
						if ((test = nextchar()) == -1) return -2;
						*act++ = test;
						if (++l == MAXCMD) {
							pushback(l, act_pat);
							return 0;
						}
						if (ignore_case && smode == ASCII)
								test = toupper(test);
					}
					pushback(1, --act);
					l--;
					scan++;
				} else if (count > 1) {	/* characters in bracket */
					if (*scan == '^') {
						do {
/* If we found something matching the next part of the expression, we
 * abandon the search for not-matching characters. */
							if (bmregexec(scan + count)) {
								*act++ = test;	/* May be wrong case !! */
								l++;
								scan += count;
								bytepos--;
								break;
							}
							if (sbracket(test, scan, count)) {
								bytepos++;
								if (l > 1) pushback(--l, act_pat + 1);
								return 0;
							} else {
								if ((test = nextchar()) == -1) return -3;
								*act++ = test;
								if (++l == MAXCMD) {
									pushback(l, act_pat);
									return 0;
								}
								if (ignore_case && smode == ASCII)
										test = toupper(test);
							}
						} while(1);
					} else {
						while(!sbracket(test, scan, count)) {
							if ((test = nextchar()) == -1) return -4;
							*act++ = test;
							if (++l == MAXCMD) {
								pushback(l, act_pat);
								return 0;
							}
							if (ignore_case && smode == ASCII)
									test = toupper(test);
						}
						scan += count;
						pushback(1, --act);
						l--;
					}
				} else {	 /* ".*"  */
					do {
						if ((test = nextchar()) == -1) return -5;
						*act++ = test;
						if (++l == MAXCMD) {
							pushback(l, act_pat);
							return 0;
						}
						pushback(1, act - 1);
						bytepos--;
					} while (bmregexec(scan) == 0);
					bytepos++;
					act--;
					l--;
				}
				break;
		}
	}
	pushback(l, act_pat);
	return 1;	/* found */
}


int
sbracket(start, scan, count)
	int		start;
	char	*scan;
	int		count;
{
	if (*scan++ == '^') {
		if (!memchr(scan, start, --count)) return 0;
	} else {
		if (memchr(scan, start, --count)) return 0;
	}
	return 1;
}


void
bmsearch(ch)
	int	ch;
{
	int	i;

	if (sstring[0] == '\0') {
		emsg("No previous regular expression");
		return;
	}
	if (ch == '/') {
		/* if (ascii_comp(search_pat, sstring)) return; */
		if (ascii_comp(bmore_search_pat, sstring)) return;
	}
	if (ch == '\\') {
		/* if (hex_comp(search_pat, sstring)) return; */
		if (hex_comp(bmore_search_pat, sstring)) return;
	}
	oldpos = bytepos;
	last_search = screen_home;
	if (precount < 1) precount = 1;
	while (precount--) {
		/* while ((i = bmregexec(search_pat)) == 0); */
		while ((i = bmregexec(bmore_search_pat)) == 0);
		if (i == 1) {
			screen_home = bytepos;
			to_print = maxy;
		} else {		/* i == -1 -> EOF */
			if (no_intty) {
				PRINTF("\r\nPattern not found\r\n");
				do_next(1);
			} else {
/*
sprintf(string, "Pattern not found %d - %ul", i, (unsigned long)bytepos);
emsg(string);
*/
				emsg("Pattern not found");
				bytepos = oldpos;
				fseeko(curr_file, bytepos, SEEK_SET);
				break;
			}
		}
		if (precount) {
			nextchar();
			bytepos++;
		}
	}
	if (prompt) {
		PRINTF("\r\n...skipping\r\n");
	}
}


void
emsg(s)
	char	*s;
{
	putchar('\r');
	cleartoeol();
	highlight();
	PRINTF("%s", s);
	normal();
	fflush(stdout);
	prompt = 0;
}


void
bmbeep() {
	putchar(7);
}
