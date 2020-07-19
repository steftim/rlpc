#ifndef YANDEXMUSIC_H
#define YANDEXMUSIC_H

#include <stddef.h>

struct artist{
    int id;
    char* name;
};

struct album{
    int id;
    char* name;
};

struct track{
    char* title;
    struct artist* artist;
    struct album* album;
    int id;
    size_t artists_amount;
    size_t albums_amount;
};

typedef struct tracks{
    struct track* item;
    size_t tracks_col;
}tracks;

#endif /* YANDEXMUSIC_H */
