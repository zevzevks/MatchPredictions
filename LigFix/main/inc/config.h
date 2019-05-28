#ifndef _DENEME_H_
#define _DENEME_H_

#define FORMAT_STR_1  "%-15.15s | %-20.20s | %-15.15s | %-15.15s | %-15.15s | %-15.15s | %-15.15s | %-15.15s | %-17.17s | %-10.10s\n"
#define FORMAT_STR_2  "%-15c | %-20.20s | %-15d | %-15d | %-15d | %-15d | %-15d | %-15d | %-17d | %-10d\n"
//#define FORMAT_STR_3  "PLAYED AGAINTS : %-10c - %-10c - %-10c | %-15d | %-15d | %-15d | %-15d | %-15d | %-17d | %-10d\n"
#define NUMBER_OF_TEAMS 18
#define NUMBER_OF_MATCH_ARRAY 11
#define NUMBER_OF_DEFINED_FUNCTION_IN_MENU 10

#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b) ((a > b) ? b : a)

#define FALSE 0
#define TRUE 1

#define TEAMS_FILE_NAME "takimlar.txt"

enum Points
{ 
	DEFEAT, 		//0
	DRAW,			//1
	VICTORY = 3		//3
};

typedef struct team_info {
	char short_name;
	char long_name[25] ;
	int played_total;
	int played_win;
	int played_draw;
	int played_lose;	
	int scored_total;
	int scored_againts;
	int scored_average;
	int total_points;
	char played_hist[100];
	int scored_hist[100];
	int againts_hist[100];
} team_info;


typedef struct teams_infile 
{
	unsigned int count_of_team;
	team_info Teams[18];
} st_teams_infile;

typedef struct result {
	int home_team_iteration;
	char home_short_name;
	float home_scored;
	int away_team_iteration;
	char away_short_name;
	float away_scored;
} result;

typedef struct team_recent_results {
float recent_graph;
float recent_scored;
float recent_scored_againts;
} team_recent_results;

#endif /* _DENEME_H_ */
