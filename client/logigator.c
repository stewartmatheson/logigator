#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/types.h>
#include <linux/inotify.h>


#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

typedef struct lgat_watcher {
    char *path;
    int watchDescriptor;
    int lineNumber;
} lgat_watcher;

// Call back to the server once we know we have log changes to report
void call_home(lgat_watcher *changed_watcher) {
        
}

// Called if any of the watched files have changed. This is where 
// we should handle multi line logs and non descreet writes
void on_file_change(lgat_watcher *changed_watcher) {
    int current_line_count = lineCount(changed_watcher->path);

    printf("%s %i\n", changed_watcher->path, changed_watcher->lineNumber);

    if (current_line_count == changed_watcher->lineNumber + 1)
        call_home(changed_watcher);

    changed_watcher->lineNumber = lineCount(changed_watcher->path);
}

// How many watches the client is currently concernred with
int watcher_count() {
    int watcherCounter = 0;
    FILE * filePointer;
    int c;

    filePointer = fopen("/etc/logigator.conf", "r");
    if (filePointer == NULL) {
        printf("Can't open configuration file\n");
        exit(1);
    }
    // seems liek the best way to count lines
    while ( EOF != (c = fgetc(filePointer)) ) {
        if ( c == '\n' )
            ++watcherCounter;
    }

    close(filePointer);
    return watcherCounter;
} 

// Count how many lines are in a given file
int lineCount(char *path) {
    int lines = 0;
    FILE * filePointer;
    int c;

    filePointer = fopen(path, "r");
    if (filePointer == NULL) {
        printf("Can't open: %s\n", path);
        exit(1);
    }
    // seems liek the best way to count lines
    while ( EOF != (c = fgetc(filePointer)) ) {
        if ( c == '\n' )
            ++lines;
    }

    close(filePointer);
    return lines;
} 

// Create a new watcher.
int init_watcher (lgat_watcher **buf, char *path, int fileDescriptor) {
    int watchDescriptor = inotify_add_watch( fileDescriptor, path, IN_MODIFY );
    if (watchDescriptor < 0) {
        perror( "open file" );
        return -1;
    }

    (*buf)->path = strdup(path);
    (*buf)->watchDescriptor = watchDescriptor;
    (*buf)->lineNumber = lineCount(path);
    return 0;
}

// Free an old watcher. Not used: cant see a use
void destroy_lgat_watcher (lgat_watcher w, int fileDescriptor) {
    ( void ) inotify_rm_watch( fileDescriptor, w.watchDescriptor );
}

// Load the configruation and then create all watchers 
void create_watchers_from_configuration(lgat_watcher *buffer[], int fileDescriptor) {
    int i = 0;
    FILE * filePointer;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int success = 0;

    filePointer = fopen("/etc/logigator.conf", "r");
    if (filePointer == NULL) {
        printf("can't open configuration file\n");
        exit(1);
    }

    // now lets loop of the file get the lines and create the watchers
    while((read = getline(&line, &len, filePointer)) != -1) {
        int lineLength = strlen(line);
        if( line[lineLength-1] == '\n' ) {
            line[lineLength-1] = '\0';
        }
        
        if(init_watcher(&buffer[i], line, fileDescriptor) < 0) {
            printf("can't create watcher for: %s \n", buffer[i]->path);
        }
        ++i;
    }
    close(filePointer);
}

int main (int argc, char **argv) { 
    int length = 0, i = 0, j = 0, k = 0,  watcherCount = watcher_count();
    int fileDescriptor;
    char buffer[BUF_LEN];
    fileDescriptor = inotify_init();
    if (fileDescriptor < 0)  {
        perror( "open file" );
        return 1;
    }

    lgat_watcher *watchers[watcherCount];
    for (k = 0; k < watcherCount; k++)
        watchers[k] = malloc(sizeof(lgat_watcher));
   
    create_watchers_from_configuration(watchers, fileDescriptor);    

    while (1) {
        i = 0; // reset read iterator

        length = read( fileDescriptor, buffer, BUF_LEN );
        if ( length < 0 ) {
            perror( "read" );     
            return 1;
        }

        while ( i <= length ) {
            // first create the struct form the buffer
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];     

            // filter only by the modify event
            if ( event->mask & IN_MODIFY) {
                if (i == 0) { // only fire the write event once 
                    // now lets check the watchers and work out what watcher fired
                    for (j = 0; j < watcherCount; j++) {
                        if (watchers[j]->watchDescriptor == event->wd) { 
                            on_file_change(watchers[j]);
                        }
                    }
                }
            }
            i += EVENT_SIZE + event->len;
        }
    } 
    
    ( void ) close( fileDescriptor );
    return 0;
} 
