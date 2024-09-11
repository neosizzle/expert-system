#include <stdio.h>

void print_banner()
{
	char *banner = "                            _     \n\
                           | |    \n\
   _____  ___ __   ___ _ __| |_   \n\
  / _ \\ \\/ / '_ \\ / _ \\ '__| __|  \n\
 |  __/>  <| |_) |  __/ |  | |_   \n\
  \\___/_/\\_\\ .__/ \\___|_|   \\__|  \n\
           | | | |                \n\
  ___ _   _|_|_| |_ ___ _ __ ___  \n\
 / __| | | / __| __/ _ \\ '_ ` _ \\ \n\
 \\__ \\ |_| \\__ \\ ||  __/ | | | | |\n\
 |___/\\__, |___/\\__\\___|_| |_| |_|\n\
       __/ |                      \n\
      |___/                       \n";

	printf("%s", banner);
}

void print_help()
{
	printf("\nexpertsystem - Simple inference engine\n\n\
exit	Exits the program\n\
run	Executes the engine with configured facts and query\n\
query	Changes the query\n\
fact	Changes the facts\n\
ls	Prints rules, current query and facts\n\
clear	Clears the terminal\n\
help	Prints this message\n\n\
");
}