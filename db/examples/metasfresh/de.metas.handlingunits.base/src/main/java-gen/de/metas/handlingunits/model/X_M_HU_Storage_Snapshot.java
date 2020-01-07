/** Generated Model - DO NOT CHANGE */
package de.metas.handlingunits.model;

import java.math.BigDecimal;
import java.sql.ResultSet;
import java.util.Properties;

/** Generated Model for M_HU_Storage_Snapshot
 *  @author Adempiere (generated) 
 */
@SuppressWarnings("javadoc")
public class X_M_HU_Storage_Snapshot extends org.compiere.model.PO implements I_M_HU_Storage_Snapshot, org.compiere.model.I_Persistent 
{

	/**
	 *
	 */
	private static final long serialVersionUID = 336436825L;

    /** Standard Constructor */
    public X_M_HU_Storage_Snapshot (Properties ctx, int M_HU_Storage_Snapshot_ID, String trxName)
    {
      super (ctx, M_HU_Storage_Snapshot_ID, trxName);
      /** if (M_HU_Storage_Snapshot_ID == 0)
        {
			setC_UOM_ID (0);
			setM_HU_ID (0);
			setM_HU_Storage_Snapshot_ID (0);
			setM_Product_ID (0);
			setQty (BigDecimal.ZERO);
			setSnapshot_UUID (null);
        } */
    }

    /** Load Constructor */
    public X_M_HU_Storage_Snapshot (Properties ctx, ResultSet rs, String trxName)
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

	/** Set Maßeinheit.
		@param C_UOM_ID 
		Maßeinheit
	  */
	@Override
	public void setC_UOM_ID (int C_UOM_ID)
	{
		if (C_UOM_ID < 1) 
			set_Value (COLUMNNAME_C_UOM_ID, null);
		else 
			set_Value (COLUMNNAME_C_UOM_ID, Integer.valueOf(C_UOM_ID));
	}

	/** Get Maßeinheit.
		@return Maßeinheit
	  */
	@Override
	public int getC_UOM_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_C_UOM_ID);
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
			set_ValueNoCheck (COLUMNNAME_M_HU_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_M_HU_ID, Integer.valueOf(M_HU_ID));
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

	@Override
	public de.metas.handlingunits.model.I_M_HU_Storage getM_HU_Storage()
	{
		return get_ValueAsPO(COLUMNNAME_M_HU_Storage_ID, de.metas.handlingunits.model.I_M_HU_Storage.class);
	}

	@Override
	public void setM_HU_Storage(de.metas.handlingunits.model.I_M_HU_Storage M_HU_Storage)
	{
		set_ValueFromPO(COLUMNNAME_M_HU_Storage_ID, de.metas.handlingunits.model.I_M_HU_Storage.class, M_HU_Storage);
	}

	/** Set Handling Units Storage.
		@param M_HU_Storage_ID Handling Units Storage	  */
	@Override
	public void setM_HU_Storage_ID (int M_HU_Storage_ID)
	{
		if (M_HU_Storage_ID < 1) 
			set_Value (COLUMNNAME_M_HU_Storage_ID, null);
		else 
			set_Value (COLUMNNAME_M_HU_Storage_ID, Integer.valueOf(M_HU_Storage_ID));
	}

	/** Get Handling Units Storage.
		@return Handling Units Storage	  */
	@Override
	public int getM_HU_Storage_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_M_HU_Storage_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Handling Units Storage Snapshot.
		@param M_HU_Storage_Snapshot_ID Handling Units Storage Snapshot	  */
	@Override
	public void setM_HU_Storage_Snapshot_ID (int M_HU_Storage_Snapshot_ID)
	{
		if (M_HU_Storage_Snapshot_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_M_HU_Storage_Snapshot_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_M_HU_Storage_Snapshot_ID, Integer.valueOf(M_HU_Storage_Snapshot_ID));
	}

	/** Get Handling Units Storage Snapshot.
		@return Handling Units Storage Snapshot	  */
	@Override
	public int getM_HU_Storage_Snapshot_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_M_HU_Storage_Snapshot_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Produkt.
		@param M_Product_ID 
		Produkt, Leistung, Artikel
	  */
	@Override
	public void setM_Product_ID (int M_Product_ID)
	{
		if (M_Product_ID < 1) 
			set_Value (COLUMNNAME_M_Product_ID, null);
		else 
			set_Value (COLUMNNAME_M_Product_ID, Integer.valueOf(M_Product_ID));
	}

	/** Get Produkt.
		@return Produkt, Leistung, Artikel
	  */
	@Override
	public int getM_Product_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_M_Product_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Menge.
		@param Qty 
		Menge
	  */
	@Override
	public void setQty (java.math.BigDecimal Qty)
	{
		set_Value (COLUMNNAME_Qty, Qty);
	}

	/** Get Menge.
		@return Menge
	  */
	@Override
	public java.math.BigDecimal getQty () 
	{
		BigDecimal bd = (BigDecimal)get_Value(COLUMNNAME_Qty);
		if (bd == null)
			 return BigDecimal.ZERO;
		return bd;
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
}