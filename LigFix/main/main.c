#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <ctype.h>

int input_buffer_clear;
int cycle_idx,cycle_idy,idx;	//variables to cycle in loops

int get_file_size(FILE *fp);
void init_structure(st_teams_infile * pst_AllTeam);
void reset_scenario(st_teams_infile * pst_AllTeam);
void read_scenario_from_file(st_teams_infile * pst_AllTeam);
void update_file(char * file_name, st_teams_infile *pst_AllTeams);
void read_from_already_opened_file(FILE * fp_teams, st_teams_infile *pst_AllTeams);
unsigned char  get_last_match_results(team_recent_results *p_last_results, team_info *p_team_info);
unsigned char guess_result(result *p_guess_result, int p_first_team_it, int p_second_team_it,	st_teams_infile *pst_AllTeam);

void intro_page()
{
	printf ("\n\n");
	printf("###########################\n");
	printf("####### MAÇ TAHMİNİ #######\n");
	printf("## Still in Development ###\n");
	printf("###########################\n\n\n");
}

void welcome_screen(unsigned int *p_menu_selection)
{

	printf("\n--------------------------------\n");
	printf("Select Operation to Continue\n");
	printf("1 - Overview\n");
	printf("2 - Match History\n");
	printf("3 - Match Result Input via Console\n");
	printf("4 - Match Result Input via File\n");
	printf("5 - Read Scenario From File\n");
	printf("6 - To Make a Guess\n");
	printf("7 - To Reset Scenario\n");
	printf("0 - End the Program\n\n");
	printf("Seçilen işlem:");
	scanf("%d",p_menu_selection);	
	printf("--------------------------------\n");
	printf ("\n");

	if (*p_menu_selection >= NUMBER_OF_DEFINED_FUNCTION_IN_MENU || *p_menu_selection == 0)
				printf("Program has been terminated!\n\n");
}

char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if( str + len - 1 != endp )
            *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
            *str = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }


    return str;
}

/****************************************************************************
 * Function Name  : sort_all_teams
 * Inputs         : p_msg :Pointer to input api buffer
 *                  p_glb_cntxt :Pointer to DSM context
 * Outputs        : p_err :Pointer to error code
 * Returns        : NR_SUCCESS/NR_FAILURE
 * Description    : This function handles the timer expiry events
 ****************************************************************************/

void sort_all_teams(st_teams_infile *pst_all_teams)
{
	int max_index = sizeof(pst_all_teams->Teams)/sizeof(team_info);
	team_info temp_team;
	int temp_id = 0;
	int team_index = 0;

	for (temp_id = 1; temp_id < max_index; temp_id++)
	{
		for (team_index = 0; team_index < max_index - 1; team_index++)
		{
			if ( (pst_all_teams->Teams[team_index].total_points < pst_all_teams->Teams[team_index+1].total_points)	||
				((pst_all_teams->Teams[team_index].total_points == pst_all_teams->Teams[team_index+1].total_points) && 
					(pst_all_teams->Teams[team_index].scored_average < pst_all_teams->Teams[team_index+1].scored_average))
				)
			{
				memcpy(&temp_team, &pst_all_teams->Teams[team_index], sizeof(team_info));
				memcpy(&pst_all_teams->Teams[team_index], &pst_all_teams->Teams[team_index+1], sizeof(team_info));
				memcpy(&pst_all_teams->Teams[team_index+1], &temp_team, sizeof(team_info));
			}
		}
	}
}

//TO DISPLAY CURRENT SITUATION
void display_overall(st_teams_infile *pst_all_teams)
{
	int max_index = sizeof(pst_all_teams->Teams)/sizeof(team_info);

	printf(FORMAT_STR_1, "Team Code", "Team Name","Games Played","Games Won","Games Draw","Games Lost","Goals For","Goals Againts","Goals Difference","Points");
	
	for(cycle_idx=0;cycle_idx<max_index;cycle_idx++)
	{
		printf(FORMAT_STR_2,
			pst_all_teams->Teams[cycle_idx].short_name,		pst_all_teams->Teams[cycle_idx].long_name,		pst_all_teams->Teams[cycle_idx].played_total,
			pst_all_teams->Teams[cycle_idx].played_win,		pst_all_teams->Teams[cycle_idx].played_draw,	pst_all_teams->Teams[cycle_idx].played_lose,		
			pst_all_teams->Teams[cycle_idx].scored_total,	pst_all_teams->Teams[cycle_idx].scored_againts,	pst_all_teams->Teams[cycle_idx].scored_average,	
			pst_all_teams->Teams[cycle_idx].total_points );
	}
	printf("\n\n\n");
}

unsigned char GetLongNameIdx(const char *ps_long_name, int *p_team_iteration, st_teams_infile *pst_all_teams )
{
	unsigned char founded = FALSE;
	int index;
	int max_index = sizeof(pst_all_teams->Teams)/sizeof(team_info);

	for(index=0; index<max_index; index++)
	{
		if(strcmp(pst_all_teams->Teams[index].long_name, ps_long_name) == 0 )
		{
			*p_team_iteration = index;
			//printf("p_team_iteration:  %d index : %d\n",*p_team_iteration,index);
			founded = TRUE;
			break;
		}
	}

	return founded;
}

/*unsigned char GetShortNameFromLong(const char *ps_long_name,char *p_team_short_name, st_teams_infile *pst_all_teams)
{
	int max_index = sizeof(pst_all_teams->Teams)/sizeof(team_info);
	unsigned char founded = FALSE;
	for(int index=0;index<max_index;index++)
	{
		if(strcmp(pst_all_teams->Teams[index].long_name,ps_long_name) == 0)
		{
			printf("TAKIMIN KISA ADI : %c\n",pst_all_teams->Teams[index].short_name );
			*p_team_short_name = pst_all_teams->Teams[index].short_name ;
			founded = TRUE;
			break;
		}
	}
	return founded;
}*/

//TO CHECK TEAMS GIVEN IN RESULTS STRUCT ARE EXIST IN TEAM ARRAY 
unsigned char GetShortNameIdx(char c_short_name, int *p_team_iteration, st_teams_infile *pst_all_teams )
{
	//printf("INFO -> CHECKING IF GIVEN TEAM IS IN ALL TEAMS LINE -> %d , FUNC -> %s \n",__LINE__,__FUNCTION__ );
	//printf("short name : %c , iteration : %d\n", c_short_name,*p_team_iteration);
	unsigned char founded = FALSE;
	int index;
	int max_index = sizeof(pst_all_teams->Teams)/sizeof(team_info);


	for(index=0;index<max_index;index++)
	{
		if(pst_all_teams->Teams[index].short_name == c_short_name)
		{
			*p_team_iteration = index;
			founded = TRUE;
			//printf("p_team_iteration:  %d index : %d\n",*p_team_iteration,index);
			//printf("ARANAN TAKIM LISTENIN %d INDISINDE BULUNMAKTA \n",*p_team_iteration );
			break;
		}
	}

	if(founded != TRUE)
	printf("Team you have been looking for couldnt find in file LINE -> %d , FUNC -> %s \n",__LINE__,__FUNCTION__ );

	return founded;
}


result validate_file_input(st_teams_infile *pst_all_teams,char *p_user_input)
{
	result v_results;
		printf("INFO -> VALIDATING FILE INPUT -> %d\n",__LINE__);
		int idx = 0;
		int input_length;
		char user_input[4];
		strcpy(user_input,p_user_input);		
		input_length = strlen(user_input);

		printf("input_length : %d ++ user_input : %s\n",input_length,user_input );

		if(input_length != 4)
		{
			printf("WARNING -> The input is in Wrong Format Try Again i.e.-> A3B2 LINE->%d\n\n",__LINE__);
		}

		for(cycle_idx=0;cycle_idx<input_length;cycle_idx++)
		{
			if( (p_user_input[idx] == ' ' || p_user_input[idx] == '-' || p_user_input[idx] == '*' 
				|| p_user_input[idx] == ',') && (!p_user_input[idx] == '\0') ) 
			{
				printf("WARNING -> The input is in Wrong Format Try Again i.e.->A3B2 LINE-> %d\n",__LINE__);
				break;
			}
		}
		

		printf("INFO -> PARSING INPUT OF %s LINE -> %d,FUNC -> %s\n",user_input,__LINE__,__FUNCTION__ );

		while((p_user_input[idx] != ' ' || p_user_input[idx] != '-' || p_user_input[idx] != '*' || p_user_input[idx] != ',') && input_length == 4 && idx < 4)
		{		
			if(idx==0 || idx ==2)
			{
				//printf("INFO -> TAKING CHARS %c INTO STRUCT %d\n",p_user_input[idx],__LINE__);
				if(!(p_user_input[idx] >= '0' && p_user_input[idx] <= '9'))
				{	
						if(idx == 0)
						{
							v_results.home_short_name = p_user_input[idx];
						} 
						else 
						{ 
							v_results.away_short_name = p_user_input[idx];
						}
				} 
				else
				{
					//printf("UYARI -> Hatalı Formatta Giriş Yaptınız Tekrar Deneyin Örnek Format -> A3B2 LINE->%d\n\n",__LINE__);
					while ((input_buffer_clear = getchar()) != '\n' && input_buffer_clear != EOF) { }
					break;
				}
			}

			if(idx == 1 || idx == 3)
			{
				//printf("INFO -> TAKING CHARS %c INTO STRUCT %d\n",p_user_input[idx],__LINE__);
				if((p_user_input[idx] >= '0' && p_user_input[idx] <= '9'))
				{
					if(idx == 1)
					{
							v_results.home_scored = p_user_input[idx];
							v_results.home_scored = v_results.home_scored - '0';
					} 
					else
					{
						v_results.away_scored = p_user_input[idx];
						v_results.away_scored = v_results.away_scored - '0';		
					} 
				}
				else
				{
					printf("WARNING -> The input is in Wrong Format Try Again -> A3B2 LINE->%d\n\n",__LINE__);
					while ((input_buffer_clear = getchar()) != '\n' && input_buffer_clear != EOF) { }
					break;
				}
			}			
		idx++;						
		}

		printf("INFO -> CHECKING INPUT OF TEAMS %s LINE -> %d,FUNC -> %s\n",user_input,__LINE__,__FUNCTION__ );


		//printf("TESTING -> SN :%c IT: %d SC: %d \n",v_results.home_short_name,v_results.home_team_iteration,v_results.home_scored );

		if(FALSE == GetShortNameIdx(v_results.home_short_name, &v_results.home_team_iteration, pst_all_teams) || 
		   FALSE == GetShortNameIdx(v_results.away_short_name, &v_results.away_team_iteration, pst_all_teams))
		{
			//printf("TESTING2 -> SN :%c IT: %d SC: %d \n",v_results.home_short_name,v_results.home_team_iteration,v_results.home_scored );
			printf("ERROR-> Team you have been looking for couldnt find in file, %s: Line: %d\n", __FUNCTION__, __LINE__);
		} 
			else if(TRUE == GetShortNameIdx(v_results.home_short_name, &v_results.home_team_iteration, pst_all_teams) || 
		   			TRUE == GetShortNameIdx(v_results.away_short_name, &v_results.away_team_iteration, pst_all_teams))
					{				
						//printf("TESTING2 -> SN :%c IT: %d SC: %d \n",v_results.home_short_name,v_results.home_team_iteration,v_results.home_scored );
						
						printf("\nINFO -> INPUT IS BEING IN PROCESS.. , FUNCTION : %s \n",__FUNCTION__);
						return v_results;
					}	
return v_results;;
}


// TO CHECK IF INPUT IS IN CORRECT ORDER
result validate_user_input(st_teams_infile *pst_all_teams)
{
	int input_length;
	char user_input[10];
	int v_found = 0;
	struct result v_results;



	while(!v_found )
	{
		
		input_length = 0;

		printf("\nProvide Match Result Input. i.e. A3B4 \n");
		scanf("%s",user_input);
		input_length = strlen(user_input);
		idx = 0;

		printf("Input Length : %d\n", input_length);
		if(input_length != 4)
		{
			printf("WARNING -> The input is in Wrong Format Try Again -> A3B2 LINE->%d\n\n",__LINE__);
		}

		//TO DO ALTTAKİ 2 DÖNGÜYÜ DAHA DÜZGÜN YAZ
		for(cycle_idx=0;cycle_idx<input_length;cycle_idx++)
		{
			if( (user_input[idx] == ' ' || user_input[idx] == '-' || user_input[idx] == '*' || user_input[idx] == ',') 
				&& (!user_input[idx] == '\0') )
			{
				printf("WARNING -> Inputunuzda Boşluk veya Karakter Kullanmayın Ornek Format->A3B2 LINE-> %d\n",__LINE__);
				break;
			}
		}

		while((user_input[idx] != ' ' || user_input[idx] != '-' || user_input[idx] != '*' || user_input[idx] != ',') && input_length == 4 && idx < 4)
		{		
			if(idx==0 || idx ==2)
			{
				if(!(user_input[idx] >= '0' && user_input[idx] <= '9'))
				{	
						if(idx == 0)
						{
							v_results.home_short_name = user_input[idx];
						} 
						else 
						{ 
							v_results.away_short_name = user_input[idx];
						}
				} 
				else
				{
					printf("UYARI -> Hatalı Formatta Giriş Yaptınız Tekrar Deneyin Örnek Format -> A3B2 LINE->%d\n\n",__LINE__);
					while ((input_buffer_clear = getchar()) != '\n' && input_buffer_clear != EOF) { }
					break;
				}
			}

			if(idx == 1 || idx == 3)
			{
				if((user_input[idx] >= '0' && user_input[idx] <= '9'))
				{
					if(idx == 1)
					{
							v_results.home_scored = user_input[idx];
							v_results.home_scored = v_results.home_scored - '0';
					} 
					else
					{
						v_results.away_scored = user_input[idx];
						v_results.away_scored = v_results.away_scored - '0';		
					} 
				}
				else
				{
					printf("UYARI -> Hatalı Formatta Giriş Yaptınız Tekrar Deneyin Örnek Format -> A3B2 LINE->%d\n\n",__LINE__);
					while ((input_buffer_clear = getchar()) != '\n' && input_buffer_clear != EOF) { }
					break;
				}
			}			
		idx++;						
		}

		//printf("TESTING -> SN :%c IT: %d SC: %d \n",v_results.home_short_name,v_results.home_team_iteration,v_results.home_scored );

		if(FALSE == GetShortNameIdx(v_results.home_short_name, &v_results.home_team_iteration, pst_all_teams) || 
		   FALSE == GetShortNameIdx(v_results.away_short_name, &v_results.away_team_iteration, pst_all_teams))
		{
			//printf("TESTING2 -> SN :%c IT: %d SC: %d \n",v_results.home_short_name,v_results.home_team_iteration,v_results.home_scored );
			printf("ERROR-> Takımlardan Biri Dosyada Yok, %s: Line: %d\n", __FUNCTION__, __LINE__);
		} 
			else if(TRUE == GetShortNameIdx(v_results.home_short_name, &v_results.home_team_iteration, pst_all_teams) || 
		   			TRUE == GetShortNameIdx(v_results.away_short_name, &v_results.away_team_iteration, pst_all_teams))
					{				
						//printf("TESTING2 -> SN :%c IT: %d SC: %d \n",v_results.home_short_name,v_results.home_team_iteration,v_results.home_scored );
						v_found = 1;
						printf("\nINFO -> INPUT ISLEME ALINIYOR.. , FUNCTION : %s \n",__FUNCTION__);
						return v_results;
					}	
	}
	return v_results;
}


void calculate_results(st_teams_infile *pst_all_teams, result *ptr_results)
{
	int status;
	
	if(ptr_results->home_team_iteration == ptr_results->away_team_iteration )
	{
		printf("ERROR -> Aynı iki takım arasında maç yapılamaz \n");
		return;
	}

	if(		ptr_results->home_scored >  ptr_results->away_scored  ){		status = 1;	}	//	Ev sahibi kazanır
	else if(ptr_results->home_scored <  ptr_results->away_scored  ){		status = 2;	}	//	Misafir kazanır
	else if(ptr_results->home_scored == ptr_results->away_scored  ){		status = 3;	}	//  Beraberlik

	//printf("TEST->STATUS : %d\n", status);
	//printf("TEST->Home Team Iteration : %d\nAway Team Iteration : %d\n",ptr_results->home_team_iteration,ptr_results->away_team_iteration);

	printf("INFO-> %s  :%1.1f | %s  :%1.1f  maçı işleniyor\n\n",
		pst_all_teams->Teams[ptr_results->home_team_iteration].long_name,
		ptr_results->home_scored,
		pst_all_teams->Teams[ptr_results->away_team_iteration].long_name,
		ptr_results->away_scored);

	switch(status)
	{
		case 1:		//
				
				pst_all_teams->Teams[ptr_results->home_team_iteration].played_win++;
				pst_all_teams->Teams[ptr_results->away_team_iteration].played_lose++;

				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_total += ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_againts += ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_average += ptr_results->home_scored - ptr_results->away_scored;

				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_total += ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_againts += ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_average += ptr_results->away_scored - ptr_results->home_scored;

				pst_all_teams->Teams[ptr_results->home_team_iteration].total_points += VICTORY;
				pst_all_teams->Teams[ptr_results->away_team_iteration].total_points += DEFEAT;

				pst_all_teams->Teams[ptr_results->home_team_iteration].played_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = pst_all_teams->Teams[ptr_results->away_team_iteration].short_name;
				pst_all_teams->Teams[ptr_results->away_team_iteration].played_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = pst_all_teams->Teams[ptr_results->home_team_iteration].short_name;

				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->away_scored;
				
				pst_all_teams->Teams[ptr_results->home_team_iteration].againts_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].againts_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->home_scored;

				pst_all_teams->Teams[ptr_results->home_team_iteration].played_total++;
				pst_all_teams->Teams[ptr_results->away_team_iteration].played_total++;
		break;

		case 2:
				pst_all_teams->Teams[ptr_results->away_team_iteration].played_win++;
				pst_all_teams->Teams[ptr_results->home_team_iteration].played_lose++;

				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_total += ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_againts += ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_average += ptr_results->home_scored - ptr_results->away_scored;

				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_total += ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_againts += ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_average += ptr_results->away_scored - ptr_results->home_scored;

				pst_all_teams->Teams[ptr_results->away_team_iteration].total_points += VICTORY;
				pst_all_teams->Teams[ptr_results->home_team_iteration].total_points += DEFEAT;

				pst_all_teams->Teams[ptr_results->home_team_iteration].played_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = pst_all_teams->Teams[ptr_results->away_team_iteration].short_name;
				pst_all_teams->Teams[ptr_results->away_team_iteration].played_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = pst_all_teams->Teams[ptr_results->home_team_iteration].short_name;

				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->away_scored;
				
				pst_all_teams->Teams[ptr_results->home_team_iteration].againts_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].againts_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->home_scored;

				pst_all_teams->Teams[ptr_results->home_team_iteration].played_total++;
				pst_all_teams->Teams[ptr_results->away_team_iteration].played_total++;
		break;

		case 3:

				pst_all_teams->Teams[ptr_results->away_team_iteration].played_draw++;
				pst_all_teams->Teams[ptr_results->home_team_iteration].played_draw++;

				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_total += ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_againts += ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_average += ptr_results->home_scored - ptr_results->away_scored;

				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_total += ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_againts += ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_average += ptr_results->away_scored - ptr_results->home_scored;

				pst_all_teams->Teams[ptr_results->away_team_iteration].total_points += DRAW;
				pst_all_teams->Teams[ptr_results->home_team_iteration].total_points += DRAW;

				pst_all_teams->Teams[ptr_results->home_team_iteration].played_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = pst_all_teams->Teams[ptr_results->away_team_iteration].short_name;
				pst_all_teams->Teams[ptr_results->away_team_iteration].played_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = pst_all_teams->Teams[ptr_results->home_team_iteration].short_name;

				pst_all_teams->Teams[ptr_results->home_team_iteration].scored_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->home_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].scored_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->away_scored;
				
				pst_all_teams->Teams[ptr_results->home_team_iteration].againts_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->away_scored;
				pst_all_teams->Teams[ptr_results->away_team_iteration].againts_hist[pst_all_teams->Teams[ptr_results->home_team_iteration].played_total] = ptr_results->home_scored;

				pst_all_teams->Teams[ptr_results->home_team_iteration].played_total++;
				pst_all_teams->Teams[ptr_results->away_team_iteration].played_total++;
		break;
	}
		
	printf("INFO-> %s  :%1.1f | %s  :%1.1f  Match Result Has Been Processed , Writing to File\n\n",
	pst_all_teams->Teams[ptr_results->home_team_iteration].long_name,
	ptr_results->home_scored,
	pst_all_teams->Teams[ptr_results->away_team_iteration].long_name,
	ptr_results->away_scored);

	 sort_all_teams(pst_all_teams);
	 update_file(TEAMS_FILE_NAME, pst_all_teams);
}

//TAKIMIN MAÇ GEÇMİŞİ
void match_history(st_teams_infile *pst_all_teams)
{
	int input_length;
	char user_input[15];
	int i_team_iteration;
	//printf("\n \"%s\" This property is under construction !!! \n", __FUNCTION__);
	
	printf("Provide Team Code to See the Match History.\n");
	scanf("%s",user_input);
	input_length = strlen(user_input);
	
	if(input_length < 2 && TRUE == GetShortNameIdx(user_input[0], &i_team_iteration,pst_all_teams))
	{
		printf("Chosen Team : %c - %s \n", pst_all_teams->Teams[i_team_iteration].short_name, pst_all_teams->Teams[i_team_iteration].long_name);
	}
	else if(input_length > 2 && TRUE == GetLongNameIdx(user_input, &i_team_iteration,pst_all_teams))
	{
		printf("Chosen Team : %c - %s \n", pst_all_teams->Teams[i_team_iteration].short_name, pst_all_teams->Teams[i_team_iteration].long_name);
	}

	printf("Match History : \n");
	printf("Played Againts --> ");
	for(idx=0;idx<pst_all_teams->count_of_team*2;idx++)
	{
		printf("%c - ", pst_all_teams->Teams[i_team_iteration].played_hist[idx]);
	}
	printf("\n");
	printf("Scored For ------> ");
	for(idx=0;idx<pst_all_teams->count_of_team*2;idx++)
	{
		printf("%d - ", pst_all_teams->Teams[i_team_iteration].scored_hist[idx]);
	}
	printf("\n");
	printf("Scored Againts --> ");
	for(idx=0;idx<pst_all_teams->count_of_team*2;idx++)
	{
		printf("%d - ", pst_all_teams->Teams[i_team_iteration].againts_hist[idx]);
	}
	printf("\n");
}

//KONSOLDAN MAÇ GİRİŞİ
//											********** KONTROL ETTİR ****************
void via_console_input(st_teams_infile *pst_all_teams)
{

	struct result results;
		
	results = validate_user_input(pst_all_teams);	
	calculate_results(pst_all_teams, &results);


}

//DOSYADAN MAÇ GİRİŞİ
void via_file_input(st_teams_infile *pst_all_teams)
{
	printf("\n \"%s\" This property is under construction !!! \n", __FUNCTION__);

	struct result results;
	int file_size;
	FILE *fp_input = NULL;
	

	fp_input = fopen("input.txt","r");
	if(fp_input == NULL) 
	{
		printf("Dosya Açılamadı. %d\n",__LINE__);
		return;
	}

	file_size = get_file_size(fp_input);
	printf("file_size : %d\n",file_size );
	char file_input[file_size];
	printf("IN FILE INPUT -> File Size = %d\n",file_size );

	if (fp_input != NULL)
	{		
		//fseek( fp_input , 0 , SEEK_SET);
		//fread(file_input, sizeof(file_input), 1, fp_input);
		
		fscanf(fp_input,"%s",file_input);
		printf("file_input %s  \n",file_input );		
		char *token = strtok(file_input, ",");
		
		while (token != NULL) 
    	{ 
       		printf("TOKEN : %s\n", token); 
       		results = validate_file_input(pst_all_teams,token);	
			calculate_results(pst_all_teams, &results);
        	token = strtok(NULL, ",!"); 
    	} 
	}
	else printf("WARNING -> Couldnt Find input.txt");	
}

void guessing_menu(st_teams_infile * pst_AllTeam)
{
	int fguess_result = 0;
	int it_first_team,it_second_team = 1;
	char user_input[3];
	char sn_first_team,sn_second_team;
	result v_results;

	memset(&v_results, 0x00, sizeof(result));	//Initialization

	while(1)
	{
		printf("Karsilasma Yapacak Takimlarin Kodlarini Aralarına '-' Koyarak Giriniz. \n" );
		scanf("%s",user_input);
		sn_first_team = user_input[0];
		sn_second_team = user_input[2];
		// printf("İlki : %c - İkinci : %c\n",first_team,second_team );
		if(user_input[1] == '-')
		{
			if(TRUE == GetShortNameIdx(sn_first_team, &it_first_team, pst_AllTeam) &&
			   TRUE == GetShortNameIdx(sn_second_team, &it_second_team, pst_AllTeam))
			{
				printf("Seçilen Takimlar : %s - %s  \n",	pst_AllTeam->Teams[it_first_team].long_name, pst_AllTeam->Teams[it_second_team].long_name);
				break;
			}
			else 
			{
				printf("Takımlardan biri listede bulunmamakta Tekrar Giriniz \n");

			}
		}
		else
		{ 
			printf("Yanlis Formatta Girdiniz !!!\n");
			printf("TEKRAR GİRİŞ YAPINIZ !!!\n");
		}
	}

	if (TRUE != guess_result(&v_results, it_first_team,it_second_team,pst_AllTeam))
	{ 
		printf("Sonuç tahmin edilemedi :(\n");
		return;
	}

	printf("GUESSING RESULT WAS SUCCESSFUL %d , %s \n",__LINE__,__FUNCTION__);
	if(v_results.home_scored>v_results.away_scored)
	{
		fguess_result = 2;
	}
	else if(v_results.home_scored<v_results.away_scored)
	{
		fguess_result = 1;
	}
	switch(fguess_result)
	{
		case 0:
		// BERABERLIK
		printf("BERABERLIKLE SONUCLANDI \n");
		printf("TAHMINI SKOR %1.2f - %1.2f \n",v_results.home_scored,v_results.away_scored);
		break;
		case 1:
		// KONUK TAKIM KAZANIR
		printf("%c KAZANIR \n",v_results.away_short_name);
		printf("TAHMINI SKOR %1.2f - %1.2f \n",v_results.home_scored,v_results.away_scored);
		break;
		case 2:
		// EV SAHIBI KAZANIR
		printf("%c KAZANIR \n",v_results.home_short_name);
		printf("TAHMINI SKOR %1.2f - %1.2f \n",v_results.home_scored,v_results.away_scored);
		break;
	}	

	// printf("%s  -  %s  MAÇ SONUCU : \n",pst_AllTeam->Teams[it_first_team].long_name,pst_AllTeam->Teams[it_second_team].long_name );
}

int played_before(int p_first_team_it,int p_second_team_it,st_teams_infile *pst_AllTeam,result *v_results)
{
	int idx,founded = 0;
	while(idx<pst_AllTeam->Teams[p_first_team_it].played_total && !founded )
	{
		if(pst_AllTeam->Teams[p_first_team_it].played_hist[idx] == pst_AllTeam->Teams[p_second_team_it].short_name)
		{
			v_results->home_team_iteration = p_first_team_it;
			v_results->home_short_name = pst_AllTeam->Teams[p_first_team_it].short_name;
			v_results->home_scored = pst_AllTeam->Teams[p_first_team_it].scored_hist[idx];
			v_results->away_team_iteration = p_second_team_it;
			v_results->away_short_name = pst_AllTeam->Teams[p_second_team_it].short_name;
			v_results->away_scored = pst_AllTeam->Teams[p_second_team_it].scored_hist[idx];
			founded = 1;
			return founded;
		}
	idx++;
	}
	return founded;
}

//GRAFİK EĞİLİMİ KULLANILMADI , YEDİĞİ SKOR KULLANILMADI
unsigned char guess_result(result *p_guess_result, int p_first_team_it, int p_second_team_it,st_teams_infile *pst_AllTeam)
{
	team_recent_results first_team_results, second_team_results;
	result played_before_result;

	printf("NOW GUESSING RESULT LINE = %d FUNCTION = %s \n",__LINE__,__FUNCTION__ );

	p_guess_result->home_team_iteration = p_first_team_it;
	p_guess_result->away_team_iteration = p_second_team_it;
	p_guess_result->home_short_name = pst_AllTeam->Teams[p_first_team_it].short_name;
	p_guess_result->away_short_name = pst_AllTeam->Teams[p_second_team_it].short_name;;

	printf("FIRST IT :%d SECOND IT :%d\n",p_first_team_it,p_second_team_it );
	
	memset(&first_team_results, 0x00, sizeof(team_recent_results));
	get_last_match_results(&first_team_results, &pst_AllTeam->Teams[p_first_team_it]);
	
	printf("FIRST TEAM recent_graph : %1.2f \nFIRST TEAM recent_scored : %1.2f \nFIRST TEAM recent_scored_againts : %1.2f \n",
		first_team_results.recent_graph,
		first_team_results.recent_scored,
		first_team_results.recent_scored_againts);
	
	memset(&second_team_results, 0x00, sizeof(team_recent_results));
	get_last_match_results(&second_team_results, &pst_AllTeam->Teams[p_second_team_it]);
	
	printf("SECOND TEAM recent_graph : %1.2f \nSECOND TEAM recent_scored : %1.2f \nSECOND TEAM recent_scored_againts : %1.2f \n",
		second_team_results.recent_graph, 
		second_team_results.recent_scored,
		second_team_results.recent_scored_againts);
	
	if(TRUE == played_before(p_first_team_it, p_second_team_it, pst_AllTeam, &played_before_result))
	{
		// ONCEDEN MAC YAPMISLAR FUNC STRUCTI DOLDURUYOR , SON MAC STRUCTLARI DA GELDI KARAR VER
		//ILK TAKIM FORMDA
			p_guess_result->home_scored =  ((played_before_result.home_scored * 0.5 + first_team_results.recent_scored*0.5)/2);
			p_guess_result->away_scored =  ((played_before_result.away_scored * 0.5 + second_team_results.recent_scored*0.5)/2);
		
	}
	else	//ONCEDEN MAC YAPMAMISLAR
	{
			p_guess_result->home_scored =  (first_team_results.recent_scored + second_team_results.recent_scored_againts)/2;
			p_guess_result->away_scored =  (second_team_results.recent_scored + first_team_results.recent_scored_againts)/2;
	}

	// BERABERLIK ICIN BIR KONTROL
	// if(p_guess_result->home_scored - p_guess_result->away_scored > 1)
	// {
		// p_guess_result->home_scored = p_guess_result->away_scored;
	// }

	printf("FINAL RESULTS : %1.2f - %1.2f \n", p_guess_result->home_scored, p_guess_result->away_scored);
	return TRUE;
}

//checks last 5 matches and returns sctruct 
unsigned char  get_last_match_results(team_recent_results *p_last_results, team_info *p_team_info)
{
	float graph_value = 1.0;

	printf("FETCHING %s LAST MATCHES LINE : %d FUNCTION : %s \n",p_team_info->long_name,__LINE__,__FUNCTION__);

	for(idx=0;idx<5;idx++)
	{
		if(p_team_info->scored_hist[p_team_info->played_total-idx] > 
		   p_team_info->againts_hist[p_team_info->played_total-idx])
		{
			p_last_results->recent_graph += graph_value * 1;
			p_last_results->recent_scored += (float)p_team_info->scored_hist[p_team_info->played_total-idx] * graph_value;
			p_last_results->recent_scored_againts += (float)p_team_info->againts_hist[p_team_info->played_total-idx] * graph_value;
		}
		else if(p_team_info->scored_hist[p_team_info->played_total-idx] < 
			    p_team_info->againts_hist[p_team_info->played_total-idx])
		{
			p_last_results->recent_graph += graph_value * (0);
			p_last_results->recent_scored += (float)p_team_info->scored_hist[p_team_info->played_total-idx] * graph_value;
			p_last_results->recent_scored_againts += (float)p_team_info->againts_hist[p_team_info->played_total-idx] * graph_value;
		}
		else if(p_team_info->scored_hist[p_team_info->played_total-idx] == 
			    p_team_info->againts_hist[p_team_info->played_total-idx])
		{
			p_last_results->recent_graph += graph_value * (0.5);
			p_last_results->recent_scored += (float)p_team_info->scored_hist[p_team_info->played_total-idx] * graph_value;
			p_last_results->recent_scored_againts += (float)p_team_info->againts_hist[p_team_info->played_total-idx] * graph_value;
		}
		graph_value = graph_value - (0.2) ; 
	}

	p_last_results->recent_scored = p_last_results->recent_scored/3;	
	p_last_results->recent_scored_againts = p_last_results->recent_scored_againts/3;

	return TRUE;
}


	
//MAIN MENU 
int main_page(st_teams_infile * pst_AllTeam){

		unsigned int menu_selection 	= 0;
		unsigned int * p_menu_selection = &menu_selection;

	do{
		welcome_screen(p_menu_selection);
		switch (menu_selection) {
			case 1:
				//go to display_overall
				printf("Overview\n\n");
				display_overall(pst_AllTeam);	 
				break;
			case 2:
				//go to match_history
				printf("Match History\n\n");
				match_history(pst_AllTeam);
				break;
			case 3:
				printf("Match Result via Console\n\n");
				via_console_input(pst_AllTeam);
				break;
			case 4:
				printf("Match Result via File\n\n");			
				via_file_input(pst_AllTeam);
				break;
			case 5:
				printf("Read Scenario From File\n\n");			
				read_scenario_from_file(pst_AllTeam);
				break;
			case 6:
				printf("GUessing\n\n");			
				guessing_menu(pst_AllTeam);
				break;
			case 7:
				printf("Reset Scenario\n");
				reset_scenario(pst_AllTeam);
		}

	}while(menu_selection < NUMBER_OF_DEFINED_FUNCTION_IN_MENU && menu_selection != 0);
return 1;
}

void reset_scenario(st_teams_infile * pst_AllTeam)
{
	memset(pst_AllTeam, 0x00, sizeof(st_teams_infile));
	init_structure(pst_AllTeam);
	update_file(TEAMS_FILE_NAME, pst_AllTeam);
	printf("\nINFO -> SUCCESSFULLY RESETTED SCENARIO \n\n");
}


void parse_scenario_line(char *pline, result *presult, st_teams_infile *pst_AllTeam, int *pcounter)
{
	char *token = trim(strtok(pline, "	"));
	int idx = 0;

	if(TRUE == GetLongNameIdx(token, &presult->home_team_iteration, pst_AllTeam))
	{
		printf("İlk token: %s\n", token);
		presult->home_short_name = pst_AllTeam->Teams[presult->home_team_iteration].short_name;
	}	 
	else
	{
		printf("%d satırındaki %s Home Takımı bulunamadı \n",*pcounter, token);
		return;	
	}

	while (token != NULL) 
	{ 
    	token = trim(strtok(NULL, "	"));
    	if(idx == 0)
    	{
    		printf("İkinci token: %s\n", token);
			if(TRUE == GetLongNameIdx(token, &presult->away_team_iteration, pst_AllTeam))
			{
				presult->away_short_name = pst_AllTeam->Teams[presult->away_team_iteration].short_name;
				// printf("AWAY TEAM IT %d\n",presult->away_team_iteration);
			}
			else
			{
				printf("%d satırındaki %s Away Takımı bulunamadı \n",*pcounter, token);	
				return;
			}	
    	}
    	else if(idx == 1)
    	{
    		presult->home_scored = token[0] - '0';
    		presult->away_scored = token[4] - '0';
			*pcounter = (*pcounter) + 1;
			break;
    	}    	
    	idx++;
	}
	
}

/*
EXPECTING LAST MATCHES TO READ LAST NEAR, LIKE FIRST MATCHES TO BE READ FIRST*/

void read_scenario_from_file(st_teams_infile * pst_AllTeam)
{	
	FILE * fp_scenario = NULL;
	#define MAX_LEN 9000
	int successful_lines = 0;
	int total_lines = 0;
	char line[MAX_LEN];
	result scenario_result;
	memset(&scenario_result, 0x00, sizeof(result));
	
	reset_scenario(pst_AllTeam);
	fp_scenario = fopen("scoreboardcomsonuclar.txt","r");
	if(fp_scenario != NULL)
	{
		while (fgets ( line, MAX_LEN, fp_scenario ) != NULL ) /* read a line */
		{
			total_lines++;
			printf("SATIR NO ---> %d\n",total_lines );
			parse_scenario_line(line, &scenario_result, pst_AllTeam, &successful_lines);

			printf("HOME TEAM : %c %d ATTIGI %f \n", scenario_result.home_short_name, 
				scenario_result.home_team_iteration, scenario_result.home_scored );

			printf("AWAY TEAM : %c %d ATTIGI %f \n", scenario_result.away_short_name, 
				scenario_result.away_team_iteration, scenario_result.away_scored );

			calculate_results(pst_AllTeam, &scenario_result);
			display_overall(pst_AllTeam);//Oktay @sil

			printf("\n");
		}

		if(total_lines == successful_lines)
		{
			printf("ALL SCENARIO LINES SUCCESSFULLY READ FROM FILE \n");
		}
		else
		{
			printf("ERRORS HAVE OCCURED DURING READING\n");
			printf("TOTAL LINES IN FILE  : %d\n", total_lines);
			printf("LINES READ FROM FILE : %d\n", successful_lines);
		}
	}	
	else printf("Senaryo dosyası bulunamadı !\n");
	fclose(fp_scenario);
}

char *TakimAdlari[]= {
	"Galatasaray",
	"Basaksehir",
	"Besiktas",
	"Trabzonspor",
	"Malatyaspor",
	"Konyaspor",
	"Alanyaspor",
	"Antalyaspor",
	"Rizespor",
	"Fenerbahce",
	"Kayserispor",
	"Sivasspor",
	"Kasimpasa",
	"Ankaragucu",
	"Goztepe",
	"Bursaspor",
	"Erzurum BB",
	"Akhisarspor"
};

void init_structure(st_teams_infile * pst_AllTeam)
{
	int max_index = sizeof(pst_AllTeam->Teams)/sizeof(team_info);
	int index = 0;

	printf("\n#### Initting files #### \n");

	pst_AllTeam->count_of_team = 0;

	for (index = 0; index < max_index; index++)
	{
		pst_AllTeam->count_of_team++;
		pst_AllTeam->Teams[index].short_name = 'A' + index;
		memcpy(pst_AllTeam->Teams[index].long_name, 
			TakimAdlari[index], 
			MIN(strlen(TakimAdlari[index]), sizeof(pst_AllTeam->Teams[index].long_name)-1));
	}
}

void update_file(char * file_name, st_teams_infile *pst_AllTeams)
{
	FILE * fp_teams = NULL;
	fp_teams = fopen(file_name, "w");

	if (fp_teams != NULL)
	{
		fseek( fp_teams , 0 , SEEK_SET );
		fwrite(pst_AllTeams, sizeof(st_teams_infile), 1, fp_teams);
	}

	fclose(fp_teams);
}

void update_team_only(char * file_name, st_teams_infile *pst_AllTeams, int team_index)
{
	FILE * fp_teams = NULL;
	fp_teams = fopen(file_name, "w");

	if (fp_teams != NULL)
	{
		fseek( fp_teams , pst_AllTeams->count_of_team + sizeof(team_info)*team_index , SEEK_SET ); //team_index : 0,1,2....
		fwrite(&pst_AllTeams->Teams[team_index], sizeof(team_info), 1, fp_teams);
	}

	fclose(fp_teams);
}

void read_from_already_opened_file(FILE * fp_teams, st_teams_infile *pst_AllTeams)
{
	if (fp_teams != NULL)
	{
		fseek( fp_teams , 0 , SEEK_SET );
		fread(pst_AllTeams, sizeof(st_teams_infile), 1, fp_teams);
	}
}

int get_file_size(FILE *fp)
{
	if(fp == NULL)
		return -1;

	printf("Calculating File Size\n");
	int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    printf("Calculated File Size\n");
    return sz;
}

int main(void)
{
	st_teams_infile	AllTeams;
	FILE * fp_teams = NULL;
	int file_size = 0;
	// guess_print_hello();
	memset(&AllTeams, 0x00, sizeof(AllTeams));

	fp_teams = fopen(TEAMS_FILE_NAME, "a+");

	if (fp_teams != NULL)
	{	
		file_size = get_file_size(fp_teams);
		read_from_already_opened_file(fp_teams, &AllTeams);
		fclose(fp_teams);
		
		//Dosyadaki takım sayısı 0 olabilir, veya AllTeams structure boyutu değişmiş olabilir
		if ( (AllTeams.count_of_team == 0) || (file_size != sizeof(AllTeams)) )
		{
			init_structure(&AllTeams);
			update_file(TEAMS_FILE_NAME, &AllTeams);
		}   		
	}
	else
	{		
		printf("HATA-> Couldnt Find Teams.txt ,  Line: %d\n", __LINE__);
		return 0;
	}

	intro_page();
	main_page(&AllTeams);

	return 0;	
}
