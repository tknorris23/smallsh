#include <stdio.h>
#include <string.h>

void shexit()
{
    printf("exit called!\n");
}

void shcd()
{
    printf("cd called!\n");
}

void shstatus()
{
    printf("status called!\n");
}

main()
{
    int end = 0;
    //while loop for the prompt
    while(end == 0)
    {
        //THE PROMPT
        //prints colon then gets input from stdin
        printf(":");
        char input[2048];
        memset(input, 0, sizeof(input));
        fgets(input, 2048, stdin);
        //BUILT-IN COMMANDS
        //check for exit
        //I don't know why when input is "exit", strcmp returns 10
        //probably a problem to be solved later!
        if (strcmp(input, "exit") == 10)
        {
            shexit();
        }
        //check for cd
        if (strcmp(input, "cd") == 10)
        {
            shcd();
        }
        //check for status
        if (strcmp(input, "status") == 10)
        {
            shstatus();
        }
    }
}