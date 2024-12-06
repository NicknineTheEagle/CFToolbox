// RegistryKey.h: interface for the RegistryKey class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRYKEY_H__73CB1F0F_49D0_49AB_AE57_EE9A3F50340D__INCLUDED_)
#define AFX_REGISTRYKEY_H__73CB1F0F_49D0_49AB_AE57_EE9A3F50340D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RegistryValue.h"

class RegistryKey  
{
public:
//	void removeValueAt(unsigned int ind);
//	RegistryValue * valueAt(unsigned int ind);
//	unsigned int getNbValues();
	void removeValue(char * name);
	RegistryValue * getValue(char * name);
	RegistryValue * getFirstValue();
//	void setValue(RegistryValue * value);
	void addValue(RegistryValue * value);
	RegistryKey * getKey(char * path);
	RegistryKey * setKey(char * path);
	RegistryKey * getChild(char * name);
	RegistryKey *  remove();
	void addChild(RegistryKey * child);
	RegistryKey * getFirstChild();
	void setFirstChild(RegistryKey * child);
	void insertNextSibiling(RegistryKey * sibiling);
	void insertPreviousSibiling(RegistryKey * sibiling);
	void setNextSibiling(RegistryKey * sibiling);
	void setPreviousSibiling(RegistryKey * sibiling);
	RegistryKey * getLastSibiling();
	RegistryKey * getFirstSibiling();
	RegistryKey * getNextSibiling();
	RegistryKey * getPreviousSibiling();
	void setParent(RegistryKey * parent);
	RegistryKey * getParent();
	void setName(char * name);
	char * getName();
	RegistryKey();
	virtual ~RegistryKey();

	void printXML(int withHex=1, char * indent=0);

private:
		//unsigned int nbValues;
	RegistryValue * value;
	RegistryKey * firstChild;
	RegistryKey * previousSibiling;
	RegistryKey * nextSibiling;
	char * name;
	RegistryKey * parent;
};

#endif // !defined(AFX_REGISTRYKEY_H__73CB1F0F_49D0_49AB_AE57_EE9A3F50340D__INCLUDED_)
