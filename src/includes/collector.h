/* int read_pid(char *pidfile); */
/* int readconfig(); */
/* extern char *username; */
/* void read_file(char * status, const char *filename); */
void cache_data();
time_t last_collect;
void collect_and_send_data(int online);
