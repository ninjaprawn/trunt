#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>

#include "linked_list.h"

// Defines a string "segment" for concatenation later
struct trunt_segment {
    struct trunt_segment* next;
    char* str;
    uint32_t length;
};

struct trunt_segment* new_segment(char* str, uint32_t length) {
    struct trunt_segment* s = malloc(sizeof(struct trunt_segment));
    s->next = NULL;
    s->str = str;
    s->length = length;
    return s;
}

// Defines an argument thats passed into a macro
struct trunt_arg {
    struct trunt_arg* next;
    char* as_string;
    char* as_term;
};

struct trunt_arg* arg_at_idx(linked_list ll, uint32_t idx) {
    return (struct trunt_arg*)list_item_at_idx(ll, idx);
}

// When creating an arg, the function will find the "term" and "string" versions, where the former has no quotations, and the latter does
struct trunt_arg* new_arg(char* unfixed_str) {
    struct trunt_arg* a = malloc(sizeof(struct trunt_arg));
    a->next = NULL;
    a->as_string = NULL;
    a->as_term = NULL;
    
    int start_idx = 0;
    int end_idx = strlen(unfixed_str) - 1;

    while (unfixed_str[start_idx] == ' ' || unfixed_str[start_idx] == '\t' || unfixed_str[start_idx] == '\n') {
        start_idx++;
    }

    while (unfixed_str[end_idx] == ' ' || unfixed_str[end_idx] == '\t' || unfixed_str[end_idx] == '\n') {
        end_idx--;
    }

    end_idx = strlen(unfixed_str) - end_idx;

    char* fixed_str = calloc(strlen(unfixed_str) - start_idx - end_idx + 1, 1);
    strncpy(fixed_str, unfixed_str + start_idx, strlen(unfixed_str) - start_idx - end_idx + 1);

    if (fixed_str[0] == '\"') {
        a->as_string = fixed_str;
        a->as_term = calloc(strlen(fixed_str) - 1, 1);
        strncpy(a->as_term, fixed_str + 1, strlen(fixed_str) - 2);
    } else {
        a->as_term = fixed_str;
        a->as_string = calloc(strlen(fixed_str) + 2, 1);
        strncpy(a->as_string + 1, fixed_str, strlen(fixed_str));
        a->as_string[0] = '\"';
        a->as_string[strlen(fixed_str) + 1] = '\"';
    }

    return a;
}

void arg_destroy(struct trunt_arg* a) {
    free(a->as_string);
    free(a->as_term);
}

void arg_list_destroy(linked_list ll) {
    if (ll != NULL) {
        struct trunt_arg* a = (struct trunt_arg*)ll->head;
        while (a != NULL) {
            struct trunt_arg* t = a;
            a = a->next;
            arg_destroy(t);
        }
        list_destroy(ll);
    }
}

// Fetches arg terms, and creates them as a comma seperated string
char* csv_terms(struct linked_list* ll, char* prefix) {
    int arg_count = list_length(ll);
    int output_size = strlen(arg_at_idx(ll, 0)->as_term);
    output_size += strlen(prefix);
    for (int i = 1; i < arg_count; i++) {
        output_size += 2;
        output_size += strlen(arg_at_idx(ll, i)->as_term);
        output_size += strlen(prefix);
    }

    char* output = calloc(output_size + 1, 1);

    strcat(output, prefix);
    strcat(output, arg_at_idx(ll, 0)->as_term);
    for (int i = 1; i < arg_count; i++) {
        strcat(output, ", ");
        strcat(output, prefix);
        strcat(output, arg_at_idx(ll, i)->as_term);
    }

    return output;
}

// Fetches arg strings, and creates them as a comma seperated string
char* csv_strings(struct linked_list* ll) {
    int arg_count = list_length(ll);
    int output_size = strlen(arg_at_idx(ll, 0)->as_string);
    for (int i = 1; i < arg_count; i++) {
        output_size += 2;
        output_size += strlen(arg_at_idx(ll, i)->as_string);
    }

    char* output = calloc(output_size + 1, 1);

    strcat(output, arg_at_idx(ll, 0)->as_string);
    for (int i = 1; i < arg_count; i++) {
        strcat(output, ", ");
        strcat(output, arg_at_idx(ll, i)->as_string);
    }

    return output;
}

// Defines a macro to identify  
struct trunt_macro {
    char* name;
    int arg_count; // Amount of args expected excluding the variadic args
    char has_variadic_arg; // Variadic arguments will start at the end
    char* (*handler)(struct trunt_macro*, int, linked_list); // macro, arg_count, args
};

// A macro handler for a way to have enums mapped to string (so its possible to do enum_strings[enum] = string)
// This exists as its tedious to do this for a huge number of items, and I'm lazy
char* macro_handler_map_string_to_enum(struct trunt_macro* macro, int arg_count, linked_list args) {
    // First arg: name of enum, and the prefix for all values
    const char* template = 
        "static char* %s_strings[] = {\n" // arg[0]
        "\t%s\n" // All the args in a list format
        "};\n\n"
        "typedef enum %s {\n" // arg[0]
        "\t%s\n"
        "} %s;\n"; // arg[0] 

    if (arg_count < 2) {
        return "";
    }

    struct trunt_arg* first_arg = arg_at_idx(args, 0);
    
    struct trunt_arg* names_head = arg_at_idx(args, 1); 
    linked_list names = args;
    // I want to use linked lists, but am very lazy. This is the cheat approach to do it because of how I free items when
    args->head = (list_node)names_head;

    char* prefix = calloc(strlen(first_arg->as_term) + 2, 1);
    strcat(prefix, first_arg->as_term);
    strcat(prefix, "_");

    char* list_terms = csv_terms(names, prefix); 
    char* string_terms = csv_strings(names);   


    int sz = snprintf(NULL, 0, template, first_arg->as_term, string_terms, first_arg->as_term, list_terms, first_arg->as_term);
    char* buf = calloc(sz+1, 1);
    snprintf(buf, sz+1, template, first_arg->as_term, string_terms, first_arg->as_term, list_terms, first_arg->as_term);

    args->head = (list_node)first_arg;
    free(prefix);
    free(list_terms);
    free(string_terms);

    return buf;
}


#define MACRO_TERMINATOR { NULL, 0, 0, NULL },

struct trunt_macro macro_list[] = {
    { "map_string_to_enum", 1, 1, macro_handler_map_string_to_enum},
    MACRO_TERMINATOR
};

// Helper
struct trunt_macro* get_macro_with_name(char* name) {
    int idx = 0;
    while (macro_list[idx].name != NULL) {
        if (!strncmp(name, macro_list[idx].name, strlen(name))) {
            return &macro_list[idx];
        }
        idx++;
    }
    return NULL;
}

#define consume_all_whitespace() while (mapped_file[idx] == ' ' || mapped_file[idx] == '\t' || mapped_file[idx] == '\n') {idx++;}

char file_exist(char *filename, int* file_size) {
    struct stat buffer;
    int ret = stat(filename, &buffer);

    if (ret != -1) {
        *file_size = buffer.st_size;
    }

    return ret != -1;
}


int main(int argc, char* argv[]) {
    int fd = open(argv[1], O_RDWR);
 
    int file_size = 0;
    file_exist(argv[1], &file_size);

    if (file_size == 0) {
        return -1;
    }
 
    char* mapped_file = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);


    linked_list segments = list_create();

    uint32_t segment_start = 0;
    uint32_t segment_end = 0;

    int idx = 0;
    while (idx < file_size) {
        consume_all_whitespace();
        if (mapped_file[idx] == '/') {
            idx++;
            if (mapped_file[idx] == '*') {
                //printf("Multiline comment\n");
                int old_idx = idx - 1;
                idx++;
                while (!(mapped_file[idx] == '*' && mapped_file[idx+1] == '/')) {
                    idx++;
                }
                idx += 2;
                //char* s = malloc(idx - old_idx + 1);
                //strncpy(s, mapped_file+old_idx, idx-old_idx);
                //printf("%s\n", s);
            } else if (mapped_file[idx] == '/') {
                //printf("Single line comment\n");
                while (mapped_file[idx++] != '\n') {};
            }
        } else if (mapped_file[idx] == '\"') {
            //printf("String!\n");
            idx++;
            while (mapped_file[idx] != '\"') {
                if (mapped_file[idx] == '\\' && mapped_file[idx+1] == '\"') {
                    idx++;
                }
                idx++;
            }
            idx++;    
        } else if (!strncmp(&mapped_file[idx], "struct", 6)) {
            idx += 6;
            consume_all_whitespace();
            int start_name = idx;
            while (mapped_file[idx] != '\n' && mapped_file[idx] != '\t' && mapped_file[idx] != ' ') {
                idx++;
            }
            char* struct_name = calloc(idx - start_name + 1, 1);
            strncpy(struct_name, &mapped_file[start_name], idx - start_name);
            printf("Found struct %s!\n", struct_name);
            consume_all_whitespace();
            if (mapped_file[idx] == '{') {
                printf("Standard struct!\n");
            } else if (!strncmp(&mapped_file[idx], "inherits", 8)) {
                idx += 8;
                consume_all_whitespace();
                start_name = idx;
                while (mapped_file[idx] != '\n' && mapped_file[idx] != '\t' && mapped_file[idx] != ' ') {
                    idx++;
                }
                char* parent_name = calloc(idx - start_name + 1, 1);
                strncpy(parent_name, &mapped_file[start_name], idx - start_name);
                printf("Struct inherits %s\n", parent_name);
            }
        } else if (mapped_file[idx] == '!') {
            segment_end = idx;
            idx++;
            if (mapped_file[idx] == '!') {
                struct trunt_segment* s = new_segment(mapped_file + segment_start, segment_end - segment_start);
                list_append(segments, (list_node)s); 

                idx++;
                //printf("Found preprocessor function!\n");
                int name_start = idx;
                while (mapped_file[idx] != '(') {
                    idx++;
                }
                char* name = calloc(idx - name_start + 1, 1);
                strncpy(name, mapped_file + name_start, idx - name_start);
                idx++;

                struct trunt_macro* m = get_macro_with_name(name);

                if (m == NULL) {
                    //printf("Unknown macro named '%s'\n", name);
                    // TODO: Handle this case
                } else {
                    //printf("Found macro %s\n", m->name);
                }

                free(name);

                linked_list al = list_create();

                int arg_count = 0;

                consume_all_whitespace();
                int start_idx = idx;
                while (mapped_file[idx] != ')') {
                    if (mapped_file[idx] == ',') {
                        arg_count++;
                        
                        char* arg_name = calloc(idx - start_idx + 1, 1);
                        strncpy(arg_name, mapped_file+start_idx, idx - start_idx);
                        struct trunt_arg* current_arg = new_arg(arg_name);

                        list_append(al, (list_node)current_arg);

                        free(arg_name);

                        idx++;
                        start_idx = idx;
                    } else {
                        idx++;
                    }
                }

                if (mapped_file[idx-1] != '(') {
                    arg_count++;
                    char* arg_name = calloc(idx - start_idx + 1, 1);
                    strncpy(arg_name, mapped_file+start_idx, idx - start_idx);
                    struct trunt_arg* current_arg = new_arg(arg_name);
                    list_append(al, (list_node)current_arg);

                    free(arg_name);
                }


                char* macro_result = m->handler(m, arg_count, al);

                s = new_segment(macro_result, strlen(macro_result));
                list_append(segments, (list_node)s);

                segment_start = idx + 1;

                arg_list_destroy(al);
            }
        } else {
            idx++;
        }
    }

    struct trunt_segment* s = new_segment(mapped_file + segment_start, idx - segment_start);
    list_append(segments, (list_node)s); 


    uint32_t length = 0;
    s = (struct trunt_segment*)segments->head;
    while (s != NULL) {
        length += s->length;
        s = s->next;
    }

    char* o = calloc(length + 1, 1);
    s = (struct trunt_segment*)segments->head;
    while (s != NULL) {
        strncat(o, s->str, s->length);
        s = s->next;
    }

    printf("%s", o);

    free(o);
    list_destroy(segments);

    munmap(mapped_file, file_size);
    return 0;
}
