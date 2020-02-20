#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//"exit" built-in command function
void shexit()
{
    printf("exit called!\n");
    exit(0);
    // TODO: Kill all child processes before exiting!
}


//"cd" built-in command function
void shcd()
{
    printf("cd called!\n");
}

//"status" built-in command function
void shstatus()
{
    printf("status called!\n");
}

main()
{
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
        //check for exit!
        if (strcmp(command[0], "exit") == 0)
        {
            shexit();
        }
        //check for cd
        if (strcmp(input, "cd") == 0)
        {
            shcd();
        }
        //check for status
        if (strcmp(input, "status") == 0)
        {
            shstatus();
        }
    }
}