#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calculator.h"

static int has_so_extension(const char *filename) {
    size_t length = strlen(filename);

    return length > 3 && strcmp(filename + length - 3, ".so") == 0;
}

static int compare_operations(const void *first, const void *second) {
    const Operation *a = first;
    const Operation *b = second;

    return a->order - b->order;
}

static int add_operation(Operation **operations,
                         int *count,
                         const Operation *operation) {
    Operation *new_operations = realloc(
        *operations,
        (size_t)(*count + 1) * sizeof(Operation)
    );

    if (new_operations == NULL) {
        return 0;
    }

    *operations = new_operations;
    (*operations)[*count] = *operation;
    (*count)++;

    return 1;
}

static int load_one_library(const char *directory,
                            const char *filename,
                            Operation **operations,
                            int *count) {
    char path[512];
    void *handle;
    void *function_address;
    const char *name;
    const char *symbol;
    const int *order;
    const char *error;
    Operation loaded;

    if (snprintf(path, sizeof(path), "%s/%s", directory, filename) >=
        (int)sizeof(path)) {
        return 0;
    }

    handle = dlopen(path, RTLD_NOW);
    if (handle == NULL) {
        fprintf(stderr, "Не удалось загрузить %s: %s\n", path, dlerror());
        return 0;
    }

    dlerror();
    function_address = dlsym(handle, "operation");
    name = dlsym(handle, "operation_name");
    symbol = dlsym(handle, "operation_symbol");
    order = dlsym(handle, "operation_order");
    error = dlerror();

    if (error != NULL || function_address == NULL || name == NULL ||
        symbol == NULL || order == NULL) {
        fprintf(stderr, "В библиотеке %s нет обязательных данных.\n", filename);
        dlclose(handle);
        return 0;
    }

    memset(&loaded, 0, sizeof(loaded));
    snprintf(loaded.name, sizeof(loaded.name), "%s", name);
    snprintf(loaded.library_name, sizeof(loaded.library_name), "%s", filename);
    loaded.symbol = *symbol;
    loaded.order = *order;
    loaded.library_handle = handle;

    memcpy(&loaded.function, &function_address, sizeof(loaded.function));

    if (!add_operation(operations, count, &loaded)) {
        dlclose(handle);
        return 0;
    }

    return 1;
}

int operations_load(const char *directory,
                    Operation **operations,
                    int *count) {
    DIR *folder;
    struct dirent *entry;

    *operations = NULL;
    *count = 0;

    folder = opendir(directory);
    if (folder == NULL) {
        return 0;
    }

    while ((entry = readdir(folder)) != NULL) {
        if (!has_so_extension(entry->d_name)) {
            continue;
        }

        load_one_library(directory, entry->d_name, operations, count);
    }

    closedir(folder);

    if (*count > 1) {
        qsort(*operations,
              (size_t)*count,
              sizeof(Operation),
              compare_operations);
    }

    return *count > 0;
}

void operations_unload(Operation **operations, int *count) {
    int i;

    if (operations == NULL || *operations == NULL) {
        if (count != NULL) {
            *count = 0;
        }
        return;
    }

    for (i = 0; i < *count; i++) {
        if ((*operations)[i].library_handle != NULL) {
            dlclose((*operations)[i].library_handle);
        }
    }
    free(*operations);
    *operations = NULL;
    *count = 0;
}
