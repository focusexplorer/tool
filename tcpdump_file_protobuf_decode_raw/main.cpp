#include<stdio.h>
#include<fstream>
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<iostream>
#include<arpa/inet.h>
using std::cout;
using std::endl;
int main(int argc,char*argv[])
{
	const char*file_name=argv[1];
	std::ifstream infile(file_name);
	if(!infile)
	{
		cout<<"tcpdump file  open error:"<<file_name<<endl;
		return -1;
	}
	
	char buf[10240];
	int len=0;
	//skip head
	infile.read(buf,24);
	while(infile)
	{
		//time
		infile.read(buf,8);
		//len
		int la=0,lb=0;
		if(!infile.read((char*)&la,sizeof(la)))
			break;
		if(!infile.read((char*)&lb,sizeof(lb)))
			break;
		if(la!=lb)
		{
			printf("la(%d)!=lb(%d)\n",la,lb);
			return -1;
		}
		if(!infile.read(buf,la))
			break;

		//transfer to child process
		{
			int pipefd[2];
			pid_t cpid;
			char ch;


			if (pipe(pipefd) == -1) {
				perror("pipe");
				exit(EXIT_FAILURE);
			}

			cpid = fork();
			if (cpid == -1) {
				perror("fork");
				exit(EXIT_FAILURE);
			}

			if (cpid == 0) {    /* Child reads from pipe */
				close(pipefd[1]);          /* Close unused write end */
				
				dup2(pipefd[0],0);

				execl("./protoc","protoc","--decode_raw",(char*)0);

				close(pipefd[0]);
				_exit(EXIT_SUCCESS);

			} else {            /* Parent writes argv[1] to pipe */
				close(pipefd[0]);          /* Close unused read end */
				printf("la=%d\n",la-4);
				write(pipefd[1], buf+44+4, la-44-4);
				close(pipefd[1]);          /* Reader will see EOF */
				wait(NULL);                /* Wait for child */
				continue;
			}
		}


	}

	infile.close();
	return 0;
}
