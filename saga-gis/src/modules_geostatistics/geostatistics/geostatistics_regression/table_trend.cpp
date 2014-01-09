/**********************************************************
 * Version $Id: Table_Trend.cpp 1098 2011-06-16 16:06:32Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//               geostatistics_regression                //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                   table_trend.cpp                     //
//                                                       //
//                 Copyright (C) 2006 by                 //
//                      Olaf Conrad                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation; version 2 of the License.   //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not,       //
// write to the Free Software Foundation, Inc.,          //
// 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, //
// USA.                                                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "table_trend.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CTable_Trend_Base::Initialise(void)
{
	//-----------------------------------------------------
	Set_Author		(SG_T("O.Conrad (c) 2006"));

	Set_Description	(_TW(
		""
	));

	//-----------------------------------------------------
	CSG_Parameter	*pNode	= Parameters("TABLE");

	Parameters.Add_Table_Field(
		pNode	, "FIELD_X"		, _TL("X Values"),
		_TL("")
	);

	Parameters.Add_Table_Field(
		pNode	, "FIELD_Y"		, _TL("Y Values"),
		_TL("")
	);

	Parameters.Add_String(
		NULL	, "FORMULA"		, _TL("Formula"),
		_TL(""),
		SG_T("m * x + b")
	);

	Parameters.Add_Choice(
		NULL	, "FORMULAS"	, _TL("Pre-defined Formulas"),
		_TL(""),

		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s|%s|%s|"),
			_TL("Linear: a + b * x"),
			_TL("Quadric: a + b * x + c * x^2"),
			_TL("Cubic: a + b * x + c * x^2 + d * x^3"),
			_TL("Logarithmic: a + b * ln(x)"),
			_TL("Power: a + b * x^c"),
			_TL("a + b / x"),
			_TL("a + b * (1 - exp(-x / c))"),
			_TL("a + b * (1 - exp(-(x / c)^2))")
		), 0
	);

	Parameters.Add_Table(
		NULL	, "TREND"		, _TL("Table (with Trend)"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL
	);
}

//---------------------------------------------------------
int CTable_Trend_Base::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), SG_T("FORMULAS")) )
	{
		const SG_Char	*Formula;

		switch( pParameter->asInt() )
		{
		default:	return( false );
		case 0:	Formula	= SG_T("a + b * x");						break;
		case 1:	Formula	= SG_T("a + b * x + c * x^2");				break;
		case 2:	Formula	= SG_T("a + b * x + c * x^2 + d * x^3");	break;
		case 3:	Formula	= SG_T("a + b * ln(x)");					break;
		case 4:	Formula	= SG_T("a + b * x^c");						break;
		case 5:	Formula	= SG_T("a + b / x");						break;
		case 6:	Formula	= SG_T("a + b * (1 - exp(-x / c))");		break;
		case 7:	Formula	= SG_T("a + b * (1 - exp(-(x / c)^2))");	break;
		}

		pParameters->Get_Parameter("FORMULA")->Set_Value(Formula);
	}

	if( !SG_STR_CMP(pParameter->Get_Identifier(), SG_T("FORMULA")) )
	{
		if( !m_Trend.Set_Formula(pParameter->asString()) )
		{
			Message_Dlg(m_Trend.Get_Error(), _TL("Error in Formula"));

			return( false );
		}
	}

	return( true );
}

//---------------------------------------------------------
bool CTable_Trend_Base::On_Execute(void)
{
	int					i, j, xField, yField;
	CSG_String			Name;
	CSG_Table_Record	*pRecord;
	CSG_Table			*pTable;

	pTable	= Parameters("TABLE")	->asTable();
	xField	= Parameters("FIELD_X")	->asInt();
	yField	= Parameters("FIELD_Y")	->asInt();

	//-----------------------------------------------------
	if( m_Trend.Set_Formula(Parameters("FORMULA")->asString()) )
	{
		m_Trend.Clr_Data();

		for(i=0; i<pTable->Get_Record_Count(); i++)
		{
			pRecord	= pTable->Get_Record(i);

			m_Trend.Add_Data(pRecord->asDouble(xField), pRecord->asDouble(yField));
		}

		//-------------------------------------------------
		if( m_Trend.Get_Trend() )
		{
			Message_Add(SG_T("\n"), false);
			Message_Add(m_Trend.Get_Formula(), false);
			Message_Add(SG_T("\n"), false);
			Message_Add(CSG_String::Format(SG_T("R2 : %f"), 100.0 * m_Trend.Get_R2()), false);

			if( Parameters("TREND")->asTable() == NULL )
			{
				pTable->Add_Field("TREND"	, SG_DATATYPE_Double);

				for(i=0, j=pTable->Get_Field_Count()-1; i<m_Trend.Get_Data_Count(); i++)
				{
					pRecord	= pTable->Get_Record(i);
					pRecord->Set_Value(j, m_Trend.Get_Value(m_Trend.Get_Data_X(i)));
				}
			}
			else
			{
				Name.Printf(SG_T("%s [%s]"), pTable->Get_Name(), _TL("Trend"));
				pTable	= Parameters("TREND")->asTable();
				pTable->Destroy();
				pTable->Set_Name(Name);
				pTable->Add_Field("X"		, SG_DATATYPE_Double);
				pTable->Add_Field("Y"		, SG_DATATYPE_Double);
				pTable->Add_Field("Y_TREND"	, SG_DATATYPE_Double);

				for(i=0; i<m_Trend.Get_Data_Count(); i++)
				{
					pRecord	= pTable->Add_Record();
					pRecord->Set_Value(0, m_Trend.Get_Data_X(i));
					pRecord->Set_Value(1, m_Trend.Get_Data_Y(i));
					pRecord->Set_Value(2, m_Trend.Get_Value(m_Trend.Get_Data_X(i)));
				}
			}

			return( true );
		}
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CTable_Trend::CTable_Trend(void)
	: CTable_Trend_Base()
{
	Set_Name		(_TL("Trend Analysis"));

	Parameters.Add_Table(
		NULL	, "TABLE"	, _TL("Table"),
		_TL(""),
		PARAMETER_INPUT
	);

	Initialise();
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CTable_Trend_Shapes::CTable_Trend_Shapes(void)
	: CTable_Trend_Base()
{
	Set_Name		(_TL("Trend Analysis (Shapes)"));

	Parameters.Add_Shapes(
		NULL	, "TABLE"	, _TL("Shapes"),
		_TL(""),
		PARAMETER_INPUT
	);

	Initialise();
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
