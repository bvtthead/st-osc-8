#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "hl.h"

void
openlink(char *uri)
{
	int rid, fail = 0;
	regex_t regex;
	regmatch_t pmatch[3];
	pid_t child, sid;

	char actualuri[2083];
	char hostname[HOST_NAME_MAX + 1];

	rid = regcomp(&regex, "^file:\\/\\/([^\\/]*)(/.*)$", REG_EXTENDED);
	if (rid) {
		fprintf(stderr, "Regex didn't compile\n");
		return;
	}

	rid = regexec(&regex, uri, 3, pmatch, 0);
	if (!rid) {
		gethostname(hostname, HOST_NAME_MAX);
		if (!(!strncmp(hostname, uri + pmatch[1].rm_so,
					  pmatch[1].rm_eo - pmatch[1].rm_so) ||
			  !strncmp("localhost", uri + pmatch[1].rm_so,
					  pmatch[1].rm_eo - pmatch[1].rm_so))) {
			fprintf(stderr, "Hostnames do not match\n");
			fail = 1;
		}
		snprintf(actualuri, 2083, "file://%.*s", pmatch[2].rm_eo - pmatch[2].rm_so,
				 uri + pmatch[2].rm_so);
	} else if (rid == REG_NOMATCH) {
		if (strlen(uri) >= 2082)
			fail = 1;
		else
			strcpy(actualuri, uri);
	}

	regfree(&regex);

	if (fail)
		return;

	child = fork();
	if (child == 0) {
		if (fork() == 0) {
			sid = setsid();
			if (sid < 0)
				exit(1);
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			execlp("xdg-open", "xdg-open", actualuri, NULL);
		}
		exit(1);
	}
	if (child > 0)
		waitpid(child, NULL, 0);
	else if (child < 0)
		fprintf(stderr, "err: failed to spawn child\n");
}
