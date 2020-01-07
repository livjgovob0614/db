/** Generated Model - DO NOT CHANGE */
package org.compiere.model;

import java.sql.ResultSet;
import java.util.Properties;

/** Generated Model for AD_AttachmentEntry
 *  @author Adempiere (generated) 
 */
@SuppressWarnings("javadoc")
public class X_AD_AttachmentEntry extends org.compiere.model.PO implements I_AD_AttachmentEntry, org.compiere.model.I_Persistent 
{

	/**
	 *
	 */
	private static final long serialVersionUID = 1221070142L;

    /** Standard Constructor */
    public X_AD_AttachmentEntry (Properties ctx, int AD_AttachmentEntry_ID, String trxName)
    {
      super (ctx, AD_AttachmentEntry_ID, trxName);
      /** if (AD_AttachmentEntry_ID == 0)
        {
			setAD_AttachmentEntry_ID (0);
			setFileName (null);
			setType (null);
        } */
    }

    /** Load Constructor */
    public X_AD_AttachmentEntry (Properties ctx, ResultSet rs, String trxName)
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

	/** Set Attachment entry.
		@param AD_AttachmentEntry_ID Attachment entry	  */
	@Override
	public void setAD_AttachmentEntry_ID (int AD_AttachmentEntry_ID)
	{
		if (AD_AttachmentEntry_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_AD_AttachmentEntry_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_AD_AttachmentEntry_ID, Integer.valueOf(AD_AttachmentEntry_ID));
	}

	/** Get Attachment entry.
		@return Attachment entry	  */
	@Override
	public int getAD_AttachmentEntry_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_AttachmentEntry_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_AD_Attachment getAD_Attachment() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_AD_Attachment_ID, org.compiere.model.I_AD_Attachment.class);
	}

	@Override
	public void setAD_Attachment(org.compiere.model.I_AD_Attachment AD_Attachment)
	{
		set_ValueFromPO(COLUMNNAME_AD_Attachment_ID, org.compiere.model.I_AD_Attachment.class, AD_Attachment);
	}

	/** Set Anlage.
		@param AD_Attachment_ID 
		Anlage zum Eintrag
	  */
	@Override
	public void setAD_Attachment_ID (int AD_Attachment_ID)
	{
		if (AD_Attachment_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_AD_Attachment_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_AD_Attachment_ID, Integer.valueOf(AD_Attachment_ID));
	}

	/** Get Anlage.
		@return Anlage zum Eintrag
	  */
	@Override
	public int getAD_Attachment_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_Attachment_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Binärwert.
		@param BinaryData 
		Binärer Wert
	  */
	@Override
	public void setBinaryData (byte[] BinaryData)
	{
		set_ValueNoCheck (COLUMNNAME_BinaryData, BinaryData);
	}

	/** Get Binärwert.
		@return Binärer Wert
	  */
	@Override
	public byte[] getBinaryData () 
	{
		return (byte[])get_Value(COLUMNNAME_BinaryData);
	}

	/** Set Content type.
		@param ContentType Content type	  */
	@Override
	public void setContentType (java.lang.String ContentType)
	{
		set_Value (COLUMNNAME_ContentType, ContentType);
	}

	/** Get Content type.
		@return Content type	  */
	@Override
	public java.lang.String getContentType () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ContentType);
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

	/** Set File Name.
		@param FileName 
		Name of the local file or URL
	  */
	@Override
	public void setFileName (java.lang.String FileName)
	{
		set_Value (COLUMNNAME_FileName, FileName);
	}

	/** Get File Name.
		@return Name of the local file or URL
	  */
	@Override
	public java.lang.String getFileName () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_FileName);
	}

	/** Set Tags.
		@param Tags Tags	  */
	@Override
	public void setTags (java.lang.String Tags)
	{
		set_Value (COLUMNNAME_Tags, Tags);
	}

	/** Get Tags.
		@return Tags	  */
	@Override
	public java.lang.String getTags () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Tags);
	}

	/** 
	 * Type AD_Reference_ID=540751
	 * Reference name: AD_AttachmentEntry_Type
	 */
	public static final int TYPE_AD_Reference_ID=540751;
	/** Data = D */
	public static final String TYPE_Data = "D";
	/** URL = U */
	public static final String TYPE_URL = "U";
	/** Set Art.
		@param Type Art	  */
	@Override
	public void setType (java.lang.String Type)
	{

		set_ValueNoCheck (COLUMNNAME_Type, Type);
	}

	/** Get Art.
		@return Art	  */
	@Override
	public java.lang.String getType () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Type);
	}

	/** Set URL.
		@param URL 
		Full URL address - e.g. http://www.adempiere.org
	  */
	@Override
	public void setURL (java.lang.String URL)
	{
		set_Value (COLUMNNAME_URL, URL);
	}

	/** Get URL.
		@return Full URL address - e.g. http://www.adempiere.org
	  */
	@Override
	public java.lang.String getURL () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_URL);
	}
}