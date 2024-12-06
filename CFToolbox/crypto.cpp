#include "malloc.h"
#include "memory.h"
#include <process.h>
#include "socketTools.h"
#include "Registry.h"
#include <stdio.h>
#include "SHA1.h"
#include "Rijndael.h"

void XOR(char * dest, char * src, int len)
{
	while ( len ) // remaining
	{
		* dest ++ ^= * src ++;
		len --;
	}
}


void getAesKey1(const char * password, const char * salt, char * aesKey1){
	char data[160];
	
	int len=strlen(password);
	
	memcpy(data,salt,4); // first 4 bytes of salt
	memcpy(data+4,password,len);// password
	memcpy(data+4+len,salt+4,4); // last 4 bytes of salt
	
	CSHA1 sha1;
	sha1.Reset();
	sha1.Update((UINT_8 *)data,len+8);
	sha1.Final();
	sha1.GetHash((UINT_8 *)data); //SHA-1 160 bytes digest
	
	memcpy(aesKey1,data,16); // 16 first bytes of sha digest ;
}

void getAesKey2(unsigned long localIp, unsigned long externalIp, char * aesKey2)
{
	char data[160];
	
	memcpy(data,(char*)&externalIp,4); // external ip big-endian
	memcpy(data+4,(char*)&localIp,4); // local ip big-endian
	
	
	//printf("plainkey2:");	hexaPrintf((char*)&data,8);
	CSHA1 sha1;
	sha1.Reset();
	sha1.Update((UINT_8 *)data,8);
	sha1.Final();
	sha1.GetHash((UINT_8 *)data); // SHA-1 160 bytes digest;
	
	memcpy(aesKey2,data,8); // 8 first bytes of sha digest 
}

void buildPasswordCheckCypher(char * password,char * salt,unsigned long localIp, unsigned long externalIp, char * iv,char * cypher)
{
	char plain[16];
	char aesKey1[16];
	char aesKey2[8];
	
	getAesKey1(password,salt,aesKey1);
	getAesKey2(localIp,externalIp,aesKey2);
	
	ULONGLONG time=getNanosecondsSinceTime0(); // time stamp
	// xor of timestamp and key2
    //localIp=reverseBytes(localIp);
	memcpy(plain,(char*)&time,8);		// xored timestamp
	memcpy(plain+8,(char*)&localIp,4);	// + local ip
	unsigned long filler=0x04040404; // dummy filler
	memcpy(plain+12,(char*)&filler,4); // append filler
	
	XOR(plain,aesKey2,8);
	// encrypt this with key 1
	CRijndael aes;
	aes.MakeKey(aesKey1, iv, 16, 16);
	aes.ResetChain();
	aes.Encrypt(plain, cypher,16, CRijndael::CBC);
	
}

ULONGLONG checkPasswordCypher(char * password,char * salt,unsigned long localIp, unsigned long externalIp, char * iv, char * cypher)
{
	if (!password) return 0;
	char plain[16];
	char aesKey1[16];
	char aesKey2[8];
	
	getAesKey1(password,salt,aesKey1);
	getAesKey2(localIp,externalIp,aesKey2);
	
	// decrypt with key 1
	CRijndael aes;
	aes.MakeKey(aesKey1, iv, 16, 16);
	aes.ResetChain();
	aes.Decrypt(cypher,plain,16,CRijndael::CBC);
	
	XOR((char*)plain,aesKey2,8);
	
	ULONGLONG * timestamp=(ULONGLONG *)plain;
	unsigned long * decryptedIp=(unsigned long *)(plain+8);
	unsigned long * check=(unsigned long *)(plain+12);
	
	if (*check!=0x04040404) return 0;
	
	if (*decryptedIp!=localIp) return 0;
	return *timestamp;
}


void getLoginDataSignature(char * key3, char * data, unsigned long int dataSize, char * signature)
{
	
	//printf("\nsigning (%d):\n",dataSize);hexaPrintf(data,dataSize);
	char newKey[64];
	memset(newKey,0,64);
	memcpy(newKey,key3,16);
	
	char xor[64];
	memset(xor,0x36,64);
	
	char newKeyA[64];
	memcpy(newKeyA,newKey,64);
	XOR(newKeyA,xor,64);
	
	memset(xor,0x5C,64);
	char newKeyB[64];
	memcpy(newKeyB,newKey,64);
	XOR(newKeyB,xor,64);
	
	char checksumA[160];
	CSHA1 sha1;
	sha1.Reset();
	sha1.Update((UINT_8 *)newKeyA,64);
	sha1.Update((UINT_8 *)data,dataSize);
	sha1.Final();
	sha1.GetHash((UINT_8 *)checksumA); 
	
	char checksumB[160];
	sha1.Reset();
	sha1.Update((UINT_8 *)newKeyB,64);
	sha1.Update((UINT_8 *)checksumA,20);
	sha1.Final();
	sha1.GetHash((UINT_8 *)checksumB); 
	
	memcpy(signature,checksumB,20);
}


void ttt(char * part1,char * part2, char * part3,char * iv3, char * key3)
{
	printf("   ---------------------\n");
	char decPart1[600];
	CRijndael aes2;
	aes2.MakeKey(key3, iv3, 16, 16);
	aes2.ResetChain();
	aes2.Decrypt(part1,decPart1,0x80,CRijndael::CBC);
	
	printf("\ndecPart1:\n");hexaPrintf(decPart1,0x80);
	
	aes2.ResetChain();
	aes2.Decrypt(part2,decPart1,0x50,CRijndael::CBC);
	
	printf("\ndecPart2:\n");hexaPrintf(decPart1,0x50);
	
	aes2.ResetChain();
	aes2.Decrypt(part3,decPart1,0x80,CRijndael::CBC);
	
	printf("\ndecPart3:\n");hexaPrintf(decPart1,0x80);
}
void debugTicket(char * ticket, long len, char * key1,char * key3,char * iv2,char * blobIv)
{
	
	char * iv3=ticket+4;
	char * part1=iv3+0x10;
	WORD * part2DecLen=(WORD *)(part1+0x80);
	
	WORD * part2EncLen=(WORD *)(part2DecLen+1);
	*part2DecLen=reverseBytes(*part2DecLen,2);
	*part2EncLen=reverseBytes(*part2EncLen,2);
	char * part2=(char*)(part2EncLen+1);
	char * part3= part2+*part2EncLen+2+0x10;
	printf("part2EncLen=%d\n",*part2EncLen);
	printf("part2DecLen=%d\n",*part2DecLen);
	printf("\niv3:\n");hexaPrintf(iv3,0x10);
	printf("\npart1:\n");hexaPrintf(part1,0x80);
	printf("\npart2:\n");hexaPrintf(part2,*part2EncLen);
	printf("\npart3:\n");hexaPrintf(part3,0x80);
	
	/*	ttt(part1,part2,part3,iv3,key1);
	ttt(part1,part2,part3,iv2,key1);
	ttt(part1,part2,part3,blobIv,key1);	
	
	  ttt(part1,part2,part3,iv3,key3);
	  ttt(part1,part2,part3,iv2,key3);
	  ttt(part1,part2,part3,blobIv,key3);	
	*/
}

void debugAccountInformation(char * pass, char * salt, unsigned int localip,unsigned int externalip, char * data, unsigned int size)
{
	printf("initial data : \n");hexaPrintf(data,size);
	
	char aesKey1[16];
	char aesKey2[8];
	
	getAesKey1(pass,salt,aesKey1);
	getAesKey2(localip,externalip,aesKey2);
	
	
	char *iv2=(char*)((int)data+2);
	char * cypher2=(char*)((int)data+2+16+4);
	WORD * ticketLen=(WORD *)((int)cypher2+64);
	int tL=reverseBytes(*ticketLen,2);
	char * ticket=(char*)((int)ticketLen+2);
	DWORD * blobCLen=(DWORD *)((int)ticket+tL);
	DWORD bCL=reverseBytes(*blobCLen);
	char *blobContainer=(char*)((int)blobCLen+4);
	char * signature=(char*)((int)blobContainer+bCL);
	
	//printf("\niv2:\n");hexaPrintf(iv2,16);
	//printf("\ncypher2:\n");hexaPrintf(cypher2,64);
	//printf("\nticketLen(%d):\n",tL);hexaPrintf((char*)ticketLen,2);
	printf("\nticket:\n");hexaPrintf(ticket,tL);
	//printf("\nblobCLen(%d):\n",bCL);hexaPrintf((char*)blobCLen,4);
	//printf("\nblobContainer:\n");hexaPrintf(blobContainer,bCL);
	//printf("\nsignature:\n");hexaPrintf(signature,20);
	
	
	char plain2[64];
	CRijndael aes;
	aes.MakeKey(aesKey1, iv2, 16, 16);
	aes.ResetChain();
	aes.Decrypt(cypher2,plain2,64,CRijndael::CBC);
	
	printf("\nplain2:\n");hexaPrintf(plain2,64);
	
	char * key3=plain2;
	
	char sign[20];
	getLoginDataSignature(key3,data,size-20,sign);
	/*printf("\nCOMPARE checksum with signature:\n");
	hexaPrintf(signature,20);
	hexaPrintf(sign,20);
	*/
	
	DWORD * blobLen= (DWORD*)((int)blobContainer+2);
	char * blob=(char*)((int)blobContainer+10);
	char * blobSignature=(char*)((int)blob+*blobLen-20-10);
	
	/*printf("\nblobLen(%d):\n",*blobLen);hexaPrintf((char*)blobLen,4);
	printf("\nencBlob:\n");hexaPrintf(blob,*blobLen);*/
	//printf("\nblobSignature:\n");hexaPrintf(blobSignature,20);
	
	DWORD * decBlobLen=(DWORD *)blob;
	DWORD dbl=*decBlobLen;
	char * blobIv=(char*)((int)blob+4);
	char * encBlob=(char*)((int)blob+20);
	
	
	DWORD encLen=*blobLen-14-20-16;
	/*printf("\ndecBlobLen(%d):\n",dbl);hexaPrintf((char*)decBlobLen,4);
	printf("\nblobIv:\n");hexaPrintf((char*)blobIv,16);
	printf("\nencBlobData:\n");hexaPrintf((char*)encBlob,encLen);
	*/
	
	char sign2[20];
	
	getLoginDataSignature(key3,(char*)decBlobLen,encLen+20,sign2);
	/*printf("\nCOMPARE blob checksum with signature:\n");
	hexaPrintf(blobSignature,20);
	hexaPrintf(sign2,20);
	*/
	
	char decBlob[600];
	CRijndael aes2;
	aes2.MakeKey(key3, blobIv, 16, 16);
	aes2.ResetChain();
	aes2.Decrypt(encBlob,decBlob,encLen,CRijndael::CBC);
	
	printf("\nblob:\n");hexaPrintf(decBlob, dbl);
	
	Registry reg;
	RegistryVector * root= reg.parseVector(decBlob);
	
	/*	printf("\nserialized blob:\n");
	char a[1000];
	reg.serializeVector(root,a);
	hexaPrintf(a,dbl);
	*/
	printf("\nblob parse:\n");
	root->printXML();
	//free(data);
	
	
	if (1)
	{
		printf("key1  :\n");hexaPrintf(aesKey1,16);
		printf("key2  :\n");hexaPrintf(aesKey2,8);
		printf("key3  :\n");hexaPrintf(key3,16);
		printf("iv2   :\n");hexaPrintf(iv2,16);
		printf("blobIv:\n");hexaPrintf(blobIv,16);
		
	}
	
	debugTicket(ticket,tL, aesKey1,key3,iv2,blobIv);
	
	//	printf("reconstructed data : \n");hexaPrintf(res,resl);
}

long encryptAccountRecordBlob(char * encryptedBlobContainer, char * blob, DWORD blobLen, char * blobIv, char * key3)
{
	char temp[10000];
	memset(temp,0x10,10000);
	memcpy(temp,blob,blobLen);
	char * encryptedBlob=encryptedBlobContainer + 10;
	
	*((DWORD*)encryptedBlob)=blobLen;
	memcpy(encryptedBlob+4,blobIv,16);
	
	long encryptedBlobLen=blobLen+16;
	if (encryptedBlobLen%16)
	{
		encryptedBlobLen-=(encryptedBlobLen%16);
		encryptedBlobLen+=16;
	}
	
	CRijndael aes;
	aes.MakeKey(key3, blobIv, 16, 16);
	aes.ResetChain();
	aes.Encrypt(temp, encryptedBlob+20 ,encryptedBlobLen, CRijndael::CBC);
	
	encryptedBlobLen+=20;
	
	// sign blob
	getLoginDataSignature(key3,encryptedBlob,encryptedBlobLen,encryptedBlob+encryptedBlobLen);
	
	
	long encryptedBlobContainerLen=10+encryptedBlobLen+20;
	// create blob container
	*((WORD*)encryptedBlobContainer)=0x4501;
	*((DWORD*)(encryptedBlobContainer+2))=encryptedBlobContainerLen;
	*((DWORD*)(encryptedBlobContainer+6))=0;
	return encryptedBlobContainerLen;
}

long buildAccountInformation(char * blob, int blobLen, char * ticket, int ticketLen, char * plain2, int plain2Len, char * key1, char * key3, char * iv2, char * blobIv,char * result)
{
 	char encryptedBlobContainer[10000];
	// encrypt blob
	
	long encryptedBlobContainerLen=	encryptAccountRecordBlob(encryptedBlobContainer, blob, blobLen, blobIv, key3);
	
	// encrypt plain 2
	char key3AndPlain2[10000];
	memset(key3AndPlain2,0x0A,10000);
	
	memcpy(key3AndPlain2,key3,16);
	memcpy(key3AndPlain2+16,plain2,plain2Len);
	
	long encryptedPlain2Len=16+plain2Len;
	if (encryptedPlain2Len%16)
	{
		encryptedPlain2Len-=(encryptedPlain2Len%16);
		encryptedPlain2Len+=16;
	}
	
	char encryptedPlain2[10000];
	
	CRijndael aes;
	aes.MakeKey(key1, iv2, 16, 16);
	aes.ResetChain();
	aes.Encrypt(key3AndPlain2, encryptedPlain2 ,encryptedPlain2Len, CRijndael::CBC);
	
	char * data=result+2;
	
	memcpy(data,iv2,16);data+=16;
	WORD len=reverseBytes(plain2Len+16,2);
	memcpy(data,(char*)&len,2);data+=2;
	len=reverseBytes(encryptedPlain2Len,2);
	memcpy(data,(char*)&len,2);data+=2;
	memcpy(data,encryptedPlain2,encryptedPlain2Len);data+=encryptedPlain2Len;
	len=reverseBytes(ticketLen,2);
	memcpy(data,(char*)&len,2);data+=2;
	memcpy(data,ticket,ticketLen);data+=ticketLen;
	DWORD len2=reverseBytes(encryptedBlobContainerLen,4);
	memcpy(data,(char*)&len2,4);data+=4;
	memcpy(data,encryptedBlobContainer,encryptedBlobContainerLen);data+=encryptedBlobContainerLen;
	
	long dataLen=data-(result);
	
	// header
	*((WORD *)result)=0x0200;
	
	// sign everything
	getLoginDataSignature(key3,result,dataLen,result+dataLen);
	
	return dataLen+20;
}

int decryptPostTicketData(char loginKey3[16], char* ticket, DWORD ticketLen,char iv[16],DWORD decrLen,DWORD encrLen,char sha[20],char * encrData,char *decrData)
{
	int res=1;
	// checking sha
	/*printf("ticket:\n");
	hexaPrintf(ticket,ticketLen);
	printf("iv:\n");
	hexaPrintf(iv,16);
	printf("encrData:\n");
	hexaPrintf(encrData,encrLen);
	printf("sha:\n");
	hexaPrintf(sha,20);
	*/
	char six[0x40];memset(six,0x36,0x40); // first 0x10 should be xored with login key3
	XOR(six,loginKey3,16);
	char antislash[0x40];memset(antislash,0x5C,0x40); // first 0x10 should be xored with login key3
	XOR(antislash,loginKey3,16);
	char ticketSHA[160];
	
	CSHA1 sha1;
	sha1.Reset();
	sha1.Update((UINT_8 *)six,0x40);
	WORD tmp=reverseBytes(ticketLen,2);
	sha1.Update((UINT_8 *)&tmp,2);  
	sha1.Update((UINT_8 *)ticket,ticketLen);
	sha1.Update((UINT_8 *)iv,0x10);
	tmp=reverseBytes(decrLen,2);
	sha1.Update((UINT_8 *)&tmp,2);
	tmp=reverseBytes(encrLen,2);
	sha1.Update((UINT_8 *)&tmp,2);
	sha1.Update((UINT_8 *)encrData,encrLen);
	sha1.Final();
	sha1.GetHash((UINT_8 *)ticketSHA); //SHA-1 160 bits digest
	//printf("test1:\n");hexaPrintf(ticketSHA,20);
	char dataSHA[160];
	sha1.Reset();
	sha1.Update((UINT_8 *)antislash,0x40);
	sha1.Update((UINT_8 *)ticketSHA,0x14);
	sha1.Final();
	sha1.GetHash((UINT_8 *)dataSHA); //SHA-1 160 bits digest
	//printf("test2:\n");hexaPrintf(dataSHA,20);
	if (memcmp(sha,dataSHA,20)) res=0;
	
	// decrypt
	
	//	char key[16];memset(key,0,16); // first 0x10 should be xored with login key3
	
	CRijndael aes2;
	
	aes2.MakeKey(loginKey3, iv, 16, 16);
	aes2.ResetChain();
	aes2.Decrypt(encrData,decrData,encrLen,CRijndael::CBC);
	
	return res;
	
}