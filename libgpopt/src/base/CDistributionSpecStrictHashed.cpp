//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.

#include "gpopt/base/CDistributionSpecStrictHashed.h"

namespace gpopt
{
	CDistributionSpecStrictHashed::CDistributionSpecStrictHashed
			(
					DrgPexpr *pdrgpexpr,
					BOOL fNullsColocated
			):CDistributionSpecHashed(pdrgpexpr, fNullsColocated)
	{}

	CDistributionSpec::EDistributionType CDistributionSpecStrictHashed::Edt() const
	{
		return CDistributionSpec::EdtStrictHashed;
	}

	BOOL CDistributionSpecStrictHashed::FSatisfies(const CDistributionSpec *pds) const
	{
		return CDistributionSpecHashed::FSatisfies(pds);
	}

	BOOL CDistributionSpecStrictHashed::FMatch(const CDistributionSpec *pds) const
	{
		if (Edt() != pds->Edt())
		{
			return false;
		}

		return true;
	}

	IOstream &CDistributionSpecStrictHashed::OsPrint(IOstream &os) const
	{
		return os << "HAISHENG";
	}
}
