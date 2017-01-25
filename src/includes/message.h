void *save_config(char *file, const char *msg);
void *save_cert(char *msg);
char *parse_message(const char *msg);
void *process_response(char *msg);
void *process_cmd(char *cmd, char *id);
void *run_special(const char *type);
void process_message(const char *msg, char *cmd, char *id, char *type, int len);
