//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 Pivotal Software
//
//---------------------------------------------------------------------------

#include "gpopt/base/CDistributionSpecStrictHashed.h"

using namespace gpopt;


//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecStrictHashed::CDistributionSpecStrictHashed
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CDistributionSpecStrictHashed::CDistributionSpecStrictHashed
	(
	DrgPexpr *pdrgpexpr,
	BOOL fNullsColocated
	)
	:
	CDistributionSpecHashed(pdrgpexpr, fNullsColocated)
{}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecHashed::CDistributionSpecHashed
//
//	@doc:
//		Ctor
//
//---------------------------------------------------------------------------
CDistributionSpecStrictHashed::CDistributionSpecStrictHashed
	(
	DrgPexpr *pdrgpexpr,
	BOOL fNullsColocated,
	CDistributionSpecHashed *pdshashedEquiv
	)
	:
	CDistributionSpecHashed(pdrgpexpr, fNullsColocated, pdshashedEquiv)
	{}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecStrictHashed::FMatch
//
//	@doc:
//		Match function
//
//---------------------------------------------------------------------------
BOOL
CDistributionSpecStrictHashed::FMatch
	(
	const CDistributionSpec *pds
	)
	const
{
	CDistributionSpec::EDistributionType inputDistributionType = pds->Edt();

	if (CDistributionSpec::EdtHashed != inputDistributionType
			&& CDistributionSpec::EdtStrictHashed != inputDistributionType)
	{
		return false;
	}

	const CDistributionSpecStrictHashed *pdshashed = CDistributionSpecStrictHashed::PdsConvert(pds);

	// TODO: I skipped the join condition comparison here.

	return FMatchHashedDistribution(pdshashed);
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecStrictHashed::FSatisfies
//
//	@doc:
//		StrictHash only satisfy ANY
//
//---------------------------------------------------------------------------
BOOL
CDistributionSpecStrictHashed::FSatisfies
	(
	const CDistributionSpec *pdss
	)
	const
{	
	if (FMatch(pdss))
	{
		return true;
	}

	if (EdtAny == pdss->Edt())
	{
		return true;
	}
	
	return false;
}

//---------------------------------------------------------------------------
//	@function:
//		CDistributionSpecStrictHashed::OsPrint
//
//---------------------------------------------------------------------------
IOstream &
CDistributionSpecStrictHashed::OsPrint
	(
	IOstream &os
	)
	const
{
	os << "STRICT ";
	CDistributionSpecHashed::OsPrint(os);

	return os;
}

// EOF

