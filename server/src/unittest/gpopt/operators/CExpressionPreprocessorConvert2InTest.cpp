
//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 EMC Corp.
//
//	@filename:
//		CExpressionPreprocessorConvert2InTest.cpp
//
//	@doc:
//		Test for expression preprocessing
//---------------------------------------------------------------------------
#define ALLOW_strstr

#include <string.h>

#include "gpos/io/COstreamString.h"
#include "gpos/string/CWStringDynamic.h"
#include "gpos/task/CAutoTraceFlag.h"

#include "gpopt/base/CUtils.h"
#include "gpopt/eval/CConstExprEvaluatorDefault.h"
#include "gpopt/mdcache/CAutoMDAccessor.h"
#include "gpopt/operators/CPredicateUtils.h"
#include "gpopt/operators/CScalarProjectElement.h"
#include "gpopt/operators/CLogicalNAryJoin.h"
#include "gpopt/operators/CLogicalInnerJoin.h"
#include "gpopt/operators/CLogicalLeftOuterJoin.h"
#include "gpopt/operators/CExpressionUtils.h"
#include "gpopt/xforms/CXformUtils.h"

#include "unittest/base.h"
#include "../../../../include/unittest/gpopt/operators/CExpressionPreprocessorConvert2InTest.h"
#include "unittest/gpopt/CTestUtils.h"

#include "naucrates/md/CMDProviderMemory.h"
#include "naucrates/md/CMDIdGPDB.h"

using namespace gpopt;
using namespace gpmd;

//---------------------------------------------------------------------------
//	@function:
//		CExpressionPreprocessorConvert2InTest::fContainsArrayExpr
//
//	@doc:
//		checks if the expression or any of its children contains an array predicate
//
//---------------------------------------------------------------------------
BOOL
CExpressionPreprocessorConvert2InTest::fContainsArrayExpr
	(
		CExpression *pexpr
	)
{
	if (CPredicateUtils::FCompareIdentToConstArray(pexpr))
	{
		return true;
	}

	BOOL fchildIsArray = false;
	for (ULONG ulChild = 0; !fchildIsArray && ulChild < pexpr->UlArity(); ulChild++)
	{
		fchildIsArray = fContainsArrayExpr((*pexpr)[ulChild]);
	}
	return fchildIsArray;
}

////---------------------------------------------------------------------------
////	@function:
////		CExpressionPreprocessorConvert2InTest::PExprCreateScalarArray
////
////	@doc:
////		Takes ownership of the passed array.
////
////---------------------------------------------------------------------------
//CExpression*
//CExpressionPreprocessorConvert2InTest::PExprCreateScalarArray(IMemoryPool *pmp, DrgPexpr *prngexprScalarConsts)
//{
//	GPOS_ASSERT(NULL != prngexprScalarConsts);
//	GPOS_ASSERT(0 < prngexprScalarConsts->UlLength());
//
//	CScalarConst *pScalarConst = CScalarConst::PopConvert((*prngexprScalarConsts)[0]->pop());
//	IMDId *pmdidArrType = pScalarConst->PmdidType();
//
//	// Used twice
//	pmdidArrType->AddRef();
//	pmdidArrType->AddRef();
//
//	return GPOS_NEW(pmp) CExpression
//								(
//								pmp,
//								GPOS_NEW(pmp) CScalarArray(pmp, pmdidArrType, pmdidArrType, false /*fMultiDimensional*/),
//								prngexprScalarConsts
//								);
//}
//
//CExpression*
//CExpressionPreprocessorConvert2InTest::PExprCreateArrayInExpression(IMemoryPool *pmp, CExpression *pexprConstArray)
//{
//
//
//	CExpression *pexprIdent = CUtils::PexprScalarIdent(pmp, m_pcr);
//
//	CExpression *pexprArrayCmp =
//			GPOS_NEW(pmp) CExpression
//						(
//						pmp,
//						GPOS_NEW(pmp) CScalarArrayCmp(pmp,
//								pmdidCmpOp,
//								GPOS_NEW(pmp) CWStringConst(pmp, strOp.Wsz()),
//								isIn ? CScalarArrayCmp::EarrcmpAny : CScalarArrayCmp::EarrcmpAll),
//						pexprIdent,
//						pexprArray
//						);
//
//	return pexprArrayCmp;
//}

//---------------------------------------------------------------------------
//	@function:
//		CExpressionPreprocessorConvert2InTest::EresUnittest
//
//	@doc:
//		Unittest for predicate utilities
//
//---------------------------------------------------------------------------
GPOS_RESULT
CExpressionPreprocessorConvert2InTest::EresUnittest()
{

	CUnittest rgut[] =
		{
		GPOS_UNITTEST_FUNC(EresUnittest_PreProcessConvert2InPredicate),
		GPOS_UNITTEST_FUNC(EresUnittest_PreProcessConvertArrayWithEquals),
		};

	return CUnittest::EresExecute(rgut, GPOS_ARRAY_SIZE(rgut));
}

//---------------------------------------------------------------------------
//	@function:
//		CExpressionPreprocessorConvert2InTest::EresUnittest_PreProcessConvert2InPredicate
//
//	@doc:
//		Test of preprocessing
//
//---------------------------------------------------------------------------
GPOS_RESULT
CExpressionPreprocessorConvert2InTest::EresUnittest_PreProcessConvert2InPredicate()
{
	CAutoMemoryPool amp;
	IMemoryPool *pmp = amp.Pmp();

	// reset metadata cache
	CMDCache::Reset();

	// setup a file-based provider
	CMDProviderMemory *pmdp = CTestUtils::m_pmdpf;
	pmdp->AddRef();
	CMDAccessor mda(pmp, CMDCache::Pcache(), CTestUtils::m_sysidDefault, pmdp);

	CAutoOptCtxt aoc(pmp, &mda, NULL /*pceeval*/, CTestUtils::Pcm(pmp));

	CExpression *pexprGet = CTestUtils::PexprLogicalGet(pmp);
	CColRef *pcrLeft = CDrvdPropRelational::Pdprel(pexprGet->PdpDerive())->PcrsOutput()->PcrAny();

	CScalarBoolOp *pcsclrOp = GPOS_NEW(pmp) CScalarBoolOp(pmp, CScalarBoolOp::EboolopOr);
	CExpression *pexprDisjunct =
			GPOS_NEW(pmp) CExpression(
									pmp,
									pcsclrOp,
									CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 1 /*iVal*/)),
									CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 2 /*iVal*/))
									);

	CExpression *pexprConvert = CExpressionPreprocessor::PexprConvert2In(pmp, pexprDisjunct);

	GPOS_ASSERT(fContainsArrayExpr(pexprConvert));

	pexprGet->Release();
	pexprDisjunct->Release();
	pexprConvert->Release();

	return GPOS_OK;
//
//	{
//		CAutoTrace at(pmp);
//		at.Os() << "Disj:\n";
//		pexprDisjunct->OsPrint(at.Os());
//		at.Os() << "convert:\n";
//		pexprConvert->OsPrint(at.Os());
//	}
//
//	pexprConvert->Release();
//	pexprDisjunct->Release();
//	pexprGet->Release();
//
//	return GPOS_OK;
}

//---------------------------------------------------------------------------
//	@function:
//		CExpressionPreprocessorConvert2InTest::EresUnittest_PreProcessConvertArrayWithEquals
//
//	@doc:
//		Test that an array expression like A IN (1,2,3,4,5) OR A = 6 OR A = 7
//		converts to A IN (1,2,3,4,5,6,7).
//
//---------------------------------------------------------------------------
GPOS_RESULT
CExpressionPreprocessorConvert2InTest::EresUnittest_PreProcessConvertArrayWithEquals()
{
	CAutoMemoryPool amp;
	IMemoryPool *pmp = amp.Pmp();

	// reset metadata cache
	CMDCache::Reset();

	// setup a file-based provider
	CMDProviderMemory *pmdp = CTestUtils::m_pmdpf;
	pmdp->AddRef();
	CMDAccessor mda(pmp, CMDCache::Pcache(), CTestUtils::m_sysidDefault, pmdp);

	CAutoOptCtxt aoc(pmp, &mda, NULL /*pceeval*/, CTestUtils::Pcm(pmp));

	CExpression *pexpr = CTestUtils::PexprLogicalSelectArrayCmp(pmp);
	// Get a ref to the comparison column
	CColRef *pcrLeft = CDrvdPropRelational::Pdprel(pexpr->PdpDerive())->PcrsOutput()->PcrAny();

	// Remove the array child and then make an OR node with two equality comparisons
	CExpression *pexprArrayComp = (*pexpr->PdrgPexpr())[1];
	GPOS_ASSERT(CUtils::FScalarArrayCmp(pexprArrayComp));

	DrgPexpr *prngexprDisjChildren = GPOS_NEW(pmp) DrgPexpr(pmp);
	prngexprDisjChildren->Append(pexprArrayComp);
	prngexprDisjChildren->Append(CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 6 /*iVal*/)));
	prngexprDisjChildren->Append(CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 7 /*iVal*/)));

	CScalarBoolOp *pcsclrOp = GPOS_NEW(pmp) CScalarBoolOp(pmp, CScalarBoolOp::EboolopOr);
	CExpression *pexprDisjunct = GPOS_NEW(pmp) CExpression(pmp, pcsclrOp, prngexprDisjChildren);
	pexprArrayComp->AddRef(); // needed for Replace
	pexpr->PdrgPexpr()->Replace(1, pexprDisjunct);

	CExpression *pexprConvert = CExpressionPreprocessor::PexprConvert2In(pmp, pexprDisjunct);

	GPOS_ASSERT(fContainsArrayExpr(pexprConvert));
	pexprDisjunct->DbgPrint();
	pexprConvert->DbgPrint();

	pexpr->Release();
	pexprConvert->Release();

	return GPOS_OK;
}
