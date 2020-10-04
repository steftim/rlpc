#ifndef YANDEXMUSIC_H
#define YANDEXMUSIC_H
#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
#include <stddef.h>
struct artist{
    unsigned int id;
    char* name;
};

struct album{
    unsigned int id;
    char* name;
    unsigned int year;
    char* genre;
    char* coverUri;
};

typedef struct track{
    char* title;
    struct artist* artist;
    struct album* album;
    unsigned int id;
    size_t artists_amount;
    size_t albums_amount;
}track;

typedef struct tracks{
    track* item;
    size_t tracks_col;
}tracks;

typedef struct download{
    char* codec;
    bool gain;
    bool preview;
    char* downloadInfoUrl;
    bool direct;
    unsigned int bitrateInKbps;
}download;

typedef struct userInfo{
    char* access_token;
    unsigned int expires_in;
    char* token_type;
    unsigned int uid;
}userInfo;

typedef struct cover{
    char* data;
    size_t len;
}cover;

tracks* yam_search(char* query, userInfo* userinfo, char* proxy);
char* get_download_url(unsigned int trackId, userInfo* userinfo, char* proxy);
int download_track(const char* name, const char* url, char* proxy);
userInfo* get_token(char* grant_type, char* username, char* password, char* proxy);
track* get_track_info_from_id(unsigned int id, userInfo* userinfo, char* proxy);
cover* get_cover(char* url, char* proxy);

#ifdef __cplusplus
}
#endif
#endif /* YANDEXMUSIC_H */
