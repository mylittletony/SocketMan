void process_message(const char *msg, char *cmd, char *id);
char *parse_message(const char *msg);
void *process_response(char *msg);
void *process_cmd(char *cmd, char *id);
