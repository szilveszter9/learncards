#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>
//#include <regex.h>

char* curtime()

{
    time_t current_time;
    char* c_time_string;

    /* Obtain current time as seconds elapsed since the Epoch. */
    current_time = time(NULL);

    if (current_time == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to compute the current time.\n");
        return EXIT_FAILURE;
    }

    /* Convert to local time format. */
    c_time_string = ctime(&current_time);

    if (c_time_string == NULL)
    {
        (void) fprintf(stderr, "Failure to convert the current time.\n");
        return EXIT_FAILURE;
    }

    /* Print to stdout. ctime() has already added a terminating newline character. */
    //(void) printf("Current time is %s", c_time_string);
    //return EXIT_SUCCESS;
    return c_time_string;
}

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <errno.h>
//#include <sys/types.h>
//#include <regex.h>
//
//#define tofind    "[a-z A-Z] $"
//
//int main(int argc, char **argv)
//{
//    FILE *fp;
//    char line[1024];
//    int retval = 0;
//    regex_t re;
//    //this file has this line "DAEMONS=(sysklogd network sshd !netfs !crond)"
//    const char *filename = "/etc/rc.conf";
//
//    if (argc > 1)
//        filename = argv[1];
//
//    if (regcomp(&re, tofind, REG_EXTENDED) != 0)
//    {
//        fprintf(stderr, "Failed to compile regex '%s'\n", tofind);
//        return EXIT_FAILURE;
//    }
//
//    fp = fopen(filename, "r");
//    if (fp == 0)
//    {
//        fprintf(stderr, "Failed to open file %s (%d: %s)\n",
//                filename, errno, strerror(errno));
//        return EXIT_FAILURE;
//    }
//
//    while ((fgets(line, 1024, fp)) != NULL)
//    {
//        line[strlen(line)-1] = '\0';
//        if ((retval = regexec(&re, line, 0, NULL, 0)) == 0)
//            printf("<<%s>>\n", line);
//    }
//    return EXIT_SUCCESS;
//}

int main(void)

{
    int lines_allocated = 128;
    int max_line_len = 100;

    /* Allocate lines of text */
    char **words = (char **)malloc(sizeof(char*)*lines_allocated);
    if (words==NULL)
    {
        fprintf(stderr,"Out of memory (1).\n");
        exit(1);
    }

    FILE *fp = fopen("ankidb.txt", "r");
    if (fp == NULL)
    {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }

    int i;
    for (i=0;1;i++)
    {
        int j;

        /* Have we gone over our line allocation? */
        if (i >= lines_allocated)
        {
            int new_size;

            /* Double our allocation and re-allocate */
            new_size = lines_allocated*2;
            words = (char **)realloc(words,sizeof(char*)*new_size);
            if (words==NULL)
            {
                fprintf(stderr,"Out of memory.\n");
                exit(3);
            }
            lines_allocated = new_size;
        }
        /* Allocate space for the next line */
        words[i] = malloc(max_line_len);
        if (words[i]==NULL)
        {
            fprintf(stderr,"Out of memory (3).\n");
            exit(4);
        }
        if (fgets(words[i],max_line_len-1,fp)==NULL)
            break;

        /* Get rid of CR or LF at end of line */
        for (j=strlen(words[i])-1;j>=0 && (words[i][j]=='\n' || words[i][j]=='\r');j--)
            ;
        words[i][j+1]='\0';
    }
    /* Close file */
    fclose(fp);

    int j;
    //for(j = 0; j < i; j++)
    for(j = 0; j < 10; j++)
        printf("%s\n", words[j]);

    char *curr_time = curtime();
    printf("%s\n", curr_time);

    struct tm time1 = {0};

    char timestr[] = "2015-08-13 01:10:52 0000 BST";
    char strformat[] = "%Y-%m-%d %H:%M:%S %z %Z";

    strptime(timestr, strformat, &time1);
    //puts(timestr);
    //strptime(timestr, strformat, &time1);
    //mktime(&time1);
    //strftime(timestr, sizeof(timestr), strformat, &time1);
    //puts(timestr);

    //double elapsed = difftime(&time1, time(NULL));
    //double elapsed1 = difftime(0, time(NULL));
    printf("%zu\n", time(NULL) - mktime(&time1));
    //printf("%i\n", );
    //printf("%d\n", elapsed);
    //printf("%d\n", elapsed1);

    /* Good practice to free memory */
    for (;i>=0;i--)
        free(words[i]);
    free(words);
    return 0;
}
