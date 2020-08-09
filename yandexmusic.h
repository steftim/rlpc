#ifndef YANDEXMUSIC_H
#define YANDEXMUSIC_H
#ifdef __cplusplus
extern "C"{
#endif
#include <stddef.h>

struct artist{
    unsigned int id;
    char* name;
};

struct album{
    unsigned int id;
    char* name;
};

struct track{
    char* title;
    struct artist* artist;
    struct album* album;
    unsigned int id;
    size_t artists_amount;
    size_t albums_amount;
};

typedef struct tracks{
    struct track* item;
    size_t tracks_col;
}tracks;

typedef struct userInfo{
    char* access_token;
    unsigned int expires_in;
    char* token_type;
    unsigned int uid;
}userInfo;

extern tracks* yam_search(char* query, userInfo* userinfo);
extern char* get_download_url(unsigned int trackId, userInfo* userinfo);
userInfo* get_token(char* grant_type, char* username, char* password);

#ifdef __cplusplus
}
#endif
#endif /* YANDEXMUSIC_H */
