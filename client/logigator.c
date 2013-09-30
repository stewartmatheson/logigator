#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

int main (int argc, char **argv) { 
    int length, i = 0;
    int fileDescriptor;
    int watchDescriptor;
    char buffer[BUF_LEN];

    fileDescriptor = inotify_init();
    if (fileDescriptor < 0)  {
        perror( "open file" );
        return 1;
    }

    watchDescriptor = inotify_add_watch( fileDescriptor, "/tmp/test_file", IN_MODIFY );
    if (watchDescriptor < 0) {
        perror( "open file" );     
        return 1;
    }

    while (1) {
        i = 0; // reset read iterator

        length = read( fileDescriptor, buffer, BUF_LEN );
        if ( length < 0 ) {
            perror( "read" );     
            return 1;
        }

        while ( i <= length ) {
            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];    

            printf("Length: %i\n", length);
            printf("i: %i\n", i);
            if ( event->mask & IN_MODIFY ) {
                printf("The file %s has been changed\n", event->name);
            }
            
            i += EVENT_SIZE + event->len;
        }
    
    } 

    ( void ) inotify_rm_watch( fileDescriptor, watchDescriptor );
    ( void ) close( fileDescriptor );
    return 0;
} 
