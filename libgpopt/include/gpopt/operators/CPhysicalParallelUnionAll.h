//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.

#ifndef GPOPT_CPhysicalParallelUnionAll_H
#define GPOPT_CPhysicalParallelUnionAll_H

#include "gpopt/operators/CPhysicalUnionAll.h"

namespace gpopt
{
	class CPhysicalParallelUnionAll : public CPhysicalUnionAll
	{
		private:
			// array of child hashed distributions -- used locally for distribution derivation
			DrgPds *m_pdrgpds;

			void BuildHashedDistributions(IMemoryPool* pmp);

		public:
			CPhysicalParallelUnionAll(IMemoryPool *pmp, DrgPcr *pdrgpcrOutput, DrgDrgPcr *pdrgpdrgpcrInput,
									  ULONG ulScanIdPartialIndex);

			virtual EOperatorId Eopid() const;

			virtual const CHAR *SzId() const;

			virtual CDistributionSpec *
			PdsRequired(IMemoryPool *pmp, CExpressionHandle &exprhdl, CDistributionSpec *pdsRequired,
						ULONG ulChildIndex, DrgPdp *pdrgpdpCtxt, ULONG ulOptReq) const;

			virtual CDistributionSpec *PdsDerive(IMemoryPool *pmp, CExpressionHandle &exprhdl) const;

			virtual CEnfdProp::EPropEnforcingType
			EpetDistribution(CExpressionHandle &exprhdl, const CEnfdDistribution *ped) const;

			virtual ~CPhysicalParallelUnionAll();
	};
}

#endif //GPOPT_CPhysicalParallelUnionAll_H
