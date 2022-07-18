void __stdcall GetSignature(char* signature); 
int __stdcall CheckRegister(char* license,char* message);
int __stdcall CheckLicense(char* message, int *surplus);
int __stdcall CancelLicense();


void GenerateLicense(char* signature,int licenseType,char* license);
void GenerateTerm(int days, char *term);
int CheckTermState (char *license, char *term, int *days);
int ParseLicense(char* license,char* computer,int* licenseType,unsigned int* regTime,int* days);
