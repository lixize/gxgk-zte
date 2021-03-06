#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#define GOOGLE_CODE_URL_ZTE "http://gxgk-zte.googlecode.com/svn/zte-client-win/Release/ZTE.exe"

typedef struct _FileInfo{
	time_t modifytime;
	size_t filesize;
} FileInfo;

typedef struct _buffer{
	char *buffer;
	size_t size;
	int offset;
} Buffer;

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
int AutoUpdate();
FileInfo *GetFileModifyTime();
FileInfo *GetHttpModifyTime(const char *url);
char *RandomSessionID(const char *url);
int DownLoadFile(const char *savename, const char *url, const char *ip, const int timeout);
int DownLoadFileToBuffer(char *buffer, int size, const char *url, const char *ip, const int timeout, const char *cookies);

#endif