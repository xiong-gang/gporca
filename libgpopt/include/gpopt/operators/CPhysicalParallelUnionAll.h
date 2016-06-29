//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software
//
//---------------------------------------------------------------------------
#ifndef GPOPT_CPhysicalParallelUnionAll_H
#define GPOPT_CPhysicalParallelUnionAll_H

#include "gpos/base.h"
#include "gpopt/operators/CPhysicalUnionAll.h"

namespace gpopt
{
	class CPhysicalParallelUnionAll : public CPhysicalUnionAll
	{
		protected:
			virtual CDistributionSpecHashed* BuildHashedDistribution(IMemoryPool* pmp, DrgPexpr *pdrgpexpr);

		public:
			CPhysicalParallelUnionAll
				(
				IMemoryPool *pmp,
				DrgPcr *pdrgpcrOutput,
				DrgDrgPcr *pdrgpdrgpcrInput,
				ULONG ulScanIdPartialIndex
				);

			virtual
			CDistributionSpec *PdsRequired
				(
				IMemoryPool *pmp,
				CExpressionHandle &exprhdl,
				CDistributionSpec *pdsRequired,
				ULONG ulChildIndex,
				DrgPdp *pdrgpdpCtxt,
				ULONG ulOptReq
				)
				const;

			const CHAR* SzId() const;
	};
}

#endif // !GPOPT_CPhysicalParallelUnionAll_H
