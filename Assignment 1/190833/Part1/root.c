#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

int main(int argc, char** argv)
{
	//error condition
	if(argc <= 1){
		printf("UNABLE TO EXECUTE");
		exit(-1);
	} 
	//base contidion
	if(argc==2){
		if(0<= (argv[1][0]-'0') && argv[1][0]-'0' <=9){
			unsigned long long n = atoll(argv[1]);
			printf("%lld",(unsigned long long)round(sqrt(n)));
			exit(0);
		}
		//error condition
		else{
			printf("UNABLE TO EXECUTE");
			exit(-1);
		}
	}
	
	//exec calling conditions with varied value
	else{
		if(0<= (argv[argc-1][0]-'0') && argv[argc-1][0]-'0' <=9){
			unsigned long long n = atoll(argv[argc-1]);
			n = round(sqrt(n));
			sprintf(argv[argc-1], "%lld", n);
		}
		//error condition
		else {
			printf("UNABLE TO EXECUTE");
			exit(-1);
		}
		
		// removing the previous operation
		argv = argv + 1;
	
		if(!strcmp(argv[0], "double")){
			if(argv[argc-1] != NULL){
				argv[argc-1] = NULL;
			}
			if(execvp("./double", argv)){
				printf("UNABLE TO EXECUTE");
				exit(-1);
			}
		}
		else if(!strcmp(argv[0], "square")){
			if(argv[argc-1] != NULL){
				argv[argc-1] = NULL;
			}
			if(execvp("./square", argv)){
				printf("UNABLE TO EXECUTE");
				exit(-1);
			}
		}
		else if(!strcmp(argv[0], "root")){
			if(argv[argc-1] != NULL){
				argv[argc-1] = NULL;
			}
			if(execvp("./root", argv)){
				printf("UNABLE TO EXECUTE");
				exit(-1);
			}
		}
		// if random expression is given in between
		else{
			printf("UNABLE TO EXECUTE");
			exit(-1);
		}
	}
	return 0;
}
