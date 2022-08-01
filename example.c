#include <stdio.h>

#include "logger.h"
        
void *write_message_function(void *ptr);

int
main(int argc, char **argv)
{
    pthread_t thread1, thread2;
    char *message1 = "Thread 1";
    char *message2 = "Thread 2";
    int  iret1, iret2;
    iret1 = pthread_create( &thread1, NULL, write_message_function, (void*) message1);
    iret2 = pthread_create( &thread2, NULL, write_message_function, (void*) message2);
    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL); 

    printf("Thread 1 returns: %d\n",iret1);
    printf("Thread 2 returns: %d\n",iret2);
    return 0;  
}

void *write_message_function(void *ptr){
    int brk_pnt = 0;
    log_init("log");
    JSON_STRUCT proot = JSON_ARRAY();
    while(brk_pnt++ < 5) {    
        JSON_STRUCT root = 
        object(
            LOG_KEEP,
            object_string("msg", "records added successfully"), 
            object_double("count", 2.2),
                object_object("test",
                    array(LOG_KEEP,
                        array_double(
                            1.1
                        ),
                        array_object(
                            object(
                                LOG_KEEP,
                                object_object("test", 
                                    object(
                                        LOG_KEEP,
                                        object_string("msg", "records added successfully"), 
                                        object_int("count", 2)
                                    )
                                )
                            )
                        ),
                        array_object(
                            object(
                                LOG_KEEP,
                                object_object("test", 
                                    object(
                                        LOG_KEEP,
                                        object_string("msg", "records added successfully"), 
                                        object_int("count", 2)
                                    )
                                )
                            )
                        )
                    )
                ),
                object_object("test",
                    array(LOG_KEEP,
                        array_double(
                            1.1
                        ),
                        array_object(
                            object(
                                LOG_KEEP,
                                object_object("test", 
                                    object(
                                        LOG_KEEP,
                                        object_string("msg", "records added successfully"), 
                                        object_int("count", 2)
                                    )
                                )
                            )
                        ),
                        array_object(
                            object(
                                LOG_KEEP,
                                object_object("test", 
                                    object(
                                        LOG_KEEP,
                                        object_string("msg", "records added successfully"), 
                                        object_int("count", 2)
                                    )
                                )
                            )
                        )
                    )
                )
            );

        json_array_add(proot, root);

        // object to log
        log_object(LOG_COPY, 
            object_array("data", proot)
        );

        // array to log
        // log_array(LOG_COPY, 
        //     array_array(proot)
        // );
        
        char *ptr = JSON_DUMPS(proot);
        printf("LOG: %d %s\n", pthread_self(), ptr);
        free(ptr);

        usleep(100); 
    }
    json_decref(proot);
    log_close();
}