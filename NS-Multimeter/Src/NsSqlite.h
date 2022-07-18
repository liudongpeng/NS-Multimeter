int __stdcall OpenSqliteDB(char *dbFile);
int __stdcall CloseSqliteDB();
int __stdcall SqliteQuery(char* sql,int* num,char ****data,unsigned int maxSize);
int __stdcall SqliteExec(char* sql);
int __stdcall SqliteCreateTable(char *table,char *key,char *cols[],int num);
int __stdcall SqliteSave(char *table,char *cols[],int num, int type, ...);
int __stdcall SqliteSaveStringArray(char *table,char *cols[],int num, int arraySize, char values[][arraySize]);
int __stdcall SqliteUpdate(char *table,char *key,char *cols[],char *values[],int num);
int __stdcall SqliteDelete(char *table,char *col,char *value);
char __stdcall * GetLastSqlitCommand (); 
void __stdcall FreeSqlData ();
#define POINTER_ARRAY 0
#define DOUBLE_ARRAY 1
#define INTEGER_ARRAY 2 
