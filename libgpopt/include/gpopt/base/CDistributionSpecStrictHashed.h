//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software
//
//---------------------------------------------------------------------------
#ifndef GPOPT_CDistributionSpecStrictHashed_H
#define GPOPT_CDistributionSpecStrictHashed_H

#include "gpos/base.h"
#include "gpos/utils.h"

#include "gpopt/base/CDistributionSpecHashed.h"

namespace gpopt
{
	using namespace gpos;
	
	//---------------------------------------------------------------------------
	//	@class:
	//		CDistributionSpecStrictHashed
	//
	//	@doc:
	//		Strict hash distribution only compatible with ANY, and not compatible with any other distributions
	//		This will force generating a hash redistribution motion.
	//
	//---------------------------------------------------------------------------
	class CDistributionSpecStrictHashed : public CDistributionSpecHashed
	{
			
		private:

			// copy ctor
			CDistributionSpecStrictHashed(const CDistributionSpecStrictHashed &);

		public:
		
			// ctor
			CDistributionSpecStrictHashed(DrgPexpr *pdrgpexpr, BOOL fNullsColocated);

			// ctor
			CDistributionSpecStrictHashed(DrgPexpr *pdrgpexpr, BOOL fNullsColocated, CDistributionSpecHashed *pdshashedEquiv);
			
			// distribution type accessor
			virtual 
			EDistributionType Edt() const
			{
				return CDistributionSpec::EdtStrictHashed;
			}

			// does this distribution match the given one
			virtual
			BOOL FMatch(const CDistributionSpec *pds) const;

			// does this distribution satisfy the given one
			virtual 
			BOOL FSatisfies(const CDistributionSpec *pds) const;
			
			// print
			virtual
			IOstream &OsPrint(IOstream &os) const;

			// conversion function
			static
			CDistributionSpecStrictHashed *PdsConvert
				(
				CDistributionSpec *pds
				)
			{
				GPOS_ASSERT(NULL != pds);
				GPOS_ASSERT(EdtStrictHashed == pds->Edt());

				return dynamic_cast<CDistributionSpecStrictHashed*>(pds);
			}

			// conversion function: const argument
			static
			const CDistributionSpecStrictHashed *PdsConvert
				(
				const CDistributionSpec *pds
				)
			{
				GPOS_ASSERT(NULL != pds);
				GPOS_ASSERT(EdtStrictHashed == pds->Edt());

				return dynamic_cast<const CDistributionSpecStrictHashed*>(pds);
			}


	}; // class CDistributionSpecStrictHashed

}

#endif // !GPOPT_CDistributionSpecStrictHashed_H

// EOF
