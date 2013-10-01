#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


typedef struct lgat_watcher {
    char *path;
    int watchDescriptor;
} lgat_watcher;


lgat_watcher create_lgat_watcher (char *path, int fileDescriptor) {
    //first create the watch descriptor for the path
    int watchDescriptor = inotify_add_watch( fileDescriptor, path, IN_MODIFY );

    if (watchDescriptor < 0) {
        perror( "open file" );
        exit(1);
    }

    lgat_watcher w = { path, watchDescriptor };
    return w;
}

void destroy_lgat_watcher (lgat_watcher w, int fileDescriptor) {
    ( void ) inotify_rm_watch( fileDescriptor, w.watchDescriptor );
}

int main (int argc, char **argv) { 
    int length, i = 0, j = 0, watcherCount = 2;
    int fileDescriptor;
    char buffer[BUF_LEN];
    
    fileDescriptor = inotify_init();
 
    if (fileDescriptor < 0)  {
        perror( "open file" );
        return 1;
    }
 
    char *filesToWatch[watcherCount]; 
    //memset( filesToWatch, 0, watcherCount * sizeof(lgat_watcher) );
    filesToWatch[0] = "/tmp/test_file";
    filesToWatch[1] = "/tmp/test_file2";
    
    lgat_watcher watchers[watcherCount];
    
    for (j = 0; j < watcherCount; j++) {
        watchers[j] = create_lgat_watcher(filesToWatch[j], fileDescriptor);
    }

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
                        if (watchers[j].watchDescriptor == event->wd) { 
                            printf("%s\n", watchers[j].path);
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
