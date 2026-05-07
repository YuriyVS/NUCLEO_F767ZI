#ifndef WEB_SERVER_H
#define WEB_SERVER_H

extern char web_json_buffer[4096];
int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);

#endif
