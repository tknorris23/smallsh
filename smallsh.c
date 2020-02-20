#include <stdio.h>
#include <string.h>


main()
{
    int end = 0;
    //while loop for the prompt
    while(end == 0)
    {
        if(end == 0)
        {
            //THE PROMPT
            //prints colon then gets input from stdin
            printf(":");
            char input[2048];
            memset(input, 0, sizeof(input));
            fgets(input, 500, stdin);
        }
    }
}