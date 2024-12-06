#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include "afxsock.h"

void XOR(char * dest, char * src, int len);

void buildPasswordCheckCypher(char * password,char * salt,unsigned long localIp, unsigned long externalIp, char * iv, char * cypher);

ULONGLONG checkPasswordCypher(char * password,char * salt,unsigned long localIp, unsigned long externalIp, char * iv, char * cypher);
void getAesKey2(unsigned long localIp, unsigned long externalIp, char * aesKey2);
void getAesKey1(const char * password, const char * salt, char * aesKey1);
void getLoginDataSignature(char * key3, char * data, unsigned long int dataSize, char * signature);
void debugAccountInformation(char * pass, char * salt, unsigned int localip,unsigned int externalip, char * data, unsigned int size);
long buildAccountInformation(char * blob, int blobLen, char * ticket, int ticketLen, char * plain2, int plain2Len, char * key1, char * key3, char * iv2, char * blobIv,char * result);
int decryptPostTicketData(char loginKey3[16], char* ticket, DWORD ticketLen,char iv[16],DWORD decrLen,DWORD encrLen,char sha[20],char * encrData,char *decrData);
long encryptAccountRecordBlob(char * encryptedBlobContainer, char * blob, DWORD blobLen, char * blobIv, char * key3);

#endif