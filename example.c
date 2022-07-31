#include <stdio.h>

#include "logger.h"
        
int
main(int argc, char **argv)
{
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
        log_object(LOG_COPY, 
            object_array("data", proot)
        );
        
        char *ptr = JSON_DUMPS(root);
        printf("LOG: %s\n", ptr);
        free(ptr);

        sleep(1); 
    }
    json_decref(proot);
    log_close();
    return 0;  
}
