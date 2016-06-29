//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software
//
//---------------------------------------------------------------------------

#include "gpopt/operators/CPhysicalParallelUnionAll.h"
#include "gpopt/exception.h"
#include "gpopt/base/CDistributionSpecHashed.h"
#include "gpopt/base/CDistributionSpecStrictHashed.h"
#include "gpopt/base/CUtils.h"

using namespace gpopt;

CPhysicalParallelUnionAll::CPhysicalParallelUnionAll
(
IMemoryPool *pmp,
DrgPcr *pdrgpcrOutput,
DrgDrgPcr *pdrgpdrgpcrInput,
ULONG ulScanIdPartialIndex
):CPhysicalUnionAll
(
		pmp,
		pdrgpcrOutput,
		pdrgpdrgpcrInput,
		ulScanIdPartialIndex
		)
{
	// ParallelUnionAll creates only one distribution requests to enforce distribution of its children:
	// (1) (StrictHashed, StrictHashed, ...): used to pass hashed distribution (requested from above)
	//     to child operators and match request Exactly
	SetDistrRequests(1 /*ulDistrReq*/);
	m_pdrgpds->Release();
	m_pdrgpds = nullptr;
	BuildHashedDistributions(pmp);
}

CDistributionSpec * CPhysicalParallelUnionAll::PdsRequired
(
IMemoryPool *pmp,
CExpressionHandle &exprhdl,
CDistributionSpec *pdsRequired,
ULONG ulChildIndex,
DrgPdp * /*pdrgpdpCtxt*/,
ULONG  ulOptReq
)
const
{
	GPOS_ASSERT(NULL != PdrgpdrgpcrInput());
	GPOS_ASSERT(ulChildIndex < PdrgpdrgpcrInput()->UlLength());
	GPOS_ASSERT(0 == ulOptReq);

	CDistributionSpec *pds = PdsMasterOnlyOrReplicated(pmp, exprhdl, pdsRequired, ulChildIndex, ulOptReq);
	if (NULL != pds)
	{
		return pds;
	}

	CDistributionSpec* yolo = PdrgpdsChildHashedDistributions(ulChildIndex);
	yolo->AddRef();
	return yolo;
}

const CHAR* CPhysicalParallelUnionAll::SzId() const
{
	return "CPhysicalParallelUnionAll";
}

CDistributionSpecHashed* CPhysicalParallelUnionAll::BuildHashedDistribution(IMemoryPool* pmp, DrgPexpr *pdrgpexpr)
{
	return GPOS_NEW(pmp) CDistributionSpecStrictHashed(pdrgpexpr, true /*fNullsColocated*/);
}
