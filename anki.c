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

void welcome_help(char *cmd){
    printf("\n Help");
    printf("\n 1) Press any key to show the solution.");
    printf("\n 2) Use the (j), (k), and (l) buttons in order to add some points to your experience.");
    printf("\n        key    extra points");
    printf("\n        ~~~    ~~~~~~~~~~~~");
    printf("\n        (j)        +1");
    printf("\n        (k)        +4");
    printf("\n        (l)        +9");
    printf("\n 3) After every 4 cards you will be asked whether to continue.");
    printf("\n        Meanwhile an automated save happens.");
    printf("\n        (n) will stop the study and quit, hope to see you soon.");
    printf("\n        any other keys will keep going, have fun with another 4 cards.");
    printf("\n Run %s -h for more help", cmd);
    printf("\n You can press Ctrl-c any time to quit without any changes to your scores.");
}

typedef struct lesson_class_struct {
    char date[19];
    //char card_front[128];
    //char card_back[128];
    char *card_front;
    char *card_back;
    char *experience;
    int seen;
} lesson;

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


typedef int (*compfn)(const void*, const void*);

char nextchar;
int shortwelcome;

int max_line_len = 500000;

int compare(lesson *elem1, lesson *elem2)
{
    if (atoi(elem1->experience) < atoi(elem2->experience))
        return -1;

    else if (atoi(elem1->experience) > atoi(elem2->experience))
        return 1;

    else
        return 0;
}

lesson *read_lessons(char *filename){
    lesson *lessons = malloc(max_line_len * sizeof(struct lesson_class_struct));;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }

    for (int i=0;;i++)
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
        if(nextchar==EOF){
            /* Close file */
            fclose(fp);
            qsort(lessons, sizeof(lessons), sizeof(lesson), (compfn)compare);
            return lessons;
        }
    }
}

int main(int argc, char *argv[])

{
    for (int i=1;i<argc;i++) {
        if(strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) {
            printf("\nanki clone in your console");
            printf("\n\nOptions");
            printf("\n -h, --help    show help");
            printf("\n -s, --shortwelcome  short welcome message on start.");
            printf("\n");
            return 0;
        }
        if(strcmp(argv[i], "-s")==0 || strcmp(argv[i], "--shortwelcome")==0) {
            shortwelcome = 1;
        }
    }

    if(!shortwelcome)
        welcome_help(argv[0]);

    printf("\n\n           *** Happy studying! ***");
    printf("\n\n%-20s%-20s%-5s%-20s", "card", "solution", "exp", "sum experience");
    printf("\n%-20s%-20s%-5s%-20s", "~~~~", "~~~~~~~~", "~~~", "~~~~~~~~~~~~~~");

    int lines_allocated = 128;

    /* Allocate lines of text */
    char **words = (char **)malloc(sizeof(char*)*lines_allocated);
    //struct lesson lessons[sizeof(char*)*lines_allocated];
    if (words==NULL)
    {
        fprintf(stderr,"Out of memory (1).\n");
        exit(1);
    }

    lesson* lessons;
    lessons = read_lessons("ankidb.txt");

    int j = 0;
    int finished=0;

    int jkl(int keycode){
        switch(keycode){
            case 58: return 1;      //button 'j' => 1
            case 59: return 2;      //button 'k' => 2
            case 60: return 3;      //button 'l' => 3
        }
    }

    while(finished==0) {
        for(int k=0;k<4;k++,j++){
            if(j>sizeof(lessons)-2) {
                j = 0;
                lessons = read_lessons("ankidb.txt");
            }

            int did_know=0;
            printf("\n%-20s", lessons[j].card_front);
            getch();
            printf("%-20s", lessons[j].card_back);
            while(did_know<1 || did_know>3) {
                did_know = getch() - '0';
                if(did_know>3) {
                    did_know = jkl(did_know);
                }
            }
            printf("(%i)", did_know);
            printf("%5i+%i", atoi(lessons[j].experience), did_know*did_know);
            sprintf(lessons[j].experience, "%i", atoi(lessons[j].experience) + did_know*did_know);
            printf("=%4s\n", lessons[j].experience);
        }

        FILE *fpw = fopen("ankidbw.txt", "w");
        if (fpw == NULL)
        {
            fprintf(stderr,"Error opening file.\n");
            exit(2);
        }
        else {
            for(int line = 0; line < sizeof(lessons); line++){
                fprintf(fpw, "%s;%i;%s;%s\n", lessons[line].date, atoi(lessons[line].experience), lessons[line].card_front, lessons[line].card_back);
                printf("%s;%i;%s;%s\n", lessons[line].date, atoi(lessons[line].experience), lessons[line].card_front, lessons[line].card_back);
            }
            fclose(fpw);
        }

        //TODO check new filesize, should not be smaller than the previous
        if(rename("ankidb.txt","ankidb.bak")==0)
            rename("ankidbw.txt","ankidb.txt");

        printf("\ncontinue?  (n)-no  (any other keys)-yes\n");
        finished = getch() - '0' == 62;
    }
    printf("\n");

    ///////char *curr_time = curtime();
    ///////printf("\n %s", curr_time);

    ///////struct tm time1 = {0};

    ///////char timestr[] = "2015-08-13 01:10:52 0000 BST";
    ///////char strformat[] = "%Y-%m-%d %H:%M:%S %z %Z";

    ///////strptime(timestr, strformat, &time1);
    //puts(timestr);
    //strptime(timestr, strformat, &time1);
    //mktime(&time1);
    //strftime(timestr, sizeof(timestr), strformat, &time1);
    //puts(timestr);

    //double elapsed = difftime(&time1, time(NULL));
    //double elapsed1 = difftime(0, time(NULL));
    ///////printf("%zu\n", time(NULL) - mktime(&time1));
    //printf("%i\n", );
    //printf("%d\n", elapsed);
    //printf("%d\n", elapsed1);

    /* Good practice to free memory */
    //for (;i>=0;i--)
    //    free(words[i]);
    //free(words);
    return 0;
}
