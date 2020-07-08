#include "yandex_api.h"
#include <curl/curl.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <stdlib.h>

char* resp;

char* yandex_search(char* query){
    //char* resstr;
    CURL* curl = curl_easy_init();
        if(curl){
            char search_query[512];

            char* ptr = strchr(query, ' ');
            while(ptr){ *ptr = '+'; ptr = strchr(query, ' '); }

            snprintf(search_query, 512, "%s%s%s", "https://api.music.yandex.net/search?text=", query, "&nocorrect=false&type=all&page=0&playlist-in-best=true");
            printf("%s\n", search_query);

            curl_easy_setopt(curl, CURLOPT_URL, search_query);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);
            //I don't know how make this damn CURLOPT_WRITEDATA work.
            //Што поделоть, кругом костыли...
            //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resstr);
            curl_easy_perform(curl);
            //printf("%s", resp);
            cJSON *JSON_resp = cJSON_Parse(resp);

            const cJSON *name = cJSON_GetObjectItemCaseSensitive(JSON_resp, "name");
            if(name != NULL) printf("%s", name->valuestring);

            curl_easy_cleanup(curl);
        }

    curl_global_cleanup();
    return resp;
}

void static writedata(void* data, size_t size, size_t nmemb, char* userdata){
    resp = (char*)malloc((strlen((char*)data) + 1) * sizeof(char));
    strcpy((char*)resp, (char*)data);
    printf("%lu", strlen((char*)data));
}

//cJSON jsonparser(const char* const string){
//    cJSON *json = cJSON_Parse(string);
//}
