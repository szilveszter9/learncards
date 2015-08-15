#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

char *db_file_name;
char *db_tempfile_name;
char *db_backup_file_name;
int lessons_size = 0;
char nextchar;
int shortwelcome;
int max_line_len = 50000;
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

void init() {
    db_file_name = "ankidb.txt";
    db_tempfile_name = "ankidbw.txt";
    db_backup_file_name = "ankidb.bak";
}

void print_help() {
    printf("\nanki clone in your console"
           "\n\nOptions"
           "\n h, -h, --help              show help"
           "\n s, -s, --shortwelcome      short welcome message on start"
           "\n i, -i, --init-template     create initial template file if does not exist yet"
           "\n a, -a, --add               add new cards"
           "\n");
}

void welcome_help() {
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

void print_header() {
    printf("\n\n*********************** Happy studying! ********************");
    printf("\n\n %-20s%-20s%-5s%-20s", "card", "solution", "exp", "sum experience");
    printf("\n %-20s%-20s%-5s%-20s",   "~~~~", "~~~~~~~~", "~~~", "~~~~~~~~~~~~~~");
}

void create_new_db_file() {
    FILE *fp = fopen(db_file_name, "r");
    if(fp == NULL) {
        FILE *fpw = fopen(db_file_name, "w");
        fprintf(fpw, "2015-08-13 07:07:25;0;penalty;buntetes\n");
        fprintf(fpw, "2015-08-13 07:07:25;0;drivetrain;hajtomu\n");
        printf("***info*** %s has been created.\n", db_file_name);
        fclose(fpw);
    }
    else {
        fclose(fp);
        fprintf(stderr,"***error*** %s already exists.\n", db_file_name);
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

int get_jkl_buttons_value(int keycode) {
    switch(keycode) {
        case 58: return 1;      //button 'j' => 1
        case 59: return 2;      //button 'k' => 2
        case 60: return 3;      //button 'l' => 3
        default: return keycode;
    }
}

typedef int (*compfn)(const void*, const void*);

int compare_for_qsort(lesson *elem1, lesson *elem2)
{
    if(atoi(elem1->experience) < atoi(elem2->experience))
        return -1;

    else if(atoi(elem1->experience) > atoi(elem2->experience))
        return 1;

    else
        return 0;
}

lesson *load_lessons() {
    free(lessons);
    lessons = malloc(max_line_len * sizeof(struct lesson_class_struct));

    FILE *fp = fopen(db_file_name, "r");
    if(fp == NULL)
    {
        fprintf(stderr,"Could not find the default database text file.\n");
        fprintf(stderr,"You can create a template with %s --create-template\n", cmd);
        exit(2);
    }

    for(int c_line = 0;; c_line++)
    {
        int c_char;
        int column = 0;

        lessons[c_line].card_front = malloc(128);
        lessons[c_line].card_back = malloc(128);
        lessons[c_line].experience = malloc(32);

        for(c_char = 0;; c_char++) {
            nextchar = fgetc(fp);
            if(nextchar == '\n' || nextchar == '\r' || nextchar == EOF) {
                break;
            }
            else if(nextchar == ';') {
                column++;
                c_char = -1;
            }
            else if(column == 0) lessons[c_line].date[c_char] = nextchar;
            else if(column == 1) lessons[c_line].experience[c_char] = nextchar;
            else if(column == 2) lessons[c_line].card_front[c_char] = nextchar;
            else if(column == 3) lessons[c_line].card_back[c_char] = nextchar;
        }
        if(nextchar == EOF) {
            fclose(fp);
            lessons_size = c_line;
            qsort(lessons, lessons_size, sizeof(lesson), (compfn)compare_for_qsort);
            return lessons;
        }
    }
}

void save_lessons() {
    FILE *fpw = fopen(db_tempfile_name, "w");
    if(fpw == NULL)
    {
        fprintf(stderr,"Error opening file.\n");
        exit(2);
    }
    else {
        for(int c_line = 0; c_line<lessons_size; c_line++) {
            fprintf(fpw, "%s;%i;%s;%s\n", lessons[c_line].date, atoi(lessons[c_line].experience), lessons[c_line].card_front, lessons[c_line].card_back);
        }
        fclose(fpw);
    }

    //TODO check new filesize, should not be smaller than the previous
    if(rename(db_file_name, db_backup_file_name) == 0)
        rename(db_tempfile_name, db_file_name);
}

int ask_for_proper_did_know() {
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
    lessons = load_lessons();
    printf("\n***info*** save and reload lessons\n");
}

void create_new_card(){
    char *card_front = malloc(128);
    char *card_back = malloc(128);
    char *experience = malloc(32);
    int xp_int = 0;

    printf("\n enter card front:");
    scanf("%s", card_front);
    printf("    enter card back:");
    scanf("%s", card_back);
    printf("    enter experience:");

    // consume \n that scanf left there in order to use fgets after scanf at all
    getchar();
    fgets(experience, 32, stdin);
    strtok(experience, "\n");
    printf("-%s-", experience);

    lessons[lessons_size].card_front = malloc(128);
    lessons[lessons_size].card_back = malloc(128);
    lessons[lessons_size].experience = malloc(32);

    lessons[lessons_size].card_front = card_front;
    lessons[lessons_size].card_back = card_back;
    sprintf(lessons[lessons_size].experience, "%i", atoi(experience));

    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    char strformat[] = "%Y-%m-%d %H:%M:%S";
    strftime(lessons[lessons_size].date, 20, strformat, &now);

    lessons_size++;
}

void handle_cli_options(int argc, char *argv[]) {
    for(int c_arg_idx = 1; c_arg_idx<argc; c_arg_idx++) {
        int _is(char *str) {
            return strcmp(argv[c_arg_idx], str) == 0;
        }
        int is(char *str) {
            char dash[30], dash2[30];
            strcpy(dash, "-");
            strcpy(dash2, "--");
            return _is(str) || _is(strcat(dash, str)) || _is(strcat(dash2, str));
        }
        if(is("h") || is("help")) {
            print_help();
            exit(0);
        }
        if(is("i") || is("init-template")) {
            create_new_db_file();
            exit(0);
        }
        if(is("a") || is("add")) {
            load_lessons();
            while(1) {
                create_new_card();
                save_lessons();
            }
        }
        if(is("s") || is("shortwelcome")) {
            shortwelcome = 1;
        }
    }
}

int main(int argc, char *argv[])

{
    cmd = argv[0];
    init();

    handle_cli_options(argc, argv);

    load_lessons();

    if(!shortwelcome)
        welcome_help();

    print_header();

    int c_line = 0;
    int finished = 0;

    while(finished == 0) {
        for(int c_quiz = 0; c_quiz<4; c_quiz++, c_line++) {

            // reload if out of lessons
            if(c_line>lessons_size-1) {
                c_line = 0;
                save_and_reload_lessons();
            }

            // print card_front
            printf("\n %-20s", lessons[c_line].card_front);

            // wait for card_back
            getch();
            printf("%-20s", lessons[c_line].card_back);

            // ask for experience
            int did_know = ask_for_proper_did_know(0);
            printf("(%i)", did_know);

            // print experience calculation
            int old_xp = atoi(lessons[c_line].experience);
            int add_xp = did_know*did_know;
            printf("%5i+%i", old_xp, add_xp);

            // set new experience
            sprintf(lessons[c_line].experience, "%i", old_xp + add_xp);

            // print sum experience
            printf("=%4s\n", lessons[c_line].experience);
        }

        save_lessons();

        printf("\n***continue?***  (n)-no  (any other keys)-yes\n");
        finished = getch() - '0' == 62;
    }
    printf("\n");

    free(lessons);
    return 0;
}
