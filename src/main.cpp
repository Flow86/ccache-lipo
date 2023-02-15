#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <iterator>
#include <sstream>

#include <unistd.h>
#include <sys/wait.h>

//#include "execute.h"

int main(int argc, char* argv[])
{
	std::string gcc = argv[0];

	/*std::stringstream s;
	for(int i = 0; i < argc; ++i)
		s << argv[i] << " ";

	execute(s.str().c_str());
	exit(1);*/

	size_t pos = gcc.find_last_of("/\\");
	if(pos != std::string::npos)
		gcc = gcc.substr(pos+1);

	//std::cout << "gcc: "  << gcc  << std::endl;

	std::string lipo;
	pos = gcc.find_last_of("-");
	if(pos == std::string::npos)
		lipo = "lipo";
	else
		lipo = gcc.substr(0, pos+1) + "lipo";

	//std::cout << "lipo: " << lipo << std::endl;

	bool compile = false;
	std::string file, tmpfile;
	std::vector<std::string> archs;
	std::vector<std::string> params;
	for(int i = 1; i < argc; ++i)
	{
		if(std::string(argv[i]) == "-arch")
		{
			++i;
			archs.push_back(argv[i]);
		}

		else
			params.push_back(argv[i]);

		if(std::string(argv[i]) == "-o")
			file = argv[i+1];

		if(std::string(argv[i]) == "-c")
			compile = true;
	}

	// WE DO NOT HAVE STDOUT SUPPORT (currently) and only support multi-arch compiles
	if(file == "" || !compile || archs.size() < 2)
	{
		// memory leak ...
		argv[0] = strdup(gcc.c_str());
		return execvp(argv[0], argv);
	}

	tmpfile = file;
	pos = tmpfile.find_last_of("/\\");
	if(pos != std::string::npos)
		tmpfile = tmpfile.substr(pos+1);

	tmpfile = "/tmp/" + tmpfile;

	std::stringstream mpid;
	mpid << getpid();

	for(std::vector<std::string>::const_iterator arch = archs.begin(); arch != archs.end(); ++arch)
	{
		std::stringstream cmd;
		cmd << "/usr/lib/ccache/" << gcc;

		char **argv = new char*[params.size() + 4];

		int i = 0;
		argv[i] = new char[cmd.str().size()+1];
		strcpy(argv[i], cmd.str().c_str());
		++i;

		bool last_o = false;
		for(std::vector<std::string>::const_iterator param = params.begin(); param != params.end(); ++param)
		{
			if(*param == "-c")
			{
				argv[i] = new char[6];
				strcpy(argv[i], "-arch");
				++i;
				argv[i] = new char[arch->size()+1];
				strcpy(argv[i], arch->c_str());
				++i;
			}

			std::string p = *param;

			if(last_o)
			{
				p = tmpfile + "." + *arch + "." + mpid.str();
				last_o = false;
			}

			if(*param == "-o")
				last_o = true;

			argv[i] = new char[p.size()+1];
			strcpy(argv[i], p.c_str());
			++i;
		}
		argv[i] = NULL;

		int status;
		pid_t pid = fork();
		if ( pid == 0 ) // child
		{
			int e = execvp(argv[0], argv);
			perror("execvp: ");
			return e;
		}
		else if ( pid < 0 )
			perror("fork: ");
		else // parent
			waitpid(pid, &status, WUNTRACED | WCONTINUED);

		for(int i = 0; i < params.size() + 4; ++i) {
			//std::cout << argv[i] << std::endl;
			delete[] argv[i];
		}

		delete[] argv;

		if(WEXITSTATUS(status) != 0)
			return WEXITSTATUS(status);
	}

	// run lipo to produce outputfile
	{
		char **argv = new char*[archs.size() + 5];

		int i = 0;
		argv[i] = new char[lipo.size()+1];
		strcpy(argv[i], lipo.c_str());
		++i;

		argv[i] = new char[8];
		strcpy(argv[i], "-create");
		++i;

		for(std::vector<std::string>::const_iterator arch = archs.begin(); arch != archs.end(); ++arch)
		{
			std::string f = tmpfile + "." + *arch + "." + mpid.str();
			argv[i] = new char[f.size()+1];
			strcpy(argv[i], f.c_str());
			++i;
		}

		argv[i] = new char[8];
		strcpy(argv[i], "-output");
		++i;

		argv[i] = new char[file.size()+1];
		strcpy(argv[i], file.c_str());
		++i;

		argv[i] = NULL;

		int status;
		pid_t pid = fork();
		if ( pid == 0 ) // child
		{
			int e = execvp(argv[0], argv);
			perror("execvp: ");
			return e;
		}
		else if ( pid < 0 )
			perror("fork: ");
		else // parent
			waitpid(pid, &status, WUNTRACED | WCONTINUED);

		// remove temporary
		for(std::vector<std::string>::const_iterator arch = archs.begin(); arch != archs.end(); ++arch)
		{
			std::string f = tmpfile + "." + *arch + "." + mpid.str();
			unlink(f.c_str());
		}

		for(int i = 0; i < archs.size() + 5; ++i) {
			//std::cout << argv[i] << std::endl;
			delete[] argv[i];
		}

		return WEXITSTATUS(status);
	}

	return 0;
}
