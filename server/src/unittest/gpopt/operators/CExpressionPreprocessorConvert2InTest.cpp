
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
//		CExpressionPreprocessorConvert2InTest::fCountOperator
//
//	@doc:
//		counts the number of times a certain operator appears
//
//---------------------------------------------------------------------------
INT
CExpressionPreprocessorConvert2InTest::fCountOperator
	(
		CExpression *pexpr,
		INT Eopid
	)
{
	INT iopCnt = 0;
	if (pexpr->Pop()->Eopid() == Eopid)
	{
		iopCnt += 1;
	}


	for (ULONG ulChild = 0; ulChild < pexpr->UlArity(); ulChild++)
	{
		iopCnt += fCountOperator((*pexpr)[ulChild], Eopid);
	}
	return iopCnt;
}

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
		GPOS_UNITTEST_FUNC(EresUnittest_PreProcessConvert2InPredicateDeepExpressionTree)
		};

	return CUnittest::EresExecute(rgut, GPOS_ARRAY_SIZE(rgut));
}

//---------------------------------------------------------------------------
//	@function:
//		CExpressionPreprocessorConvert2InTest
//				::EresUnittest_PreProcessConvert2InPredicate
//
//	@doc:
//		Tests that a expression with nested OR statements will convert them into
//		an array IN statement. The statement we are testing looks is equivalent to
//		+LogicalGet
//			+-ScalarBoolOp (Or)
//				+-ScalarBoolCmp
//					+-ScalarId
//					+-ScalarConst
//				+-ScalarBoolCmp
//					+-ScalarId
//					+-ScalarConst
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
	COperator *popGet = pexprGet->Pop();
	popGet->AddRef();

	// Create a disjunct, add as a child
	CColRef *pcrLeft = CDrvdPropRelational::Pdprel(pexprGet->PdpDerive())->PcrsOutput()->PcrAny();
	CScalarBoolOp *pcsclrOp = GPOS_NEW(pmp) CScalarBoolOp(pmp, CScalarBoolOp::EboolopOr);
	CExpression *pexprDisjunct =
			GPOS_NEW(pmp) CExpression(
									pmp,
									pcsclrOp,
									CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 1 /*iVal*/)),
									CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 2 /*iVal*/)),
									CUtils::PexprScalarEqCmp(pmp, pcrLeft, pcrLeft)
									);

	CExpression *pexprGetWithChildren = GPOS_NEW(pmp) CExpression(pmp, popGet, pexprDisjunct);
	pexprGet->Release();

	GPOS_ASSERT(3 == fCountOperator(pexprGetWithChildren, COperator::EopScalarCmp));

	CExpression *pexprConvert = CExpressionPreprocessor::PexprConvert2In(pmp, pexprGetWithChildren);

	GPOS_ASSERT(1 == fCountOperator(pexprConvert, COperator::EopScalarArrayCmp));
	GPOS_ASSERT(1 == fCountOperator(pexprConvert, COperator::EopScalarCmp));
	// the OR node should be removed because there is only one child
	GPOS_ASSERT(0 == fCountOperator(pexprConvert, COperator::EopScalarBoolOp));

	pexprGetWithChildren->Release();
	pexprConvert->Release();

	return GPOS_OK;
}

//---------------------------------------------------------------------------
//	@function:
//		CExpressionPreprocessorConvert2InTest
//				::EresUnittest_PreProcessConvertArrayWithEquals
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
	// get a ref to the comparison column
	CColRef *pcrLeft = CDrvdPropRelational::Pdprel(pexpr->PdpDerive())->PcrsOutput()->PcrAny();

	// remove the array child and then make an OR node with two equality comparisons
	CExpression *pexprArrayComp = (*pexpr->PdrgPexpr())[1];
	GPOS_ASSERT(CUtils::FScalarArrayCmp(pexprArrayComp));

	DrgPexpr *prngexprDisjChildren = GPOS_NEW(pmp) DrgPexpr(pmp);
	prngexprDisjChildren->Append(pexprArrayComp);
	prngexprDisjChildren->Append(CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 6 /*iVal*/)));
	prngexprDisjChildren->Append(CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 7 /*iVal*/)));

	CScalarBoolOp *pcsclrOp = GPOS_NEW(pmp) CScalarBoolOp(pmp, CScalarBoolOp::EboolopOr);
	CExpression *pexprDisjunct = GPOS_NEW(pmp) CExpression(pmp, pcsclrOp, prngexprDisjChildren);
	pexprArrayComp->AddRef(); // needed for Replace()
	pexpr->PdrgPexpr()->Replace(1, pexprDisjunct);

	GPOS_ASSERT(2 == fCountOperator(pexpr, COperator::EopScalarCmp));

	CExpression *pexprConvert = CExpressionPreprocessor::PexprConvert2In(pmp, pexprDisjunct);

	GPOS_ASSERT(0 == fCountOperator(pexprConvert, COperator::EopScalarCmp));
	GPOS_ASSERT(7 == fCountOperator(pexprConvert, COperator::EopScalarConst));
	GPOS_ASSERT(1 == fCountOperator(pexprConvert, COperator::EopScalarArrayCmp));

	pexpr->Release();
	pexprConvert->Release();

	return GPOS_OK;
}


//---------------------------------------------------------------------------
//	@function:
//		CExpressionPreprocessorConvert2InTest
//				::EresUnittest_PreProcessConvert2InPredicateDeepExpressionTree
//
//	@doc:
//		Test of preprocessing with a whole expression tree. The expression tree
//		looks like this predicate (x = 1 OR x = 2 OR (x = y AND (y = 3 OR y = 4)))
//		which should be converted to (x in (1,2) OR (x = y AND y IN (3,4)))
//
//---------------------------------------------------------------------------
GPOS_RESULT
CExpressionPreprocessorConvert2InTest::EresUnittest_PreProcessConvert2InPredicateDeepExpressionTree()
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
	COperator *popGet = pexprGet->Pop();
	popGet->AddRef();

	// get a column ref from the outermost Get expression
	DrgPcr *pdrgpcr = CDrvdPropRelational::Pdprel(pexprGet->PdpDerive())->PcrsOutput()->Pdrgpcr(pmp);
	GPOS_ASSERT(1 < pdrgpcr->UlLength());
	CColRef *pcrLeft = (*pdrgpcr)[0];
	CColRef *pcrRight = (*pdrgpcr)[1];

	// inner most OR
	CScalarBoolOp *pcsclrOpOrInner = GPOS_NEW(pmp) CScalarBoolOp(pmp, CScalarBoolOp::EboolopOr);
	CExpression *pexprDisjunctInner =
			GPOS_NEW(pmp) CExpression(
									pmp,
									pcsclrOpOrInner,
									CUtils::PexprScalarEqCmp(pmp, pcrRight, CUtils::PexprScalarConstInt4(pmp, 3 /*iVal*/)),
									CUtils::PexprScalarEqCmp(pmp, pcrRight, CUtils::PexprScalarConstInt4(pmp, 4 /*iVal*/))
									);
	// middle and expression
	CScalarBoolOp *pcsclrOpAnd = GPOS_NEW(pmp) CScalarBoolOp(pmp, CScalarBoolOp::EboolopAnd);
	CExpression *pexprConjunct =
			GPOS_NEW(pmp) CExpression(
									pmp,
									pcsclrOpAnd,
									pexprDisjunctInner,
									CUtils::PexprScalarEqCmp(pmp, pcrLeft, pcrRight)
									);
	// outer most OR
	CScalarBoolOp *pcsclrOp = GPOS_NEW(pmp) CScalarBoolOp(pmp, CScalarBoolOp::EboolopOr);
	CExpression *pexprDisjunct =
			GPOS_NEW(pmp) CExpression(
									pmp,
									pcsclrOp,
									CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 1)),
									CUtils::PexprScalarEqCmp(pmp, pcrLeft, CUtils::PexprScalarConstInt4(pmp, 2)),
									pexprConjunct
									);

	CExpression *pexprGetWithChildren = GPOS_NEW(pmp) CExpression(pmp, popGet, pexprDisjunct);
	pexprGet->Release();

	GPOS_ASSERT(5 == fCountOperator(pexprGetWithChildren, COperator::EopScalarCmp));

	CExpression *pexprConvert = CExpressionPreprocessor::PexprConvert2In(pmp, pexprGetWithChildren);

	GPOS_ASSERT(2 == fCountOperator(pexprConvert, COperator::EopScalarArrayCmp));
	GPOS_ASSERT(1 == fCountOperator(pexprConvert, COperator::EopScalarCmp));

	pexprGetWithChildren->Release();
	pexprConvert->Release();
	pdrgpcr->Release();

	return GPOS_OK;
}
