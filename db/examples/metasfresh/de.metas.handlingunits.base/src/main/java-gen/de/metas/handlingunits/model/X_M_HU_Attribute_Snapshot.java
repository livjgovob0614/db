/** Generated Model - DO NOT CHANGE */
package de.metas.handlingunits.model;

import java.math.BigDecimal;
import java.sql.ResultSet;
import java.util.Properties;

/** Generated Model for M_HU_Attribute_Snapshot
 *  @author Adempiere (generated) 
 */
@SuppressWarnings("javadoc")
public class X_M_HU_Attribute_Snapshot extends org.compiere.model.PO implements I_M_HU_Attribute_Snapshot, org.compiere.model.I_Persistent 
{

	/**
	 *
	 */
	private static final long serialVersionUID = 2096551265L;

    /** Standard Constructor */
    public X_M_HU_Attribute_Snapshot (Properties ctx, int M_HU_Attribute_Snapshot_ID, String trxName)
    {
      super (ctx, M_HU_Attribute_Snapshot_ID, trxName);
      /** if (M_HU_Attribute_Snapshot_ID == 0)
        {
			setM_Attribute_ID (0);
			setM_HU_Attribute_Snapshot_ID (0);
			setM_HU_ID (0);
			setSnapshot_UUID (null);
        } */
    }

    /** Load Constructor */
    public X_M_HU_Attribute_Snapshot (Properties ctx, ResultSet rs, String trxName)
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

	/** Set Merkmal.
		@param M_Attribute_ID 
		Produkt-Merkmal
	  */
	@Override
	public void setM_Attribute_ID (int M_Attribute_ID)
	{
		if (M_Attribute_ID < 1) 
			set_Value (COLUMNNAME_M_Attribute_ID, null);
		else 
			set_Value (COLUMNNAME_M_Attribute_ID, Integer.valueOf(M_Attribute_ID));
	}

	/** Get Merkmal.
		@return Produkt-Merkmal
	  */
	@Override
	public int getM_Attribute_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_M_Attribute_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public de.metas.handlingunits.model.I_M_HU_Attribute getM_HU_Attribute()
	{
		return get_ValueAsPO(COLUMNNAME_M_HU_Attribute_ID, de.metas.handlingunits.model.I_M_HU_Attribute.class);
	}

	@Override
	public void setM_HU_Attribute(de.metas.handlingunits.model.I_M_HU_Attribute M_HU_Attribute)
	{
		set_ValueFromPO(COLUMNNAME_M_HU_Attribute_ID, de.metas.handlingunits.model.I_M_HU_Attribute.class, M_HU_Attribute);
	}

	/** Set Handling Unit Attribute.
		@param M_HU_Attribute_ID Handling Unit Attribute	  */
	@Override
	public void setM_HU_Attribute_ID (int M_HU_Attribute_ID)
	{
		if (M_HU_Attribute_ID < 1) 
			set_Value (COLUMNNAME_M_HU_Attribute_ID, null);
		else 
			set_Value (COLUMNNAME_M_HU_Attribute_ID, Integer.valueOf(M_HU_Attribute_ID));
	}

	/** Get Handling Unit Attribute.
		@return Handling Unit Attribute	  */
	@Override
	public int getM_HU_Attribute_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_M_HU_Attribute_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Handling Units Attribute.
		@param M_HU_Attribute_Snapshot_ID Handling Units Attribute	  */
	@Override
	public void setM_HU_Attribute_Snapshot_ID (int M_HU_Attribute_Snapshot_ID)
	{
		if (M_HU_Attribute_Snapshot_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_M_HU_Attribute_Snapshot_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_M_HU_Attribute_Snapshot_ID, Integer.valueOf(M_HU_Attribute_Snapshot_ID));
	}

	/** Get Handling Units Attribute.
		@return Handling Units Attribute	  */
	@Override
	public int getM_HU_Attribute_Snapshot_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_M_HU_Attribute_Snapshot_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public de.metas.handlingunits.model.I_M_HU getM_HU()
	{
		return get_ValueAsPO(COLUMNNAME_M_HU_ID, de.metas.handlingunits.model.I_M_HU.class);
	}

	@Override
	public void setM_HU(de.metas.handlingunits.model.I_M_HU M_HU)
	{
		set_ValueFromPO(COLUMNNAME_M_HU_ID, de.metas.handlingunits.model.I_M_HU.class, M_HU);
	}

	/** Set Handling Unit.
		@param M_HU_ID Handling Unit	  */
	@Override
	public void setM_HU_ID (int M_HU_ID)
	{
		if (M_HU_ID < 1) 
			set_Value (COLUMNNAME_M_HU_ID, null);
		else 
			set_Value (COLUMNNAME_M_HU_ID, Integer.valueOf(M_HU_ID));
	}

	/** Get Handling Unit.
		@return Handling Unit	  */
	@Override
	public int getM_HU_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_M_HU_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Handling Units Packing Instructions Attribute.
		@param M_HU_PI_Attribute_ID Handling Units Packing Instructions Attribute	  */
	@Override
	public void setM_HU_PI_Attribute_ID (int M_HU_PI_Attribute_ID)
	{
		if (M_HU_PI_Attribute_ID < 1) 
			set_Value (COLUMNNAME_M_HU_PI_Attribute_ID, null);
		else 
			set_Value (COLUMNNAME_M_HU_PI_Attribute_ID, Integer.valueOf(M_HU_PI_Attribute_ID));
	}

	/** Get Handling Units Packing Instructions Attribute.
		@return Handling Units Packing Instructions Attribute	  */
	@Override
	public int getM_HU_PI_Attribute_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_M_HU_PI_Attribute_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Snapshot UUID.
		@param Snapshot_UUID Snapshot UUID	  */
	@Override
	public void setSnapshot_UUID (java.lang.String Snapshot_UUID)
	{
		set_Value (COLUMNNAME_Snapshot_UUID, Snapshot_UUID);
	}

	/** Get Snapshot UUID.
		@return Snapshot UUID	  */
	@Override
	public java.lang.String getSnapshot_UUID () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Snapshot_UUID);
	}

	/** Set Suchschlüssel.
		@param Value 
		Suchschlüssel für den Eintrag im erforderlichen Format - muss eindeutig sein
	  */
	@Override
	public void setValue (java.lang.String Value)
	{
		set_Value (COLUMNNAME_Value, Value);
	}

	/** Get Suchschlüssel.
		@return Suchschlüssel für den Eintrag im erforderlichen Format - muss eindeutig sein
	  */
	@Override
	public java.lang.String getValue () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Value);
	}

	/** Set Merkmals-Wert (initial).
		@param ValueInitial 
		Initial Value of the Attribute
	  */
	@Override
	public void setValueInitial (java.lang.String ValueInitial)
	{
		set_ValueNoCheck (COLUMNNAME_ValueInitial, ValueInitial);
	}

	/** Get Merkmals-Wert (initial).
		@return Initial Value of the Attribute
	  */
	@Override
	public java.lang.String getValueInitial () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ValueInitial);
	}

	/** Set Zahlwert.
		@param ValueNumber 
		Numeric Value
	  */
	@Override
	public void setValueNumber (java.math.BigDecimal ValueNumber)
	{
		set_Value (COLUMNNAME_ValueNumber, ValueNumber);
	}

	/** Get Zahlwert.
		@return Numeric Value
	  */
	@Override
	public java.math.BigDecimal getValueNumber () 
	{
		BigDecimal bd = (BigDecimal)get_Value(COLUMNNAME_ValueNumber);
		if (bd == null)
			 return BigDecimal.ZERO;
		return bd;
	}

	/** Set Zahlwert (initial).
		@param ValueNumberInitial 
		Initial Numeric Value
	  */
	@Override
	public void setValueNumberInitial (java.math.BigDecimal ValueNumberInitial)
	{
		set_ValueNoCheck (COLUMNNAME_ValueNumberInitial, ValueNumberInitial);
	}

	/** Get Zahlwert (initial).
		@return Initial Numeric Value
	  */
	@Override
	public java.math.BigDecimal getValueNumberInitial () 
	{
		BigDecimal bd = (BigDecimal)get_Value(COLUMNNAME_ValueNumberInitial);
		if (bd == null)
			 return BigDecimal.ZERO;
		return bd;
	}
}