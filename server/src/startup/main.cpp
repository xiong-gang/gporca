//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2008 Greenplum, Inc.
//
//	@filename:
//		main.cpp
//
//	@doc:
//		Startup routines for optimizer
//---------------------------------------------------------------------------

#include "gpos/_api.h"
#include "gpos/types.h"
#include "gpopt/init.h"

#include "naucrates/init.h"

#include "gpos/common/CMainArgs.h"
#include "gpos/memory/CAutoMemoryPool.h"
#include "gpos/test/CFSimulatorTestExt.h"
#include "gpos/test/CUnittest.h"
#include "gpos/test/CTimeSliceTest.h"


#include "gpopt/engine/CEnumeratorConfig.h"
#include "gpopt/engine/CStatisticsConfig.h"
#include "gpopt/mdcache/CMDCache.h"
#include "gpopt/minidump/CMinidumperUtils.h"
#include "gpopt/xforms/CXformFactory.h"
#include "gpopt/optimizer/COptimizerConfig.h"

#include "gpdbcost/CCostModelGPDBLegacy.h"

// test headers

#include "unittest/base.h"
#include "unittest/gpopt/search/CTreeMapTest.h"

#include "unittest/dxl/CDXLMemoryManagerTest.h"
#include "unittest/dxl/CDXLUtilsTest.h"
#include "unittest/dxl/CMDProviderCommTest.h"
#include "unittest/dxl/CParseHandlerManagerTest.h"
#include "unittest/dxl/CParseHandlerTest.h"

#include "unittest/dxl/CXMLSerializerTest.h"

#include "unittest/dxl/base/CDatumTest.h"


#include "unittest/dxl/comm/CCommunicatorTest.h"

#include "unittest/gpopt/CTestUtils.h"
#include "unittest/gpopt/base/CColRefSetIterTest.h"
#include "unittest/gpopt/base/CColRefSetTest.h"
#include "unittest/gpopt/base/CColumnFactoryTest.h"
#include "unittest/gpopt/base/CDistributionSpecTest.h"
#include "unittest/gpopt/base/CFunctionalDependencyTest.h"
#include "unittest/gpopt/base/CKeyCollectionTest.h"
#include "unittest/gpopt/base/CMaxCardTest.h"
#include "unittest/gpopt/base/CStateMachineTest.h"
#include "unittest/gpopt/base/COrderSpecTest.h"
#include "unittest/gpopt/base/CRangeTest.h"
#include "unittest/gpopt/base/CConstraintTest.h"
#include "unittest/gpopt/base/CStatsEquivClassTest.h"
#include "unittest/gpopt/engine/CEngineTest.h"
#include "unittest/gpopt/engine/CEnumeratorTest.h"

#include "unittest/gpopt/metadata/CColumnDescriptorTest.h"
#include "unittest/gpopt/metadata/CNameTest.h"
#include "unittest/gpopt/metadata/CTableDescriptorTest.h"
#include "unittest/gpopt/metadata/CIndexDescriptorTest.h"
#include "unittest/gpopt/metadata/CPartConstraintTest.h"

#include "unittest/gpopt/mdcache/CMDAccessorTest.h"
#include "unittest/gpopt/mdcache/CMDProviderTest.h"

#include "unittest/gpopt/minidump/CArrayExpansionTest.h"
#include "unittest/gpopt/minidump/CPullUpProjectElementTest.h"
#include "unittest/gpopt/minidump/CMiniDumperDXLTest.h"
#include "unittest/gpopt/minidump/CMinidumpWithConstExprEvaluatorTest.h"
#include "unittest/gpopt/minidump/CWindowTest.h"
#include "unittest/gpopt/minidump/CICGTest.h"
#include "unittest/gpopt/minidump/CTpcdsTest.h"
#include "unittest/gpopt/minidump/CMultilevelPartitionTest.h"
#include "unittest/gpopt/minidump/CSetopTest.h"
#include "unittest/gpopt/minidump/CTVFTest.h"
#include "unittest/gpopt/minidump/CDMLTest.h"
#include "unittest/gpopt/minidump/CAggTest.h"
#include "unittest/gpopt/minidump/CPhysicalParallelUnionAllTest.h"
#include "unittest/gpopt/minidump/CPruneColumnsTest.h"
#include "unittest/gpopt/minidump/CMissingStatsTest.h"
#include "unittest/gpopt/minidump/CIndexTest.h"
#include "unittest/gpopt/minidump/CPartTblTest.h"
#include "unittest/gpopt/minidump/CBitmapTest.h"
#include "unittest/gpopt/minidump/CCTETest.h"
#include "unittest/gpopt/minidump/CEscapeMechanismTest.h"
#include "unittest/gpopt/minidump/CDirectDispatchTest.h"

#include "unittest/gpopt/operators/CCNFConverterTest.h"
#include "unittest/gpopt/operators/CContradictionTest.h"
#include "unittest/gpopt/operators/CExpressionPreprocessorTest.h"
#include "unittest/gpopt/operators/CExpressionTest.h"
#include "unittest/gpopt/operators/CPredicateUtilsTest.h"

#include "unittest/gpopt/search/CSchedulerTest.h"
#include "unittest/gpopt/search/CSearchStrategyTest.h"
#include "unittest/gpopt/minidump/CMultilevelPartitionTest.h"
#include "unittest/gpopt/search/COptimizationJobsTest.h"

#include "unittest/gpopt/translate/CTranslatorDXLToExprTest.h"
#include "unittest/gpopt/translate/CTranslatorExprToDXLTest.h"
#include "unittest/gpopt/translate/CTpchTest.h"

#include "unittest/gpopt/csq/CCorrelatedExecutionTest.h"
#include "unittest/gpopt/eval/CConstExprEvaluatorDefaultTest.h"
#include "unittest/gpopt/eval/CConstExprEvaluatorDXLTest.h"
#include "unittest/gpopt/xforms/CDecorrelatorTest.h"
#include "unittest/gpopt/xforms/CJoinOrderTest.h"
#include "unittest/gpopt/xforms/CSubqueryHandlerTest.h"
#include "unittest/gpopt/xforms/CXformTest.h"
#include "unittest/gpopt/xforms/CXformFactoryTest.h"

#include "unittest/dxl/statistics/CStatisticsTest.h"
#include "unittest/gpopt/cost/CCostTest.h"

using namespace gpos;
using namespace gpopt;
using namespace gpdxl;
using namespace gpnaucrates;
using namespace gpdbcost;

// static array of all known unittest routines
static gpos::CUnittest rgut[] =
{
	// naucrates
	GPOS_UNITTEST_STD(CCostTest),
	GPOS_UNITTEST_STD(CDatumTest),
	GPOS_UNITTEST_STD(CDXLMemoryManagerTest),
	GPOS_UNITTEST_STD(CDXLUtilsTest),
	GPOS_UNITTEST_STD(CMDAccessorTest),
	GPOS_UNITTEST_STD(CMDProviderTest),

#if !defined(GPOS_SunOS)
	GPOS_UNITTEST_STD(CMDProviderCommTest),
#endif  // !defined(GPOS_SunOS)
	GPOS_UNITTEST_STD(CMiniDumperDXLTest),
	GPOS_UNITTEST_STD(CExpressionPreprocessorTest),
	GPOS_UNITTEST_STD(CWindowTest),
	GPOS_UNITTEST_STD(CICGTest),
	GPOS_UNITTEST_STD(CMultilevelPartitionTest),
	GPOS_UNITTEST_STD(CSetopTest),
	GPOS_UNITTEST_STD(CDMLTest),
	GPOS_UNITTEST_STD(CDirectDispatchTest),
	GPOS_UNITTEST_STD(CTVFTest),
	GPOS_UNITTEST_STD(CAggTest),
	GPOS_UNITTEST_STD(CPruneColumnsTest),
	GPOS_UNITTEST_STD(CPhysicalParallelUnionAllTest),
	GPOS_UNITTEST_STD(CMissingStatsTest),
	GPOS_UNITTEST_STD(CIndexTest),
	GPOS_UNITTEST_STD(CPartTblTest),
	GPOS_UNITTEST_STD(CBitmapTest),
	GPOS_UNITTEST_STD(CCTETest),
	GPOS_UNITTEST_STD(CEscapeMechanismTest),

	GPOS_UNITTEST_STD(CMinidumpWithConstExprEvaluatorTest),
	GPOS_UNITTEST_STD(CParseHandlerManagerTest),
	GPOS_UNITTEST_STD(CParseHandlerTest),
	GPOS_UNITTEST_STD(CStatisticsTest),
	GPOS_UNITTEST_STD(CTranslatorDXLToExprTest),
	GPOS_UNITTEST_STD(CTranslatorExprToDXLTest),
	GPOS_UNITTEST_STD(CXMLSerializerTest),

	// opt
	GPOS_UNITTEST_STD(CArrayExpansionTest),
	GPOS_UNITTEST_STD(CPullUpProjectElementTest),
	GPOS_UNITTEST_STD(CCNFConverterTest),
	GPOS_UNITTEST_STD(CColumnDescriptorTest),
	GPOS_UNITTEST_STD(CColumnFactoryTest),
	GPOS_UNITTEST_STD(CColRefSetIterTest),
	GPOS_UNITTEST_STD(CColRefSetTest),
	GPOS_UNITTEST_STD(CConstraintTest),
	GPOS_UNITTEST_STD(CStatsEquivClassTest),
	GPOS_UNITTEST_STD(CContradictionTest),
	GPOS_UNITTEST_STD(CCorrelatedExecutionTest),
	GPOS_UNITTEST_STD(CDecorrelatorTest),
	GPOS_UNITTEST_STD(CDistributionSpecTest),
#if !defined(GPOS_32BIT)
	GPOS_UNITTEST_STD(CSubqueryHandlerTest),
#endif  // !defined(GPOS_32BIT)
	GPOS_UNITTEST_STD(CEngineTest),
	GPOS_UNITTEST_STD(CExpressionTest),
	GPOS_UNITTEST_STD(CJoinOrderTest),
	GPOS_UNITTEST_STD(CKeyCollectionTest),
	GPOS_UNITTEST_STD(CMaxCardTest),
	GPOS_UNITTEST_STD(CFunctionalDependencyTest),
	GPOS_UNITTEST_STD(CNameTest),
	GPOS_UNITTEST_STD(COrderSpecTest),
	GPOS_UNITTEST_STD(CRangeTest),
	GPOS_UNITTEST_STD(CPredicateUtilsTest),
	GPOS_UNITTEST_STD(CPartConstraintTest),
#if !defined(GPOS_SunOS)
	GPOS_UNITTEST_STD(CSchedulerTest),
	GPOS_UNITTEST_STD(CSearchStrategyTest),
#endif  // !defined(GPOS_SunOS)
	GPOS_UNITTEST_STD(COptimizationJobsTest),
	GPOS_UNITTEST_STD(CStateMachineTest),
	GPOS_UNITTEST_STD(CTableDescriptorTest),
	GPOS_UNITTEST_STD(CIndexDescriptorTest),
	GPOS_UNITTEST_STD(CTreeMapTest),
	GPOS_UNITTEST_STD(CXformFactoryTest),
	GPOS_UNITTEST_STD(CXformTest),
#if defined(GPOS_DEBUG) && !defined(GPOS_SunOS)
	GPOS_UNITTEST_STD(CCommunicatorTest),
#endif  // !defined(GPOS_DEBUG) && !defined(GPOS_SunOS)
	GPOS_UNITTEST_STD(CConstExprEvaluatorDefaultTest),
	GPOS_UNITTEST_STD(CConstExprEvaluatorDXLTest),
	// disable CEnumeratorTest until it is fixed
//#if !defined(GPOS_SunOS)
//	GPOS_UNITTEST_STD(CEnumeratorTest),
//#endif // GPOS_SunOS
	// extended tests
#ifdef GPOS_FPSIMULATOR
	GPOS_UNITTEST_EXT(CFSimulatorTestExt),
#endif // GPOS_FPSIMULATOR

#ifdef GPOS_DEBUG
	GPOS_UNITTEST_EXT(CTimeSliceTest),
#endif // GPOS_DEBUG
};

//---------------------------------------------------------------------------
//	@function:
//		ConfigureTests
//
//	@doc:
//		Configurations needed before running unittests
//
//---------------------------------------------------------------------------
void ConfigureTests()
{
	// initialize DXL support
	InitDXL();

	CMDCache::Init();

	// load metadata objects into provider file
	{
		CAutoMemoryPool amp;
		IMemoryPool *pmp = amp.Pmp();
		CTestUtils::InitProviderFile(pmp);

		// detach safety
		(void) amp.PmpDetach();
	}

#ifdef GPOS_DEBUG
	// reset xforms factory to exercise xforms ctors and dtors
	CXformFactory::Pxff()->Shutdown();
	GPOS_RESULT eres = CXformFactory::EresInit();

	GPOS_ASSERT(GPOS_OK == eres);
#endif // GPOS_DEBUG
}


//---------------------------------------------------------------------------
//	@function:
//		Cleanup
//
//	@doc:
//		Cleanup after unittests are done
//
//---------------------------------------------------------------------------
void Cleanup()
{
	CMDCache::Shutdown();
	CTestUtils::DestroyMDProvider();
}

// static variable counting the number of failed tests; PvExec overwrites with
// the actual count of failed tests
static ULONG tests_failed = 0;

//---------------------------------------------------------------------------
//	@function:
//		PvExec
//
//	@doc:
//		Function driving execution.
//
//---------------------------------------------------------------------------
static void *
PvExec
	(
	void *pv
	)
{
	CMainArgs *pma = (CMainArgs*) pv;
	CBitVector bv(ITask::PtskSelf()->Pmp(), CUnittest::UlTests());

	CHAR ch = '\0';

	CHAR *szFileName = NULL;
	BOOL fMinidump = false;
	BOOL fUnittest = false;
	
	while (pma->FGetopt(&ch))
	{
		CHAR *szTestName = NULL;
		
		switch (ch)
		{
			case 'U':
				szTestName = optarg;
				// fallthru
			case 'u':
				CUnittest::FindTest(bv, CUnittest::EttStandard, szTestName);
				fUnittest = true;
				break;

			case 'x':
				CUnittest::FindTest(bv, CUnittest::EttExtended, NULL /*szTestName*/);
				fUnittest = true;
				break;

			case 'T':
				CUnittest::SetTraceFlag(optarg);
				break;
				
			case 'd':
				fMinidump = true;
				szFileName = optarg;
				break;

			default:
				// ignore other parameters
				break;
		}
	}

	if (fMinidump && fUnittest)
	{
		GPOS_TRACE(GPOS_WSZ_LIT("Cannot specify -d and -U/-u options at the same time"));
		return NULL;
	}
	
	if (fMinidump)
	{	
		// initialize DXL support
		InitDXL();

		CMDCache::Init();
		
		CAutoMemoryPool amp;
		IMemoryPool *pmp = amp.Pmp();

		// load dump file
		CDXLMinidump *pdxlmd = CMinidumperUtils::PdxlmdLoad(pmp, szFileName);
		GPOS_CHECK_ABORT;

		COptimizerConfig *poconf = pdxlmd->Poconf();

		if (NULL == poconf)
		{
			poconf = COptimizerConfig::PoconfDefault(pmp);
		}
		else
		{
			poconf -> AddRef();
		}

		ULONG ulSegments = CTestUtils::UlSegments(poconf);

		CDXLNode *pdxlnPlan = CMinidumperUtils::PdxlnExecuteMinidump
								(
								pmp,
								szFileName,
								ulSegments,
								1 /*ulSessionId*/,
								1 /*ulCmdId*/,
								poconf,
								NULL /*pceeval*/
								);

		GPOS_DELETE(pdxlmd);
		poconf->Release();
		pdxlnPlan->Release();
		CMDCache::Shutdown();
	}
	else
	{
		GPOS_ASSERT(fUnittest);
		tests_failed = CUnittest::Driver(&bv);
	}

	return NULL;
}


//---------------------------------------------------------------------------
//	@function:
//		main
//
//	@doc:
//		Entry point for stand-alone optimizer binary; ignore arguments for the
//		time being
//
//---------------------------------------------------------------------------
INT main
	(
	INT iArgs,
	const CHAR **rgszArgs
	)
{	

	// Use default allocator
	struct gpos_init_params gpos_params = { NULL, NULL };

	gpos_init(&gpos_params);
	gpdxl_init();
	gpopt_init();

	GPOS_ASSERT(iArgs >= 0);

	if (gpos_set_threads(4, 20))
	{
		return GPOS_FAILED;
	}

	// setup args for unittest params
	CMainArgs ma(iArgs, rgszArgs, "uU:d:xT:");
	
	// initialize unittest framework
	CUnittest::Init(rgut, GPOS_ARRAY_SIZE(rgut), ConfigureTests, Cleanup);

	gpos_exec_params params;
	params.func = PvExec;
	params.arg = &ma;
	params.stack_start = &params;
	params.error_buffer = NULL;
	params.error_buffer_size = -1;
	params.abort_requested = NULL;

	if (gpos_exec(&params) || (tests_failed != 0))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


// EOF
