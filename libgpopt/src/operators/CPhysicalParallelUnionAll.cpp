//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.

#include "gpopt/operators/CPhysicalParallelUnionAll.h"
#include "gpopt/base/CDistributionSpecStrictHashed.h"
#include "gpopt/base/CUtils.h"
#include "gpopt/operators/CExpressionHandle.h"

namespace gpopt
{
	CPhysicalParallelUnionAll::CPhysicalParallelUnionAll
		(
			IMemoryPool *pmp,
			DrgPcr *pdrgpcrOutput,
			DrgDrgPcr *pdrgpdrgpcrInput,
			ULONG ulScanIdPartialIndex
		) : CPhysicalUnionAll
		(
			pmp,
			pdrgpcrOutput,
			pdrgpdrgpcrInput,
			ulScanIdPartialIndex
		),
			m_pdrgpds(NULL)
	{
		BuildHashedDistributions(pmp);
	}

	COperator::EOperatorId CPhysicalParallelUnionAll::Eopid() const
	{
		return EopPhysicalParallelUnionAll;
	}

	const CHAR *CPhysicalParallelUnionAll::SzId() const
	{
		return "CPhysicalParallelUnionAll";
	}

	CDistributionSpec *CPhysicalParallelUnionAll::PdsDerive(IMemoryPool*, CExpressionHandle&) const
	{
		CDistributionSpec *pdsFirstChild = (*m_pdrgpds)[0];
		pdsFirstChild->AddRef();
		return pdsFirstChild;
	}

	CEnfdProp::EPropEnforcingType
	CPhysicalParallelUnionAll::EpetDistribution(CExpressionHandle& , const CEnfdDistribution* ) const
	{
		return CEnfdProp::EpetRequired;
	}

	CDistributionSpec *
	CPhysicalParallelUnionAll::PdsRequired
		(
			IMemoryPool *,
			CExpressionHandle &,
			CDistributionSpec *,
			ULONG ulChildIndex,
			DrgPdp *,
			ULONG
		)
	const
	{
		CDistributionSpec *pdsChild = (*m_pdrgpds)[ulChildIndex];
		pdsChild->AddRef();
		return pdsChild;
	}

	void CPhysicalParallelUnionAll::BuildHashedDistributions(IMemoryPool *pmp)
	{
		{
			GPOS_ASSERT(NULL == m_pdrgpds);

			m_pdrgpds = GPOS_NEW(pmp) DrgPds(pmp);
			const ULONG ulCols = PdrgpcrOutput()->UlLength();
			const ULONG ulArity = PdrgpdrgpcrInput()->UlLength();
			for (ULONG ulChild = 0; ulChild < ulArity; ulChild++)
			{
				DrgPcr *pdrgpcr = (*PdrgpdrgpcrInput())[ulChild];
				DrgPexpr *pdrgpexpr = GPOS_NEW(pmp) DrgPexpr(pmp);
				for (ULONG ulCol = 0; ulCol < ulCols; ulCol++)
				{
					CExpression *pexpr = CUtils::PexprScalarIdent(pmp, (*pdrgpcr)[ulCol]);
					pdrgpexpr->Append(pexpr);
				}

				// create a hashed distribution on input columns of the current child
				CDistributionSpecHashed *pdshashed = GPOS_NEW(pmp) CDistributionSpecStrictHashed(pdrgpexpr, true /*fNullsColocated*/);
				m_pdrgpds->Append(pdshashed);
			}
		}
	}

	CPhysicalParallelUnionAll::~CPhysicalParallelUnionAll()
	{
		m_pdrgpds->Release();
	}
}
