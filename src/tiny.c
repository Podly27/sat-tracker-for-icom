/* Small locale-free formatter/parser: no RTOS, TLS, heap or Linux services. */
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
double atof(const char*s){while(*s==' '||*s=='\t')s++;double sign=1,v=0,k=0.1;if(*s=='-'||*s=='+'){if(*s=='-')sign=-1;s++;}while(*s>='0'&&*s<='9')v=v*10+*s++-'0';if(*s=='.'){s++;while(*s>='0'&&*s<='9'){v+=(*s++-'0')*k;k*=0.1;}}return v*sign;}
static void ch(char*b,size_t cap,size_t*n,char c){if(*n+1<cap)b[*n]=c;(*n)++;}
static void num(char*b,size_t cap,size_t*n,uint32_t v,int width,char pad){char s[16];int i=0;do{s[i++]='0'+v%10;v/=10;}while(v);while(width>i){ch(b,cap,n,pad);width--;}while(i)ch(b,cap,n,s[--i]);}
int snprintf(char*b,size_t cap,const char*f,...){va_list ap;va_start(ap,f);size_t n=0;
 while(*f){if(*f!='%'){ch(b,cap,&n,*f++);continue;}f++;int plus=0,width=0,prec=0;char pad=' ';if(*f=='+'){plus=1;f++;}if(*f=='0'){pad='0';f++;}while(*f>='0'&&*f<='9')width=width*10+*f++-'0';if(*f=='.'){f++;while(*f>='0'&&*f<='9')prec=prec*10+*f++-'0';}int lng=*f=='l';if(lng)f++;char t=*f++;
 if(t=='s'){const char*s=va_arg(ap,const char*);while(*s)ch(b,cap,&n,*s++);}
 else if(t=='d'||t=='u'){uint32_t v;if(t=='d'){int32_t x=lng?va_arg(ap,long):va_arg(ap,int);v=x<0?-(uint32_t)x:x;if(x<0||plus){ch(b,cap,&n,x<0?'-':'+');width--;}}else v=lng?va_arg(ap,unsigned long):va_arg(ap,unsigned);num(b,cap,&n,v,width,pad);}
 else if(t=='x'||t=='X'){uint32_t v=lng?va_arg(ap,unsigned long):va_arg(ap,unsigned);char digits[8];int i=0;const char*hex=t=='X'?"0123456789ABCDEF":"0123456789abcdef";do{digits[i++]=hex[v&15];v>>=4;}while(v);while(width>i){ch(b,cap,&n,pad);width--;}while(i)ch(b,cap,&n,digits[--i]);}
 else if(t=='f'){double x=va_arg(ap,double);if(x<0||plus){ch(b,cap,&n,x<0?'-':'+');if(x<0)x=-x;}uint32_t p=1;for(int i=0;i<prec;i++)p*=10;uint32_t q=(uint32_t)(x*p+0.5);num(b,cap,&n,q/p,0,' ');if(prec){ch(b,cap,&n,'.');num(b,cap,&n,q%p,prec,'0');}}
 else if(t=='%')ch(b,cap,&n,'%');
 }if(cap)b[n<cap?n:cap-1]=0;va_end(ap);return n;
}
/* Bounded firmware workloads do not need the host libc's vector string
 * machinery. Keep these byte routines freestanding and deterministic. */
void *memcpy(void*d,const void*s,size_t n){unsigned char*a=d;const unsigned char*b=s;for(size_t i=0;i<n;i++)a[i]=b[i];return d;}
void *memset(void*d,int c,size_t n){unsigned char*a=d;for(size_t i=0;i<n;i++)a[i]=(unsigned char)c;return d;}
int memcmp(const void*a,const void*b,size_t n){const unsigned char*x=a,*y=b;for(size_t i=0;i<n;i++)if(x[i]!=y[i])return (int)x[i]-y[i];return 0;}
size_t strlen(const char*s){size_t n=0;while(s[n])n++;return n;}
int strcmp(const char*a,const char*b){while(*a&&*a==*b){a++;b++;}return (unsigned char)*a-(unsigned char)*b;}
int strncmp(const char*a,const char*b,size_t n){for(size_t i=0;i<n;i++){unsigned char x=a[i],y=b[i];if(x!=y)return (int)x-y;if(!x)return 0;}return 0;}
char *strchr(const char*s,int c){for(;;s++){if((unsigned char)*s==(unsigned char)c)return (char*)s;if(!*s)return 0;}}
char *strstr(const char*s,const char*p){if(!*p)return (char*)s;for(;*s;s++){size_t i=0;while(p[i]&&s[i]==p[i])i++;if(!p[i])return (char*)s;}return 0;}

char *strcpy(char*d,const char*s){char*out=d;do{*d++=*s;}while(*s++);return out;}
