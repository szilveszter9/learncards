#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>
//#include <regex.h>

#include <termios.h>
#include <stdio.h>

static struct termios old, new;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  new = old; /* make new settings same as old settings */
  new.c_lflag &= ~ICANON; /* disable buffered i/o */
  new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo)
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch(void)
{
  return getch_(0);
}

/* Read 1 character with echo */
char getche(void)
{
  return getch_(1);
}

/* Let's test it out */
//int main(void) {
//  char c;
//  printf("(getche example) please type a letter: ");
//  c = getche();
//  printf("\nYou typed: %c\n", c);
//  printf("(getch example) please type a letter...");
//  c = getch();
//  printf("\nYou typed: %c\n", c);
//  return 0;
//}
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

struct lesson {
    char date[19];
    //char card_front[128];
    //char card_back[128];
    char *card_front;
    char *card_back;
    char *experience;
    int seen;
};

typedef int (*compfn)(const void*, const void*);

int verbose;
char nextchar;

int main(int argc, char *argv[])

{
    for (int i=0;i<argc;i++) {
        if(strcmp(argv[i], "-v")==0)
            verbose = 1;
        if(strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) {
            printf("\nanki clone in your console");
            printf("\n\nOptions");
            printf("\n -h, --help    show help");
            printf("\n -v            increase verbosity");
            printf("\n");
            return 0;
        }
    }
    int lines_allocated = 128;
    int max_line_len = 500000;

    /* Allocate lines of text */
    char **words = (char **)malloc(sizeof(char*)*lines_allocated);
    //struct lesson lessons[sizeof(char*)*lines_allocated];
    struct lesson* lessons = malloc(max_line_len * sizeof(struct lesson));;
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
    for (i=0;;i++)
    {
        int j;
        int column = 0;

        lessons[i].card_front = malloc(128);
        lessons[i].card_back = malloc(128);
        lessons[i].experience = malloc(32);

        /*
        //printf("%i %i %i\n",i,lines_allocated);
        // Have we gone over our line allocation?
        if (i >= lines_allocated)
        {
            int new_size;

            // Double our allocation and re-allocate
            new_size = lines_allocated*2;
            words = (char **)realloc(words,sizeof(char*)*new_size);
            if (words==NULL)
            {
                fprintf(stderr,"Out of memory.\n");
                exit(3);
            }
            lines_allocated = new_size;
        }
        // Allocate space for the next line
        words[i] = malloc(max_line_len);
        if (words[i]==NULL)
        {
            fprintf(stderr,"Out of memory (3).\n");
            exit(4);
        }
        if (fgets(words[i],max_line_len-1,fp)==NULL)
            break;

        // Get rid of CR or LF at end of line
        for (j=strlen(words[i])-1;j>=0 && (words[i][j]=='\n' || words[i][j]=='\r');j--)
            ;
        words[i][j+1]='\0';
        */
        for (j=0;;j++){
            nextchar=fgetc(fp);
            if(nextchar == '\n' || nextchar == '\r' || nextchar == EOF){
                break;
            }
            else if(nextchar == ';'){
                column++;
                j = -1;
            }
            else if(column==0) lessons[i].date[j] = nextchar;
            else if(column==1) lessons[i].experience[j] = nextchar;
            else if(column==2) lessons[i].card_front[j] = nextchar;
            else if(column==3) lessons[i].card_back[j] = nextchar;
        }
        if(nextchar==EOF) break;
    }
    /* Close file */
    fclose(fp);

    int j;
    for(j = 0; j < i; j++)
        printf("date: %s, experience: %i, front: %s ----- back: %s\n", lessons[j].date, atoi(lessons[j].experience), lessons[j].card_front, lessons[j].card_back);

    int compare(struct lesson *elem1, struct lesson *elem2)
    {
        if (atoi(elem1->experience) < atoi(elem2->experience))
            return -1;

        else if (atoi(elem1->experience) > atoi(elem2->experience))
            return 1;

        else
            return 0;
    }

    printf("----------\n");

    qsort(lessons,
            i,
            sizeof(struct lesson),
            (compfn)compare
         );

    for(j = 0; j < 10; j++) {
        printf("\n\n%i) date: %s, experience: %i, front: %s", j+1, lessons[j].date, atoi(lessons[j].experience), lessons[j].card_front);
        char *feels_know, *did_know;
        feels_know = malloc(1);
        did_know = malloc(1);

        int f, d, lower, higher;
        printf("\n Do you know the solution? Enter a number between 0 and 5, (0)-no idea, (5)-know it perfectly: ");
        while(f==0) {
            f = atoi(feels_know);
            *feels_know = getche();
        }
        printf("\n The solution is: %s", lessons[j].card_back);
        printf("\n Did you know the solution? Enter a number between 0 and 5, (0)-I missed it, (5)-I knew it: ");
        *did_know = getche();
        d = atoi(did_know);
        if(f>d) {lower = d; higher = f;}
        else {lower = f; higher = d;}
        //if(f == 5 || d == 5) {
            //printf("\n Experience points increased by (%i-%i+1)*(5-%i)=%i", higher, lower, d, (higher-lower+1)*(5-d));
            printf("\n Experience points increased by %i", d);
            sprintf(lessons[j].experience, "%i", atoi(lessons[j].experience) + d);
            printf("%i", *lessons[j].experience);
        //}
    }

    FILE *fpw = fopen("ankidbw.txt", "w");
    if (fpw == NULL)
    {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }
    else {
        for(j = 0; j < i; j++)
            fprintf(fpw, "%s;%i;%s;%s\n", lessons[j].date, atoi(lessons[j].experience), lessons[j].card_front, lessons[j].card_back);
    }

    char *curr_time = curtime();
    printf("\n %s", curr_time);

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
    //for (;i>=0;i--)
    //    free(words[i]);
    //free(words);
    return 0;
}
