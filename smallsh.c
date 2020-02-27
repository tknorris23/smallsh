#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

////////////////////////////////////////////////////////////////
//
//  Thomas Kelly Norris
//  CS344 Operating Systems Winter 2020 Oregon State University
//  February 26th, 2020
//  Program 3 - smallsh
//  This program is a very limited scope shell for UNIX.
//
/////////////////////////////////////////////////////////////////

//"exit" built-in command function
void shexit()
{
    printf("exit called!\n");
    exit(0);
}


//"cd" built-in command function
void shcd(char** cwd, char* input, const char* PATH);

//"status" built-in command function
void shstatus(int exit, int sig, int exitORsig);

main()
{
    //set working directory
    const char *PATH = getenv("HOME");
    char* cwd;
    cwd = (char *)malloc(50);
    char* buf;
    buf = (char *)malloc(50);
    size_t size;
    size = 1024;
    getcwd(cwd, size);

    //looping vars
    int end = 0;
    int i;
    int j;
    char *input;
    input = (char *)malloc(2048);
    char *command[518];
    char *command2[518];
    int argc1;
    int argc2;
    argc1 = 0;
    argc2 = 0;
    char *p;
    p = (char *)malloc(2048);
    int bgPID[50];
    int bgnum = 0;
    int amper_index = -1;

    //pseudo bools for redirection info
    int passin, passout, bac;
    passin = 0;
    passout = 0;
    bac = 0;
    //int for keeping track of redirection
    int index_in = 0;
    int index_out = 0;
    int saved_stdin, saved_stdout;

    //ints for checking exit status
    int exitStatus = 0;
    int termSignal = 0;
    int exitORsig = 0; // 0 = exit, 1 = signal

    //exec() vars
    pid_t spawnPid = -5;
    int childExitMethod = -5;
    //while loop for shell prompt
    while(end == 0)
    {
        //make sure command arrays are empty with NULLS
        for(i = 0; i < 518; i++)
        {
            command[i] = NULL;
            command2[i] = NULL;
        }
        //check for terminated children
        int tempPID;
        tempPID = waitpid(-1, &childExitMethod, WNOHANG);
        if (tempPID != 0 && tempPID != -1)
        {
            printf("Child process returned with PID: [%d]\n", tempPID);
            if (WIFEXITED(childExitMethod) != 0)
            {
                printf("Child process exited with value of [%d]\n", WEXITSTATUS(childExitMethod));
            }
            else if (WIFSIGNALED(childExitMethod) != 0)
            {
                printf("Child process killed by signal [%d]\n", WTERMSIG(childExitMethod));
            }
        }



        // THE PROMPT //
        //flush stdout before printing prompt
        fflush(stdout);

        //prints colon then gets input from stdin
        printf(":");
        
        memset(input, 0, sizeof(input));
        fgets(input, 2048, stdin);
        if(strcmp(input, "\n") != 0)
        {
            input[strcspn(input, "\n")] = 0; //removes \n from end of input
        }
        
        //Tokenizes input by spaces and puts into command array
        memset(p, 0, sizeof(p));
        strcpy(p, input);
        char *tok = NULL;
        i = 0;
        j = 0;
        for (tok = strtok (p, " "); tok; tok = strtok(NULL, " \n"))
        {
            //set command[i] to value of token
            char *output_tok = strdup(tok);
            command[i] = (char *)malloc(sizeof(output_tok));
            strcpy(command[i], output_tok);
            argc1++;
            //set value of command2[i] to value of token, given it's not a redirection character
            if (strcmp(output_tok, "<") != 0 && strcmp(output_tok, ">") != 0 && strcmp(output_tok, "&") != 0)
            {
                command2[j] = (char *)malloc(sizeof(output_tok));
                strcpy(command2[j], output_tok);
                argc2++;
            }
            //if token is a redirect chacter, decrement j so that there's no empty array indexes
            else
            {
                j--;
            }
            //check for redirects
            //in redir
            if(strcmp(output_tok, "<") == 0)
            {
                passin = 1;
                index_in = i;
                //file to get data from will be index_in + 1.
            }
            //out redir
            if(strcmp(output_tok, ">") == 0)
            {
                passout = 1;
                index_out = i;
            }
            //check for ampersand command
            if(strcmp(output_tok, "&") == 0)
            {
                amper_index = i;
            }
            //check for $$ to expand into pid
            if(strcmp(output_tok, "$$") == 0)
            {
                //get PID of shell
                pid_t shell;
                shell = getpid();
                //set up vars to change int to string
                int len = snprintf(NULL, 0, "%d", shell);
                char* str = malloc(len + 1);
                //get string of pid value
                snprintf(str, len+1, "%d", shell);
                //Switch command[i] and command2[j] with the PID
                memset(command[i], '\0', sizeof(command[i]));
                memset(command2[j], '\0', sizeof(command2[j]));
                command[i] = (char *)malloc(sizeof(str));
                command2[j] = (char *)malloc(sizeof(str));
                memset(command[i], '\0', sizeof(str));
                strcpy(command[i], str);
                memset(command2[j], '\0', sizeof(str));
                strcpy(command2[j], str);
            }
            //free token var
            free(output_tok);
            //increment looping vars
            i++;
            j++;
        }
        //check if a caught ampersand was the last command
        if (amper_index == i - 1)
        {
            bac = 1;
        }
        //reset vars
        i = 0;
        j = 0;
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
            //check if it's a HOME cd or one with a path
            char* pth;
            //if an input is used, set pth to be sent to shcd
            if (command[1] != NULL)
            {
                pth = (char *) malloc(sizeof(command[1]));
                strcpy(pth, command[1]);
            }
            //if it's a HOME call, set pth to "-1"
            else
            {
                pth = (char *)malloc(3);
                strcpy(pth, "-1");
            }
            //call shcd
            shcd(&cwd, pth, PATH);
            //reset pth
            memset(pth, 0, sizeof(pth));
        }
        //check for status
        else if (strcmp(command[0], "status") == 0)
        {
            shstatus(exitStatus, termSignal, exitORsig);
        }
        //check for nothing
        else if (command[0] == NULL)
        {
            //do nothing
        }
        //check for comments
        else if (command[0][0] == '#')
        {
            // do nothing
        }
        else if (command[0][0] == '\n')
        {
            //do nothing
        }

        //exec() commands
        else
        {
            //check for redirects
            if(passin)
            {
                //make vars for function use
                int fdin;
                int result;
                //save stdin's file descriptor for later
                saved_stdin = dup(0);
                //open file from given path for read only
                fdin = open(command[index_in+1], O_RDONLY);
                //set stdin to be from file
                result = dup2(fdin, 0);
                close(fdin);
                result = 0;
            }
            if(passout)
            {
                int fdout;
                int result;
                //save stdout's file descriptor for later
                saved_stdout = dup(1);
                //open file from given path for write only (will wipe file if it has data)
                fdout = open(command[index_out+1], O_WRONLY);
                //set stdout to send to file
                result = dup2(fdout, 1);
                close(fdout);
                result = 0;
            }


            //fork process
            if (bac)
            {
                //redirect stdin and stdout to /dev/null if not already set
                if(!passin)
                {
                    int fdin;
                    int result;
                    //save stdin's file descriptor for later
                    saved_stdin = dup(0);
                    //open devnull
                    fdin = open("/dev/null", O_RDONLY);
                    //set stdin to send to devnull
                    result = dup2(fdin, 0);
                    close(fdin);
                    result = 0;
                }
                if(!passout)
                {
                    int fdout;
                    int result;
                    //save stdout's file descriptor for later
                    saved_stdout = dup(1);
                    //open devnull
                    fdout = open("/dev/null", O_WRONLY);
                    //set stdout to send to devnull
                    result = dup2(fdout, 1);
                    close(fdout);
                    result = 0;
                }
                spawnPid = fork();
                //grab PID and put in bg process array
                bgPID[bgnum] = spawnPid;
                bgnum++;
                switch (spawnPid)
                {
                    case -1: { perror("Fork failed!\n"); exit(1); break;} // error case
                    case 0: {
                        //child
                        //execute command
                        execvp(command2[0], command2);
                        perror("CHILD: exec failed.\n");
                        exit(2); break;
                    }
                    default: {
                        //parent
                        dup2(saved_stdout, 1);
                        printf("bg process spawned with PID of: [%d]\n", spawnPid);
                    }
                }   
            } // end of background exec() handling

            else // foreground exec() handling
            {
                spawnPid = fork();
                switch (spawnPid)
                {
                    case -1: { perror("Fork failed!\n"); exit(1); break;} // error case
                    case 0: {
                        //child
                        //execute command
                        execvp(command2[0], command2);
                        perror("CHILD: exec failed.\n");
                        exit(2); break;
                    }
                    default: {
                        //parent
                        //wait for child to exit
                        pid_t actualPid = waitpid(spawnPid, &childExitMethod, 0);
                        //check exit method for child, and set vars as appropriate
                        if (WIFEXITED(childExitMethod) != 0)
                        {
                            exitStatus = WEXITSTATUS(childExitMethod);
                            if (exitStatus == 2)
                            {
                                exitStatus = 1;
                            }
                            exitORsig = 0;
                        }
                        else if (WIFSIGNALED(childExitMethod) != 0)
                        {
                            termSignal = WTERMSIG(childExitMethod);
                            exitORsig = 1;
                        }
                    }
                }
            }
        } // end of exec() block


        // CLEANUP //
        //reset file descriptors
        if(passin)
        {
            int err;
            err = dup2(saved_stdin, 0);
            close(saved_stdin);
            if (err == -1)
            {
                printf("Could not open file for stdin redirection.\n");
                exitStatus = 1;
            }
        }
        if(passout)
        {
            int err;
            err = dup2(saved_stdout, 1);
            close(saved_stdout);
            if (err == -1)
            {
                printf("Could not open file for stdout redirection.\n");
                exitStatus = 1;
            }
        }
        if(bac && !passin)
        {
            int err;
            err = dup2(saved_stdin, 0);
            close(saved_stdin);
            if (err == -1)
            {
                printf("Could not save stdin!\n");
                exitStatus = 1;
            }
        }
        if(bac && !passout)
        {
            int err;
            err = dup2(saved_stdout, 1);
            close(saved_stdout);
            if (err == -1)
            {
                printf("Could not save stdout!\n");
                exitStatus = 1;
            }
        }
        //reset command array
        for(i = 0; i < 518; i++)
        {
            if (command[i] != NULL)
            {
                free(command[i]);
            }
            if (command2[i] != NULL)
            {
                free(command2[i]);
            }
        }
        argc1 = 0;
        argc2 = 0;
        //reset other prompt vars
        passin = 0;
        passout = 0;
        bac = 0;
    }
}


//"cd" built-in command function
void shcd(char** cwd, char* input, const char* PATH)
{
    //var to grab cwd
    char buf[100];

    //If a path is being sent from input
    if (strcmp(input, "-1") != 0)
    {
        //Try to open directory
        //opendir() and chdir() support relative paths!
        DIR *abs;
        abs = opendir(input);
        if (abs != NULL)
        {
            //change directory and set cwd
            chdir(input);
            getcwd(buf, sizeof(buf));
            *cwd = (char *)malloc(sizeof(buf));
            strcpy(*cwd, buf);
        }
        //if directory doesn't exist, throw error
        else if (ENOENT == errno)
        {
            printf("path not found.\n");
        }
        //close directory stream
        closedir(abs);
    }
    //If there was no input:
    else if (strcmp(input, "-1") == 0)
    {
        //change cwd to PATH and set chdir()
        *cwd = (char *)malloc(sizeof(PATH));
        memset(*cwd, '\0', sizeof(PATH));
        strcpy(*cwd, PATH);
        chdir(*cwd);
    }
}

void shstatus(int exit, int sig, int exitORsig) 
{
    //exitORsig = 0 if child exited normally
    if(exitORsig)
    {
        printf("terminating signal: [%d]\n", sig);
    }
    //exitORsig = 1 if child exited from a signal
    else if (!exitORsig)
    {
        printf("exit status: [%d]\n", exit);
    }
}