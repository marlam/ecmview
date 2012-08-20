/*
 * Copyright (C) 2009, 2010, 2011, 2012
 * Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <curl/curl.h>

#include "fio.h"
#include "msg.h"
#include "exc.h"

#include "download.h"


static struct curl_ctor_dtor
{
    CURLcode curl_initialization_code;

    curl_ctor_dtor()
    {
        curl_initialization_code = curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~curl_ctor_dtor()
    {
        if (curl_initialization_code == 0) {
            curl_global_cleanup();
        }
    }
} curl_ctor_dtor;


void download(FILE* destination_file, const std::string& url,
        const std::string& username, const std::string &password)
{
    if (curl_ctor_dtor.curl_initialization_code != 0) {
        throw exc("Cannot initialize libcurl");
    }

    CURLcode res;
    char curl_errmsg[CURL_ERROR_SIZE];
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw exc("Cannot initialize libcurl easy interface");
    }
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errmsg);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, PACKAGE_TARNAME "-" PACKAGE_VERSION " (" PLATFORM ")");
    curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_OPTIONAL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, destination_file);
    if (!username.empty() && !password.empty()) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    }
    res = curl_easy_perform(curl);
    int sys_errno = 0;
    if (res != 0) {
        if (res == CURLE_REMOTE_FILE_NOT_FOUND
                || res == CURLE_FILE_COULDNT_READ_FILE
                || res == CURLE_TFTP_NOTFOUND
                || res == CURLE_FTP_COULDNT_RETR_FILE) {
            sys_errno = ENOENT;
        } else if (res == CURLE_HTTP_RETURNED_ERROR) {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code == 404) {
                sys_errno = ENOENT;
            }
        }
    }
    curl_easy_cleanup(curl);
    if (res != 0) {
        throw exc("Cannot download " + url + ": " + curl_errmsg, sys_errno);
    }
}

void download(const std::string& destination_file, const std::string& url,
        const std::string& username, const std::string& password)
{
    FILE* f = fio::open(destination_file, "w");
    download(f, url, username, password);
    fio::close(f);
}
