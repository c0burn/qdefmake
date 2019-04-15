/*============================================================
qdefmake
  a utility to read QuakeC source files and output a .DEF file,
  which is then usable in map editors such as TrenchBroom
  created by Michael Coburn - michael.s.coburn@gmail.com
  ============================================================*/

#define VERSION "1.1"
#define _CRT_SECURE_NO_WARNINGS // disable warnings in MSVC
#define STRING_MAX 1024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

FILE *inputfile;
FILE *outputfile;
char path[STRING_MAX];
char progs[STRING_MAX]; 
char output[STRING_MAX];
unsigned int count;
bool verbose;

/*==========
replace_slashes
==========*/
void replace_slashes(char *s)
{
	for (unsigned int i = 0; i < strlen(s); i++)
	{
		if (s[i] == '\\')
			s[i] = '/';
	}
}

/*==========
strip_comments
==========*/
void strip_comments(char *s)
{
	char *p = strstr(s, "//");
	if (p == NULL)
		return;
	*p = '\0';
}

/*==========
strip_directives
==========*/
void strip_directives(char *s)
{
	char *p = strstr(s, "#");
	if (p == NULL)
		return;
	*p = '\0';

}
/*==========
strip_whitespace
==========*/
void strip_whitespace(char *s)
{
	for (unsigned int i = 0; i < strlen(s); i++)
	{
		if (isspace(s[i]))
		{
			s[i] = '\0';
			return;
		}
	}
}

/*==========
open_files
==========*/
void open_files(void)
{
	// open the progs.src for reading
	if (strlen(path) > 0)
	{
		char temp[STRING_MAX];
		snprintf(temp, STRING_MAX, "%s/%s", path, progs);
		printf("Reading from %s\n", temp);
		inputfile = fopen(temp, "r");
	}
	else
	{
		printf("Reading from %s\n", progs);
		inputfile = fopen(progs, "r");
	}
	if (inputfile == NULL)
	{
		perror("Unable to open progs.src");
		exit(EXIT_FAILURE);
	}

	// open the output.def for writing
	printf("Writing to %s\n", output);
	outputfile = fopen(output, "w");
	if (outputfile == NULL)
	{
		perror("Unable to save output.def");
		exit(EXIT_FAILURE);
	}
}

/*==========
close_files
==========*/
void close_files(void)
{
	fclose(inputfile);
	fclose(outputfile);
}

/*==========
parse_qc_file
==========*/
void parse_qc_file(const char filename[STRING_MAX])
{
	FILE *qcfile;

	// open the .qc file
	if (strlen(path))
	{
		char fullpath[STRING_MAX];
		snprintf(fullpath, STRING_MAX, "%s/%s", path, filename);
		qcfile = fopen(fullpath, "r");
	}
	else
		qcfile = fopen(filename, "r");
	if (qcfile == NULL)
	{
		perror("Error opening .qc file");
		exit(EXIT_FAILURE);
	}

	// read the .qc file line by line
	char buf[STRING_MAX];
	char *s;
	bool found = false;
	while (!feof(qcfile))
	{
		s = fgets(buf, STRING_MAX, qcfile);
		if (s == NULL || ferror(qcfile))
			break;
		
		// look for /*QUAKED*/ entries
		if (!found)
		{
			s = strstr(buf, "/*QUAKED");
			if (s)
			{
				found = true;
				if (verbose)
					printf("%s", buf);
				fputs(buf, outputfile);
				count++;
			}
		}
		// currently within a /*QUAKED*/ entry
		else
		{
			s = strstr(buf, "*/");
			if (s)
				found = false;
			if (verbose)
				printf("%s", buf);
			fputs(buf, outputfile);
		}
	}
	fclose(qcfile);
}

/*==========
parse_progs_src
==========*/
void parse_progs_src(void)
{
	// read the progs.src line by line and parse all of the .qc files listed within
	char buf[STRING_MAX];
	bool first = true;
	while (!feof(inputfile))
	{
		char *s = fgets(buf, STRING_MAX, inputfile);
		if (s == NULL || ferror(inputfile))
			break;

		// strip any crap out
		strip_comments(buf); 
		strip_directives(buf);
		strip_whitespace(buf);
		if (strlen(buf) == 0)	// ignore blank lines
			continue;
		replace_slashes(buf);

		// always ignore the first entry in the progs.src,
		// as it's the path to the progs.dat not a path to a .qc file
		if (!first)
		{
			if (verbose)
				printf("%s\n", buf);
			parse_qc_file(buf);
		}
		else
			first = false;
	}
	printf("%u QUAKED definitions found\n", count);
}

/*==========
display_help
==========*/
void display_help(void)
{
	fprintf(stderr, "usage: qdefmake -path <> -progs <> -output <> -verbose\n\n");
	fprintf(stderr, "all parameters are optional\n");
	fprintf(stderr, "-path is the path to your source files, default is the current working directory\n");
	fprintf(stderr, "-progs can specify an alternative name for your progs.src, default is progs.src\n");
	fprintf(stderr, "-output is the name of the .def file to write, default is output.def in the current working directory\n");
	fprintf(stderr, "-verbose prints more detailed information\n");
	fprintf(stderr, "example: qdefmake -path c:\\quake\\mod\\source -progs mod.src -output c:\\quake\\mod\\mod.def\n");
	system("pause");
}

/*==========
main
==========*/
int main(int argc, char *argv[])
{
	printf("qdefmake %s - compiled on %s %s\n", VERSION, __DATE__, __TIME__);
	printf("created by Michael Coburn, michael.s.coburn@gmail.com\n");

	// check for any command line arguments
	if (argc > 1)
	{
		argc = argc - 1;
		int arg = 1;
		while (arg <= argc)
		{
			// ?
			if (strchr(argv[arg], '?'))
			{
				display_help();
				exit(EXIT_SUCCESS);
			}
			// -verbose
			if (strcmp(argv[arg], "-verbose") == 0)
			{
				verbose = true;
			}
			// -path
			if (strcmp(argv[arg], "-path") == 0)
			{
				// check -path is followed by another arg
				if (arg + 1 > argc)
					display_help();
				else
				{
					snprintf(path, STRING_MAX, "%s", argv[arg + 1]);
					replace_slashes(path);
				}
			}
			// -progs
			if (strcmp(argv[arg], "-progs") == 0)
			{
				// check -progs is followed by another arg
				if (arg + 1 > argc)
					display_help();
				else
					snprintf(progs, STRING_MAX, "%s", argv[arg + 1]);
			}
			// -output
			if (strcmp(argv[arg], "-output") == 0)
			{
				// check -output is followed by another arg
				if (arg + 1 > argc)
					display_help();
				else
				{
					snprintf(output, STRING_MAX, "%s", argv[arg + 1]);
					replace_slashes(output);
				}
			}
			arg = arg + 1;
		}
	}

	// default values if they're not specified on the command line
	if (strlen(progs) == 0)
		snprintf(progs, STRING_MAX, "%s", "progs.src");
	if (strlen(output) == 0)
		snprintf(output, STRING_MAX, "%s", "output.def");

	open_files();
	parse_progs_src();
	close_files();

	return EXIT_SUCCESS;
}
