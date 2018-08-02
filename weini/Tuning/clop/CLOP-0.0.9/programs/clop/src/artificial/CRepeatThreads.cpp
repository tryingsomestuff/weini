/////////////////////////////////////////////////////////////////////////////
//
// CRepeatThreads.cpp
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CRepeatThreads.h"
#include "CCPListener.h"
#include "CArtificialExperiment.h"

/////////////////////////////////////////////////////////////////////////////
// Dispatch next seed
/////////////////////////////////////////////////////////////////////////////
bool CRepeatThreads::Dispatch(CRepeatThread &rt)
{
 //boost::lock_guard<boost::mutex> lg(mutDispatch);
 boost::mutex::scoped_lock sl(mutDispatch);

 rt.Seed = --CurrentSeed;

 return rt.Seed >= 0 && (!pcpl || pcpl->Continue());
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CRepeatThreads::CRepeatThreads(int TotalRepeats,
                               int Samples,
                               CCPListener *pcpl):
 TotalRepeats(TotalRepeats),
 Samples(Samples),
 CurrentSeed(TotalRepeats),
 pcpl(pcpl)
{
 vpcpd.push_back(new CCheckPointData(TotalRepeats, Samples, pcpl));
}

/////////////////////////////////////////////////////////////////////////////
// Start
/////////////////////////////////////////////////////////////////////////////
void CRepeatThreads::Start()
{
 if (pcpl)
  pcpl->OnStart(*vpcpd[0]);
 for (int i = int(vpartexp.size()); --i >= 0;)
 {
  vpartexp[i]->Reserve(Samples);
  tg.create_thread(CRepeatThread(*this, *vpartexp[i]));
 }
}

/////////////////////////////////////////////////////////////////////////////
// Wait for termination
/////////////////////////////////////////////////////////////////////////////
void CRepeatThreads::WaitForTermination()
{
 tg.join_all();
 if (pcpl)
  pcpl->OnStop(*vpcpd[0]);
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
CRepeatThreads::~CRepeatThreads()
{
 for (int i = int(vpcpd.size()); --i >= 0;)
  delete vpcpd[i];
}
