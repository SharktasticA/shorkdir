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



static struct termios oldTERMIO;
static int rawModeEnabled = 0;



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
    INVALID
};



/**
 * Awaits for any user input.
 */
void awaitInput(void)
{
    printf("Press any key to continue... ");
    getchar();
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
    if (rawModeEnabled)
        tcsetattr(STDIN_FILENO, TCSANOW, &oldTERMIO);
}

/**
 * Disables the terminal's canonical input so that things like getchar do not
 * wait until enter is pressed.
 */
void enableRawMode(void)
{
    struct termios newTERMIO;
    tcgetattr(STDIN_FILENO, &oldTERMIO);
    newTERMIO = oldTERMIO;
    newTERMIO.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTERMIO);
    rawModeEnabled = 1;
}

/**
 * Adds new lines to a given string based on the requested line width.
 * @param buffer Input string
 * @param width Characters per line
 * @return Number of lines in the string
 */
int formatNewLines(char *buffer, int width)
{
    if (!buffer || width < 1) return 0;

    size_t bufferStrLen = strlen(buffer);
    int lines = 1;
    int lastSpace = -1;
    int widthCount = 1;
    for (int i = 0; i < bufferStrLen; i++)
    {
        if (buffer[i] == ' ') lastSpace = i;
        else if (buffer[i] == '\n')
        {
            lines++;
            widthCount = 0;
        }

        if (widthCount == width)
        {
            if (buffer[i] == ' ') buffer[i] = '\n';
            else if (lastSpace != -1) buffer[lastSpace] = '\n';
            widthCount = 0;
            lines++;
        }

        widthCount++;
    }

    return lines;
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
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
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
 * @param termSize winsize struct containing the current terminal size
 * @param dirContents Pointer to one or more dirent structs for each file in the current directory
 * @param entryCount Number of entries in current directory
 * @param cursor Current line/row cursor position
 */
void printDir(struct winsize termSize, struct dirent **dirContents, int entryCount, int cursor)
{
    int availHeight = termSize.ws_row - 4;

    if (!dirContents || entryCount == 0)
    {
        printf("(empty)\n");
        for (int i = 1; i < availHeight; i++)
            printf("\n");
        return;
    }

    int offset = 0;
    if (cursor >= availHeight)
        offset = cursor - availHeight + 1;
    if (offset > entryCount - availHeight)
        offset = entryCount - availHeight;
    if (offset < 0)
        offset = 0;

    int canGoUp = offset > 0;
    int canGoDown = (offset + availHeight) < entryCount;
    int linesPrinted = 0;

    for (int i = offset; i < entryCount && i < offset + availHeight; i++)
    {
        char prefix = '?';
        switch (dirContents[i]->d_type)
        {
            case DT_DIR:    prefix = 'd'; break;
            case DT_REG:    prefix = 'f'; break;
            case DT_LNK:    prefix = 'l'; break;
            case DT_FIFO:   prefix = '|'; break;
            case DT_CHR:    prefix = 'c'; break;
            case DT_BLK:    prefix = 'b'; break;
            case DT_SOCK:   prefix = 's'; break;
            case DT_UNKNOWN:
            default:
                prefix = '?';
                break;
        }

        if (canGoUp && i == offset)
            printf("↑\n");
        else if (canGoDown && i == offset + availHeight - 1)
            printf("↓\n");
        else if (i == cursor - 1)
            printf("[*] %c %s\n", prefix, dirContents[i]->d_name);
        else
            printf("[ ] %c %s\n", prefix, dirContents[i]->d_name);

        linesPrinted++;
    }

    if (!canGoUp && !canGoDown)
        for (int i = linesPrinted; i < availHeight; i++)
            printf("\n");
}

/**
 * @param termSize winsize struct containing the current terminal size in columns and rows
 */
void printFooter(struct winsize termSize)
{
    for (int i = 0; i < termSize.ws_col; i++)
        printf("-");
    printf("[hjkl] Navigate  [i] Inspect  [?] Help  [q] Quit ");
}

/**
 * @param termSize winsize struct containing the current terminal size in columns and rows
 */
void printHelp(struct winsize termSize)
{
    system("clear");
    printf("Help\n");
    for (int i = 0; i < termSize.ws_col; i++) printf("-");
    
    char help[400] = "Key binds:\n[H/A/left] up directory [J/S/down] cursor down [K/W/up] cursor up [L/D/right] down directory [i] inspect selected (if file installed) [h] show help [q] quit\n\nEntry types:\n'd' directory 'f' regular file 'b' block device 'c' character device 'l' symbolic link 's' UNIX domain socket '|' named pipe (FIFO) '?' unknown";
    int lines = formatNewLines(help, termSize.ws_col);
    int availHeight = termSize.ws_row - lines - 3;
    printf("%s\n", help);
    for (int i = 1; i < availHeight; i++) printf("\n");
    
    for (int i = 0; i < termSize.ws_col; i++) printf("-");
    awaitInput();
}

/**
 * @param termSize winsize struct containing the current terminal size
 * @param currPath Current working directory path
 */
void printHeader(struct winsize termSize, char *currPath)
{
    size_t dirLen = strlen(currPath);
    if (dirLen <= termSize.ws_col) printf("%s\n", currPath);
    else
    {
        size_t visibleLen = termSize.ws_col - 3;
        char *start = currPath + (dirLen - visibleLen);
        printf("...%s\n", start);
    }
    
    for (int i = 0; i < termSize.ws_col; i++)
        printf("-");
}

/**
 * @param termSize winsize struct containing the current terminal size
 * @param currPath Current working directory path
 */
void inspectEntry(struct winsize termSize, char *currPath, struct dirent *entry)
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
    }
    else return;

    system("clear");
    printHeader(termSize, filePath);
    int lines = formatNewLines(buffer, termSize.ws_col);
    int availHeight = termSize.ws_row - lines - 3;
    printf("%s\n", buffer);
    for (int i = 1; i < availHeight; i++) printf("\n");
    for (int i = 0; i < termSize.ws_col; i++) printf("-");
    awaitInput();
}



int main(void)
{
    atexit(disableRawMode);

    struct winsize termSize = getTerminalSize();
    if (termSize.ws_col < 50 || termSize.ws_row < 16)
    {
        perror("ERROR: terminal size too small (must be 50x16 or more)");
        return 1;
    }

    char currPath[PATH_MAX];
    size_t currPathLen;
    if (getcwd(currPath, sizeof(currPath)) == NULL)
    {
        perror("ERROR: getcwd");
        return 1;
    }

    enableRawMode();
    int running = 1;
    struct dirent **dirContents = NULL;
    int entryCount = 0;
    int cursor = 1;
    int updateDirContents = 1;

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

        system("clear");
        printHeader(termSize, currPath);
        printDir(termSize, dirContents, entryCount, cursor);
        printFooter(termSize);

        enum NavInput input = getNavInput();

        switch (input)
        {
            case CURSOR_UP:
                cursor--;
                if (cursor < 1) cursor = entryCount;
                break;

            case CURSOR_DOWN:
                cursor++;
                if (cursor > entryCount) cursor = 1;
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

            case DIR_DOWN:
                if (entryCount > 0)
                {
                    if (dirContents[cursor - 1]->d_type != DT_DIR)
                        break;
                    size_t entryLen = strlen(dirContents[cursor - 1]->d_name);
                    if (currPathLen + entryLen + 1 >= PATH_MAX)
                        break;
                    if (strcmp(currPath, "/") != 0)
                    {
                        currPath[currPathLen] = '/';
                        strcpy(currPath + currPathLen + 1, dirContents[cursor - 1]->d_name);
                    }
                    else strcpy(currPath + 1, dirContents[cursor - 1]->d_name);                
                    updateDirContents = cursor = 1;
                }
                break;

            case INSPECT:
                inspectEntry(termSize, currPath, dirContents[cursor - 1]);
                break;

            case HELP:
                printHelp(termSize);
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

    printf("\n");
    return 0;   
}
