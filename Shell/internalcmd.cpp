#include "shell_func.h"

// checks if the command matches one of the internal commands
// and calls the appropriate function
// does not include cd and quit, which are called directly by executecmd()
bool internalcmd(struct command *cmd)
{
    const char *intcmds[] = {"clr", "dir", "environ", "echo", "help", "pause"};

    if (strcmp(cmd->name, "clr") == 0)
    {
        f_clr();
        return true;
    }
    if (strcmp(cmd->name, "dir") == 0)
    {
        f_dir(cmd->args[1]);
        return true;
    }
    if (strcmp(cmd->name, "environ") == 0)
    {
        f_environ();
        return true;
    }
    if (strcmp(cmd->name, "echo") == 0)
    {
        f_echo(cmd->args);
        return true;
    }
    if (strcmp(cmd->name, "help") == 0)
    {
        f_help();
        return true;
    }
    if (strcmp(cmd->name, "pause") == 0)
    {
        f_pause();
        return true;
    }

    return false;
}

//// INTERNAL COMMAND FUNCTIONS ////

// changes the current working directory
void f_cd(char *directory)
{
    if (directory == NULL)
    {
        std::cerr << "Please specify a directory to change to." << std::endl;
        return;
    }
    if (chdir(directory) == 0)
    {
        return;
    }
    std::cerr << "Could not find directory: " << directory << std::endl;
}

// clears the terminal
void f_clr()
{
    printf("\033[2J\r");
}

// print contents of current directory
void f_dir(char *directory)
{
    // create directory pointer
    DIR *d;

    // if no arguments given, set directory name to current directory
    if (directory == NULL)
        directory = get_current_dir_name();

    // initialize directory pointer
    d = opendir(directory);

    // error checking
    if (d == NULL)
    {
        std::cerr << "Directory not found." << std::endl;
        return;
    }

    // print every directory name to a new line
    dirent *entry = readdir(d);
    while (entry != NULL)
    {
        std::cout << entry->d_name << std::endl;
        entry = readdir(d);
    }

    // clean up
    closedir(d);
}

// print out some environmental variables
void f_environ()
{
    std::cout << "User: " << getenv("USER") << std::endl
              << "Path: " << getenv("PATH") << std::endl
              << "Shell: " << getenv("SHELL") << std::endl;
}

// print the arguments back to the user
// on the next line of the terminal
void f_echo(char **line)
{
    for (int i = 1; line[i] != NULL; i++)
    {
        std::cout << line[i] << " ";
    }
    std::cout << std::endl;
}

// open the readme
void f_help()
{
    // open the readme as a FILE pointer
    FILE *readme = fopen("readme", "r");

    // error checking
    if (!readme)
    {
        std::cerr << "Couldn't find file," << std::endl;
        return;
    }

    // allocate memory for lines read from the readme
    char *line = (char *)malloc(500 * sizeof(char));
    size_t buffersize = 500 * sizeof(char);

    // make it pretty
    std::cout << "~" << std::endl;
    // read each line of the readme and print it to the terminal
    while (!feof(readme))
    {
        std::cout << fgets(line, buffersize, readme);
    }
    // make it pretty
    std::cout << "\n~" << std::endl;
}

// pauses execution until you hit enter
void f_pause()
{
    int s;
    while (s != '\n')
    {
        s = getchar();
    }
    return;
}