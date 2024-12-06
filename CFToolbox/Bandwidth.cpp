#include "bandwidth.h"
 
/*int __BWlock__=0;

void BWLock()
{
	while (1)
	{
		if (!__BWlock__)
		{
			__BWlock__++;
			return;
		}

		Sleep(10);
	}
}

void BWUnlock()
{
	__BWlock__--;
}*/
CRITICAL_SECTION bwCs; 
void BWLock()
{

	/* Enter the critical section -- other threads are locked out */
	EnterCriticalSection(&bwCs);
}
 
void BWUnlock()
{
	/* Leave the critical section -- other threads can now EnterCriticalSection() */
	LeaveCriticalSection(&bwCs);
}

CBandwidthMonitor * theOneAndUniqueBandwidthMonitor=0;
void registerDownloaderBandwidthMonitor(DWORD id)
{
	BWLock();
	CBandwidthMonitor * monitor=getBandwidthMonitor();
	if (monitor) 
	{
 		monitor->registerDownloader(id);
	}
	BWUnlock();
}
void unregisterDownloaderBandwidthMonitor(DWORD id)
{

	CBandwidthMonitor * monitor=getBandwidthMonitor();
	if (monitor) 
	{
 		monitor->unregisterDownloader(id);
	}
}
void notifyBandwidthMonitor(DWORD id, DWORD size)
{

	CBandwidthMonitor * monitor=getBandwidthMonitor();
	if (monitor) 
	{
 		monitor->notify(id,size);
	}
}

void startBandwidthMonitor( )
{
	/* Initialize the critical section -- This must be done before locking */
	InitializeCriticalSection(&bwCs);
	
	BWLock();
	theOneAndUniqueBandwidthMonitor=new CBandwidthMonitor(BW_INTERVAL,BW_HISTORY/BW_INTERVAL);
	theOneAndUniqueBandwidthMonitor->Start();
	BWUnlock();
	theOneAndUniqueBandwidthMonitor->registerDownloader(-1); // use to keep trace of finished downloads
}

CBandwidthMonitor * getBandwidthMonitor()
{
	if (!theOneAndUniqueBandwidthMonitor) startBandwidthMonitor();
	return theOneAndUniqueBandwidthMonitor;
}

CBandwidthMonitor::CBandwidthMonitor(int _intervalSec, int _nbInterval)
{
	intervalSec=_intervalSec;
	nbInterval=_nbInterval;
	index=0;
	downloaders=new HashMapDword(HASHMAP_FREE);
 
}


CBandwidthMonitor::~CBandwidthMonitor()
{
	theOneAndUniqueBandwidthMonitor=0; 	
	delete downloaders;
	/* Release system object when all finished -- usually at the end of the cleanup code */
	DeleteCriticalSection(&bwCs);

}



void CBandwidthMonitor::registerDownloader(DWORD id)
{
 	BWLock();
	DWORD * history=(DWORD*)malloc(4*nbInterval);
	memset((char*)history,0,4*nbInterval);
	downloaders->put(id,history);
	BWUnlock();
}


void CBandwidthMonitor::unregisterDownloader(DWORD id)
{
	BWLock();
	if (id!=-1)
	{
		DWORD * s=(DWORD *)downloaders->get(id);
		if (s)
		{
			DWORD * d=(DWORD *)downloaders->get(-1); // old downloads
			for (int ind=0;ind<nbInterval;ind++)
			{
				d[ind]+=s[ind];
			}
		}
	}
	downloaders->remove(id);
	BWUnlock();
}


void CBandwidthMonitor::unregisterAll()
{
	BWLock();
	downloaders->clear(); 
	BWUnlock();
}

void CBandwidthMonitor::notify(DWORD id, DWORD size)
{
	int i=index;
	DWORD * history=(DWORD *)downloaders->get(id);
	if (!history) return;
 	history[i]+=size;
}

void CBandwidthMonitor::getBandwidth(DWORD id, DWORD * history)
{
	
	memset((char*)history,0,4*nbInterval);
	DWORD * h=(DWORD *)downloaders->get(id);
	if (!h) return;
	int i=index;
	for (int ind=1;ind<nbInterval;ind++)
	{
		history[ind]=h[(i+ind)%nbInterval];
	}
}

void CBandwidthMonitor::getTotalBandwidth(DWORD * history)
{
	memset((char*)history,0,4*nbInterval);
	
	int size=downloaders->size();
	DWORD * ids=(DWORD*)malloc(4*size);
	size=downloaders->getDWordKeys(ids);
	
	int i=index;
	
	for (int n=0;n<size;n++)
	{
		DWORD * h=(DWORD *)downloaders->get(ids[n]);
		if (h)
		{
			for (int ind=1;ind<nbInterval;ind++)
			{
				history[ind]+=h[(i+ind)%nbInterval];
			}
		}
	}
 
	free(ids);
}
 

DWORD CBandwidthMonitor::Run( LPVOID /* arg */ )
{
	while (1)
	{
		Sleep(1000*intervalSec);
		BWLock();
		int i=index+1;
		if (i>=nbInterval) i=0;
		int size=downloaders->size();
		DWORD * ids=(DWORD*)malloc(4*size);
		size=downloaders->getDWordKeys(ids);
		for (int n=0;n<size;n++)
		{
			DWORD * h=(DWORD *)downloaders->get(ids[n]);
			if (h)
			{
				h[i]=0;
			}
		}
		free(ids);
		index=i;

		BWUnlock();
	}
}