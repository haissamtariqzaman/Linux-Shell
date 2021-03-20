#include<cstring>

class command
{
public:
	command();
	char gettype() const;
	virtual ~command();
protected:
	char type;
};

class commandexec:public command
{
public:
	commandexec(char*);
	void parse(char*);
	const char* getfName() const;
	int getargc() const;
	char*const* getargv() const;
	~commandexec();
private:
	char* fName;
	int argc;
	char** argv;
};

class commandrd: public command
{
public:
	commandrd(command*,char*,char);
	const command* getcmnd();
	const char* getfilename();
	const char* getfmode();
	int getfd();
	~commandrd();
private:
	command* cmnd;
	char* filename;
	int fd;
};

class commandpipe: public command
{
public:
	commandpipe(command*,command*);
	const command* getleftside();
	const command* getrightside();
	~commandpipe();
private:
	command* leftside;
	command* rightside;
};