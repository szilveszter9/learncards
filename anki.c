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
    printf("\n 2) Use the <j>, <k>, and <l> buttons in order to add some points to your experience.");
    printf("\n        key    extra points");
    printf("\n        ~~~    ~~~~~~~~~~~~");
    printf("\n        <j>         +1");
    printf("\n        <k>         +4");
    printf("\n        <l>         +9");
    printf("\n");
    printf("\n 3) After every 4 cards you will be asked whether to continue.*");
    printf("\n        key          behaviour");
    printf("\n        ~~~          ~~~~~~~~~");
    printf("\n        <n>          will stop the study and quit, hope to see you soon.");
    printf("\n        any other    will keep going, have fun with another 4 cards.");
    printf("\n");
    printf("\n    *Meanwhile an automated save happens.");
    printf("\n");
    printf("\n");
    printf("\n Run %s -h for more help", cmd);
    printf("\n You can use Ctrl-c any time to quit without any changes to your scores.");
    printf("\n");
    printf("\n");
}

typedef struct lesson_class_struct {
    char date[19];
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

void save_lessons(lesson *lessons) {
    FILE *fpw = fopen("ankidbw.txt", "w");
    if (fpw == NULL)
    {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }
    else {
        for(int line = 0; line < sizeof(lessons); line++){
            fprintf(fpw, "%s;%i;%s;%s\n", lessons[line].date, atoi(lessons[line].experience), lessons[line].card_front, lessons[line].card_back);
        }
        fclose(fpw);
    }

    //TODO check new filesize, should not be smaller than the previous
    if(rename("ankidb.txt","ankidb.bak")==0)
        rename("ankidbw.txt","ankidb.txt");
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
    printf("\n\n %-20s%-20s%-5s%-20s", "card", "solution", "exp", "sum experience");
    printf("\n %-20s%-20s%-5s%-20s", "~~~~", "~~~~~~~~", "~~~", "~~~~~~~~~~~~~~");

    lesson* lessons;
    lessons = read_lessons("ankidb.txt");

    int j = 0;
    int finished=0;

    int jkl(int keycode){
        switch(keycode){
            case 58: return 1;      //button 'j' => 1
            case 59: return 2;      //button 'k' => 2
            case 60: return 3;      //button 'l' => 3
            default: return keycode;
        }
    }

    while(finished==0) {
        for(int k=0;k<4;k++,j++){
            if(j>sizeof(lessons)-2) {
                j = 0;
                save_lessons(lessons);
                lessons = read_lessons("ankidb.txt");
                printf("\n***info*** save and reload lessons\n");
            }

            int did_know=0;
            printf("\n %-20s", lessons[j].card_front);
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

        save_lessons(lessons);

        printf("\n***continue?***  (n)-no  (any other keys)-yes\n");
        finished = getch() - '0' == 62;
    }
    printf("\n");

    return 0;
}
