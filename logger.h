/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Brian J. Downs
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _LOGGER_H
#define _LOGGER_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_JANSSON
/**
 * if lib jansson was included in
 */    
#include <jansson.h>

#define JSON_OBJECT_ADD(key, x)     json_object_set_new(root, key, x)
#define JSON_ARRAY_ADD(x)           json_array_append_new(root, x)
#define JSON_INTEGER(x)             json_integer(x)
#define JSON_STRING(x)              json_string(x)
#define JSON_REAL(x)                json_real(x)
#define JSON_OBJECT()               json_object()
#define JSON_ARRAY()                json_array()
#define JSON_TRUE()                 json_true()
#define JSON_FALSE()                json_false()
#define JSON_DUMPS(x)               json_dumps(x, 0)
#define JSON_DECREF(x)              json_decref(x)
#define JSON_IS_OBJECT(x)           json_is_object(x)
#define JSON_IS_ARRAY(x)            json_is_array(x)
#define JSON_IS_INTEGER(x)          json_is_integer(x)
#define JSON_IS_STRING(x)           json_is_string(x)
#define JSON_IS_BOOLEAN(x)          json_is_boolean(x)
#define JSON_IS_REAL(x)             json_is_real(x)
#define JSON_COPY(x)                json_deep_copy(x)
#define JSON_STRUCT                 json_t*

#endif


#ifdef THREAD_ENABLE
/**
 * if thread is enable
 */   
#include <pthread.h>

static pthread_mutex_t lock_edit_log = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

#endif
/**
 * object_field_types is an enum of the supported log field types.
 */
static enum { 
    LOG_INT, 
    LOG_REAL, 
    LOG_STRING,
    LOG_ARRAY,
    LOG_OBJECT,
    LOG_BOOLEAN,
    LOG_JSON,
    LOG_OTHER
} object_field_types;
        
static enum { 
    LOG_KEEP, 
    LOG_COPY
} object_behave_types;

static enum { 
    LOG_NO_ACTION,
    LOG_OPEN, 
    LOG_CLOSE,
    LOG_FAIL
} log_behave_types;
/**
 * object_field_t represents a field in a log entry and it's
 * associated type.
 */
struct object_field_t {
    uint8_t type;
    char *key;
    JSON_STRUCT json_any;
} object_field_t;

struct array_field_t {
    uint8_t type;
    JSON_STRUCT json_any;
} array_field_t;

struct any_type_t {
    enum {
      TYPE_UNDEFINED,
      TYPE_INT,      
      TYPE_STRING,
      TYPE_REAL,
      TYPE_JSON
    } iType;
    union
    {
        int i;
        char* s;
        double r;
        JSON_STRUCT j;
    } value;
} any_type_t;

/**
 * log_output contains the location we're
 * going to write our log entries to
 */
static FILE *log_output = NULL;

enum {
    LOG_OUT_STDERR,
    LOG_OUT_STDOUT,
};

int 
check_json_type(JSON_STRUCT root);
/**
 * reallog provides the functionality of the logger. Returns
 * the number os bytes written.
 */
int
reallogobject(int behave_type, ...);


int
reallogarray(int behave_type, ...);
/**
 * reallobject provides the functionality of the logger. Returns
 * the number os bytes written.
 */
JSON_STRUCT 
reallobject(int behave_type, ...);

/**
 * reallarray provides the functionality of the logger. Returns
 * the number os bytes written.
 */
JSON_STRUCT 
reallarray(int behave_type, ...);

/**
 * log is the main entry point for adding data
 * to the logger to create log entries
 */
#define log_object(x, ...) ({ reallogobject(x, __VA_ARGS__, NULL); })

#define log_array(x, ...) ({ reallogarray(x, __VA_ARGS__, NULL); })

#define object(x, ...) ({ reallobject(x, __VA_ARGS__, NULL); })

#define array(x, ...) ({ reallarray(x, __VA_ARGS__, NULL); })
/**
 * log_init initializes the logger and sets up
 * where the logger writes to.
 */
int
log_init(const char* file_name)
{
    int wc;
#ifdef THREAD_ENABLE
    while(pthread_mutex_lock(&lock_edit_log) != 0){ usleep(1); }
#endif
    if(!log_output){
        log_output = fopen(file_name, "a+");
        wc = (log_output != NULL) ? LOG_OPEN : LOG_FAIL;
    }
    else
        wc = LOG_NO_ACTION;
#ifdef THREAD_ENABLE
    pthread_mutex_unlock(&lock_edit_log);
#endif
    return wc;
}

int
log_close()
{
    int wc;
#ifdef THREAD_ENABLE
    while(pthread_mutex_lock(&lock_edit_log) != 0){ usleep(1); }
#endif
    if(log_output){
        int close_con = fclose(log_output);
        wc = (close_con == NULL) ? LOG_CLOSE : LOG_FAIL;
        log_output = NULL;
    }
    else
        wc = LOG_NO_ACTION;
#ifdef THREAD_ENABLE
    pthread_mutex_unlock(&lock_edit_log);
#endif
    return wc;
}

/**
 * object_field_new allocates memory for a new log field,
 * sets the memory to 0, and returns a pointer to it.
 */
static struct object_field_t*
object_field_new(const char *key)
{
    struct object_field_t *field = 
        (struct object_field_t *)calloc(1, sizeof(object_field_t));
    if (field == NULL) {
        perror("unable to allocation memory for new field");
        return NULL;
    }
    memset(field, 0, sizeof(object_field_t));
    field->key = (char *)calloc(1, strlen(key)+1);
    strcpy(field->key, key);
    return field;
}

static struct array_field_t*
array_field_new()
{
    struct array_field_t *field = 
        (struct array_field_t *)calloc(1, sizeof(array_field_t));
    if (field == NULL) {
        perror("unable to allocation memory for new field");
        return NULL;
    }
    memset(field, 0, sizeof(array_field_t));\
    return field;
}
/**
 * object_field_free frees the memory used by the object_field_t.
 * struct.
 */
static void
object_field_free(struct object_field_t *sf)
{
    if (sf != NULL) {
        if (sf->key != NULL) {
            free(sf->key);
        }
        // if ((sf->type == LOG_STRING) && (sf->char_value != NULL)) {
        //     free(sf->char_value);
        // }
        free(sf);
    }
}

static void
array_field_free(struct array_field_t *sf)
{
    if (sf != NULL) {
        // if ((sf->type == LOG_STRING) && (sf->char_value != NULL)) {
        //     free(sf->char_value);
        // }
        free(sf);
    }
}
/**
 * object_int is used to add an integer value
 * to the log entry.
 */
struct object_field_t*
object_int(const char *key, const int value)
{
    struct object_field_t *field = object_field_new(key);
    field->type = LOG_INT;
    field->json_any = JSON_INTEGER(value);
    return field;
}

/**
 * object_double is used to add a double to the
 * log entry.
 */
struct object_field_t*
object_double(const char *key, const double value)
{
    struct object_field_t *field = object_field_new(key);
    field->type = LOG_REAL;
    field->json_any = JSON_REAL(value);
    return field;
}

/**
 * object_string is used to add a string to the
 * log entry.
 */
struct object_field_t*
object_string(const char *key, const char *value)
{
    struct object_field_t *field = object_field_new(key);
    field->type = LOG_STRING;
    field->json_any = JSON_STRING(value);
    return field;
}

/**
 * object_array is used to add a array to the
 * log entry.
 */
struct object_field_t*
object_array(const char *key, JSON_STRUCT array)
{
    struct object_field_t *field = object_field_new(key);
    field->type = LOG_ARRAY;
    field->json_any = array;
    return field;
}

/**
 * object_object is used to json object a array to the
 * log entry.
 */
struct object_field_t*
object_object(const char *key, JSON_STRUCT object)
{
    struct object_field_t *field = object_field_new(key);
    field->type = LOG_OBJECT;
    field->json_any = object;
    return field;
}

/**
 * object_any_json is used to json object a array to the
 * log entry.
 */
struct object_field_t*
object_any_json(const char *key, JSON_STRUCT object)
{
    switch(check_json_type(object)){
        case LOG_ARRAY:
            return object_array(key, object);
        case LOG_OBJECT:
            return object_object(key, object);
        default:
            return NULL;
    }
}

/**
 * object_array is used to add a array to the
 * log entry.
 */

#define object_any(key, x) _Generic((x),  \
        int: object_int(key, x),              \
        double: object_double(key, x),              \
        char *: object_string(key, x),        \
        JSON_STRUCT: object_any_json(key, x),        \
        default: NULL)


// array_func

/**
 * array_int is used to add an integer value
 * to the log entry.
 */
struct array_field_t*
array_int(const int value)
{
    struct array_field_t *field = array_field_new();
    field->type = LOG_INT;
    field->json_any = JSON_INTEGER(value);
    return field;
}

/**
 * array_double is used to add a double to the
 * log entry.
 */
struct array_field_t*
array_double(const double value)
{
    struct array_field_t *field = array_field_new();
    field->type = LOG_REAL;
    field->json_any = JSON_REAL(value);
    return field;
}

/**
 * array_string is used to add a string to the
 * log entry.
 */
struct array_field_t*
array_string(const char *value)
{
    struct array_field_t *field = array_field_new();
    field->type = LOG_STRING;
    field->json_any = JSON_STRING(value);
    return field;
}

/**
 * array_array is used to add a array to the
 * log entry.
 */
struct array_field_t*
array_array(JSON_STRUCT array)
{
    struct array_field_t *field = array_field_new();
    field->type = LOG_ARRAY;
    field->json_any = array;
    return field;
}

/**
 * array_object is used to json object a array to the
 * log entry.
 */
struct array_field_t*
array_object(JSON_STRUCT object)
{
    struct array_field_t *field = array_field_new();
    field->type = LOG_OBJECT;
    field->json_any = object;
    return field;
}

/**
 * array_any is used to json object a array to the
 * log entry.
 */
struct array_field_t*
array_any_json(JSON_STRUCT object)
{
    switch(check_json_type(object)){
        case LOG_ARRAY:
            return array_array(object);
        case LOG_OBJECT:
            return array_object(object);
        default:
            return NULL;
    }
}

#define array_any(x) _Generic((x),  \
        int: array_int(x),              \
        double: array_double(x),              \
        char *: array_string(x),        \
        JSON_STRUCT: array_any_json(x),        \
        default: NULL)

int
reallogobject(int behave_type, ...)
{
    va_list ap;
    int wc;
    unsigned long now = (unsigned long)time(NULL); // UNIX timestamp format

    JSON_STRUCT root = JSON_OBJECT();
    JSON_OBJECT_ADD("timestamp", json_integer(now));

#ifdef THREAD_ENABLE
    JSON_OBJECT_ADD("thread_id", json_integer(pthread_self()));
#endif

    va_start(ap, behave_type);

    for (int i = 1;; i++) {
        struct object_field_t *arg = va_arg(ap, struct object_field_t*);
        if (arg == NULL) {
            break;
        }
        switch(behave_type){
            case LOG_KEEP:
                JSON_OBJECT_ADD(arg->key, arg->json_any);
            break;
            case LOG_COPY:
                JSON_OBJECT_ADD(arg->key, JSON_COPY(arg->json_any));
            break;
            default:
            break;
        }   
        object_field_free(arg);
        continue;
    }

    va_end(ap); 

    char* json_2_str = JSON_DUMPS(root);

#ifdef THREAD_ENABLE
    while(pthread_mutex_lock(&lock_edit_log) != 0){ usleep(1); }
#endif
    if(log_output){
        fflush(log_output);
        wc = fprintf(log_output, "%s\n", json_2_str);
    }
    else
        wc = LOG_NO_ACTION;
#ifdef THREAD_ENABLE
    pthread_mutex_unlock(&lock_edit_log);
#endif

    free(json_2_str);
    
    JSON_DECREF(root); // decrement the count on the JSON object

    // if (strcmp(l, LOG_FATAL) == 0) {
    //     exit(1);
    // }

    return wc;
}

int
reallogarray(int behave_type, ...)
{
    va_list ap;
    int wc;
    JSON_STRUCT root = JSON_ARRAY();

    va_start(ap, behave_type);

    for (int i = 1;; i++) {
        struct array_field_t *arg = va_arg(ap, struct array_field_t*);
        if (arg == NULL) {
            break;
        }
        switch(behave_type){
            case LOG_KEEP:
                JSON_ARRAY_ADD(arg->json_any);
            break;
            case LOG_COPY:
                JSON_ARRAY_ADD(JSON_COPY(arg->json_any));
            break;
            default:
            break;
        }  
        array_field_free(arg);
        continue;
    }

    va_end(ap); 

    char* json_2_str = JSON_DUMPS(root);

#ifdef THREAD_ENABLE
    while(pthread_mutex_lock(&lock_edit_log) != 0){ usleep(1); }
#endif
    if(log_output){
        fflush(log_output);
        wc = fprintf(log_output, "%s\n", json_2_str);
    }
    else
        wc = LOG_NO_ACTION;
#ifdef THREAD_ENABLE
    pthread_mutex_unlock(&lock_edit_log);
#endif

    free(json_2_str);

    JSON_DECREF(root); // decrement the count on the JSON object

    // if (strcmp(l, LOG_FATAL) == 0) {
    //     exit(1);
    // }

    return wc;
}

JSON_STRUCT 
reallobject(int behave_type, ...)
{
    va_list ap;

    JSON_STRUCT root = JSON_OBJECT();

    va_start(ap, behave_type);

    for (int i = 1;; i++) {
        struct object_field_t *arg = va_arg(ap, struct object_field_t*);
        if (arg == NULL) {
            break;
        }
        switch(behave_type){
            case LOG_KEEP:
                JSON_OBJECT_ADD(arg->key, arg->json_any);
            break;
            case LOG_COPY:
                JSON_OBJECT_ADD(arg->key, JSON_COPY(arg->json_any));
            break;
            default:
            break;
        }   
        object_field_free(arg);
        continue;
    }

    va_end(ap); 

    return root;
}

JSON_STRUCT 
reallarray(int behave_type, ...)
{
    va_list ap;

    JSON_STRUCT root = JSON_ARRAY();

    va_start(ap, behave_type);

    for (int i = 1;; i++) {
        struct array_field_t *arg = va_arg(ap, struct array_field_t*);
        if (arg == NULL) {
            break;
        }
        switch(behave_type){
            case LOG_KEEP:
                JSON_ARRAY_ADD(arg->json_any);
            break;
            case LOG_COPY:
                JSON_ARRAY_ADD(JSON_COPY(arg->json_any));
            break;
            default:
            break;
        }  
        array_field_free(arg);
        continue;
    }

    va_end(ap); 

    return root;
}

int 
check_json_type(JSON_STRUCT root){
    if(JSON_IS_ARRAY(root))
        return LOG_ARRAY;
    else if(JSON_IS_STRING(root))
        return LOG_STRING;
    else if(JSON_IS_INTEGER(root))
        return LOG_INT;
    else if(JSON_IS_REAL(root))
        return LOG_REAL;
    else if(JSON_IS_REAL(root))
        return LOG_REAL;
    else if(JSON_IS_OBJECT(root))
        return LOG_OBJECT;
    else
        return LOG_OTHER;
}

JSON_STRUCT json_array_add(JSON_STRUCT root, JSON_STRUCT value){
    return JSON_ARRAY_ADD(value);
}

JSON_STRUCT json_object_add(JSON_STRUCT root, const char* key, JSON_STRUCT value){
    return JSON_OBJECT_ADD(key, value);
}


#endif /** end _LOGGER_H */