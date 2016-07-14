//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2016 EMC Corp.
//
//	@filename:
//		CExpressionPreprocessorConvert2InTest.h
//
//	@doc:
//		Test expression normalization
//---------------------------------------------------------------------------
#ifndef GPOPT_CExpressionPreprocessorConvert2InTest_H
#define GPOPT_CExpressionPreprocessorConvert2InTest_H

#include "gpos/base.h"

namespace gpopt
{
	//---------------------------------------------------------------------------
	//	@class:
	//		CExpressionPreprocessorConvert2InTest
	//
	//	@doc:
	//		Unittests
	//
	//---------------------------------------------------------------------------
	class CExpressionPreprocessorConvert2InTest
	{
		private:

			static CExpression* PExprCreateScalarArray(IMemoryPool *pmp);

			// counts the occurrences of a given operator id in an expression tree
			static INT fCountOperator(CExpression *pexpr, INT Eopid);

		public:

			// unittests
			static GPOS_RESULT EresUnittest_PreProcessConvert2InPredicate();
			static GPOS_RESULT EresUnittest_PreProcessConvert2InPredicateDeepExpressionTree();
			static GPOS_RESULT EresUnittest_PreProcessConvertArrayWithEquals();
			static GPOS_RESULT EresUnittest();

	}; // class CExpressionPreprocessorConvert2InTest
}

#endif // !GPOPT_CExpressionPreprocessorConvert2InTest_H

// EOF
