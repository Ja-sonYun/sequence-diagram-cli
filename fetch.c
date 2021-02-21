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

void check_version()
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    struct string s;

    int version_size = 6;

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

        char *version = strstr(s.ptr, "\"name\": \"v");
        if (version != NULL)
        {
            char *end = strstr(version, "\",");
            if (end != NULL)
            {
                int size = end - version - 9;
                version += 9;
                version[size] = '\0';
                if (size <= version_size && size > 0 && version[0] == 'v')
                {
                    if (strcmp(version, VERSION))
                    {
                        printf("** New version %s is released! **\nYou can update with below command,\n+-----------------------------------------------------------------------------------------+\n| wget -qO - https://github.com/Ja-sonYun/sequence-diagram-cli/raw/main/install.sh | bash |\n+-----------------------------------------------------------------------------------------+\n", version);
                        printf("** You can also download or see release note from below link.\n - https://github.com/Ja-sonYun/sequence-diagram-cli\n");
                    }
                }

            }

        }

    }

    free(s.ptr);

    curl_easy_cleanup(curl);

}
