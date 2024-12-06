#ifndef _BANDWIDTH_H_
#define _BANDWIDTH_H_

#include "HashMap.h"

#include "Thread.h"
#define BW_INTERVAL 4 // 4 secs interval
#define BW_HISTORY 60*4 // 4 minutes

class CBandwidthMonitor : public CThread
{
public : 
	void registerDownloader(DWORD id);
	void unregisterDownloader(DWORD id);
	void unregisterAll();
	void notify(DWORD id, DWORD size);
	CBandwidthMonitor(int intervalSec, int nbInterval);
	~CBandwidthMonitor();
	virtual DWORD Run( LPVOID /* arg */ );
	int intervalSec;
	int nbInterval;

	void getBandwidth(DWORD id, DWORD * history);
	void getTotalBandwidth(DWORD * history);
	HashMapDword * downloaders;
private:
	int index;

};

void registerDownloaderBandwidthMonitor(DWORD id);
void unregisterDownloaderBandwidthMonitor(DWORD id);
void notifyBandwidthMonitor(DWORD id, DWORD size);

//void startBandwidthMonitor();

CBandwidthMonitor * getBandwidthMonitor();

#endif