//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.

#ifndef GPOPT_CDistributionSpecStrictHashed_H
#define GPOPT_CDistributionSpecStrictHashed_H

#include "gpopt/base/CDistributionSpecHashed.h"
#include "gpopt/base/CDistributionSpec.h"

namespace gpopt
{
	class CDistributionSpecStrictHashed : public CDistributionSpecHashed
	{
		public:
			CDistributionSpecStrictHashed(DrgPexpr *pdrgpexpr, BOOL fNullsColocated);

			CDistributionSpec::EDistributionType Edt() const;

			BOOL FSatisfies(const CDistributionSpec *pds) const;

			BOOL FMatch(const CDistributionSpec *pds) const;

			IOstream &OsPrint(IOstream &os) const;
	};
}
#endif //GPOPT_CDistributionSpecStrictHashed_H
