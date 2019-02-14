#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "map.h"

#define MAX_CMD_LENGTH 255
#define MAX_LINE_LENGTH 255

void undecorate_name(char *name) {
    int cnt = 0;
    while (name[cnt]) {
        if (name[cnt] == '_')
            name[cnt] = ' ';
        cnt++;
    }
}

int main(int argc, char *argv[]) {

    COUNTRY **countries = map_load();

    while (1) {
        printf(">>");
        char cmd[MAX_CMD_LENGTH];
        scanf("%s", cmd);
        if (strcmp(cmd, "add") == 0) {
            char name[MAX_LINE_LENGTH];
            int population;
            int area;
            scanf("%s", name);
            scanf("%d", &population);
            scanf("%d", &area);
            undecorate_name(name);
            map_add(countries, name, population, area);
        } else if (strcmp(cmd, "save") == 0) {
            map_save(countries);
        } else if (strcmp(cmd, "quit") == 0) {
            map_clear(countries);
            exit(0);
        } else if (strcmp(cmd, "dump") == 0) {
            map_dump(countries);
        } else if (strcmp(cmd, "view") == 0) {
            char name[MAX_LINE_LENGTH];
            scanf("%s", name);
            undecorate_name(name);
            COUNTRY *target = map_find(countries, name);
            print_country(target);
        } else if (strcmp(cmd, "delete") == 0) {
            char name[MAX_LINE_LENGTH];
            scanf("%s", name);
            undecorate_name(name);
            map_delete(countries, name);
        }
    }
}
