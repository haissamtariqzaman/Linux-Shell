#include "command.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
using namespace std;

//--------------------------------------------command------------------------------------------------

command::command()
{
	type='\0';
}

char command::gettype() const
{
	return type;
}

command::~command()
{
	//nothing for destructor
}

//-------------------------------------commandexec---------------------------------------------------

commandexec::commandexec(char* str)
{
	type='n';
	argc=0;
	argv=nullptr;
	fName=nullptr;
    parse(str);
}

void commandexec::parse(char* cmd)
{
	int x=0;
	int z=0;
	int y=0;
	int lastSlashindex=-1;
	fName=new char[50];

	for(int x=0;x<strlen(cmd);x++)
	{
		if(cmd[x]==' ' && (cmd[x+1]!=' ' && cmd[x+1] != '\0'))
		{
			argc++;
		}
	}
    argc+=2;

	argv=new char*[argc];

	for (int x=0;x<argc;x++)
	{
		argv[x]=nullptr;
	}

	argv[0]=new char[20];

	while(cmd[x]!=' ' && cmd[x]!='\0')
	{
		if(cmd[x]=='/')
		{
			lastSlashindex=x;
		}
		fName[x]=cmd[x];
		x++;
	}
	fName[x]='\0';

	for (y=lastSlashindex+1;y<x;y++)
	{
		argv[0][y-lastSlashindex-1]=cmd[y];
	}

	while(cmd[x]!='\0')
	{
		if(cmd[x]==' ')
		{
			while(cmd[++x]==' ');
			argv[z][y]='\0';
			z++;
			y=0;
			if(cmd[x]!='\0')
			{
				argv[z]=new char[50];
			}
		}
		else
		{
			argv[z][y]=cmd[x];
			y++;
			x++;
			if(cmd[x]=='\0')
			{
				argv[z][y]='\0';
			}
		}
	}
}

const char* commandexec::getfName() const
{
	return fName;
}

int commandexec::getargc() const
{
	return argc;
}

char*const* commandexec::getargv() const
{
	return argv;
}

commandexec::~commandexec()
{
    if(fName!=nullptr)
    {
        delete[] fName;
    }

    if(argv!=nullptr)
    {
        for (int x=0;x<argc-1;x++)
        {
            delete[] argv[x];
        }
        delete[] argv;
    }
}

//-----------------------------------------------commandrd--------------------------------------------

commandrd::commandrd(command* c, char* fn,char m)
{
	type=m;
	cmnd=c;
	filename=new char[strlen(fn)+1];
	strcpy(filename,fn);
	if(m=='<')
	{
		fd=open(filename,O_RDONLY);
	}
	else if(m=='>')
	{
		fd=open(filename,O_WRONLY | O_CREAT,S_IRWXU);
	}
}

const command* commandrd::getcmnd()
{
	return cmnd;
}

const char* commandrd::getfilename()
{
	return filename;
}

int commandrd::getfd()
{
	return fd;
}

commandrd::~commandrd()
{
	if (cmnd!=nullptr)
	{
		delete cmnd;
	}
	if(filename!=nullptr)
	{
		delete[] filename;
	}
	close(fd);
}

//-----------------------------------------------commandpipe---------------------------------------------

commandpipe::commandpipe(command* l,command* r)
{
	type='|';
	leftside=l;
	rightside=r;
}

const command* commandpipe::getleftside()
{
	return leftside;
}

const command* commandpipe::getrightside()
{
	return rightside;
}

commandpipe::~commandpipe()
{
	if(leftside!=nullptr)
	{
		delete leftside;
	}
	if(rightside!=nullptr)
	{
		delete rightside;
	}
}