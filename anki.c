#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

char *db_file;
char *db_temp_file;
char *db_backup_file;
int lessons_size = 0;
char nextchar;
int shortwelcome;
int max_line_len = 500000;
char *cmd;

typedef struct lesson_class_struct {
    char date[19];
    char *card_front;
    char *card_back;
    char *experience;
    int seen;
} lesson;

lesson *lessons;

static struct termios old, new;

void init(){
    db_file = "ankidb.txt";
    db_temp_file = "ankidbw.txt";
    db_backup_file = "ankidb.bak";
}

void print_help(){
    printf("\nanki clone in your console"
           "\n\nOptions"
           "\n -h, --help    show help"
           "\n -s, --shortwelcome  short welcome message on start."
           "\n -c, --create-template  short welcome message on start."
           "\n");
}

void welcome_help(){
    printf("\n Help"
           "\n 1) Press any key to show the solution."
           "\n 2) Use the <j>, <k>, and <l> buttons in order to add some points to your experience."
           "\n        key    extra points"
           "\n        ~~~    ~~~~~~~~~~~~"
           "\n        <j>         +1"
           "\n        <k>         +4"
           "\n        <l>         +9"
           "\n"
           "\n 3) After every 4 cards you will be asked whether to continue.*"
           "\n        key          behaviour"
           "\n        ~~~          ~~~~~~~~~"
           "\n        <n>          will stop the study and quit, hope to see you soon."
           "\n        any other    will keep going, have fun with another 4 cards."
           "\n"
           "\n    *Meanwhile an automated save happens."
           "\n"
           "\n");
    printf("\n Run %s -h for more help", cmd);
    printf("\n You can use Ctrl-c any time to quit without any changes to your scores."
           "\n"
           "\n");
}

void print_header(){
    printf("\n\n           *** Happy studying! ***");
    printf("\n\n %-20s%-20s%-5s%-20s", "card", "solution", "exp", "sum experience");
    printf("\n %-20s%-20s%-5s%-20s",   "~~~~", "~~~~~~~~", "~~~", "~~~~~~~~~~~~~~");
}

void create_new_db_file(){
    FILE *fp = fopen(db_file, "r");
    if(fp == NULL) {
        FILE *fpw = fopen(db_file, "w");
        fprintf(fpw, "2015-08-13 07:07:25;0;penalty;buntetes\n");
        fprintf(fpw, "2015-08-13 07:07:25;0;drivetrain;hajtomu\n");
        printf("***info*** %s has been created.\n", db_file);
        fclose(fpw);
    }
    else {
        fclose(fp);
        fprintf(stderr,"***error*** %s already exists.\n", db_file);
    }
}

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

int get_jkl_buttons_value(int keycode){
    switch(keycode){
        case 58: return 1;      //button 'j' => 1
        case 59: return 2;      //button 'k' => 2
        case 60: return 3;      //button 'l' => 3
        default: return keycode;
    }
}

typedef int (*compfn)(const void*, const void*);

int compare_for_qsort(lesson *elem1, lesson *elem2)
{
    if (atoi(elem1->experience) < atoi(elem2->experience))
        return -1;

    else if (atoi(elem1->experience) > atoi(elem2->experience))
        return 1;

    else
        return 0;
}

lesson *read_lessons(char *filename){
    lessons = malloc(max_line_len * sizeof(struct lesson_class_struct));

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr,"Could not find the default database text file.\n");
        fprintf(stderr,"You can create a template with %s --create-template\n", cmd);
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
            fclose(fp);
            lessons_size = i;
            qsort(lessons, lessons_size, sizeof(lesson), (compfn)compare_for_qsort);
            return lessons;
        }
    }
}

void save_lessons() {
    FILE *fpw = fopen(db_temp_file, "w");
    if (fpw == NULL)
    {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }
    else {
        for(int line = 0;line<lessons_size;line++) {
            fprintf(fpw, "%s;%i;%s;%s\n", lessons[line].date, atoi(lessons[line].experience), lessons[line].card_front, lessons[line].card_back);
        }
        fclose(fpw);
    }

    //TODO check new filesize, should not be smaller than the previous
    if(rename(db_file,db_backup_file)==0)
        rename(db_temp_file,db_file);
}

void handle_options(int argc, char *argv[]){
    for (int i=1;i<argc;i++) {
        if(strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) {
            print_help();
            exit(0);
        }
        if(strcmp(argv[i], "-c")==0 || strcmp(argv[i], "--create-template")==0) {
            create_new_db_file();
            exit(0);
        }
        if(strcmp(argv[i], "-s")==0 || strcmp(argv[i], "--shortwelcome")==0) {
            shortwelcome = 1;
        }
    }
}

int ask_for_proper_did_know(){
    int did_know = 0;
    while(did_know<1 || did_know>3) {
        did_know = getch() - '0';
        if(did_know>3) {
            did_know = get_jkl_buttons_value(did_know);
        }
    }
    return did_know;
}

void save_and_reload_lessons() {
    save_lessons();
    lessons = read_lessons(db_file);
    printf("\n***info*** save and reload lessons\n");
}

int main(int argc, char *argv[])

{
    cmd = argv[0];
    init();

    handle_options(argc, argv);

    lesson* lessons;
    lessons = read_lessons(db_file);

    if(!shortwelcome)
        welcome_help();

    print_header();

    int j = 0;
    int finished=0;

    while(finished==0) {
        for(int k=0;k<4;k++,j++){

            if(j>lessons_size-1) {
                j = 0;
                save_and_reload_lessons();
            }

            // print card_front
            printf("\n %-20s", lessons[j].card_front);

            // wait for card_back
            getch();
            printf("%-20s", lessons[j].card_back);

            // ask for experience
            int did_know = ask_for_proper_did_know(0);
            printf("(%i)", did_know);

            // print experience calculation
            int old_xp = atoi(lessons[j].experience);
            int add_xp = did_know*did_know;
            printf("%5i+%i", old_xp, add_xp);

            // set new experience
            sprintf(lessons[j].experience, "%i", old_xp + add_xp);

            // print sum experience
            printf("=%4s\n", lessons[j].experience);
        }

        save_lessons();

        printf("\n***continue?***  (n)-no  (any other keys)-yes\n");
        finished = getch() - '0' == 62;
    }
    printf("\n");

    return 0;
}
