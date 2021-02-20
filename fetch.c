#include "fetch.h"

struct string {
    char *ptr;
    size_t len;
};

void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = malloc_s(s->len+1);
    s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc_s(s->ptr, new_len+1);
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

char* check_version()
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    struct string s;

    int version_size = 6;
    char *version = calloc_s(version_size, sizeof(char));
    bool succeed = false;

    if(curl) {
        init_string(&s);

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Accept: application/vnd.github.v3+json");
        chunk = curl_slist_append(chunk, "User-Agent: curl/7.64.1");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/Ja-sonYun/sequence-diagram-cli/tags");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);

        char *version_st = strstr(s.ptr, "\"name\": \"v");
        char *end = strstr(version_st, "\",");
        int size = end - version_st - 9;
        version_st += 9;
        version_st[size] = '\0';
        if (size <= version_size && size > 0 && version_st[0] == 'v')
        {
            strcpy(version, version_st);
            version[version_size] = '\0';
            succeed = true;
        }

    curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    if (succeed)
        return version;

    return NULL;
}
