#include <stdio.h>
#include <curl/curl.h>

int main(void) {
  printf("Hello, World! libcurl version: %s\n", curl_version());
  return 0;
}
