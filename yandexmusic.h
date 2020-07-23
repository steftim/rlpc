#ifndef YANDEXMUSIC_H
#define YANDEXMUSIC_H
#ifdef __cplusplus
extern "C"{
#endif
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

extern tracks* yam_search(char* query);
extern void get_download_url(int trackId, char* codec, int bitrate);

#ifdef __cplusplus
}
#endif
#endif /* YANDEXMUSIC_H */
