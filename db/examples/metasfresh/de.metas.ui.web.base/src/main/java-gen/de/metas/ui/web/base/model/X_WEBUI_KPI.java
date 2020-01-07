/** Generated Model - DO NOT CHANGE */
package de.metas.ui.web.base.model;

import java.sql.ResultSet;
import java.util.Properties;

/** Generated Model for WEBUI_KPI
 *  @author Adempiere (generated) 
 */
@SuppressWarnings("javadoc")
public class X_WEBUI_KPI extends org.compiere.model.PO implements I_WEBUI_KPI, org.compiere.model.I_Persistent 
{

	/**
	 *
	 */
	private static final long serialVersionUID = 355217345L;

    /** Standard Constructor */
    public X_WEBUI_KPI (Properties ctx, int WEBUI_KPI_ID, String trxName)
    {
      super (ctx, WEBUI_KPI_ID, trxName);
      /** if (WEBUI_KPI_ID == 0)
        {
			setChartType (null);
			setES_Index (null);
			setES_Type (null);
			setIsGenerateComparation (false); // N
			setName (null);
			setPollIntervalSec (0); // 10
			setWEBUI_KPI_ID (0);
        } */
    }

    /** Load Constructor */
    public X_WEBUI_KPI (Properties ctx, ResultSet rs, String trxName)
    {
      super (ctx, rs, trxName);
    }


    /** Load Meta Data */
    @Override
    protected org.compiere.model.POInfo initPO (Properties ctx)
    {
      org.compiere.model.POInfo poi = org.compiere.model.POInfo.getPOInfo (ctx, Table_Name, get_TrxName());
      return poi;
    }

	/** 
	 * ChartType AD_Reference_ID=540701
	 * Reference name: WEBUI_ChartType
	 */
	public static final int CHARTTYPE_AD_Reference_ID=540701;
	/** BarChart = BC */
	public static final String CHARTTYPE_BarChart = "BC";
	/** PieChart = PC */
	public static final String CHARTTYPE_PieChart = "PC";
	/** AreaChart = AC */
	public static final String CHARTTYPE_AreaChart = "AC";
	/** Metric = M */
	public static final String CHARTTYPE_Metric = "M";
	/** Set Chart Type.
		@param ChartType 
		Type fo chart to render
	  */
	@Override
	public void setChartType (java.lang.String ChartType)
	{

		set_Value (COLUMNNAME_ChartType, ChartType);
	}

	/** Get Chart Type.
		@return Type fo chart to render
	  */
	@Override
	public java.lang.String getChartType () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ChartType);
	}

	/** Set Offset.
		@param CompareOffset Offset	  */
	@Override
	public void setCompareOffset (java.lang.String CompareOffset)
	{
		set_Value (COLUMNNAME_CompareOffset, CompareOffset);
	}

	/** Get Offset.
		@return Offset	  */
	@Override
	public java.lang.String getCompareOffset () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_CompareOffset);
	}

	/** Set Beschreibung.
		@param Description Beschreibung	  */
	@Override
	public void setDescription (java.lang.String Description)
	{
		set_Value (COLUMNNAME_Description, Description);
	}

	/** Get Beschreibung.
		@return Beschreibung	  */
	@Override
	public java.lang.String getDescription () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Description);
	}

	/** Set Elasticsearch Index.
		@param ES_Index Elasticsearch Index	  */
	@Override
	public void setES_Index (java.lang.String ES_Index)
	{
		set_Value (COLUMNNAME_ES_Index, ES_Index);
	}

	/** Get Elasticsearch Index.
		@return Elasticsearch Index	  */
	@Override
	public java.lang.String getES_Index () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ES_Index);
	}

	/** Set Elasticsearch query.
		@param ES_Query Elasticsearch query	  */
	@Override
	public void setES_Query (java.lang.String ES_Query)
	{
		set_Value (COLUMNNAME_ES_Query, ES_Query);
	}

	/** Get Elasticsearch query.
		@return Elasticsearch query	  */
	@Override
	public java.lang.String getES_Query () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ES_Query);
	}

	/** Set Time range.
		@param ES_TimeRange 
		Time range using format 'PnDTnHnMn.nS'
	  */
	@Override
	public void setES_TimeRange (java.lang.String ES_TimeRange)
	{
		set_Value (COLUMNNAME_ES_TimeRange, ES_TimeRange);
	}

	/** Get Time range.
		@return Time range using format 'PnDTnHnMn.nS'
	  */
	@Override
	public java.lang.String getES_TimeRange () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ES_TimeRange);
	}

	/** Set Time range end.
		@param ES_TimeRange_End 
		Time range's ending offset (relative to now)
	  */
	@Override
	public void setES_TimeRange_End (java.lang.String ES_TimeRange_End)
	{
		set_Value (COLUMNNAME_ES_TimeRange_End, ES_TimeRange_End);
	}

	/** Get Time range end.
		@return Time range's ending offset (relative to now)
	  */
	@Override
	public java.lang.String getES_TimeRange_End () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ES_TimeRange_End);
	}

	/** Set Elasticsearch Type.
		@param ES_Type Elasticsearch Type	  */
	@Override
	public void setES_Type (java.lang.String ES_Type)
	{
		set_Value (COLUMNNAME_ES_Type, ES_Type);
	}

	/** Get Elasticsearch Type.
		@return Elasticsearch Type	  */
	@Override
	public java.lang.String getES_Type () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ES_Type);
	}

	/** Set Compare.
		@param IsGenerateComparation Compare	  */
	@Override
	public void setIsGenerateComparation (boolean IsGenerateComparation)
	{
		set_Value (COLUMNNAME_IsGenerateComparation, Boolean.valueOf(IsGenerateComparation));
	}

	/** Get Compare.
		@return Compare	  */
	@Override
	public boolean isGenerateComparation () 
	{
		Object oo = get_Value(COLUMNNAME_IsGenerateComparation);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Name.
		@param Name 
		Alphanumeric identifier of the entity
	  */
	@Override
	public void setName (java.lang.String Name)
	{
		set_Value (COLUMNNAME_Name, Name);
	}

	/** Get Name.
		@return Alphanumeric identifier of the entity
	  */
	@Override
	public java.lang.String getName () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Name);
	}

	/** Set Polling interval (sec).
		@param PollIntervalSec Polling interval (sec)	  */
	@Override
	public void setPollIntervalSec (int PollIntervalSec)
	{
		set_Value (COLUMNNAME_PollIntervalSec, Integer.valueOf(PollIntervalSec));
	}

	/** Get Polling interval (sec).
		@return Polling interval (sec)	  */
	@Override
	public int getPollIntervalSec () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_PollIntervalSec);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set KPI.
		@param WEBUI_KPI_ID KPI	  */
	@Override
	public void setWEBUI_KPI_ID (int WEBUI_KPI_ID)
	{
		if (WEBUI_KPI_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_WEBUI_KPI_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_WEBUI_KPI_ID, Integer.valueOf(WEBUI_KPI_ID));
	}

	/** Get KPI.
		@return KPI	  */
	@Override
	public int getWEBUI_KPI_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_WEBUI_KPI_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}
}