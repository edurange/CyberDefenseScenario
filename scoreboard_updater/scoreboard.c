/*******************************************************************************************************
* file: scoreboard.c
* date 1/7/15
* author: Devin Ercolano, David Weinman
*
* Details: This program writes the blue team's challenge points recieved, their up time and down time, 
* as well as the red team's challenge points recieved to the challenge score board.
*
********************************************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<regex.h>
#include<unistd.h>
#include<fcntl.h>
 
#define NUMREDCHALL 6
#define NUMBLUECHALL 7
#define MAXFILESIZE 181
#define MAXSEARCHTERM 37
#define MAXUPTIMEMIN 4
#define MAXUPTIMESEC 3
#define MAXDOWNTIMEMIN 4
#define MAXDOWNTIMESEC 3
#define MAXLENPOINTSBUF 6 

#define UPTIMEFILE "up_and_down_time.txt"
#define scoreboard_file  "scoreboard_file.txt"
#define challenge_list "challenge_list.txt"

char *scoreboard = "\
	CYBER DEFENSE SCOREBOARD\n\
\n\
	uptime: %smin %ssec	downtime: %smin %ssec\n\
\n\
	blue team injects	- total: %s pts\n\
\n\
	[ftp server : %d]	[stan : %d]			[plugin : %d]\n\
\n\
	[compress stan : %d]	[forgot my pass : %d]		[compress stan : %d]\n\
\n\
	[pro tools : %d]\n\
\n\
	red team challenges  - total: %s pts\n\
\n\
	[sql version : %d]	[vandalism : %d]			[ssn recon : %d]\n\
\n\
	[persistent access : %d]	[wrench in the gears : %d]	[logout user : %d]\n";

const char * redchallenges_names[] = {
                                  "sql_version", "vandalism", "ssn_recon",
                                  "peresistent_access", "wrench_in_the_gears", "logout_user"
                               };

const char * bluechallenges_names[] = {
                                  "ftp_server", "stan", "plugin", "chat_server",
                                  "forgot_my_pass", "compress_stan", "pro_tools"
				};

char * bluetotalpoints_buffer;
char * redtotalpoints_buffer;


int redpoints[] = {1, 2, 3, 4, 5, 6};
int bluepoints[] = {1, 2, 3, 4, 5, 6, 7};

unsigned int *redchallenges_status;
unsigned int *bluechallenges_status;

void fatal(char *message){
	char error_message[100];
	strcpy(error_message, "[Error Message] Fatal Error");
	strncat(error_message, message, 83);
	perror(error_message);
	exit(-1);
}

void *ec_malloc(unsigned int size){
	void *ptr;
	ptr = malloc(size);
	if (ptr == NULL)
		fatal("in ec_malloc() on memory allocation");
	return ptr;
}

char *  totalpoints(int teamcolorindicator){	
	int redtotalpoints = 0;  
	int bluetotalpoints = 0;
	int i;

	if (teamcolorindicator == 1) {

		redtotalpoints_buffer = (char *)ec_malloc(MAXLENPOINTSBUF);
		memset(redtotalpoints_buffer, 0, MAXLENPOINTSBUF);

		for (i= 0; sizeof(redchallenges_names)/sizeof(char*) > i; i++){
			if (*(redchallenges_status + i) == 1) {
				redtotalpoints = redpoints[i] + redtotalpoints;
				snprintf(redtotalpoints_buffer, MAXLENPOINTSBUF, "%d", redtotalpoints);
			}
		}

		if (redtotalpoints == 0)
			sprintf(redtotalpoints_buffer, "0");

		return redtotalpoints_buffer;
	}

	else if (teamcolorindicator == 0) {
		bluetotalpoints_buffer = (char *)ec_malloc(MAXLENPOINTSBUF);
		memset(bluetotalpoints_buffer, 0, MAXLENPOINTSBUF);
		for (i= 0; sizeof(bluechallenges_names)/sizeof(char*) > i; i++){
			if (*(bluechallenges_status + i) == 1) {
				bluetotalpoints = bluepoints[i] + bluetotalpoints;
				snprintf(bluetotalpoints_buffer, MAXLENPOINTSBUF ,"%d", bluetotalpoints);
			}
		}

		if (bluetotalpoints == 0)
			sprintf(bluetotalpoints_buffer, "0");

		return bluetotalpoints_buffer;
	}
}

void main() {
	regex_t regex;
	regmatch_t matchlocation[1];
	int reti, i; //return integer to check regx compilation
	int fd; // file descriptor
	char *readbuffer, *writebuffer, *search_term, *uptimemin, *uptimesec, *downtime_min, *downtime_sec;
	
	search_term = (char *)ec_malloc(MAXSEARCHTERM);
	readbuffer = (char *)ec_malloc(MAXFILESIZE);
	redchallenges_status = (unsigned int *)ec_malloc(sizeof(unsigned int) * NUMREDCHALL);
	bluechallenges_status =(unsigned int *)ec_malloc(sizeof(unsigned int) * NUMBLUECHALL);
	memset(readbuffer, 0, MAXFILESIZE);
	memset(redchallenges_status, 0, NUMREDCHALL);
	memset(bluechallenges_status, 0, NUMBLUECHALL);

	//open challenge list file, and search it for: challenges completed for each team, red up time, red down time
	fd = open(challenge_list, O_RDONLY, S_IRUSR);
	if (fd == -1)
		fatal("in main() while opening challenge list");
	
	//read challenge list file
	if (read(fd, readbuffer, 181) == -1)
		fatal("in main() could not read uptimefile");

	for (i= 0; sizeof(redchallenges_names)/sizeof(char*) > i; i++){
		//compile regular expression "search_term"
		memset(search_term, 0, MAXSEARCHTERM);
		snprintf(search_term, 23, "%s: 1", *(redchallenges_names + i));
		reti = regcomp(&regex,search_term , 0);
		if (reti) {
			fatal("in main() could not compile regex");
		 }
		 //search file to see if has been completed but not updated
		 reti = regexec(&regex, readbuffer, 0, NULL, 0);
		 regfree(&regex);
		 if (!reti) {
			//update challenge status
			*(redchallenges_status + i) = 1;
		 }
	}

	for (i = 0; sizeof(bluechallenges_names)/sizeof(char *) > i; i++) {
		//compile regular expression "search_term"
		memset(search_term, 0, MAXSEARCHTERM);
		snprintf(search_term, 23, "%s: 1", *(bluechallenges_names + i));
		reti = regcomp(&regex,search_term , 0);
		if (reti) {
			fatal("in main() could not compile regx");
		}
		//search file to see if has been completed but not updated
		reti = regexec(&regex, readbuffer, 0, NULL, 0);
		regfree(&regex);
		if (!reti) {
				//update challenge status
				*(bluechallenges_status + i) = 1;
		}
	}
	free(readbuffer);
	
	if (close(fd)== -1)
		fatal("in main() while closing challenge list");

	//check blue team's up-time and down-time	
	readbuffer = (char *)ec_malloc(MAXFILESIZE);
	memset(readbuffer, 0, MAXFILESIZE);
	memset(search_term, 0, MAXSEARCHTERM);

	fd = open(UPTIMEFILE, O_RDONLY, S_IRUSR);
	if (fd == -1)
		fatal("in main() while opening uptime file");

	if (read(fd, readbuffer, 181) == -1)
		fatal("in main() could not read uptimefile");
	
	//find and save up-time minutes and seconds
	snprintf(search_term, MAXSEARCHTERM, "%s:([0-9][0-9]min, [0-9][0-9]sec)", "up");	
	reti = regcomp(&regex,search_term , 0);
	
	if (reti) {
		fprintf(stderr, "Could not compile regx");
		exit(1);
	}	
	reti = regexec(&regex, readbuffer, 1, matchlocation, 0);	

	uptimemin = (char *)ec_malloc(MAXUPTIMEMIN);
	memset(uptimemin, 0, MAXUPTIMEMIN);
	snprintf(uptimemin, MAXUPTIMEMIN, (readbuffer + matchlocation[0].rm_so + 4));
	memset(search_term, 0, MAXSEARCHTERM);
	uptimesec = (char *)ec_malloc(MAXUPTIMESEC);
	memset(uptimesec, 0, MAXUPTIMESEC);
	snprintf(uptimesec, MAXUPTIMESEC, (readbuffer + matchlocation[0].rm_so + 11));
	regfree(&regex);
	
	//find and save down-time
	memset(search_term, 0, MAXSEARCHTERM);
	snprintf(search_term, MAXSEARCHTERM, "%s:([0-9][0-9]min, [0-9][0-9]sec)", "down");
	reti = regcomp(&regex,search_term , 0);
	if (reti) {
		fatal("in main() could not compile regex (#1)");
	}

	reti = regexec(&regex, readbuffer, 1, matchlocation, 0);
	if (reti){
		fatal("in main() could not compile regex (#2)");
	}

	regfree(&regex);
	downtime_min = (char *)ec_malloc(MAXDOWNTIMEMIN);
	memset(downtime_min, 0, MAXDOWNTIMEMIN);
	snprintf(downtime_min, MAXDOWNTIMEMIN, (readbuffer + matchlocation[0].rm_so + 6));
	downtime_sec = (char *)ec_malloc(MAXDOWNTIMESEC);
	memset(downtime_sec, 0, MAXDOWNTIMESEC);
	snprintf(downtime_sec, MAXDOWNTIMESEC, (readbuffer + matchlocation[0].rm_so + 13));	
	
	//close uptime file
	if (close(fd) == -1)
		fatal("in main() while closing uptimefile");

	writebuffer = (char *)ec_malloc(442);
	memset(writebuffer, 0, 442);
	//open scoreboard file
	fd = open(scoreboard_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	if (fd == -1)
		fatal("in main() while opening scoreboard_file");

	snprintf(writebuffer, 442, scoreboard, uptimemin, uptimesec, downtime_min, downtime_sec, totalpoints(0),
		(*bluechallenges_status) ? bluepoints[0] : 0, (*(bluechallenges_status + 1)) ? bluepoints[1] : 0,
		(*(bluechallenges_status + 2)) ? bluepoints[2] : 0, (*(bluechallenges_status + 3)) ? bluepoints[3] : 0,
		(*(bluechallenges_status + 4)) ? bluepoints[4] : 0, (*(bluechallenges_status + 5)) ? bluepoints[5] : 0,
		(*(bluechallenges_status + 6)) ? bluepoints[6] : 0,

		totalpoints(1),
		(*redchallenges_status) ? redpoints[0] : 0, (*(redchallenges_status + 1)) ? redpoints[1] : 0,
		(*(redchallenges_status + 2)) ? redpoints[2] : 0, (*(redchallenges_status + 3)) ? redpoints[3] : 0,
		(*(redchallenges_status + 4)) ? redpoints[4] : 0, (*(redchallenges_status + 5)) ? redpoints[5] : 0);
	
	//write to scoreboard
	if (write(fd, writebuffer, strlen(writebuffer)) == -1 )
		fatal("in main() while writing writebuffer to file");
	
	//closing scoreboard file
	if (close(fd) == -1)
		fatal("in main while closing scoreboard_file");

	printf("Scoreboard has been updated.\n");
}
