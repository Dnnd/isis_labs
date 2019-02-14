#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define  MAX_LINE_LENGTH 256
#define  DBNAME "db.csv"

/* Тип указателя на функцию для сравнения элементов списка */
typedef int (*CMP_FUNC)(COUNTRY *v1, COUNTRY *v2);

/*
 * Выделяет динамическую память для нового элемента списка list,
 * инициализирует поля и добавляет элемент в начало списка
 * Изменяет указатель на начало списка. Возвращает 0 если новый элемент
 * добавлен или 1 если не удалось выделить память для элемента.
 */
int add(COUNTRY **list, char *name, int population, int area) {
    COUNTRY *new_elem = malloc(sizeof(COUNTRY));
    if (new_elem == NULL) {
        return 1;
    }
    strncpy(new_elem->name, name, sizeof(new_elem->name));
    new_elem->population = population;
    new_elem->area = area;
    new_elem->next = *list;
    *list = new_elem;
    return 0; /* ok */
}

/* Удаляет элемент списка list и освобождает выделенную по него память.
 *   Если элемент первый, изменяет указатель на список
 */
void delete(COUNTRY **list, COUNTRY *v) {
    if (list == NULL || *list == NULL || v == NULL) {
        return;
    }
    if (compare_name(*list, v) == 0) {
        COUNTRY *target = *list;
        if ((*list)->next != NULL) {
            (*list)->next = (*list)->next->next;
            *list = (*list)->next;
        } else {
            *list = NULL;
        }
        free(target);
        return;
    }
    for (COUNTRY *it = (*list); it->next != NULL; it = it->next) {
        if (compare_name(it->next, v) == 0) {
            COUNTRY *target = it->next;
            it->next = it->next->next;
            free(target);
            return;
        }
    }
}

/*
 * Ищет в списке страну с заданным именем name. Возвращает указатель на
 * найденный элеемнт списка или NULL, если страна с таким названием отсутсвует
 * в списке.
 */
COUNTRY *find(COUNTRY *list, char *name) {
    for (COUNTRY *it = list; it != NULL; it = it->next) {
        if (strcmp(it->name, name) == 0) {
            return it;
        }
    }
    return NULL;
}

/*
 * Возвращает количество элементов в списке
 */
int count(COUNTRY *list) {
    int cnt = 0;

    for (COUNTRY *it = list; it != NULL; it = it->next) {
        ++cnt;
    }

    return cnt;
}

/* Быстрая сортировка массива указателей на элементы списка */
void quick_sort(COUNTRY **ca, int first, int last, CMP_FUNC compare) {
    int i, j;
    COUNTRY *v;
    COUNTRY *p;

    i = first;
    j = last;
    v = ca[(first + last) / 2];

    do {
        while (compare(ca[i], v) < 0) i++;
        while (compare(ca[j], v) > 0) j--;

        if (i <= j) {
            if (compare(ca[i], ca[j]) > 0) {
                p = ca[j];
                ca[j] = ca[i];
                ca[i] = p;
            }
            i++;
            j--;
        }
    } while (i <= j);

    if (i < last)
        quick_sort(ca, i, last, compare);

    if (first < j)
        quick_sort(ca, first, j, compare);
}

int compare_name(COUNTRY *v1, COUNTRY *v2) {
    return strcmp(v1->name, v2->name);
}

int compare_area(COUNTRY *v1, COUNTRY *v2) {
    if (v1->area < v2->area) {
        return -1;
    } else if (v1->area == v2->area) {
        return 0;
    }
    return 1;
}

int compare_population(COUNTRY *v1, COUNTRY *v2) {
    if (v1->population < v2->population) {
        return -1;
    } else if (v1->population == v2->population) {
        return 0;
    }
    return 1;
}

/*
 * Сортирует список по именам стран в алфавитном порядке методом пузырьковой
 * сортировки. Указатель на начало списка может измениться в результате
 * сортировки
 */
int sort(COUNTRY **list, CMP_FUNC compare) {
    int cnt, i;
    COUNTRY *p, **ca = NULL;

    cnt = count(*list);
    if (cnt < 2)
        return 0;

    /* выделяем память под массив указателей */
    ca = (COUNTRY **) malloc(cnt * sizeof(COUNTRY *));
    if (!ca)
        return 1;

    /* заполняем массив указателями на элементы списка */
    ca[0] = *list;
    for (i = 1; i < cnt; i++)
        ca[i] = ca[i - 1]->next;

    quick_sort(ca, 0, cnt - 1, compare);

    /* заполняем список элементами из отсортированного массива */
    *list = NULL;
    while (cnt > 0) {
        ca[cnt - 1]->next = *list;
        *list = ca[cnt - 1];
        cnt--;
    }
    free(ca);
    return 0;
}


int sort_by_name(COUNTRY **list) {
    if (list == NULL || *list == NULL) {
        return 1;
    }
    return sort(list, compare_name);
}

int sort_by_area(COUNTRY **list) {
    if (list == NULL || *list == NULL) {
        return 1;
    }
    return sort(list, compare_area);
}

int sort_by_population(COUNTRY **list) {
    if (list == NULL || *list == NULL) {
        return 1;
    }
    return sort(list, compare_population);
}

void print_country(COUNTRY *p) {
    if (p == NULL) {
        return;
    }
    if (p->population == 0 && p->area == 0) {
        printf("%s, не указано, не указано\n", p->name);
    } else if (p->population == 0) {
        printf("%s, не указано, %d\n", p->name, p->population);
    } else if (p->area == 0) {
        printf("%s, %d, не указано\n", p->name, p->area);
    } else {
        printf("%s, %d, %d\n", p->name, p->population, p->area);
    }
}

void dump(COUNTRY *list) {
    if (list == NULL) {
        return;
    }
    COUNTRY *p;
    p = list; /* Начало списка */
    while (p != NULL) {
        print_country(p);
        p = p->next;
    }
}

/*
 * удаление всех эдементов списка и
 * освобождение выделенной под эти элементы памяти
 */
void clear(COUNTRY *list) {
    COUNTRY *it = list;
    while (it != NULL) {
        COUNTRY *tmp  = it;
        it = it->next;
        free(tmp);
    }
}

/* Загружает список стран из файла */
COUNTRY *load() {
    char buf[MAX_LINE_LENGTH + 1];
    char *par[3];
    int cnt, pcount = 0;
    COUNTRY *p, *list = NULL;
    FILE *f = fopen(DBNAME, "r");

    buf[MAX_LINE_LENGTH] = 0x00;

    if (f) {
        while (fgets(buf, MAX_LINE_LENGTH, f)) {
            pcount = 0;
            par[pcount++] = buf;
            cnt = 0;
            while (buf[cnt]) {
                if (buf[cnt] == ',') {
                    buf[cnt] = 0x00;
                    par[pcount++] = &buf[cnt + 1];
                }
                cnt++;
            }
            if (pcount == 3) {
                add(&list, par[0], atoi(par[1]), atoi(par[2]));
            }
        }
        fclose(f);
    }
    return list;
}

/* Сохраняет список стран в файл */
void save(COUNTRY *list) {
    FILE *f = fopen(DBNAME, "w+");
    COUNTRY *p = list;

    if (f) {
        while (p != NULL) {
            fprintf(f, "%s,%d,%d\n", p->name, p->population, p->area);
            p = p->next;
        }

        fclose(f);
    }
}


