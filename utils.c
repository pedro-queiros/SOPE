#include "utils.h"

int numTimesAppears(char* string, char ch)
{
    int i;
    int count = 0;
    for(i = 0; string[i] != '\0' ; ++i)
    {
        if (string[i] == ch)
        {
            count++;
        }
    }
    return count;
}

int getSubString(char *source, char *target,int from, int to)
{
    int length=0;
    int i=0,j=0;

    //get length
    while(source[i++]!='\0')
        length++;

    if(from<0 || from>length){
        printf("Invalid \'from\' index\n");
        return 1;
    }
    if(to>length){
        printf("Invalid \'to\' index\n");
        return 1;
    }

    for(i=from,j=0;i<=to;i++,j++){
        target[j]=source[i];
    }

    //assign NULL at the end of string
    target[j]='\0';

    return 0;
}

int isNumeric (const char * s)
{
    if (s == NULL || *s == '\0' || isspace(*s))
        return 0;
    char * p;
    strtod (s, &p);
    return *p == '\0';
}
