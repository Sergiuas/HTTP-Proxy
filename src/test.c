#include <stdio.h>
#include <string.h>

int main() {
    char request[] = "HTTP/1.1 200 OKDate: Wed, 17 Jan 2024 00:09:07 GMTContent-Type: text/html; charset=utf-8Content-Length: 9593Server: gunicorn/19.9.0";

    char *content = strstr(request, "Content-Length:");
         if (content != NULL) {
            printf("Content-Length: %s\n", content);

         }

    return 0;
}