// Registry.cpp: implementation of the Registry class.
//
//////////////////////////////////////////////////////////////////////
#include <exception>
#include "malloc.h"
#include "stdio.h"
#include "stdafx.h"
#include "Registry.h"
#include "zlib/zlib.h"
#include "socketTools.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
int __Rlock__=0;

void RLock()
{
	while (1)
	{
		if (!__Rlock__)
		{
			__Rlock__++;
			return;
		}

		Sleep(100);
	}
}

void RUnlock()
{
	__Rlock__--;
}
*/
int initedCS=0;
CRITICAL_SECTION rCs; 
void RLock()
{
	if (!initedCS) 
	{
		initedCS=1;
		/* Initialize the critical section -- This must be done before locking */
		InitializeCriticalSection(&rCs);
	}
	/* Enter the critical section -- other threads are locked out */
	EnterCriticalSection(&rCs);
}
 
void RUnlock()
{
	if (!initedCS) 
	{
		initedCS=1;
		/* Initialize the critical section -- This must be done before locking */
		InitializeCriticalSection(&rCs);
	}
		/* Leave the critical section -- other threads can now EnterCriticalSection() */
	LeaveCriticalSection(&rCs);
}

Registry::Registry()
{
	
}

Registry::~Registry()
{
	
}

RegistryKey *  Registry::load(char * fileName)
{
	RLock();
	FILE * file=fopen(fileName,"rb");
	if (file==NULL)
	{
		//throw exception("File not found");
		RUnlock();
		return 0;
	}
	
	// obtain file size.
	fseek (file , 0 , SEEK_END);
	long lSize = ftell (file);
	rewind (file);
	
	char * blob;
	// allocate memory to contain the whole file.
	blob = (char*) malloc (lSize);
	if (!blob)  
	{
		RUnlock();
		return 0;//throw exception("File error");
	}
	
	// copy the file into the buffer.
	//fread (blob,1,lSize,file);
	DWORD read=0;
	while (read<lSize)
	{
		DWORD toRead=lSize-read>1000 ? 1000 : lSize-read;
		read+=fread(blob+read,1,toRead,file);
	}
	fclose(file);
	//printf(">load\n");
	RegistryKey * root=parseBlob(blob);
	
	free(blob);
	//printf("<load\n");
	RUnlock();
	return root;
}

unsigned long Registry::save(char * fileName, RegistryKey * root)
{
	RLock();

	
	char * blob=0;
	// allocate memory to contain the whole file.
	blob = (char*) malloc (0xFFFFFF*2);
	// dirty fix : doubled size on 2011-07 CFT 1.0.13
	
	unsigned long size=	serializeBlob(root,blob);
	
//	unlink(fileName); 
	FILE * file=fopen(fileName,"wb");
	if (!file)
	{
		free(blob);
		RUnlock();
		throw exception("File error");
		return 0;
	}	
	// copy the file into the buffer.

	// fix in cft 1.0.13 : unable to serialize in 1 time
	DWORD written=0;
	while (written<size)
	{
		DWORD toWrite=size-written>1000 ? 1000 : size-written;
		written+=fwrite (blob+written,1,toWrite,file);
	}
	fclose(file);
	//printf(">load\n");
	
	free(blob);
	//printf("<load\n");
	RUnlock();
	return size;
}


unsigned long Registry::parseValue(char * valueBlob,RegistryValue * value)
{
	//Sleep(500);
	//	 printf("parseValue\n");
	NodeHeader valueHeader;
	long readed=0;
	
	readed+=parseNodeHeader(&valueHeader,valueBlob);
	if (readed==valueHeader.dataLength) return readed+valueHeader.nullPadding;
	DWORD valueType=2;
	while (readed<valueHeader.dataLength)
	{
		// read value nodes at this level
		Node node;
		readed+=parseNode(&node,valueBlob+readed);
		DWORD nodeType;
		memcpy(&nodeType,valueBlob+readed,4);
		readed+=node.descriptorLength;
		//	printf("!! %d %d\n",readed,node.dataLength);
		//if (readed==node.dataLength) break;
		char * nodeData=valueBlob+readed;
		switch (nodeType)
		{
		case 1 :
			{
				memcpy(&valueType,nodeData,4);
				//				value->setType(valueType);
				// printf("value type : %d\n",valueType);
				break;
			}
		case 2:
			{
				char * valuea=nodeData;
				switch (valueType)
				{
				case TYPE_STRING : /*printf("%s\n",valuea); */value->setString(nodeData);break;
				case TYPE_DWORD :  /*printf("%d\n",valuea); */value->setDWORD(*(int*)nodeData);break;
				case TYPE_DATA : /*hexaPrintf(valuea,node.dataLength);*/ value->setData(nodeData,node.dataLength);break;
				default : throw exception("Invalid registry blob");
					
				}
				break;
			}
		default : throw exception("Invalid registry blob");
		}
		readed+=node.dataLength;
	}
	readed+=valueHeader.nullPadding;	
	return readed;
	
}

unsigned long Registry::parseValues(char * valuesBlob, RegistryKey * key)
{
	//printf(">parseValues\n");
	NodeHeader valuesHeader;
	long readed=0;
	
	readed+=parseNodeHeader(&valuesHeader,valuesBlob);
	if (readed==valuesHeader.dataLength) return readed+valuesHeader.nullPadding;
	while (readed<valuesHeader.dataLength)
	{
		// read values nodes at this level
		Node value;
		RegistryValue * keyValue;
		keyValue=new RegistryValue();
		readed+=parseNode(&value,valuesBlob+readed);
		char valueName[65535];
		memcpy(valueName,valuesBlob+readed,value.descriptorLength);valueName[value.descriptorLength]=0;
		keyValue->setName(valueName);
		//printf("#%s added at %s\n",valueName,key->getName());
		readed+=value.descriptorLength;
		
		char * valueContent=valuesBlob+readed;
		
		readed+=parseValue(valueContent,keyValue);
		
		
		
		key->addValue(keyValue);
		
	}
	readed+=valuesHeader.nullPadding;
	return readed;
}

unsigned long Registry::parseKeyContent(char * keyContent, RegistryKey * key)
{ //printf(">parseKeyContent\n");
	long readed=0;
	NodeHeader contentHeader;
	
	
	readed+=parseNodeHeader(&contentHeader,keyContent);
	if (readed==contentHeader.dataLength) return readed+contentHeader.nullPadding;
	while (readed<contentHeader.dataLength)
	{
		// read content nodes at this level
		
		Node node;
		readed+=parseNode(&node,keyContent+readed);
		DWORD nodeType;
		memcpy(&nodeType,keyContent+readed,4);
		readed+=node.descriptorLength;
		
		char * nodeData=keyContent+readed;
		
		
		
		switch (nodeType)
		{
		case 1 :
			{
				readed+=parseKey(nodeData,key);
				break;
			}
		case 2:
			{
				readed+=parseValues(nodeData,key);
				break;
			}
		default : throw exception("Invalid registry blob");
		}
		
		
		
	}
	readed+=contentHeader.nullPadding;
	return readed;
	
}

unsigned long Registry::parseKey(char * keyBlob,RegistryKey * parent)
{
	// Sleep(500);
	//printf(">parseKey\n");
	NodeHeader keyHeader;
	long readed=0;
	
	readed+=parseNodeHeader(&keyHeader,keyBlob);
	//printf("inparsekey %d %d\n",readed,keyHeader.dataLength);
	if (readed==keyHeader.dataLength) return readed+keyHeader.nullPadding;
	while (readed<keyHeader.dataLength)
	{
		// read keys Names at this level
		RegistryKey * key=new RegistryKey();
		
		Node keyNode;
		readed+=parseNode(&keyNode,keyBlob+readed);
		
		char keyName[65535];
		memcpy(keyName,keyBlob+readed,keyNode.descriptorLength);keyName[keyNode.descriptorLength]=0;
		key->setName(keyName);
		//	printf(">> %s / %s\n",parent->getName(),key->getName());
		//printf(">%s\n",keyName);
		readed+=keyNode.descriptorLength;
		
		char * keyContent=keyBlob+readed;
		
		readed+=parseKeyContent(keyContent,key);
		parent->addChild(key);
	}
	//printf("<parseKey\n");
	readed+=keyHeader.nullPadding;
	return readed;
	
}


RegistryKey * Registry::parseBlob(char * blob)
{
	//		printf("parseBlob\n");
	//printf(">parseRoot\n");
	RegistryKey preRoot;
	parseKey(blob,&preRoot);
	RegistryKey * res= preRoot.getFirstChild();
	preRoot.setFirstChild(0);
	return res;
	//	printf("<parseRoot\n");
}

unsigned long Registry::serializeBlob(RegistryKey * root,char * blob)
{
	//		printf("parseBlob\n");
	//printf(">serializeRoot\n");
	
	return serializeKey(root,blob);
	
	//	printf("<serializeRoot\n");
	//	return size;
	
}


RegistryVector * Registry::parseVector(char * blob)
{
	//		printf("parseBlob\n");
    //printf(">parseRoot\n");
	RegistryVector * vector=new RegistryVector();
	parseVectorHeader(blob,vector);
	return vector;
	//	printf("<parseRoot\n");
}

unsigned long Registry::parseNodeHeader(NodeHeader * nodeHeader,const char * blob)
{
	//		printf("parseNodeHeader\n");
	memcpy(&(nodeHeader->magic),blob,2);
	memcpy(&(nodeHeader->dataLength),(char *)((unsigned long)blob+2),4);
	memcpy(&(nodeHeader->nullPadding),(char *)((unsigned long)blob+6),4);
	if (nodeHeader->magic!=0x5001) throw exception("Invalid registry blob");
	//printf(">>>padding:%d\n",nodeHeader->nullPadding);
	return 10;
}

unsigned long Registry::parseNode(Node *node,const char * blob)
{
	//		printf("parseNode\n");
	memcpy(&(node->descriptorLength),blob,2);
	memcpy(&(node->dataLength),(char *)((unsigned long)blob+2),4);
	//	printf("parseNode %d %d\n",node->descriptorLength,node->dataLength);
	return 6;
}

unsigned long Registry::parseVectorHeader(char * blob, RegistryVector * vector)
{
	NodeHeader keyHeader;
	long readed=0;
	
	readed+=parseNodeHeader(&keyHeader,blob);
	
	if (readed==keyHeader.dataLength) return readed;
	while (readed<keyHeader.dataLength)
	{
		// read keys Names at this level
		//		RegistryKey * key=new RegistryKey();
		
		Node node;
		RegistryNode * rnode=new RegistryNode();
		vector->addNode(rnode);
		readed+=parseNode(&node,blob+readed);
		DWORD id;
		char name[65535];
		if (node.descriptorLength==4)
			memcpy(&id,blob+readed,node.descriptorLength);
		else
		{
			memcpy(name,blob+readed,node.descriptorLength);
			name[node.descriptorLength]=0;
		}
		//key->setName(keyName);
		//printf(">> %s\n",key->getName());
		//printf(">%s\n",keyName);
		readed+=node.descriptorLength;
		
		char * value=blob+readed;
		readed+=node.dataLength;
		//readed+=parseKeyContent(keyContent,key);
		if (node.descriptorLength==4)
			//printf(" - %d (%d bytes) : \n",id,node.dataLength);
			rnode->setDwordDescription(id);
		else
			//printf("- %s (%d bytes) : \n",name,node.dataLength);
		rnode->setStringDescription(name);
		
		WORD *magic=(WORD *)value;
		if (*magic==0x5001)
		{
			//printf("\n");
			RegistryVector * subVector=new RegistryVector();
			parseVectorHeader(value,subVector);
			rnode->setVectorValue(subVector);
		}
		else
		{
			switch (node.dataLength)
			{
			case 1:{
				rnode->setByteValue(*(BYTE*)value);
				break;
				   }
			case 2:{
				rnode->setWordValue(*(WORD*)value);
				break;
				   }
			case 4:{
				rnode->setDWordValue(*(DWORD*)value);
				break;
				   }
			case 8:{
				rnode->setUlonglongValue(*(ULONGLONG*)value);
				break;
				   }
			default:
				if (node.dataLength && !value[node.dataLength-1])
					rnode->setStringValue(value);
				else
					rnode->setDataValue(value,node.dataLength);
			}
		}
		//		Sleep(1000);
		//parent->addChild(key);
	}
	//printf("<parseKey\n");
	return readed;
	
}

unsigned long Registry::serializeKey(RegistryKey * key, char * keyBlob)
{
	//if (key)	printf(">serializeKey %s\n",key->getName());
	NodeHeader keyHeader;
	long written=10;
	keyHeader.dataLength=0;
	//if (readed==keyHeader.dataLength) return readed;
	while (key)
	{
		// read keys Names at this level
		Node keyNode;
		unsigned long start=written;
		written+=6;
		keyNode.descriptorLength=strlen(key->getName());
		memcpy(keyBlob+written,key->getName(),keyNode.descriptorLength);
		written+=keyNode.descriptorLength;
		
		keyNode.dataLength=serializeKeyContent(key,keyBlob+written);
		
		written+=keyNode.dataLength;
		
		serializeNode(&keyNode,keyBlob+start);
		
		//	keyHeader+=(2+4+keyNode.descriptorLength+keyNode.dataLength);
		
		key=key->getNextSibiling();
	}
	//printf("<parseKey\n");
	keyHeader.dataLength=written;
	serializeNodeHeader(&keyHeader,keyBlob);
	//if (key)	printf("<serializeKey %s\n",key->getName());
	return written;
}


unsigned long Registry::serializeNodeHeader(NodeHeader * nodeHeader,char * blob)
{
	//printf("serializeNodeHeader\n");
	nodeHeader->magic=0x5001;
	nodeHeader->nullPadding=0;
	memcpy(blob,&(nodeHeader->magic),2);
	memcpy((char *)((unsigned long)blob+2),&(nodeHeader->dataLength),4);
	memcpy((char *)((unsigned long)blob+6),&(nodeHeader->nullPadding),4);
	return 10;
}

unsigned long Registry::serializeNode(Node *node,char * blob)
{
	//printf("serializeNode\n");
	memcpy(blob,&(node->descriptorLength),2);
	memcpy((char *)((unsigned long)blob+2),&(node->dataLength),4);
	return 6;
}

unsigned long Registry::serializeKeyContent(RegistryKey * key,char * keyContent)
{
	//printf(">serializeKeyContent\n");
	long written=0;
	NodeHeader contentHeader;
	
	contentHeader.dataLength=0;
	
	written+=10;
	
	Node childrenNode;
	unsigned long start=written;
	childrenNode.descriptorLength=4;
	written+=6;
	unsigned long nodeType=1;
	memcpy(keyContent+written,&nodeType,4);
	
	
	written+=childrenNode.descriptorLength;
	char * nodeData=keyContent+written;
	childrenNode.dataLength=serializeKey(key->getFirstChild(),nodeData);
	written+=childrenNode.dataLength;
	serializeNode(&childrenNode,keyContent+start);
	
	
	Node valuesNode;
	start=written;
	valuesNode.descriptorLength=4;
	written+=6;
	nodeType=2;
	memcpy(keyContent+written,&nodeType,4);
	
	
	written+=valuesNode.descriptorLength;
	nodeData=keyContent+written;
	valuesNode.dataLength=serializeValues(key,nodeData);
	written+=valuesNode.dataLength;
	serializeNode(&valuesNode,keyContent+start);
	
	contentHeader.dataLength=written;
	serializeNodeHeader(&contentHeader,keyContent);
	//printf("<serializeKeyContent\n");
	return written;
	
}

unsigned long Registry::serializeValues(RegistryKey * key,char * valuesBlob)
{
	//printf(">serializeValues\n");
	NodeHeader valuesHeader;
	long written=0;
	valuesHeader.dataLength=0;
	
	written+=10;
	RegistryValue * keyValue=key->getFirstValue();
	while (keyValue)
	{
		
		// read values nodes at this level
		Node value;
		unsigned long start=written;
		written+=6;
		value.descriptorLength=strlen(keyValue->getName());
		memcpy(valuesBlob+written,keyValue->getName(),value.descriptorLength);
		written+=value.descriptorLength;
		
		char * valueContent=valuesBlob+written;
		
		value.dataLength=serializeValue(keyValue,valueContent);
		
		written+=value.dataLength;
		
		serializeNode(&value,valuesBlob+start);
		keyValue=keyValue->getNextValue();		
	}
	valuesHeader.dataLength=written;
	serializeNodeHeader(&valuesHeader,valuesBlob);
	//	printf(">serializeValues\n");
	return written;
}

unsigned long Registry::serializeValue(RegistryValue * value,char * valueBlob)
{
	//	printf(">serializeValue\n");
	//Sleep(500);
	//printf("parseValue\n");
	NodeHeader valueHeader;
	valueHeader.dataLength=0;
	long written=0;
	
	written+=10;
	
	//if (readed==valueHeader.dataLength) return readed;
	//	DWORD valueType=2;
	//while (readed<valueHeader.dataLength)
	//{
	// read value nodes at this level
	Node typeNode;
	unsigned long start=written;
	written+=6;
	
	DWORD description=1;
	memcpy(valueBlob+written,&description,4);
	typeNode.descriptorLength=4;
	written+=4;
	unsigned long type=value->getType();
	memcpy(valueBlob+written,&type,4);
	typeNode.dataLength=4;
	written+=4;
	
	serializeNode(&typeNode,valueBlob+start);
	
	
	Node valueNode;
	start=written;
	written+=6;
	
	unsigned long len=0;
	char * nodeData=0;
	DWORD dword;
	
	switch (value->getType())
	{
	case TYPE_STRING : len=strlen(value->getString())+1;nodeData=value->getString();break;
	case TYPE_DWORD : len=4;dword=value->getDWORD();nodeData=(char*)&dword;break;
	case TYPE_DATA : len=value->getDataSize();nodeData=value->getData();break;
	}
	description=2;
	memcpy(valueBlob+written,(char*)&description,4);
	valueNode.descriptorLength=4;
	written+=4;
	memcpy(valueBlob+written,nodeData,len);
	valueNode.dataLength=len;
	written+=len;
	
	serializeNode(&valueNode,valueBlob+start);
	
	
	
	//	written+=valueNode.dataLength;
	
	valueHeader.dataLength=written;
	serializeNodeHeader(&valueHeader,valueBlob);	
	//	printf(">serializeValue\n");
	return written;
	
				
}

RegistryKey * Registry::createNewRegistry()
{
	RegistryKey * key=new RegistryKey();
	key->setName("TopKey");
	return key;
}


unsigned long Registry::serializeVector(RegistryVector * vector,char * blob)
{
 	NodeHeader keyHeader;
	long written=0;
	
//	readed+=parseNodeHeader(&keyHeader,blob);
	written+=10;	
	
	for (int ind=0;ind<vector->getNbNodes();ind++)
	{
		// read keys Names at this level
		//		RegistryKey * key=new RegistryKey();
		
		Node node;
		RegistryNode * rnode=vector->getNodeAt(ind);
	//	printf("serializing : %d %d at %d typr=%d\n",rnode->getDwordDescription(),rnode->getByteValue(),written,rnode->getDescriptionType());
		unsigned long start=written;
		written+=6;

//		readed+=parseNode(&node,blob+readed);

		if (rnode->getDescriptionType()==NODE_DESC_TYPE_DWORD)
		{
			node.descriptorLength=4;
			DWORD id=rnode->getDwordDescription();
			memcpy(blob+written,&id,node.descriptorLength);
		}
		else
		{

			node.descriptorLength=strlen(rnode->getStringDescription());
			memcpy(blob+written,rnode->getStringDescription(),node.descriptorLength);
		}
		//	printf("want serializing : %d %d at %d, ds=%d,vs=%d\n",rnode->getDwordDescription(),rnode->getByteValue(),start,node.descriptorLength,node.dataLength);
	
		written+=node.descriptorLength;
		
		char * value=blob+written;

		switch (rnode->getValueType())
		{
		case NODE_VALUE_TYPE_BYTE : {
			node.dataLength=1;
			BYTE v=rnode->getByteValue();
			memcpy(value,&v,node.dataLength);
			break;
			   }
		case NODE_VALUE_TYPE_WORD : {
			node.dataLength=2;
			WORD v=rnode->getWordValue();
			memcpy(value,&v,node.dataLength);
			break;
			   }
		case NODE_VALUE_TYPE_DWORD : {
			node.dataLength=4;
			DWORD v=rnode->getDWordValue();
			memcpy(value,&v,node.dataLength);
			break;
			   }
		case NODE_VALUE_TYPE_ULONGLONG : {
			node.dataLength=8;
			ULONGLONG v=rnode->getUlonglongValue();
			memcpy(value,&v,node.dataLength);
			break;
			   }
		case NODE_VALUE_TYPE_STRING : {
			node.dataLength=strlen(rnode->getStringValue())+1;
			memcpy(value,rnode->getStringValue(),node.dataLength);
			break;
			   }
		case NODE_VALUE_TYPE_VECTOR : {
			node.dataLength=serializeVector(rnode->getVectorValue(),value);
			break;
			   }
		default:
			{
				node.dataLength=rnode->getDataValueSize();
				memcpy(value,rnode->getDataValue(),node.dataLength);
			}

		}

		written+=node.dataLength;
		//		Sleep(1000);
		//parent->addChild(key);
	//		printf("really serializing : %d %d at %d, ds=%d,vs=%d\n",rnode->getDwordDescription(),rnode->getByteValue(),start,node.descriptorLength,node.dataLength);

		serializeNode(&node,blob+start);
	
	}
		
	keyHeader.dataLength=written;
	serializeNodeHeader(&keyHeader,blob);			

	//printf("<parseKey\n");
	return written;
}

unsigned long Registry::zipBlob(char * blob,unsigned long blobSize, char ** zippedBlob)
{
	uLongf maxZipSize=compressBound(blobSize);
	char * zbuffer=(char*)malloc(maxZipSize);
	int result=compress2((unsigned char*)zbuffer,&maxZipSize,(unsigned char*)blob,blobSize,Z_BEST_COMPRESSION);
	if (result) throw exception("Zip error");

	ZipHeader header;
	header.zipLength=maxZipSize;
	header.blobLength=blobSize;
	header.zData=zbuffer;
	*zippedBlob=(char*)malloc(0x14+maxZipSize);
	serializeZipHeader(&header,*zippedBlob);

	free(zbuffer);
	return header.zipLength;
}

unsigned long Registry::unzipBlob(char * zippedBlob,char ** blob)
{
	ZipHeader header;
	parseZipHeader(&header,zippedBlob);
	*blob=(char*)malloc(header.blobLength);

	int result=uncompress((unsigned char*)*blob,&header.blobLength,(unsigned char*)header.zData,(DWORD)header.zipLength);
	if (result) throw exception("Unzip error");

	return header.blobLength;
}
unsigned long Registry::serializeZipHeader(ZipHeader * zipHeader,char * blob)
{
	//printf("serializeNodeHeader\n");
	zipHeader->magic=0x4301;
	zipHeader->zipLength+=0x14;
	zipHeader->zipPadding=0;
	zipHeader->blobPadding=0;
	zipHeader->unknown=9;
	memcpy(blob,&(zipHeader->magic),2);
	memcpy((char *)((unsigned long)blob+2),&(zipHeader->zipLength),4);
	memcpy((char *)((unsigned long)blob+6),&(zipHeader->zipPadding),4);
	memcpy((char *)((unsigned long)blob+10),&(zipHeader->blobLength),4);
	memcpy((char *)((unsigned long)blob+14),&(zipHeader->blobPadding),4);
	memcpy((char *)((unsigned long)blob+18),&(zipHeader->unknown),2);
	memcpy((char *)((unsigned long)blob+20), zipHeader->zData,zipHeader->zipLength-0x14);
	return 0x14;
}
unsigned long Registry::parseZipHeader(ZipHeader * zipHeader,char * blob)
{
		//		printf("parseNodeHeader\n");
	memcpy(&(zipHeader->magic),blob,2);
	memcpy(&(zipHeader->zipLength),(char *)((unsigned long)blob+0x02),4);
	memcpy(&(zipHeader->zipPadding),(char *)((unsigned long)blob+0x06),4);
	memcpy(&(zipHeader->blobLength),(char *)((unsigned long)blob+0x0A),4) ;
	memcpy(&(zipHeader->blobPadding),(char *)((unsigned long)blob+0x0E),4);
	memcpy(&(zipHeader->unknown),(char *)((unsigned long)blob+0x12),2);
	zipHeader->zipLength-=0x14;
	zipHeader->zData=blob+0x14;
	if (zipHeader->magic!=0x4301) throw exception("Invalid zipped blob");
	//printf(">>>padding:%d\n",nodeHeader->nullPadding);
	return 0x14;
}