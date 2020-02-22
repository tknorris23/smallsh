#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

//"exit" built-in command function
void shexit()
{
    printf("exit called!\n");
    exit(0);
    // TODO: Kill all child processes before exiting!
}


//"cd" built-in command function
void shcd(char** cwd, char* pathname, const char* PATH)
{
    //notes 2/21/2020
    //last thing to do hopefully is add support for multiple slashes for relative paths
    //1st idea is try to add it to cwd and check if it exists
    // if so go ahead and switch
    // actually probably a better idea than using readdir huh
    //fuck me lmao
    printf("cd called!\n");
    int check = 0;
    DIR *dirp;
    dirp = opendir(*cwd);
    //
    struct dirent *dp;
    char* dirname;
    //check if given pathname exists in cwd
    printf("about to enter loop\n");
    if (dirp)
    {
        while ((dp = readdir(dirp)) != NULL)
    {
        printf("loop entered\n");
        dirname = (char *)malloc(sizeof(dp->d_name));
        strcpy(dirname, dp->d_name);
        printf("name from directory: %s\n", dp->d_name);

        //check for named directory matches
        if (strcmp(pathname, dirname) == 0 && strcmp(".", pathname) != 0 && strcmp("..", pathname) != 0)
        {
            printf("relative path found\n");
            strcat(*cwd, "/");
            strcat(*cwd, pathname);
            check = 1;
        }
        strcpy(dirname, "");
    }
    closedir(dirp);
    }
    else if (ENOENT == errno)
    {
        printf("Warning: Previous working directory did not exist.\n");
    }
    
    
    if (strcmp("", pathname) == 0 && check == 0)
    {
        *cwd = (char *)malloc(sizeof(PATH));
        strcpy(*cwd, PATH);
    }
    else if (strcmp("..", pathname) == 0 && check == 0)
    {
        //something
    }
    else if (strcmp(".", pathname) != 0 && check == 0)
    {
        *cwd = (char *)malloc(sizeof(pathname));
        strcpy(*cwd, pathname);
    }
    printf("cwd is now: %s\n", *cwd);
    chdir(*cwd);
}

//"status" built-in command function
void shstatus()
{
    printf("status called!\n");
}

main()
{
    //set working directory
    const char *PATH = getenv("HOME");
    printf("PATH gotten is: %s\n", PATH);
    char* cwd;
    char* buf;
    size_t size;
    size = 1024;
    cwd = getcwd(buf, size);
    printf("cwd gotten is: %s\n", cwd);
    int end = 0;
    int i;
    //while loop for shell prompt
    while(end == 0)
    {
        //THE PROMPT
        //flush stdout before printing prompt
        fflush(stdout);


        //prints colon then gets input from stdin
        printf(":");
        char input[2048];
        memset(input, 0, sizeof(input));
        fgets(input, 2048, stdin);
        input[strcspn(input, "\n")] = 0; //removes \n from end of input


        //Tokenizes input by spaces and puts into command array
        char command[518][50];
        char *p;
        p = (char *) malloc(2048);
        strcpy(p, input);
        char *tok = NULL;
        i = 0;
        for (tok = strtok (p, " "); tok; tok = strtok(NULL, " \n"))
        {
            char *output_tok = strdup(tok);
            strcpy(command[i], output_tok);
            free(output_tok);
            i++;
        }
        i = 0;



        //BUILT-IN COMMANDS
        //check for exit
        if (strcmp(command[0], "exit") == 0)
        {
            shexit();
        }
        //check for cd
        if (strcmp(command[0], "cd") == 0)
        {
            char* pth;
            pth = (char *) malloc(sizeof(command[1]));
            strcpy(pth, command[1]);
            shcd(&cwd, pth, PATH);
        }
        //check for status
        if (strcmp(command[0], "status") == 0)
        {
            
            shstatus();
        }


        //reset command array
        for(i = 0; i < 518; i++)
        {
            strcpy(command[i], "");
        }
    }
}