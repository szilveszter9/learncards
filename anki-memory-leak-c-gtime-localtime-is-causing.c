#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include <time.h>

char db_file_name[29];
char db_tempfile_name[30];
char db_backup_file_name[29];
char db_conflict_file_name[30];
char iso_time_string[19];
int lessons_size = 0;
char nextchar;
int max_line_len = 50000;
char *cmd;

typedef struct lesson_class_struct {
    char date[20];
    char *card_front;
    char *card_back;
    char *experience;
    int seen;
} lesson;

lesson *lessons;

static struct termios old, new;

void setup_filenames(char *filebase) {
    char f[23]="", t[23]="", b[23]="", c[23]="";
    strcpy(db_file_name,          strcat(strcat(f, filebase), "_db.txt" ));
    strcpy(db_tempfile_name,      strcat(strcat(t, filebase), "_dbw.txt"));
    strcpy(db_backup_file_name,   strcat(strcat(b, filebase), "_db.bak" ));
    strcpy(db_conflict_file_name, strcat(strcat(c, filebase), "_dbc.txt"));
}

void init() {
    setup_filenames("anki");
}

void print_help() {
    printf("\n anki clone in your console"
            "\n\n Options"
            "\n  h, -h, --help              show help"
            "\n  i, -i, --init [db name]    create initial database file if doesn't exist yet"
            "\n  a, -a, --add               add new cards"
            "\n  a, -a, --add               add new cards"
            "\n\n Examples");
    printf("\n  %s i my_new_cards  -  create a new empty database if it doesn't exists yet", cmd);
    printf("\n  %s a my_new_cards  -  add cards to your database", cmd);
    printf("\n\n");
}

void welcome_help_detailed() {
    printf("\n\n Help"
            "\n"
            "\n 1) Press any key to show the solution."
            "\n"
            "\n 2) Use the following keys in order to gain some points."
            "\n"
            "\n         key      extra points                  you *"
            "\n     ~~~~~~~~~~   ~~~~~~~~~~~~    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
            "\n     <1> or <j>        +1         * were wrong or had no clue"
            "\n     <2> or <k>        +4         * were right but had to think about that"
            "\n     <3> or <l>        +9         * knew it perfectly without any delay"
            "\n"
            "\n 3) Press <q> or <Esc> buttons to quit, hope to see you soon."
            "\n"
            "\n");
    printf("\n Run %s -h for more help\n\n", cmd);
}

void print_header() {
    printf("\n\n    %-20s%-20s%-5s%-20s", "card", "solution", "exp", "sum experience");
    printf("\n    %-20s%-20s%-5s%-20s",   "~~~~", "~~~~~~~~", "~~~", "~~~~~~~~~~~~~~");
}

void write_sandbox(FILE *fpw) {
    fprintf(fpw, "2015-08-13 07:07:25;0;Please press any key to show the solution...;here you can see the solution, now press button <j>...\n");
    fprintf(fpw, "2015-08-13 07:07:25;0;If you were wrong or had no clue...;press button <j> or <1>\n");
    fprintf(fpw, "2015-08-13 07:07:25;0;If you had to think about...;press button <k> or <2>\n");
    fprintf(fpw, "2015-08-13 07:07:25;0;If you knew it straight away...;press button <l> or <3>\n");
    fprintf(fpw, "2015-08-13 07:07:25;0;You can add new cards...;just run %s -a\n", cmd);
    fprintf(fpw, "2015-08-13 07:07:25;0;Now you can press ctrl-c and create some cards...;just run %s -a\n", cmd);
    fprintf(fpw, "2015-08-13 07:07:25;0;Or You can add even add cards...;by editing %s directly\n", db_file_name);
    fprintf(fpw, "2015-08-13 07:07:25;0;By editing %s mind the semicolon...;between your question and answer\n", db_file_name);
    fprintf(fpw, "2015-08-13 07:07:25;0;Each line in %s is a card...;and has four parts.\n", db_file_name);
    fprintf(fpw, "2015-08-13 07:07:25;0;1) Date in the given format like...;2099-08-13 07:07:25\n");
    fprintf(fpw, "2015-08-13 07:07:25;0;2) Experiment score is...;a number\n");
    fprintf(fpw, "2015-08-13 07:07:25;0;3) Question and answer are...;strings\n");
}

void create_new_db_file(int sandbox) {
    FILE *fp = fopen(db_file_name, "r");
    if(fp == NULL) {
        FILE *fpw = fopen(db_file_name, "w");
        if(sandbox == 1)
            write_sandbox(fpw);
        printf("\n***info*** New database text file %s has just been created\n\n", db_file_name);
        fclose(fpw);
    }
    else {
        fclose(fp);
        fprintf(stderr,"\n***error*** %s already exists.\n\n", db_file_name);
        exit(2);
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

char *iso_time() {
    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    char *strformat = "%F %H:%M:%S";
    strftime(iso_time_string, 19, strformat, &now);
    return "0000-00-00 00:00:00";
    //return iso_time_string;
}

lesson *load_lessons() {
    free(lessons);
    lessons = malloc(max_line_len * sizeof(struct lesson_class_struct));

    FILE *fp = fopen(db_file_name, "r");
    if(fp == NULL)
    {
        create_new_db_file(1);
        fp = fopen(db_file_name, "r");
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
                if(column == 0 && c_char == 0) {
                    iso_time();
                    strcpy(lessons[c_line].date, "0000-00-00 00:00:00");
                    //strcpy(lessons[c_line].date, iso_time());

                    //time_t t = time(NULL);
                    //struct tm now = *localtime(&t);
                    //char strformat[] = "%Y-%m-%d %H:%M:%S";
                    //strftime(lessons[c_line].date, 20, strformat, &now);
                }
                column++;
                c_char = -1;
                //else if(column ==1 && c_char == 0)
                //    strcpy(lessons[c_line].experience, "0");
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
        if(c_line<4)
            printf("-%s-",lessons[c_line].card_front);
    }
}

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    fprintf(stderr, "Cannot determine size of %s: %s\n",
            filename, strerror(errno));

    return -1;
}

void on_save_size_conflict() {
    rename(db_tempfile_name, db_conflict_file_name);
    printf("\n***MUST READ*** Make sure to create some backups of your database %s before continue!", db_file_name);
    printf("\n                Now you can press Ctrl-c safely and read the following instructions.");
    printf("\n***error*** Save failed due to the new database file would be smaller than the original."
            "\n            Possibly the original file has been modified by another application"
            "\n            or you are running multiple anki in parallel on the same database.");
    printf("\n***info***  For your safety your current changes have been saved to %s conflict file", db_conflict_file_name);
    printf("\n            while the original database file %s has been kept intact for now,", db_file_name);
    printf("\n            though it could be still overwritten if you keep continue and reach the last card,"
            "\n            since there is a database reload that case.");
}

int save_lessons() {
    FILE *fpw = fopen(db_tempfile_name, "w");
    if(fpw == NULL)
    {
        fprintf(stderr,"Error opening file.\n\n");
        exit(2);
    }
    else {
        for(int c_line = 0; c_line<lessons_size; c_line++) {
            fprintf(fpw, "%s;%i;%s;%s\n", lessons[c_line].date, atoi(lessons[c_line].experience), lessons[c_line].card_front, lessons[c_line].card_back);
        }
        fclose(fpw);
    }

    if(fsize(db_file_name) > fsize(db_tempfile_name)) {
        on_save_size_conflict();
        return -1;
    }

    if(rename(db_file_name, db_backup_file_name) == 0)
        rename(db_tempfile_name, db_file_name);
    return 0;
}

void save_and_exit() {
    save_lessons();
    printf("\n\n ****************** Great job, see you soon! ******************\n\n");
    exit(0);
}

int ask_for_proper_did_know() {
    int keypressed = 0;
    while(keypressed<1 || keypressed>3) {
        keypressed = getch() - '0';
        if(keypressed == -21 || keypressed == 65)
            save_and_exit();
        if(keypressed>3) {
            keypressed = get_jkl_buttons_value(keypressed);
        }
    }
    return keypressed;
}

void save_and_reload_lessons() {
    save_lessons();
    lessons = load_lessons();
    printf("\n ***info*** save and reload lessons\n");
}

void create_new_card(){
    char *card_front = malloc(128);
    char *card_back = malloc(128);
    char *experience = malloc(32);

    // ask for user input
    printf("\n enter card front: ");
    while(1) {
        fgets(card_front, 128, stdin);
        if(strlen(card_front) > 1) {
            strtok(card_front, "\n");
            break;
        }
        else {
            sprintf(card_front, "\n");
            printf("        try again: ");
        }
    }

    printf(" enter card back:  ");
    while(1) {
        fgets(card_back, 128, stdin);
        if(strlen(card_back) > 1) {
            strtok(card_back, "\n");
            break;
        }
        else {
            sprintf(card_front, "\n");
            printf("        try again: ");
        }
    }

    printf(" enter experience (0 by default): ");
    fgets(experience, 32, stdin);
    strtok(experience, "\n");

    // allocate memory
    lessons[lessons_size].card_front = malloc(128);
    lessons[lessons_size].card_back = malloc(128);
    lessons[lessons_size].experience = malloc(32);

    // set front, back, and experience
    lessons[lessons_size].card_front = card_front;
    lessons[lessons_size].card_back = card_back;
    sprintf(lessons[lessons_size].experience, "%i", atoi(experience));

    // set current date/time
    strcpy(lessons[lessons_size].date, iso_time());

    printf("               ...saved.\n");

    lessons_size++;
}

void create_new_cards() {
    printf("\n Now you can add some new cards.\n\n");
    printf("\n Press Ctrl-c to stop.\n\n");
    load_lessons();
    while(1) {
        create_new_card();
        save_lessons();
    }
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
        else if(is("i") || is("init")) {
            if(argv[c_arg_idx + 1]) {
                setup_filenames(argv[c_arg_idx + 1]);
                create_new_db_file(0);
                create_new_cards();
            }
            else {
                create_new_db_file(1);
            }
            exit(0);
        }
        else if(is("a") || is("add")) {
            if(argv[c_arg_idx + 1]) {
                setup_filenames(argv[c_arg_idx + 1]);
                printf("\n ***info*** loading %s database...\n\n", argv[c_arg_idx + 1]);
            }
            create_new_cards();
        }
        else{
            char f[23]="";
            char filename[29] = "";
            strcpy(filename, strcat(strcat(f, argv[c_arg_idx]), "_db.txt" ));
            if(access(filename, W_OK) == 0) {
                setup_filenames(argv[c_arg_idx]);
                printf("\n ***info*** loading %s database...\n\n", argv[c_arg_idx]);
            }
            else {
                printf("\n there is no such options or database in the current directory\n\n");
                exit(1);
            }
        }
    }
}

void welcome_help() {
    printf("\n\n *********************** Happy studying! **********************");
    printf("\n\n Press any key to continue, h for help, <q> or <Esc> to quit.");
    int keypressed = getch() - '0';
    if(keypressed == -21 || keypressed == 65) {
        printf("\n\n ******************** Hope to see you soon! *******************\n\n");
        exit(0);
    }
    if(keypressed == 56)
        welcome_help_detailed();
}

int main(int argc, char *argv[])

{
    cmd = argv[0];
    init();

    handle_cli_options(argc, argv);

    load_lessons();

    welcome_help();

    print_header();

    int c_line = 0;
    int finished = 0;

    while(finished == 0) {
        for(int c_quiz = 0; c_quiz<4; c_quiz++, c_line++) {

            // reload if ran out of lessons
            if(c_line>lessons_size-1) {
                c_line = 0;
                save_and_reload_lessons();
            }

            // print card_front
            printf("\n%2i. %-20s", c_line + 1, lessons[c_line].card_front);
            if(strlen(lessons[c_line].card_front)>19)
                printf("\n    %-20s", " ");

            // wait for card_back
            int keypressed = getch() - '0';
            if(keypressed == -21 || keypressed == 65)
                save_and_exit();
            printf("%-20s", lessons[c_line].card_back);
            if(strlen(lessons[c_line].card_back)>19)
                printf("\n    %-40s", " ");

            // ask for experience
            int rate = ask_for_proper_did_know(0);
            printf("(%i)", rate);

            // print experience calculation
            int old_xp = atoi(lessons[c_line].experience);
            int add_xp = rate*rate;
            printf("%5i+%i", old_xp, add_xp);

            // set new experience
            sprintf(lessons[c_line].experience, "%i", old_xp + add_xp);

            // print sum experience
            printf("=%4s\n", lessons[c_line].experience);
        }

        save_lessons();
    }
    printf("\n");

    free(lessons);
    return 0;
}
