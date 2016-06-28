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
		class CDistributionSpecStrictHashed;

		private:
			// compute required hashed distribution of the n-th child
			CDistributionSpecStrictHashed *PdsstricthashedPassThru(IMemoryPool *pmp, CDistributionSpecStrictHashed *pdsstricthashedRequired, ULONG ulChildIndex) const;

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
	};
}

#endif // !GPOPT_CPhysicalParallelUnionAll_H
