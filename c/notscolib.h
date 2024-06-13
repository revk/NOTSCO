void notscofailure(SQL * sqlp, int tester,j_t rx, int code);
void notscotx (SQL*sqlp,int tester,j_t tx);
j_t notscoreply(j_t rx, j_t tx, const char *type);
void syntaxcheck(j_t,FILE*);
void responsecheck(int status,j_t j,FILE *e);

