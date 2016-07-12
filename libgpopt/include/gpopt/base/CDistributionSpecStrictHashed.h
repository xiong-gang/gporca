//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software, Inc.

#ifndef GPOPT_CDistributionSpecStrictHashed_H
#define GPOPT_CDistributionSpecStrictHashed_H

#include "gpopt/base/CDistributionSpecHashed.h"

namespace gpopt
{
	class CDistributionSpecStrictHashed : public CDistributionSpecHashed
	{
		public:
			CDistributionSpecStrictHashed(DrgPexpr *pdrgpexpr, BOOL fNullsColocated);

			virtual EDistributionType Edt() const;

	};
}

#endif
