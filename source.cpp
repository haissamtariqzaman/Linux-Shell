#include "command.h"
#include <iostream>
#include <unistd.h>
#include <wait.h>
#include <cstring>
using namespace std;

void printPrompt();
void initializeTerminal(char**&);
command** parseInput(char*);
void parsePath(char**&);
void destrct(command**);
void destrctpaths(char**);
int cexec(const commandexec*,char**);
int cexecp(commandpipe*,char**,int*,bool);
int cexecr(commandrd*,char**,int*,bool);

int main()
{
	char** paths=nullptr;
	char* buffer=new char[200];
	initializeTerminal(paths);

	while(true)
	{
		printPrompt();
		cin.getline(buffer,200);
		command** cmd=parseInput(buffer);

		if(cmd[0]->gettype()=='n')
		{
			commandexec* cmnd=dynamic_cast<commandexec*>(cmd[0]);
			int ext=cexec(cmnd,paths);
			if(ext==-1)
			{
				break;
			}
		}
		else
		{
			bool next=false;
			int p[2];
			pipe(p);

			for (int x=0;cmd[x]!=nullptr;x++)
			{
				next=(cmd[x+1]==nullptr) ? false : true;
				if(cmd[x]->gettype()=='|')
				{
					commandpipe* cmnd=dynamic_cast<commandpipe*>(cmd[x]);
					cexecp(cmnd,paths,p,next);
				}
				else if(cmd[x]->gettype()=='>' || cmd[x]->gettype()=='<')
				{
					commandrd* cmnd=dynamic_cast<commandrd*>(cmd[x]);
					int error=cexecr(cmnd,paths,p,next);

					if(error==-1)
					{
						break;
					}
				}
			}

			close(p[0]);
			close(p[1]);
		}
		destrct(cmd);
	}
	destrctpaths(paths);
	return 0;
}

int cexec(const commandexec* c,char** paths)
{
	char temp[50];

	if (!strcmp(c->getfName(),"exit"))
	{
		return -1;
	}
	else if(!strcmp(c->getfName(),"cd"))
	{
		if(c->getargv()[1]==nullptr)
		{
			strcpy(temp,"/home/");
			strcat(temp,getenv("USER"));
			chdir(temp);
		}
		else
		{			
			int dError=chdir(c->getargv()[1]);
			if(dError)
			{
				cout<<"Error! File/directory not found!"<<endl;
			}
		}
		return 0;
	}
	else if(c->getfName()[0]=='.' || c->getfName()[0]=='/')
	{
		pid_t pid=fork();
		if(pid==0)
		{
			execv(c->getfName(),c->getargv());
			cout<<"Error! File/directory not found!"<<endl;
			exit(0);
		}
		else if(pid>0)
		{
			wait(NULL);
		}
	}
	else
	{
		pid_t pid=fork();
		if(pid==0)
		{
			for(int x=0;paths[x]!=nullptr;x++)
			{
				strcpy(temp,paths[x]);
				strcat(temp,"/");
				strcat(temp,c->getfName());
				execv(temp,c->getargv());
			}
			cout<<"Error! Command not found!"<<endl;
			exit(0);
		}
		else if(pid>0)
		{
			wait(NULL);
		}
	}
	return 0;
}

int cexecp(commandpipe* c,char** paths,int* p,bool next)
{
	const command* l=c->getleftside();
	const command* r=c->getrightside();
	int reade=dup(0);
	int writee=dup(1);

	if(l->gettype()=='n')
	{
		dup2(p[1],1);
		cexec(dynamic_cast<const commandexec*>(l),paths);
	}
	close(p[1]);

	dup2(writee,1);

	int p1[2];

	if(next)
	{
		pipe(p1);
		dup2(p1[1],1);
	}
	
	dup2(p[0],0);

	cexec(dynamic_cast<const commandexec*>(r),paths);
	close(p[0]);
	
	dup2(reade,0);

	if(next)
	{
		dup2(writee,1);
		p[0]=p1[0];
		p[1]=p1[1];
	}

	return 0;
}

int cexecr(commandrd* c,char** paths,int* p,bool next)
{
	int reade=dup(0);
	int writee=dup(1);
	const command* cmd=c->getcmnd();

	if(c->getfd()==-1)
	{
		char temp[]="File not found!\n";
		write(2,temp,strlen(temp)+1);
		return -1;
	}

	if(c->gettype()=='<' && cmd->gettype()=='n')
	{
		dup2(c->getfd(),0);
		if(next)
		{
			dup2(p[1],1);
		}
		cexec(dynamic_cast<const commandexec*>(cmd),paths);
	}
	else
	{
		if(cmd->gettype()!='n')
		{
			char temp[10000];
			read(p[0],temp,10000);
			write(c->getfd(),temp,strlen(temp));
		}
		else
		{
			dup2(c->getfd(),1);
			cexec(dynamic_cast<const commandexec*>(cmd),paths);
		}
	}
	dup2(writee,1);
	dup2(reade,0);

	return 0;
}

void printPrompt()
{
	char path[100];
	char username[30];

	strcpy(username,getenv("USER"));

	if(getcwd(path,100)!=NULL)
	{
		cout<<"\033[0;31m"<<username<<"@my-shell"<<"\033[0m"<<":"<<"\033[0;34m"<<path<<"\033[0m"<<"$ ";
	}
}

void initializeTerminal(char** &paths)
{
	char username[30];
	strcpy(username,getenv("USER"));
	
	char dir[36];
	strcpy(dir,"/home/");
	strcat(dir,username);

	chdir(dir);

	parsePath(paths);

	char** parameters=new char*[2];
	parameters[0]=new char[6];
	parameters[1]=nullptr;

	strcpy(parameters[0],"clear");

	pid_t pid=fork();
	if(pid==0)
	{
		execv("/usr/bin/clear",parameters);
		return;
	}
	else
	{
		wait(NULL);
	}

	delete[] parameters[0];
	delete[] parameters;
}

command** parseInput(char* buf)
{
	command** cmd=new command*[20];
	int x=0,i=0,count=0;
	char temp[100];

	while(true)
	{
		if(buf[x]=='\0')
		{
			if(i!=0)
			{
				temp[i]='\0';
				cmd[count++]=new commandexec(temp);
			}
			break;
		}
		else if(buf[x]=='|')
		{	
			command* l=nullptr;
			command* r=nullptr;
			
			if(i!=0)
			{
				temp[i]='\0';
				i=0;
				l=new commandexec(temp);
			}
			else
			{
				l=cmd[count-1];
			}
			
			while(buf[++x]==' ');
			
			while(buf[x] != '|' && buf[x] != '>' && buf[x] != '<' && buf[x] != '\0')
			{
				temp[i++]=buf[x++];
			}
			temp[i]='\0';
			i=0;
			r=new commandexec(temp);

			cmd[count++]=new commandpipe(l,r);
		}
		else if (buf[x]=='>'||buf[x]=='<')
		{
			char mode=buf[x];
			command* l=nullptr;

			if(i!=0)
			{
				temp[i]='\0';
				i=0;
				l=new commandexec(temp);
			}
			else
			{
				l=cmd[count-1];
			}

			while(buf[++x]==' ');

			while(buf[x]!=' ' && buf[x]!='|' && buf[x]!='<' && buf[x]!='>' && buf[x]!='\0')
			{
				temp[i++]=buf[x++];
			}
			temp[i]='\0';
			i=0;

			cmd[count++]=new commandrd(l,temp,mode);
		}
		else
		{
			temp[i++]=buf[x];
			x++;
		}
	}
	cmd[count]=nullptr;
	return cmd;
}

void parsePath(char**& paths)
{
	char* temp=getenv("PATH");
	int count=0;

	for(int x=0;x<strlen(temp);x++)
	{
		if(temp[x]==':')
		{
			count++;
		}
	}

	count+=2;

	paths=new char*[count];
	paths[count-1]=nullptr;

	int x=0;
	int y=0;
	int z=0;
	
	paths[0]=new char[30];

	while(temp[x]!='\0')
	{
		if(temp[x]==':')
		{
			z=0;
			y++;
			paths[y]=new char[30];
		}
		else
		{
			paths[y][z++]=temp[x];
		}
		x++;
	}
}

void destrct(command** cmd)
{
	if (cmd!=nullptr)
	{
		int x=0;
		for (x;cmd[x]!=nullptr;x++)
		{}
		delete cmd[x-1];
		delete[] cmd;
	}
}

void destrctpaths(char** path)
{
	if(path!=nullptr)
	{
		for (int x=0;path[x]!=nullptr;x++)
		{
			delete[] path[x];
		}
		delete[] path;
	}
}