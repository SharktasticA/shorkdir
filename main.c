/*
    ######################################################
    ##             SHORK UTILITY - SHORKDIR             ##
    ######################################################
    ## A lightweight Linux terminal-based file browser  ##
    ######################################################
    ## Licence: GNU GENERAL PUBLIC LICENSE Version 3    ##
    ######################################################
    ## Kali (links.sharktastica.co.uk)                  ##
    ######################################################
*/



#include <ctype.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>



enum NavInput 
{
    CURSOR_DOWN,
    CURSOR_UP,
    DEBUG,
    DIR_UP,
    DIR_DOWN,
    HELP,
    INSPECT,
    QUIT,
    TOGGLE_HIDDEN,
    INVALID
};

typedef struct 
{
    char *name;
    char *payload;
    int visible;
} MenuItem;



#define COL_BAK_BLACK           "40"
#define COL_BAK_BLUE            "44"
#define COL_BAK_CYAN            "46"
#define COL_BAK_GREEN           "42"
#define COL_BAK_GREY            "40"
#define COL_BAK_MAGENTA         "45"
#define COL_BAK_RED             "41"
#define COL_BAK_WHITE           "47"
#define COL_BAK_YELLOW          "43"

#define COL_FOR_BLACK           "0;30"
#define COL_FOR_BLUE            "0;34"
#define COL_FOR_BOLD_BLUE       "1;34"
#define COL_FOR_BOLD_CYAN       "1;36"
#define COL_FOR_BOLD_GREEN      "1;32"
#define COL_FOR_BOLD_MAGENTA    "1;35"
#define COL_FOR_BOLD_RED        "1;31"
#define COL_FOR_BOLD_WHITE      "1;37"
#define COL_FOR_BOLD_YELLOW     "1;33"
#define COL_FOR_CYAN            "0;36"
#define COL_FOR_GREEN           "0;32"
#define COL_FOR_GREY            "1;30"
#define COL_FOR_MAGENTA         "0;35"
#define COL_FOR_RED             "0;31"
#define COL_FOR_WHITE           "0;37"
#define COL_FOR_YELLOW          "0;33"

#define COL_RESET               "0"
#define COL_FOR_RESET           "39"
#define COL_BAK_RESET           "49"

#define DT_EXE 16



static int CODE_INSTALLED = 0;
static int COL_ENABLED = 1;
static char *COL_FOR_ARROW = COL_FOR_BOLD_RED;
static char *COL_FOR_CODE = COL_FOR_BOLD_RED;
static char *COL_FOR_CURSOR = COL_FOR_BOLD_CYAN;
static char *COL_FOR_HEADING = COL_FOR_BOLD_CYAN;
static char *COL_FOR_OL = COL_FOR_GREEN;
static int DOTFILES_VISIBLE = 1;
static int EMACS_INSTALLED = 0;
static int FILE_INSTALLED = 0;
static int FLOW_CTRL_INSTALLED = 0;
static int GEDIT_INSTALLED = 0;
static int GTED_INSTALLED = 0;
static int KATE_INSTALLED = 0;
static int MG_INSTALLED = 0;
static int MOUSEPAD_INSTALLED = 0;
static int NANO_INSTALLED = 0;
static int NVIM_INSTALLED = 0;
static struct termios OLD_TERMIOS;
static int PLUMA_INSTALLED = 0;
static int RAW_MODE_ENABLED = 0;
static struct winsize TERM_SIZE;
static int VI_INSTALLED = 0;
static int VIM_INSTALLED = 0;
static int XED_INSTALLED = 0;



/**
 * Awaits for any user input.
 */
void awaitInput(void)
{
    int len = printf("Press any key to continue... ");
    if (COL_ENABLED)
        for (size_t i = len; i < TERM_SIZE.ws_col; i++)
            printf(" ");
    getchar();
}

/**
 * Moves the cursor to topleft-most position and clears below cursor.
 */
void clearScreen(void)
{
    printf("\033[H\033[J");
}

/**
 * Allows qsort to compare two directory entry names.
 * @param a First entry to compare
 * @param b Second entry to compare
 * @return negative (a < b), 0 (a == b) or positive (a > b)
 */
int compareDirName(const void *a, const void *b)
{
    const struct dirent *sa = *(const struct dirent **)a;
    const struct dirent *sb = *(const struct dirent **)b;
    return strcasecmp(sa->d_name, sb->d_name);
}

/**
 * Enables the terminal's canonical input. Used only when the program exits.
 */
void disableRawMode(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &OLD_TERMIOS);
}

/**
 * Disables the terminal's canonical input so that things like getchar do not
 * wait until enter is pressed.
 */
void enableRawMode(void)
{
    struct termios newTERMIO;
    tcgetattr(STDIN_FILENO, &OLD_TERMIOS);
    newTERMIO = OLD_TERMIOS;
    newTERMIO.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTERMIO);
    RAW_MODE_ENABLED = 1;
}

/**
 * Adds new lines to a given string based on the requested line width.
 * @param buffer Input string
 * @param width Characters per line
 * @param indent Indent to include after newly inserted new line
 * @return Number of lines in the string
 */
int formatNewLines(char *buffer, int width, char *indent)
{
    if (!buffer || width < 1) return 0;

    size_t bufferStrLen = strlen(buffer);
    size_t indentLen = indent ? strlen(indent) : 0;
    int lines = 1;
    int lastSpace = -1;
    int widthCount = 1;

    for (int i = 0; i < bufferStrLen; i++)
    {
        if (buffer[i] == '\033')
        {
            while (i < bufferStrLen && buffer[i] != 'm') i++;
            if (i >= bufferStrLen) break;
            continue; 
        }
        
        if (buffer[i] == ' ') lastSpace = i;
        else if (buffer[i] == '\n')
        {
            lines++;
            widthCount = 0;
            continue;
        }

        if (widthCount == width)
        {
            if (lastSpace != -1)
            {
                buffer[lastSpace] = '\n';
                lines++;

                if (indent && indentLen > 0)
                {
                    memmove(buffer + lastSpace + 1 + indentLen, buffer + lastSpace + 1, bufferStrLen - lastSpace);
                    memcpy(buffer + lastSpace + 1, indent, indentLen);
                    bufferStrLen += indentLen;
                    if (lastSpace <= i) i += indentLen;
                }
            }
            widthCount = i - lastSpace;
        }

        widthCount++;
    }

    return lines;
}

/**
 * Gets an integer input from the user.
 * @param prompt Prompt to give the user 
 * @param min Minimum allowed input (set to same as max to disable validation)
 * @param max Maximum allowed input (set to same as min to disable validation)
 * @param negativeIfInvalid Flags if function should return -1 if invalid input instead of looping
 * @return User's integer input
 */
int getIntInput(char *prompt, int min, int max, int negativeIfInvalid)
{
    if (min > max) min = max;
    if (max < min) max = min;

    int isValid;
    char buffer[32];
    int val;

    do
    {
        disableRawMode();

        if (COL_ENABLED)
        {
            printf("\033[%s;%sm", COL_FOR_BOLD_WHITE, COL_BAK_BLUE);
            for (int i = 0; i < TERM_SIZE.ws_col; i++) printf(" ");
            printf("\033[1G");
        }

        printf("%s", prompt);
        if (min != max) printf(" (%d-%d)", min, max);
        printf(": ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            if (sscanf(buffer, "%d", &val) == 1)
            {
                if ((min != max) && (val >= min && val <= max))
                    isValid = 1;
                else if (min == max)
                    isValid = 1;
                else
                    isValid = 0;
            }
            else isValid = 0;
        }
        else
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            isValid = 0;
        }

        enableRawMode();

        if (!isValid && negativeIfInvalid)
            return -1;

    } while (!isValid);

    if (COL_ENABLED) printf("\033[%sm", COL_RESET);

    return val;
}

/**
 * @return The nav input action detected
 */
enum NavInput getNavInput(void)
{
    int c = getchar();

    if (c == 27)
    {
        getchar();
        switch (getchar())
        {
            case 'A': return CURSOR_UP;
            case 'B': return CURSOR_DOWN;
            case 'C': return DIR_DOWN;
            case 'D': return DIR_UP;
        }
    }
    else
    {
        c = tolower(c);
        switch (c)
        {
            case 'q': return QUIT;
            case 'w': return CURSOR_UP;
            case 'e': return DEBUG;
            case 'a': return DIR_UP;
            case 's': return CURSOR_DOWN;
            case 'd': return DIR_DOWN;
            case 'i': return INSPECT;
            case 'h': return DIR_UP;
            case 'j': return CURSOR_DOWN;
            case 'k': return CURSOR_UP;
            case 'l': return DIR_DOWN;
            case '.': return TOGGLE_HIDDEN;
            case '?': return HELP;
        }
    }

    return INVALID;
}

/**
 * @return winsize struct containing the current terminal size in columns and rows
 */
struct winsize getTerminalSize(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        ws.ws_col = 80;
        ws.ws_row = 24;
    }
    return ws;
}

/**
 * @param currPath Current working directory path
 * @param entry Directory entry to check
 */
int isFileExecutable(char *currPath, struct dirent *entry)
{
    char filePath[PATH_MAX + 256];
    snprintf(filePath, PATH_MAX + 256, "%s/%s", currPath, entry->d_name);
    if (access(filePath, X_OK) == 0) return 1;
    else return 0;
}

/**
 * @param currPath Current working directory path
 * @param entryCount Number of entries in current directory (intended to be used by reference)
 * @return Pointer to one or more dirent structs for each file in the current directory
 */
struct dirent **getDirContents(char *currPath, int *entryCount)
{
    DIR *dir;
    *entryCount = 0;

    if ((dir = opendir(currPath)) != NULL)
    {
        struct dirent *entry;
        struct dirent **entries = NULL;

        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || (!DOTFILES_VISIBLE && entry->d_name[0] == '.'))
                continue;
            if (entry->d_type == DT_REG && isFileExecutable(currPath, entry))
                entry->d_type = DT_EXE;
            entries = realloc(entries, (*entryCount + 1) * sizeof(struct dirent *));
            entries[*entryCount] = malloc(sizeof(struct dirent));
            memcpy(entries[*entryCount], entry, sizeof(struct dirent));
            (*entryCount)++;
        }

        closedir(dir);
        qsort(entries, *entryCount, sizeof(struct dirent*), compareDirName);
        return entries;
    }

    return NULL;
}

int isProgramInstalled(const char *prog)
{
    char *path = getenv("PATH");
    if (!path)
    {
        char cmd[64];
        snprintf(cmd, 64, "%s --version > /dev/null 2>&1", prog);
        return (system(cmd) == 0);
    }

    char *paths = strdup(path);
    char *dir = strtok(paths, ":");
    while (dir)
    {
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, prog);
        if (access(fullpath, X_OK) == 0)
        {
            free(paths);
            return 1;
        }
        dir = strtok(NULL, ":");
    }
    free(paths);
    return 0;
}

/**
 * @param title Header's text string
 * @param body Body's text string
 */
void printGenericScreen(char *title, char *body)
{
    clearScreen();

    if (COL_ENABLED)
    {
        printf("\033[%s;%sm", COL_FOR_BOLD_WHITE, COL_BAK_BLUE);
        int len = printf("%s", title);
        for (size_t i = len; i < TERM_SIZE.ws_col; i++) printf(" ");
        printf("\033[%sm", COL_RESET);

        int lines = formatNewLines(body, TERM_SIZE.ws_col, NULL);
        int availHeight = TERM_SIZE.ws_row - lines - 1;

        printf("%s\n", body);
        for (int i = 1; i < availHeight; i++) printf("\n");
        printf("\033[%s;%sm", COL_FOR_BOLD_WHITE, COL_BAK_BLUE);
        awaitInput();
        printf("\033[%sm", COL_RESET);
    }
    else
    {
        printf("%s\n", title);
        for (int i = 0; i < TERM_SIZE.ws_col; i++) printf("-");

        int lines = formatNewLines(body, TERM_SIZE.ws_col, NULL);
        int availHeight = TERM_SIZE.ws_row - lines - 3;

        printf("%s\n", body);
        for (int i = 1; i < availHeight; i++) printf("\n");
        for (int i = 0; i < TERM_SIZE.ws_col; i++) printf("-");
        awaitInput();
    }
}

/**
 * Prints the directory listing.
 * @param dirContents Pointer to one or more dirent structs for each file in the current directory
 * @param entryCount Number of entries in current directory
 * @param cursor Current line cursor position
 * @param cursorPrev Previous line cursor position
 */
void printDir(struct dirent **dirContents, int entryCount, int cursor, int cursorPrev)
{
    int baseRow = 2;
    int availHeight = TERM_SIZE.ws_row - 2;
    if (!COL_ENABLED)
    {
        baseRow = 3;
        availHeight = TERM_SIZE.ws_row - 4;
    }

    // If directory is empty
    if (!dirContents || entryCount == 0)
    {
        printf("(empty)\n");
        for (int i = 1; i < availHeight; i++) printf("\n");
        return;
    }

    // Viewport offset and clamping for current line cursor
    int offset = (cursor - 1) - (availHeight / 2);
    if (offset < 0) offset = 0;
    if (offset > entryCount - availHeight) offset = entryCount - availHeight;
    if (offset < 0) offset = 0;

    // Viewport offset and clamping for previous line cursor
    int prevOffset = (cursorPrev - 1) - (availHeight / 2);
    if (prevOffset < 0) prevOffset = 0;
    if (prevOffset > entryCount - availHeight) prevOffset = entryCount - availHeight;
    if (prevOffset < 0) prevOffset = 0;

    int inScrolling = (prevOffset != offset);

    // cursorPrev is initialised as 0 to indicate first frame should be drawn
    if (cursorPrev == 0) inScrolling = 1;

    int prevIndex = cursorPrev - 1;
    int currIndex = cursor - 1;

    // If we don't need to scroll, just update the cursor
    if (!inScrolling)
    {
        int rowPrev = baseRow + (prevIndex - offset);
        int rowCurr = baseRow + (currIndex - offset);

        // Remove old line cursor
        printf("\x1b[%d;1H[ ]", rowPrev);

        // Print new line cursor
        printf("\x1b[%d;1H[\033[%sm*\033[%sm]", rowCurr, COL_FOR_CURSOR, COL_RESET);

        // DEBUG
        //printf("\x1b[1;%dH1", TERM_SIZE.ws_col);
        return;
    }

    // DEBUG
    //printf("\x1b[1;%dH0", TERM_SIZE.ws_col);

    int canGoUp = offset > 0;
    int canGoDown = (offset + availHeight) < entryCount;
    int linesPrinted = 0;

    for (int i = offset; i < entryCount && i < offset + availHeight; i++)
    {
        printf("\x1b[K");

        char prefix = '?';
        switch (dirContents[i]->d_type)
        {
            case DT_DIR: prefix = 'd'; break;
            case DT_REG: prefix = 'f'; break;
            case DT_EXE: prefix = 'x'; break;
            case DT_LNK: prefix = 'l'; break;
            case DT_FIFO: prefix = '|'; break;
            case DT_CHR: prefix = 'c'; break;
            case DT_BLK: prefix = 'b'; break;
            case DT_SOCK: prefix = 's'; break;
            default: prefix = '?'; break;
        }

        // Can scroll up indicator
        if (canGoUp && i == offset)
            printf("\033[%sm^\033[%sm\x1b[K\n", COL_FOR_ARROW, COL_RESET);
        // Can scroll down indicator
        else if (canGoDown && i == offset + availHeight - 1)
            printf("\033[%smv\033[%sm\n", COL_FOR_ARROW, COL_RESET);
        // Selected line
        else if (i == currIndex)
            printf("[\033[%sm*\033[%sm] %c %s\n", COL_FOR_CURSOR, COL_RESET, prefix, dirContents[i]->d_name);
        // Other lines
        else
            printf("[ ] %c %s\n", prefix, dirContents[i]->d_name);

        linesPrinted++;
    }

    // "Fill in" lines if listing is shorter than viewport
    if (!canGoUp && !canGoDown)
        for (int i = linesPrinted; i < availHeight; i++)
            printf("\n");
}

void printFooter(void)
{
    if (COL_ENABLED)
        printf("\033[%s;%sm", COL_FOR_BOLD_WHITE, COL_BAK_BLUE);
    else
        for (int i = 0; i < TERM_SIZE.ws_col; i++) printf("-");

    char *inspectStr = "";
    if (FILE_INSTALLED)
        inspectStr = " [i] Inspect";

    char *hiddenStr = " [.] Hidden off";
    if (!DOTFILES_VISIBLE)
        hiddenStr = " [.] Hidden on";

    int len = printf("[hjkl] Navigate%s%s [?] Help [q] Quit ", inspectStr, hiddenStr);

    if (COL_ENABLED)
    {
        for (int i = len; i < TERM_SIZE.ws_col; i++) printf(" ");
        printf("\033[%sm", COL_RESET);
    }
}

/**
 * @param currPath Current working directory path
 */
void printHeader(char *currPath)
{
    if (COL_ENABLED)
        printf("\033[%s;%sm", COL_FOR_BOLD_WHITE, COL_BAK_BLUE);

    size_t dirLen = strlen(currPath);
    if (dirLen <= TERM_SIZE.ws_col) 
    {
        printf("%s", currPath);
        if (COL_ENABLED)
            for (size_t i = dirLen; i < TERM_SIZE.ws_col; i++)
                printf(" ");
        else
            printf("\n");
    }
    else
    {
        size_t visibleLen = TERM_SIZE.ws_col - 3;
        char *start = currPath + (dirLen - visibleLen);
        printf("...%s\n", start);
    }

    if (COL_ENABLED)
        printf("\033[%sm", COL_RESET);
    else
        for (int i = 0; i < TERM_SIZE.ws_col; i++) printf("-");
}

/**
 * @param currPath Current working directory path
 * @param entry Directory entry to inspect
 */
void inspectEntry(char *currPath, struct dirent *entry)
{
    char filePath[PATH_MAX + 256];
    if (strcmp(currPath, "/") == 0)
        snprintf(filePath, PATH_MAX + 256, "/%s", entry->d_name);
    else
        snprintf(filePath, PATH_MAX + 256, "%s/%s", currPath, entry->d_name);

    char cmd[PATH_MAX + 256 + 8];
    snprintf(cmd, PATH_MAX + 256 + 8, "file -b %s", filePath);
    char buffer[2048];
    FILE *stream = popen(cmd, "r");
    if (stream)
    {
        if (fgets(buffer, sizeof(buffer), stream) != NULL)
            buffer[strcspn(buffer, "\n")] = '\0';
        pclose(stream);
        char title[PATH_MAX + 266];
        snprintf(title, PATH_MAX + 266, "Inspect: %s", filePath);
        printGenericScreen(title, buffer);
    }
}

void showCursor(void)
{
    printf("\033[?25h");
    if (COL_ENABLED) printf("\033[%sm", COL_RESET);
}

/**
 * @param currPath Current working directory path
 */
void writeLastDir(char *currDir)
{
    const char *tmpFile = "/tmp/shorkdir_last_dir.txt";
    FILE *stream = fopen(tmpFile, "w");
    if (stream)
    {
        fprintf(stream, "%s\n", currDir);
        fclose(stream);
    }
}

/**
 * @param currPath Current working directory path
 * @param entry Directory entry to open
 */
void openFile(char *currDir, struct dirent *entry)
{
    if (!CODE_INSTALLED &&
        !EMACS_INSTALLED &&
        !FLOW_CTRL_INSTALLED &&
        !GEDIT_INSTALLED &&
        !GTED_INSTALLED &&
        !KATE_INSTALLED &&
        !MG_INSTALLED &&
        !MOUSEPAD_INSTALLED &&
        !NANO_INSTALLED &&
        !NVIM_INSTALLED &&
        !PLUMA_INSTALLED &&
        !VI_INSTALLED &&
        !VIM_INSTALLED &&
        !XED_INSTALLED)
        return;

    char filePath[PATH_MAX + 256];
    snprintf(filePath, PATH_MAX + 256, "%s/%s", currDir, entry->d_name);

    MenuItem menu[] = {
        { "Go back", "", 1 },
        { "Emacs", "emacs", EMACS_INSTALLED },
        { "Flow Control", "flow", FLOW_CTRL_INSTALLED },
        { "gedit", "gedit", GEDIT_INSTALLED },
        { "GNOME Text Editor", "gnome-text-editor", GTED_INSTALLED },
        { "Kate", "kate", KATE_INSTALLED },
        { "Mg", "mg", MG_INSTALLED },
        { "Mousepad", "mousepad", MOUSEPAD_INSTALLED },
        { "nano", "nano", NANO_INSTALLED },
        { "Neovim", "nvim", NVIM_INSTALLED },
        { "Pluma", "pluma", PLUMA_INSTALLED },
        { "vi/Vim", "vi", VI_INSTALLED },
        { "Vim/Neovim", "vim", VIM_INSTALLED },
        { "VS Code", "code", CODE_INSTALLED },
        { "Xed", "xed", XED_INSTALLED }
    };
    int menuSize = sizeof(menu) / sizeof(menu[0]);
    int indices[menuSize];
    int choice;

    for (;;)
    {
        clearScreen();

        if (COL_ENABLED)
        {
            printf("\033[%s;%sm", COL_FOR_BOLD_WHITE, COL_BAK_BLUE);
            int len = printf("Open: %s", filePath);
            for (size_t i = len; i < TERM_SIZE.ws_col; i++) printf(" ");
            printf("\033[%sm\n", COL_RESET);
        }
        else
        {
            printf("Open: %s\n", filePath);
            for (int i = 0; i < TERM_SIZE.ws_col; i++) printf("-");
        }        

        int count = 0;

        for (int i = 0; i < (int)menuSize; i++)
        {
            if (menu[i].visible)
            {
                printf("\033[%sm%d:\033[%sm %s\n", COL_FOR_OL, count + 1, COL_RESET, menu[i].name);
                indices[count++] = i;
            }
        }

        if (COL_ENABLED)
        {
            int availHeight = TERM_SIZE.ws_row - count - 1;
            for (int i = 1; i < availHeight; i++) printf("\n");
            printf("\033[%s;%sm", COL_FOR_BOLD_WHITE, COL_BAK_BLUE);
            choice = getIntInput("Select option", 1, count, 1);
        }
        else
        {
            int availHeight = TERM_SIZE.ws_row - count - 3;
            for (int i = 1; i < availHeight; i++) printf("\n");
            for (int i = 0; i < TERM_SIZE.ws_col; i++) printf("-");
            choice = getIntInput("Select option", 1, count, 1);
        }


        if (choice == -1)
        {
            if (COL_ENABLED)
            {
                printf("\033[0m");
                clearScreen();
            }
            continue;
        }

        break;
    }

    if (choice == 1) return;

    showCursor();
    disableRawMode();
    writeLastDir(currDir);
    clearScreen();

    char *argv[] = { 
        menu[indices[choice - 1]].payload,
        filePath,
        NULL
    };
    execvp(argv[0], argv);
    perror("ERROR: failed to open editor");
    exit(1);
}



int main(int argc, char *argv[])
{
    TERM_SIZE = getTerminalSize();
    if (TERM_SIZE.ws_col < 62 || TERM_SIZE.ws_row < 14)
    {
        perror("ERROR: terminal size too small (must be 62x14 or more)");
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-nc") == 0) || (strcmp(argv[i], "--no-col") == 0))
        {
            COL_ENABLED = 0;
            COL_FOR_ARROW = COL_RESET;
            COL_FOR_CODE = COL_RESET;
            COL_FOR_CURSOR = COL_RESET;
            COL_FOR_HEADING = COL_RESET;
            COL_FOR_OL = COL_RESET;
            continue;
        }
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    atexit(showCursor);
    atexit(disableRawMode);

    CODE_INSTALLED = isProgramInstalled("code");
    EMACS_INSTALLED = isProgramInstalled("emacs");
    FILE_INSTALLED = isProgramInstalled("file");
    FLOW_CTRL_INSTALLED = isProgramInstalled("flow");
    GEDIT_INSTALLED = isProgramInstalled("gedit");
    GTED_INSTALLED = isProgramInstalled("gnome-text-editor");
    KATE_INSTALLED = isProgramInstalled("kate");
    MG_INSTALLED = isProgramInstalled("mg");
    MOUSEPAD_INSTALLED = isProgramInstalled("mousepad");
    NANO_INSTALLED = isProgramInstalled("nano");
    NVIM_INSTALLED = isProgramInstalled("nvim");
    PLUMA_INSTALLED = isProgramInstalled("pluma");
    VI_INSTALLED = isProgramInstalled("vi");
    VIM_INSTALLED = isProgramInstalled("vim");
    XED_INSTALLED = isProgramInstalled("xed");

    char currPath[PATH_MAX];
    size_t currPathLen;
    if (getcwd(currPath, sizeof(currPath)) == NULL)
    {
        perror("ERROR: failed to get current path");
        return 1;
    }

    enableRawMode();
    printf("\033[?25l");

    int running = 1;
    struct dirent **dirContents = NULL;
    int entryCount = 0;
    int cursor = 1;
    int cursorPrev = 0;
    int updateDirContents = 1;
    int fullRedraw = 1;

    char debugScreen[200] = "Term cols: %d, term rows: %d, dir entries: %d, cursor pos: %d";

    char helpScreen[700];
    snprintf(helpScreen, 700, "\033[%smKey binds\033[%sm\n\033[%sm[H/A/left]\033[%sm up directory \033[%sm[J/S/down]\033[%sm cursor down \033[%sm[K/W/up]\033[%sm cursor up \033[%sm[L/D/right]\033[%sm open directory/file \033[%sm[i]\033[%sm inspect selected (if file installed) \033[%sm[.]\033[%sm toggle hidden entires \033[%sm[h]\033[%sm show help \033[%sm[q]\033[%sm quit\n\n\033[%smEntry types\033[%sm\n\033[%sm'd'\033[%sm directory \033[%sm'f'\033[%sm regular file \033[%sm'x'\033[%sm executable file \033[%sm'b'\033[%sm block device \033[%sm'c'\033[%sm character device \033[%sm'l'\033[%sm symbolic link \033[%sm's'\033[%sm UNIX domain socket \033[%sm'|'\033[%sm named pipe (FIFO) \033[%sm'?'\033[%sm unknown", COL_FOR_HEADING, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_HEADING, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET, COL_FOR_CODE, COL_RESET);

    while (running)
    {
        if (updateDirContents)
        {
            if (dirContents)
            {
                for (int i = 0; i < entryCount; i++) free(dirContents[i]);
                free(dirContents);
            }

            dirContents = getDirContents(currPath, &entryCount);
            currPathLen = strlen(currPath);
            if (dirContents == NULL && entryCount > 0)
            {
                perror("ERROR: cannot get directory contents");
                disableRawMode();
                return 1;
            }
            updateDirContents = 0;
        }

        if (fullRedraw)
        {
            clearScreen();
            printHeader(currPath);
            printDir(dirContents, entryCount, cursor, cursorPrev);
            printFooter();
        }
        else
        {
            if (COL_ENABLED)
                printf("\x1b[2;1H");
            else
                printf("\x1b[3;1H");
            printDir(dirContents, entryCount, cursor, cursorPrev);
        }

        enum NavInput input = getNavInput();

        fullRedraw = 1;
        cursorPrev = 0;
        switch (input)
        {
            case CURSOR_UP:
                cursorPrev = cursor;
                cursor--;
                if (cursor < 1) cursor = entryCount;
                fullRedraw = 0;
                break;

            case CURSOR_DOWN:
                cursorPrev = cursor;
                cursor++;
                if (cursor > entryCount) cursor = 1;
                fullRedraw = 0;
                break;

            case DIR_UP:
                if (currPathLen > 1 && currPath[currPathLen - 1] == '/')
                    currPath[currPathLen - 1] = '\0';
                char *lastSlash = strrchr(currPath, '/');
                if (lastSlash)
                {
                    if (lastSlash != currPath)
                        *lastSlash = '\0';
                    else
                        currPath[1] = '\0';
                }
                updateDirContents = cursor = 1;
                break;

            case DEBUG:
                char debugMsgProcessed[200];
                snprintf(debugMsgProcessed, 200, debugScreen, TERM_SIZE.ws_col, TERM_SIZE.ws_row, entryCount, cursor);
                printGenericScreen("Debug", debugMsgProcessed);
                break;

            case DIR_DOWN:
                if (entryCount > 0)
                {
                    if (dirContents[cursor - 1]->d_type == DT_REG)
                        openFile(currPath, dirContents[cursor - 1]);
                    else if (dirContents[cursor - 1]->d_type == DT_DIR)
                    {
                        size_t entryLen = strlen(dirContents[cursor - 1]->d_name);
                        if (currPathLen + entryLen + 1 >= PATH_MAX) break;

                        if (strcmp(currPath, "/") != 0)
                        {
                            currPath[currPathLen] = '/';
                            strcpy(currPath + currPathLen + 1, dirContents[cursor - 1]->d_name);
                        }
                        else strcpy(currPath + 1, dirContents[cursor - 1]->d_name);  

                        updateDirContents = cursor = 1;
                    }
                }
                break;

            case INSPECT:
                if (FILE_INSTALLED && entryCount > 0)
                    inspectEntry(currPath, dirContents[cursor - 1]);
                break;
                
            case TOGGLE_HIDDEN:
                DOTFILES_VISIBLE = !DOTFILES_VISIBLE;
                cursor = updateDirContents = 1;
                break;

            case HELP:
                printGenericScreen("Help", helpScreen);
                break;

            case QUIT:
                running = 0;
                break;
        }
    }

    if (dirContents)
    {
        for (int i = 0; i < entryCount; i++) free(dirContents[i]);
        free(dirContents);
    }

    writeLastDir(currPath);
    clearScreen();
    return 0;  
}
