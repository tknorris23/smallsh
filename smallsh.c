#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

//"exit" built-in command function
void shexit()
{
    printf("exit called!\n");
    exit(0);
    // TODO: Kill all child processes before exiting!
}






//"cd" built-in command function
void shcd(char** cwd, char* input, const char* PATH);

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
    cwd = (char *)malloc(50);
    char* buf;
    buf = (char *)malloc(50);
    size_t size;
    size = 1024;
    getcwd(cwd, size);
    printf("cwd gotten is: %s\n", cwd);


    int end = 0;
    int i;
    char input[2048];
    char *command[518];
    char *command2[518];
    int argc1;
    int argc2;
    argc1 = 0;
    argc2 = 0;
    char *p;

    //exec() vars
    pid_t spawnPid = -5;
    int childExitStatus = -5;
    //while loop for shell prompt
    while(end == 0)
    {
        for(i = 0; i < 518; i++)
        {
            command[i] = NULL;
            //memset(command[i], '\0', sizeof(command[i]));
            command2[i] = NULL;
            //memset(command2[i], '\0', sizeof(command2[i]));
        }
        //THE PROMPT
        //flush stdout before printing prompt
        fflush(stdout);


        //prints colon then gets input from stdin
        printf(":");
        
        memset(input, 0, sizeof(input));
        fgets(input, 2048, stdin);
        input[strcspn(input, "\n")] = 0; //removes \n from end of input
        char *p = strchr(input, '\n');
        if (p) *p = 0;


        //Tokenizes input by spaces and puts into command array
        
        p = (char *) malloc(2048);
        strcpy(p, input);
        char *tok = NULL;
        i = 0;
        for (tok = strtok (p, " "); tok; tok = strtok(NULL, " \n"))
        {
            char *output_tok = strdup(tok);
            command[i] = (char *)malloc(sizeof(output_tok));
            strcpy(command[i], output_tok);
            argc1++;
            if (strcmp(output_tok, "<") != 0 && strcmp(output_tok, ">") != 0 && strcmp(output_tok, "&") != 0)
            {
                command2[i] = (char *)malloc(sizeof(output_tok));
                strcpy(command2[i], output_tok);
                argc2++;
            }
            free(output_tok);
            i++;
        }
        i = 0;
        command[argc1] = NULL;
        command2[argc2] = NULL;

        //BUILT-IN COMMANDS
        //check for exit
        if (strcmp(command[0], "exit") == 0)
        {
            shexit();
        }
        //check for cd
        else if (strcmp(command[0], "cd") == 0)
        {
            char* pth;
            pth = (char *) malloc(sizeof(command[1]));
            strcpy(pth, command[1]);
            shcd(&cwd, pth, PATH);
            strcpy(pth, "");
        }
        //check for status
        else if (strcmp(command[0], "status") == 0)
        {
            
            shstatus();
        }
        //check for nothing
        else if (strcmp(command[0], "") == 0)
        {
            //do nothing
        }
        //check for comments
        else if (command[0][0] == '#')
        {
            // do nothing
        }
        //exec() commands
        else
        {
            printf("command2[0] is: %s\n", command2[0]);
            spawnPid = fork();
            switch (spawnPid)
            {
                case -1: { perror("Fork failed!\n"); exit(1); break;}
                case 0: {
                    //child
                    printf("[%s]\n", cwd);
                    printf("Child process running...\n");
                    execvp(command2[0], command2);
                    printf("exec failed: %s\n", strerror(errno));
                    perror("CHILD: exec failed.\n");
                    exit(2); break;
                }
                default: {
                    //parent
                    printf("child(%d) spawned, waiting...\n", spawnPid);
                    pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
                    printf("Done, child(%d) terminated.\n", actualPid);
                }
            }
        }

        printf("resetting command arrays\n");
        //reset command array
        for(i = 0; i < 518; i++)
        {
            free(command[i]);
            argc1 = 0;
            free(command2[i]);
            argc2 = 0;
        }
        printf("done with commands\n");
    }
}


//"cd" built-in command function
void shcd(char** cwd, char* input, const char* PATH)
{
    printf("cd called!\n");
    printf("cwd is: '%s'\n", *cwd);
    printf("input is: '%s'\n", input);
    printf("PATH is: '%s'\n", PATH);

    //create relative path
    char pathname[1024];
    memset(pathname, '\0', sizeof(pathname));
    strcpy(pathname, *cwd);
    strcat(pathname, input);
    printf("relative pathname: '%s'\n", pathname);

    //test for absolute path
    DIR *abs;
    abs = opendir(input);
    if (abs)
    {
        printf("absolute path found.\n");
        *cwd = (char *)malloc(sizeof(input));
        memset(*cwd, '\0', sizeof(input));
        strcpy(*cwd, input);
        chdir(*cwd);
        printf("cwd changed to: '%s'\n", *cwd);
    }
    else if (ENOENT == errno)
    {
        printf("absolute path not found.\n");
    }
    closedir(abs);
    DIR *rel;
    rel = opendir(pathname);
    if (rel && strcmp(input, "") != 0)
    {
        printf("relative path found.\n");
        *cwd = (char *)malloc(sizeof(pathname));
        memset(*cwd, '\0', sizeof(pathname));
        strcpy(*cwd, pathname);
        chdir(*cwd);
        printf("cwd changed to: '%s'\n", *cwd);
    }
    else if (ENOENT == errno)
    {
        printf("relative path not found.\n");
    }

    if (strcmp(input, "") == 0)
    {
        printf("null call\n");
        *cwd = (char *)malloc(sizeof(PATH));
        memset(*cwd, '\0', sizeof(PATH));
        strcpy(*cwd, PATH);
        chdir(*cwd);
        printf("cwd changed to: '%s'\n", *cwd);
    }
    

}