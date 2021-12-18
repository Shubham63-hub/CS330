#include "wc.h"


extern struct team teams[NUM_TEAMS];
extern int test;
extern int finalTeam1;
extern int finalTeam2;

int processType = HOST;
const char *team_names[] = {
  "India", "Australia", "New Zealand", "Sri Lanka",   // Group A
  "Pakistan", "South Africa", "England", "Bangladesh" // Group B
};


void teamPlay(void){
  //read command
  char readcommand = 'r';

  //inputpath
  char *inppath = (char*)malloc(sizeof(char)*100);
  sprintf(inppath, "test/%d/", test);
  strcat(inppath, "inp/");
  strcat(inppath, teams[processType].name);

  //fd inputpath
  int fd = open(inppath, O_RDONLY);
  if(fd<0){
    printf("fd failed\n");
    exit(-1);
  }

  //initial read command in commpipe
  write(teams[processType].commpipe[1], &readcommand, sizeof(readcommand));

  //var to read and write the input
  char sign;
  char digit;
  
  //read from the input
  while(1){
    read(teams[processType].commpipe[0], &sign, 1);

    //exit 
    if(sign == 'f'){
      exit(0);
    }
    else{
      //read digits from input and write to matchpipe
      read(fd, &digit, 1);
      write(teams[processType].matchpipe[1], &digit, sizeof(digit));

      //next read command
      write(teams[processType].commpipe[1], &readcommand, sizeof(readcommand));
    }
    
  }
  
}



void endTeam(int teamID){
  //exit character
  char c = 'f';
  write(teams[teamID].commpipe[1], &c, sizeof(char));
}



int match(int team1, int team2){

  //toss1c and toss2c as input
  char toss1c, toss2c;
  read(teams[team1].matchpipe[0], &toss1c, 1);
  read(teams[team2].matchpipe[0], &toss2c, 1);

  //converting them to int to compute order
  int toss1 = toss1c-'0';
  int toss2 = toss2c-'0';

  int toss = toss1 + toss2;

  if(toss%2==1){
    //team1 bats
    //first innings start
    char* filename = (char*)malloc(sizeof(char)*(strlen(teams[team1].name)+ strlen(teams[team2].name)+10));
    sprintf(filename, "%sv%s", teams[team1].name, teams[team2].name);
    if(team1<NUM_TEAMS/2 && team2>NUM_TEAMS/2-1){
      strcat(filename, "-Final");
    }

    //out file path
    char *outpath = (char*)malloc(sizeof(char)*100);
    sprintf(outpath, "test/%d/", test);
    strcat(outpath, "out/");

    //full path
    char* fullpath = (char*)malloc(sizeof(char)*(strlen(outpath)+strlen(filename)+1));
    strcpy(fullpath, outpath);
    strcat(fullpath, filename);

    //create file in the out folder
    open(fullpath, O_CREAT, 0644);
    int fd = open(fullpath, O_WRONLY);

    //header of the file
    char* header = (char*)malloc(sizeof(char)*150);
    sprintf(header, "Innings1: %s bats\n", teams[team1].name);
    write(fd, header, strlen(header));

    //variables to compute runs of individual and total players
    int totalruns1 = 0, runs1 = 0, wickets1 = 0;

    for(int i=0; i<120; i++){
      //whole team got out
      if(wickets1>=10) break;

      //batc and bowlc for taking input
      char batc, bowlc;
      read(teams[team1].matchpipe[0], &batc, 1);
      read(teams[team2].matchpipe[0], &bowlc, 1);

      //converting them to int to compute further
      int bat = batc - '0';
      int bowl = bowlc - '0';

      //wicker condition
      if(bat == bowl){
        wickets1++;
        char* output = (char*) malloc (sizeof(char)*150);
        sprintf(output, "%d:%d\n", wickets1, runs1);
        write(fd, output, strlen(output));
        runs1 = 0;
      }

      //runs condition
      else{
        totalruns1 += bat;
        runs1 += bat;
      }
    }

    //if a player is not out at the end of 120 balls
    if(wickets1 != 10){
      char* output = (char*) malloc (sizeof(char)*150);
      sprintf(output, "%d:%d*\n",wickets1+1, runs1);
      write(fd, output, strlen(output));
    }

    //total runs 
    char* output = (char*) malloc (sizeof(char)*100);
    sprintf(output, "%s Total: %d\n", teams[team1].name, totalruns1);
    write(fd, output, strlen(output));


   
    //second innings start
    //header
    char* header1 = (char*)malloc(sizeof(char)*100);
    sprintf(header1, "Innings2: %s bats\n", teams[team2].name);
    write(fd, header1, strlen(header1));

    //variable to compute individual and total runs
    int totalruns2 = 0, runs2 = 0, wickets2 = 0;

    for(int i=0; i<120; i++){
      // whole team got out
      if(wickets2>=10) break;

      //batc and ballc for reading input
      char batc, bowlc;
      read(teams[team2].matchpipe[0], &batc, 1);
      read(teams[team1].matchpipe[0], &bowlc, 1);

      //converting them to int to compute further
      int bat = batc - '0';
      int bowl = bowlc - '0';

      //wicket condition
      if(bat == bowl){
        wickets2++;
        char* output = (char*) malloc (sizeof(char)*150);
        sprintf(output, "%d:%d\n", wickets2, runs2);
        write(fd, output, strlen(output));
        runs2= 0;
      }

      //runs condition
      else{
        totalruns2 += bat;
        runs2 += bat;
      }

      //if 2nd innings team make runs greater than the first
      if(totalruns2 > totalruns1) break;
    }

    //all wickets are not drop
    if(wickets2 != 10){
      char* output = (char*) malloc (sizeof(char)*150);
      sprintf(output, "%d:%d*\n",wickets2+1, runs2);
      write(fd, output, strlen(output));
    }
    
    //total runs
    char* output1 = (char*) malloc (sizeof(char)*150);
    sprintf(output1, "%s Total: %d\n", teams[team2].name, totalruns2);
    write(fd, output1, strlen(output1));
    
    //if runs scored by by team1 is greater than team2
    if(totalruns1 > totalruns2){
      char* output = (char*)malloc(sizeof(char)*200);

      //in terms of runs
      sprintf(output, "%s beats %s by %d runs\n", teams[team1].name, teams[team2].name, totalruns1 - totalruns2);
      write(fd, output, strlen(output));
      return team1;
    }

    //if team 2 scores more runs
    if(totalruns2 > totalruns1){
      char* output = (char*)malloc(sizeof(char)*200);

      //in terms of wickets
      sprintf(output, "%s beats %s by %d wickets\n", teams[team2].name, teams[team1].name, 10 - wickets2);
      write(fd, output, strlen(output));
      return team2;
    }

    //if match is a tie then team1 wins always as it has lower index
    if(totalruns1 == totalruns2){
      char* output = (char*) malloc (sizeof(char)*100);

      //team1 wins
      sprintf(output, "TIE: %s beats %s\n", teams[team1].name, teams[team2].name);
      write(fd, output, strlen(output));
      return team1;
    }
  }
  

  else{
    //team2 bats
    //first innnings start
    char* filename = (char*)malloc(sizeof(char)*(strlen(teams[team1].name)+ strlen(teams[team2].name)+10));
    sprintf(filename, "%sv%s", teams[team2].name, teams[team1].name);
    if(team1<NUM_TEAMS/2 && team2>NUM_TEAMS/2-1){
      strcat(filename, "-Final");
    }

    //outpath
    char *outpath = (char*)malloc(sizeof(char)*100);
    sprintf(outpath, "test/%d/", test);
    strcat(outpath, "out/");

    //full path
    char* fullpath = (char*)malloc(sizeof(char)*(strlen(outpath)+strlen(filename)+10));
    strcpy(fullpath, outpath);
    strcat(fullpath, filename);

    //open full path
    open(fullpath, O_CREAT, 0644);
    int fd = open(fullpath, O_WRONLY);

    //header for the output file
    char* header = (char*)malloc(sizeof(char)*100);
    sprintf(header, "Innings1: %s bats\n", teams[team2].name);
    write(fd, header, strlen(header));

    //variables to compute individual and overall score
    int totalruns1 = 0, runs1 = 0, wickets1 = 0;

    //total balls
    for(int i=0; i<120; i++){
      //all wickets fall
      if(wickets1>=10) break;

      //reading batc, ballc
      char batc, bowlc;
      read(teams[team2].matchpipe[0], &batc, 1);
      read(teams[team1].matchpipe[0], &bowlc, 1);

      //converting them to int for further calculation
      int bat = batc - '0';
      int bowl = bowlc - '0';

      //wicket condition
      if(bat == bowl){
        wickets1++;
        char* output = (char*) malloc (sizeof(char)*150);
        sprintf(output, "%d:%d\n", wickets1, runs1);
        write(fd, output, strlen(output));
        runs1= 0;
      }

      //runs condition
      else{
        totalruns1 += bat;
        runs1 += bat;
      }
    }

    // some player is not out
    if(wickets1 != 10){
      char* output = (char*) malloc (sizeof(char)*150);
      sprintf(output, "%d:%d*\n",wickets1+1, runs1);
      write(fd, output, strlen(output));
    }

    //total score
    char* output = (char*) malloc (sizeof(char)*150);
    sprintf(output, "%s Total: %d\n", teams[team2].name, totalruns1);
    write(fd, output, strlen(output));


    //2nd innings start
    char* header1 = (char*)malloc(sizeof(char)*150);
    sprintf(header1, "Innings2: %s bats\n", teams[team1].name);
    write(fd, header1, strlen(header1));

    //variables for computing individual and total runs
    int totalruns2 = 0, runs2 = 0, wickets2 = 0;

    //balls
    for(int i=0; i<120; i++){
      // all wickets fall
      if(wickets2>=10) break;

      //reading batc, ballc
      char batc, bowlc;
      read(teams[team1].matchpipe[0], &batc, 1);
      read(teams[team2].matchpipe[0], &bowlc, 1);

      //converting them to int 
      int bat = batc - '0';
      int bowl = bowlc - '0';

      //wicket condition
      if(bat == bowl){
        wickets2++;
        char* output = (char*) malloc (sizeof(char)*150);
        sprintf(output, "%d:%d\n", wickets2, runs2);
        write(fd, output, strlen(output));
        runs2= 0;
      }

      //run condition
      else{
        totalruns2 += bat;
        runs2 += bat;
      }

      //if 2nd innings team make more runs than the 1st
      if(totalruns2 > totalruns1) break;
    }

    // some player is not out
    if(wickets2 != 10){
      char* output = (char*) malloc (sizeof(char)*150);
      sprintf(output, "%d:%d*\n",wickets2+1, runs2);
      write(fd, output, strlen(output));
    }

    //total runs
    char* output1 = (char*) malloc (sizeof(char)*100);
    sprintf(output1, "%s Total: %d\n", teams[team1].name, totalruns2);
    write(fd, output1, strlen(output1));

    // if 1st inning team score more than the second
    if(totalruns1 > totalruns2){
      char* output = (char*)malloc(sizeof(char)*100);

      //win due to runs
      sprintf(output, "%s beats %s by %d runs\n", teams[team2].name, teams[team1].name, totalruns1 - totalruns2);
      write(fd, output, strlen(output));
      return team2;
    }

    //if second innings team score more than the first
    if(totalruns2 > totalruns1){
      char* output = (char*)malloc(sizeof(char)*100);

      //win due to wickets are left and runs made by team2 are crossed.
      sprintf(output, "%s beats %s by %d wickets\n", teams[team1].name, teams[team2].name, 10 - wickets2);
      write(fd, output, strlen(output));
      return team1;
    }

    //tie
    if(totalruns1 == totalruns2){
      char* output = (char*) malloc (sizeof(char)*100);

      // team1 wins
      sprintf(output, "TIE: %s beats %s\n", teams[team1].name, teams[team2].name);
      write(fd, output, strlen(output));
      return team1;
    }
  }
  return 0;
}



void spawnTeams(void){

  int i = 0;
  //copied filename, and setup the pipes
  while(i < NUM_TEAMS){
    strcpy(teams[i].name, team_names[i]);
    pipe(teams[i].commpipe);
    pipe(teams[i].matchpipe); 
    i++;
  }

  //made the fork processes
  i=0;
  while(i<NUM_TEAMS){
    pid_t pid = fork();
    if(pid < 0){
        printf("fork failed\n");
        exit(-1);
    }
    // child process
    if(pid == 0){
      processType = i;
      teamPlay();
    }
    i++;
  }
} 



void conductGroupMatches(void){
  // for group 1
  int p1[2];
  pipe(p1);
  pid_t pg1 = fork();

  //child
  if(pg1 == 0){
    //array to store count of team wins
    int teamwins[NUM_TEAMS/2];
    for(int i=0; i<NUM_TEAMS/2; i++){
      teamwins[i] = 0;
    }
    for(int i=0; i< NUM_TEAMS/2 - 1 ; i++){
      for(int j=i+1; j<NUM_TEAMS/2; j++){
        teamwins[match(i, j)] += 1;
      }
    }

    // for team with max wins
    int TeamA = -1;
    int maxwins = 0;
    for(int i=0; i<NUM_TEAMS/2; i++){
      if(teamwins[i] > maxwins){
        TeamA = i;
        maxwins = teamwins[i];
      }
    }

    //end the other team processes
    for(int i=0; i<NUM_TEAMS/2; i++){
      if( i!=TeamA) endTeam(i);
    }

    //write winner of group A to pipe
    char *teamA = (char*)malloc(sizeof(char));
    sprintf(teamA, "%d", TeamA);
    write(p1[1], teamA, 1);

    //end the process
    exit(0);
  }

  //for group 2
  int p2[2];
  pipe(p2);
  pid_t pg2 = fork();

  //child
  if(pg2 == 0){
    //array to store count of team wins
    int teamwins[NUM_TEAMS/2];
    for(int i=0; i<NUM_TEAMS/2; i++){
      teamwins[i] = 0;
    }
    for(int i=NUM_TEAMS/2; i< NUM_TEAMS-1 ; i++){
      for(int j=i+1; j<NUM_TEAMS; j++){
        teamwins[match(i, j)-NUM_TEAMS/2] += 1;
      }
    }
    
    //for team with max wins
    int TeamB = -1;
    int maxwins = 0;
    for(int i=0; i<NUM_TEAMS/2; i++){
      if(teamwins[i] > maxwins){
        TeamB = i+NUM_TEAMS/2;
        maxwins = teamwins[i];
      }
    }

    //end remaining teams
    for(int i=NUM_TEAMS/2; i<NUM_TEAMS-1; i++){
      if( i!=TeamB) endTeam(i);
    }

    //write winner fo group A to pipe;
    char *teamb = (char*)malloc(sizeof(char));
    sprintf(teamb, "%d", TeamB);
    write(p2[1], teamb, strlen(teamb));

    //end process
    exit(0);
  }
  
  //pass both group winners to the final
  char t1;
  char t2;
  read(p1[0], &t1, 1);
  finalTeam1 = t1 - '0';
  read(p2[0], &t2, 1);
  finalTeam2 = t2 - '0';

  close(pg1);
  close(pg2);
}
