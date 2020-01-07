/** Generated Model - DO NOT CHANGE */
package org.compiere.model;

import java.sql.ResultSet;
import java.util.Properties;

/** Generated Model for AD_UI_Element
 *  @author Adempiere (generated) 
 */
@SuppressWarnings("javadoc")
public class X_AD_UI_Element extends org.compiere.model.PO implements I_AD_UI_Element, org.compiere.model.I_Persistent 
{

	/**
	 *
	 */
	private static final long serialVersionUID = 2130719123L;

    /** Standard Constructor */
    public X_AD_UI_Element (Properties ctx, int AD_UI_Element_ID, String trxName)
    {
      super (ctx, AD_UI_Element_ID, trxName);
      /** if (AD_UI_Element_ID == 0)
        {
			setAD_Tab_ID (0);
			setAD_UI_Element_ID (0);
			setAD_UI_ElementType (null); // F
			setIsAdvancedField (false); // N
			setIsAllowFiltering (false); // N
			setIsDisplayed (true); // Y
			setIsDisplayed_SideList (false); // N
			setIsDisplayedGrid (false); // N
			setIsMultiLine (false); // N
			setName (null);
			setSeqNo (0); // @SQL=SELECT COALESCE(MAX(SeqNo), 0) + 10 FROM AD_UI_Element WHERE AD_UI_ElementGroup_ID=@AD_UI_ElementGroup_ID@
			setSeqNo_SideList (0); // 0
			setSeqNoGrid (0); // 0
        } */
    }

    /** Load Constructor */
    public X_AD_UI_Element (Properties ctx, ResultSet rs, String trxName)
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

	@Override
	public org.compiere.model.I_AD_Field getAD_Field()
	{
		return get_ValueAsPO(COLUMNNAME_AD_Field_ID, org.compiere.model.I_AD_Field.class);
	}

	@Override
	public void setAD_Field(org.compiere.model.I_AD_Field AD_Field)
	{
		set_ValueFromPO(COLUMNNAME_AD_Field_ID, org.compiere.model.I_AD_Field.class, AD_Field);
	}

	/** Set Feld.
		@param AD_Field_ID 
		Ein Feld einer Datenbanktabelle
	  */
	@Override
	public void setAD_Field_ID (int AD_Field_ID)
	{
		if (AD_Field_ID < 1) 
			set_Value (COLUMNNAME_AD_Field_ID, null);
		else 
			set_Value (COLUMNNAME_AD_Field_ID, Integer.valueOf(AD_Field_ID));
	}

	/** Get Feld.
		@return Ein Feld einer Datenbanktabelle
	  */
	@Override
	public int getAD_Field_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_Field_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_AD_Tab getAD_Tab()
	{
		return get_ValueAsPO(COLUMNNAME_AD_Tab_ID, org.compiere.model.I_AD_Tab.class);
	}

	@Override
	public void setAD_Tab(org.compiere.model.I_AD_Tab AD_Tab)
	{
		set_ValueFromPO(COLUMNNAME_AD_Tab_ID, org.compiere.model.I_AD_Tab.class, AD_Tab);
	}

	/** Set Register.
		@param AD_Tab_ID 
		Register auf einem Fenster
	  */
	@Override
	public void setAD_Tab_ID (int AD_Tab_ID)
	{
		if (AD_Tab_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_AD_Tab_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_AD_Tab_ID, Integer.valueOf(AD_Tab_ID));
	}

	/** Get Register.
		@return Register auf einem Fenster
	  */
	@Override
	public int getAD_Tab_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_Tab_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set UI Element.
		@param AD_UI_Element_ID UI Element	  */
	@Override
	public void setAD_UI_Element_ID (int AD_UI_Element_ID)
	{
		if (AD_UI_Element_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_AD_UI_Element_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_AD_UI_Element_ID, Integer.valueOf(AD_UI_Element_ID));
	}

	/** Get UI Element.
		@return UI Element	  */
	@Override
	public int getAD_UI_Element_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_UI_Element_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_AD_UI_ElementGroup getAD_UI_ElementGroup()
	{
		return get_ValueAsPO(COLUMNNAME_AD_UI_ElementGroup_ID, org.compiere.model.I_AD_UI_ElementGroup.class);
	}

	@Override
	public void setAD_UI_ElementGroup(org.compiere.model.I_AD_UI_ElementGroup AD_UI_ElementGroup)
	{
		set_ValueFromPO(COLUMNNAME_AD_UI_ElementGroup_ID, org.compiere.model.I_AD_UI_ElementGroup.class, AD_UI_ElementGroup);
	}

	/** Set UI Element Group.
		@param AD_UI_ElementGroup_ID UI Element Group	  */
	@Override
	public void setAD_UI_ElementGroup_ID (int AD_UI_ElementGroup_ID)
	{
		if (AD_UI_ElementGroup_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_AD_UI_ElementGroup_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_AD_UI_ElementGroup_ID, Integer.valueOf(AD_UI_ElementGroup_ID));
	}

	/** Get UI Element Group.
		@return UI Element Group	  */
	@Override
	public int getAD_UI_ElementGroup_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_UI_ElementGroup_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** 
	 * AD_UI_ElementType AD_Reference_ID=540736
	 * Reference name: AD_UI_ElementType
	 */
	public static final int AD_UI_ELEMENTTYPE_AD_Reference_ID=540736;
	/** Field = F */
	public static final String AD_UI_ELEMENTTYPE_Field = "F";
	/** Labels = L */
	public static final String AD_UI_ELEMENTTYPE_Labels = "L";
	/** Set Element type.
		@param AD_UI_ElementType Element type	  */
	@Override
	public void setAD_UI_ElementType (java.lang.String AD_UI_ElementType)
	{

		set_Value (COLUMNNAME_AD_UI_ElementType, AD_UI_ElementType);
	}

	/** Get Element type.
		@return Element type	  */
	@Override
	public java.lang.String getAD_UI_ElementType () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_AD_UI_ElementType);
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

	/** Set Kommentar/Hilfe.
		@param Help 
		Comment or Hint
	  */
	@Override
	public void setHelp (java.lang.String Help)
	{
		set_Value (COLUMNNAME_Help, Help);
	}

	/** Get Kommentar/Hilfe.
		@return Comment or Hint
	  */
	@Override
	public java.lang.String getHelp () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Help);
	}

	/** Set Advanced field.
		@param IsAdvancedField Advanced field	  */
	@Override
	public void setIsAdvancedField (boolean IsAdvancedField)
	{
		set_Value (COLUMNNAME_IsAdvancedField, Boolean.valueOf(IsAdvancedField));
	}

	/** Get Advanced field.
		@return Advanced field	  */
	@Override
	public boolean isAdvancedField () 
	{
		Object oo = get_Value(COLUMNNAME_IsAdvancedField);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Allow filtering.
		@param IsAllowFiltering Allow filtering	  */
	@Override
	public void setIsAllowFiltering (boolean IsAllowFiltering)
	{
		set_Value (COLUMNNAME_IsAllowFiltering, Boolean.valueOf(IsAllowFiltering));
	}

	/** Get Allow filtering.
		@return Allow filtering	  */
	@Override
	public boolean isAllowFiltering () 
	{
		Object oo = get_Value(COLUMNNAME_IsAllowFiltering);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Displayed.
		@param IsDisplayed 
		Determines, if this field is displayed
	  */
	@Override
	public void setIsDisplayed (boolean IsDisplayed)
	{
		set_Value (COLUMNNAME_IsDisplayed, Boolean.valueOf(IsDisplayed));
	}

	/** Get Displayed.
		@return Determines, if this field is displayed
	  */
	@Override
	public boolean isDisplayed () 
	{
		Object oo = get_Value(COLUMNNAME_IsDisplayed);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Displayed in Side List.
		@param IsDisplayed_SideList 
		Determines, if this field is displayed in Side list
	  */
	@Override
	public void setIsDisplayed_SideList (boolean IsDisplayed_SideList)
	{
		set_Value (COLUMNNAME_IsDisplayed_SideList, Boolean.valueOf(IsDisplayed_SideList));
	}

	/** Get Displayed in Side List.
		@return Determines, if this field is displayed in Side list
	  */
	@Override
	public boolean isDisplayed_SideList () 
	{
		Object oo = get_Value(COLUMNNAME_IsDisplayed_SideList);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Displayed in Grid.
		@param IsDisplayedGrid 
		Determines, if this field is displayed in grid mode
	  */
	@Override
	public void setIsDisplayedGrid (boolean IsDisplayedGrid)
	{
		set_Value (COLUMNNAME_IsDisplayedGrid, Boolean.valueOf(IsDisplayedGrid));
	}

	/** Get Displayed in Grid.
		@return Determines, if this field is displayed in grid mode
	  */
	@Override
	public boolean isDisplayedGrid () 
	{
		Object oo = get_Value(COLUMNNAME_IsDisplayedGrid);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Multi Line.
		@param IsMultiLine Multi Line	  */
	@Override
	public void setIsMultiLine (boolean IsMultiLine)
	{
		set_Value (COLUMNNAME_IsMultiLine, Boolean.valueOf(IsMultiLine));
	}

	/** Get Multi Line.
		@return Multi Line	  */
	@Override
	public boolean isMultiLine () 
	{
		Object oo = get_Value(COLUMNNAME_IsMultiLine);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	@Override
	public org.compiere.model.I_AD_Field getLabels_Selector_Field()
	{
		return get_ValueAsPO(COLUMNNAME_Labels_Selector_Field_ID, org.compiere.model.I_AD_Field.class);
	}

	@Override
	public void setLabels_Selector_Field(org.compiere.model.I_AD_Field Labels_Selector_Field)
	{
		set_ValueFromPO(COLUMNNAME_Labels_Selector_Field_ID, org.compiere.model.I_AD_Field.class, Labels_Selector_Field);
	}

	/** Set Labels selector field.
		@param Labels_Selector_Field_ID Labels selector field	  */
	@Override
	public void setLabels_Selector_Field_ID (int Labels_Selector_Field_ID)
	{
		if (Labels_Selector_Field_ID < 1) 
			set_Value (COLUMNNAME_Labels_Selector_Field_ID, null);
		else 
			set_Value (COLUMNNAME_Labels_Selector_Field_ID, Integer.valueOf(Labels_Selector_Field_ID));
	}

	/** Get Labels selector field.
		@return Labels selector field	  */
	@Override
	public int getLabels_Selector_Field_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_Labels_Selector_Field_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_AD_Tab getLabels_Tab()
	{
		return get_ValueAsPO(COLUMNNAME_Labels_Tab_ID, org.compiere.model.I_AD_Tab.class);
	}

	@Override
	public void setLabels_Tab(org.compiere.model.I_AD_Tab Labels_Tab)
	{
		set_ValueFromPO(COLUMNNAME_Labels_Tab_ID, org.compiere.model.I_AD_Tab.class, Labels_Tab);
	}

	/** Set Labels content tab.
		@param Labels_Tab_ID Labels content tab	  */
	@Override
	public void setLabels_Tab_ID (int Labels_Tab_ID)
	{
		if (Labels_Tab_ID < 1) 
			set_Value (COLUMNNAME_Labels_Tab_ID, null);
		else 
			set_Value (COLUMNNAME_Labels_Tab_ID, Integer.valueOf(Labels_Tab_ID));
	}

	/** Get Labels content tab.
		@return Labels content tab	  */
	@Override
	public int getLabels_Tab_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_Labels_Tab_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Media Types.
		@param MediaTypes Media Types	  */
	@Override
	public void setMediaTypes (java.lang.String MediaTypes)
	{
		set_Value (COLUMNNAME_MediaTypes, MediaTypes);
	}

	/** Get Media Types.
		@return Media Types	  */
	@Override
	public java.lang.String getMediaTypes () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_MediaTypes);
	}

	/** Set Lines Count.
		@param MultiLine_LinesCount Lines Count	  */
	@Override
	public void setMultiLine_LinesCount (int MultiLine_LinesCount)
	{
		set_Value (COLUMNNAME_MultiLine_LinesCount, Integer.valueOf(MultiLine_LinesCount));
	}

	/** Get Lines Count.
		@return Lines Count	  */
	@Override
	public int getMultiLine_LinesCount () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_MultiLine_LinesCount);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Name.
		@param Name Name	  */
	@Override
	public void setName (java.lang.String Name)
	{
		set_Value (COLUMNNAME_Name, Name);
	}

	/** Get Name.
		@return Name	  */
	@Override
	public java.lang.String getName () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Name);
	}

	/** Set Reihenfolge.
		@param SeqNo 
		Zur Bestimmung der Reihenfolge der Einträge; die kleinste Zahl kommt zuerst
	  */
	@Override
	public void setSeqNo (int SeqNo)
	{
		set_Value (COLUMNNAME_SeqNo, Integer.valueOf(SeqNo));
	}

	/** Get Reihenfolge.
		@return Zur Bestimmung der Reihenfolge der Einträge; die kleinste Zahl kommt zuerst
	  */
	@Override
	public int getSeqNo () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_SeqNo);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Reihenfolge (Side List).
		@param SeqNo_SideList Reihenfolge (Side List)	  */
	@Override
	public void setSeqNo_SideList (int SeqNo_SideList)
	{
		set_Value (COLUMNNAME_SeqNo_SideList, Integer.valueOf(SeqNo_SideList));
	}

	/** Get Reihenfolge (Side List).
		@return Reihenfolge (Side List)	  */
	@Override
	public int getSeqNo_SideList () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_SeqNo_SideList);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Reihenfolge (grid).
		@param SeqNoGrid 
		Zur Bestimmung der Reihenfolge der Einträge; die kleinste Zahl kommt zuerst
	  */
	@Override
	public void setSeqNoGrid (int SeqNoGrid)
	{
		set_Value (COLUMNNAME_SeqNoGrid, Integer.valueOf(SeqNoGrid));
	}

	/** Get Reihenfolge (grid).
		@return Zur Bestimmung der Reihenfolge der Einträge; die kleinste Zahl kommt zuerst
	  */
	@Override
	public int getSeqNoGrid () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_SeqNoGrid);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set UI Style.
		@param UIStyle UI Style	  */
	@Override
	public void setUIStyle (java.lang.String UIStyle)
	{
		set_Value (COLUMNNAME_UIStyle, UIStyle);
	}

	/** Get UI Style.
		@return UI Style	  */
	@Override
	public java.lang.String getUIStyle () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_UIStyle);
	}

	/** 
	 * WidgetSize AD_Reference_ID=540724
	 * Reference name: WidgetSize_WEBUI
	 */
	public static final int WIDGETSIZE_AD_Reference_ID=540724;
	/** Small = S */
	public static final String WIDGETSIZE_Small = "S";
	/** Medium = M */
	public static final String WIDGETSIZE_Medium = "M";
	/** Large = L */
	public static final String WIDGETSIZE_Large = "L";
	/** ExtraLarge = XL */
	public static final String WIDGETSIZE_ExtraLarge = "XL";
	/** XXL = XXL */
	public static final String WIDGETSIZE_XXL = "XXL";
	/** Set Widget size.
		@param WidgetSize Widget size	  */
	@Override
	public void setWidgetSize (java.lang.String WidgetSize)
	{

		set_Value (COLUMNNAME_WidgetSize, WidgetSize);
	}

	/** Get Widget size.
		@return Widget size	  */
	@Override
	public java.lang.String getWidgetSize () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_WidgetSize);
	}
}