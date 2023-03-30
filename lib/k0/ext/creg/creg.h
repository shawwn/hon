#ifndef CREG_H

#define CREG_H

// C implementation of regular expression system, written by some guy on
// the internet.   See comments in CREG.C

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#define NSUBEXP  10
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

regexp * regcomp(char *exp);
int      regexec(regexp *prog,char *string);
int      regexpmatch();
void regsub();
void regerror();

#ifdef __cplusplus
}
#endif

#endif
