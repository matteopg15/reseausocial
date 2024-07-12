#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "hmac.c"
#include "sha1.c"


int checkcode(char *totpsecret,char *code)
{
time_t epoch,e2;
int fd,nb,i;
unsigned int calc;
char tmp[150],secret[100],res[10];
u_int8_t digest[20],challenge[8];

// Génération des 6 chiffres que Google Authenticator va afficher
strcpy(secret,totpsecret);
sprintf(tmp,"echo \'%s\' | base32 -d > /tmp/totp%d",secret,getpid());
system(tmp);
sprintf(tmp,"/tmp/totp%d",getpid());
fd=open(tmp,O_RDONLY);
nb=read(fd,secret,100);
close(fd);
time(&epoch);
epoch=epoch/30;
e2=epoch;
for (i=8;i--;epoch>>=8) challenge[i] = epoch;
hmac_sha1(secret,nb,challenge,8,digest,20);
fd=digest[19]&0xF;
calc=digest[fd]*256*256*256+digest[fd+1]*256*256+digest[fd+2]*256+digest[fd+3];
calc &= 0x7FFFFFFF;
calc=calc%1000000;
if(calc>=100000) sprintf(res,"%d",calc);
if(calc<100000 && calc >=10000) sprintf(res,"0%d",calc);
if(calc<10000 && calc>=1000) sprintf(res,"00%d",calc);
if(calc<1000 && calc>=100) sprintf(res,"000%d",calc);
if(calc<100 && calc>=10) sprintf(res,"0000%d",calc);
if(calc<10) sprintf(res,"00000%d",calc);
if(strcmp(res,code)==0) return(0);
epoch=e2-1;
for (i=8;i--;epoch>>=8) challenge[i] = epoch;
hmac_sha1(secret,nb,challenge,8,digest,20);
fd=digest[19]&0xF;
calc=digest[fd]*256*256*256+digest[fd+1]*256*256+digest[fd+2]*256+digest[fd+3];
calc &= 0x7FFFFFFF;
calc=calc%1000000;
if(calc>=100000) sprintf(res,"%d",calc);
if(calc<100000 && calc >=10000) sprintf(res,"0%d",calc);
if(calc<10000 && calc>=1000) sprintf(res,"00%d",calc);
if(calc<1000 && calc>=100) sprintf(res,"000%d",calc);
if(calc<100 && calc>=10) sprintf(res,"0000%d",calc);
if(calc<10) sprintf(res,"00000%d",calc);
if(strcmp(res,code)==0) return(0);
epoch=e2+1;
for (i=8;i--;epoch>>=8) challenge[i] = epoch;
hmac_sha1(secret,nb,challenge,8,digest,20);
fd=digest[19]&0xF;
calc=digest[fd]*256*256*256+digest[fd+1]*256*256+digest[fd+2]*256+digest[fd+3];
calc &= 0x7FFFFFFF;
calc=calc%1000000;
if(calc>=100000) sprintf(res,"%d",calc);
if(calc<100000 && calc >=10000) sprintf(res,"0%d",calc);
if(calc<10000 && calc>=1000) sprintf(res,"00%d",calc);
if(calc<1000 && calc>=100) sprintf(res,"000%d",calc);
if(calc<100 && calc>=10) sprintf(res,"0000%d",calc);
if(calc<10) sprintf(res,"00000%d",calc);
if(strcmp(res,code)==0) return(0);
return(-1);
}