#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>

//untuk exit()
#include <stdlib.h>
//untuk errno
#include <errno.h>
//untuk readline
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;

void pipee(int fd[],char** cmd1,char** cmd2) {
	int pid = fork();
	switch(pid) {
		case -1:
					perror("fork");
					exit(1);
		case 0:
					//proses anak
					dup2(fd[0],0);
					close(fd[1]);
					execvp(cmd2[0],cmd2);
					perror(cmd2[0]);
		default:
					//proses parent
					dup2(fd[1],1);
					close(fd[0]);
					execvp(cmd1[0],cmd1);
					perror(cmd1[0]);
	}	
}

int main() {
	while(1) {
		//masukkan inputan ke tmp2
		string ss = get_current_dir_name();
		ss += " $ >_> FAM shell $ ";
		char* tmp2 = readline(ss.c_str());
		if (tmp2[0]!=0)	add_history(tmp2);

		//partisi tmp2, lalu add history
		char* prog = strtok(tmp2," ");
		
		//konversi prog ke vector<char*>
		vector<char*> args;
		char* tmp = prog;
		while(tmp!=NULL) {
			args.push_back(tmp);
			tmp = strtok(NULL, " ");
		}

		//konversi args ke char**
		char** argv = new char*[args.size()+1];
		for(int k=0;k<args.size();k++)
			argv[k] = args[k];
		argv[args.size()] = NULL;

		//cek pipe
		for(int i=0;i<args.size();i++) {
			if(!strcmp(argv[i],"|")) {
				int fd[2]; pipe(fd);
				char **cmd1 = argv;
				argv[i] = 0;
				char **cmd2 = argv + i + 1;
				pipee(fd,cmd1,cmd2);			
			}
		}
		struct passwd *pw = getpwuid(getuid());
		const char *homedir = pw->pw_dir;
		//cek exit, cd, PATH proccess
		if(!strcmp(prog,"exit"))
			return 0; 
		else if(!strcmp(prog,"cd")) {
			if (strcmp (prog, "cd") == 0)
	    	{
	      		if (argv[1] != NULL)
				{
					if(strcmp (argv[1],"~") == 0){
						chdir(homedir);
					}
					else{
			  			if(chdir (argv[1]) != -1){
			  				
						}
						else{
							perror(prog);
						}
					}
				}
				
	    	}
		}
		else {
			int in = -999;
  			int out = -999;
  			for (int i = 0; i < args.size(); i++){
  				if(strcmp(argv[i],">") == 0) out = i;
  				else if(strcmp(argv[i],"<") == 0) in = i;
  			}
  			int fo = -999;
  			int fi = -999;
  			int read = 0;
  			int write = 0;
				if(out != -999){
					fo = open(argv[out+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
					write = 1;
				}
				if(in != -999){
					fi = open(argv[in+1], O_RDONLY);
					read = 1;
				}
			pid_t kidpid = fork();
			if(kidpid < 0) { 
				perror("Internal error. Cannot fork");
				return -1;
			}
			else if(kidpid == 0){
		  			// I am the child.
		  			if(fo != -999 || fi != -999){
		  				char** newargv = new char*[args.size()+1];
		  				int z=0;
						int temp=0;
						while(z < args.size()){
							if(z!=out && z!= out+1 && z!=in && z!= in+1){
								newargv[temp] = argv[temp];
								temp++;	
							}
							z++;
						}
						if(read == 1){
		  					dup2(fi,0);
		  					close(fi);
		  				}
		  				if(write == 1){
		  					dup2(fo,1);
		  					close(fo);
		  				}
						argv = newargv;
						argv[temp] = NULL;
					}
					
					execvp (prog, argv);
		  			// The following lines should not happen (normally).
		 			//perror( command );
		  			return -1;
			}
			else {
				//I am the parent. wait for the child
				if(waitpid(kidpid,0,0) < 0) {
					perror("Internal error. Cannot wait for the child");
					return -1; 
				}
			}
		}
	}
}



