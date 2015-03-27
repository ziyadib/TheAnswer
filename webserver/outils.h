/*fonction divers qui permettent principalement de lire (recuperer) des données les modifier ou les envoyer*/
char *fgets_or_exit(char *buffer , int size , FILE *stream);

int copy(int in, int out);
int check_and_open(const char* url, const char* document_root);
int get_file_size(int fd);
void filterString(char string[]);
