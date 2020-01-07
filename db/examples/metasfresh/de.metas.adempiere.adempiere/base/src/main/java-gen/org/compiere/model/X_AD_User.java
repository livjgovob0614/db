/** Generated Model - DO NOT CHANGE */
package org.compiere.model;

import java.sql.ResultSet;
import java.util.Properties;

/** Generated Model for AD_User
 *  @author Adempiere (generated) 
 */
@SuppressWarnings("javadoc")
public class X_AD_User extends org.compiere.model.PO implements I_AD_User, org.compiere.model.I_Persistent 
{

	/**
	 *
	 */
	private static final long serialVersionUID = 172095627L;

    /** Standard Constructor */
    public X_AD_User (Properties ctx, int AD_User_ID, String trxName)
    {
      super (ctx, AD_User_ID, trxName);
      /** if (AD_User_ID == 0)
        {
			setAD_User_ID (0);
			setIsBillToContact_Default (false); // N
			setIsDataClarified (false); // N
			setIsDefaultContact (false); // N
			setIsFullBPAccess (true); // Y
			setIsInPayroll (false); // N
			setIsNews (false); // N
			setIsNewsletter (false); // N
			setIsPurchaseContact_Default (false); // N
			setIsSalesContact_Default (false); // N
			setIsShipToContact_Default (false); // N
			setName (null);
			setNotificationType (null); // N
        } */
    }

    /** Load Constructor */
    public X_AD_User (Properties ctx, ResultSet rs, String trxName)
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
	 * AD_Language AD_Reference_ID=327
	 * Reference name: AD_Language System
	 */
	public static final int AD_LANGUAGE_AD_Reference_ID=327;
	/** Set Sprache.
		@param AD_Language 
		Sprache für diesen Eintrag
	  */
	@Override
	public void setAD_Language (java.lang.String AD_Language)
	{

		set_Value (COLUMNNAME_AD_Language, AD_Language);
	}

	/** Get Sprache.
		@return Sprache für diesen Eintrag
	  */
	@Override
	public java.lang.String getAD_Language () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_AD_Language);
	}

	@Override
	public org.compiere.model.I_AD_Org getAD_OrgTrx() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_AD_OrgTrx_ID, org.compiere.model.I_AD_Org.class);
	}

	@Override
	public void setAD_OrgTrx(org.compiere.model.I_AD_Org AD_OrgTrx)
	{
		set_ValueFromPO(COLUMNNAME_AD_OrgTrx_ID, org.compiere.model.I_AD_Org.class, AD_OrgTrx);
	}

	/** Set Buchende Organisation.
		@param AD_OrgTrx_ID 
		Performing or initiating organization
	  */
	@Override
	public void setAD_OrgTrx_ID (int AD_OrgTrx_ID)
	{
		if (AD_OrgTrx_ID < 1) 
			set_Value (COLUMNNAME_AD_OrgTrx_ID, null);
		else 
			set_Value (COLUMNNAME_AD_OrgTrx_ID, Integer.valueOf(AD_OrgTrx_ID));
	}

	/** Get Buchende Organisation.
		@return Performing or initiating organization
	  */
	@Override
	public int getAD_OrgTrx_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_OrgTrx_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Ansprechpartner.
		@param AD_User_ID 
		User within the system - Internal or Business Partner Contact
	  */
	@Override
	public void setAD_User_ID (int AD_User_ID)
	{
		if (AD_User_ID < 0) 
			set_ValueNoCheck (COLUMNNAME_AD_User_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_AD_User_ID, Integer.valueOf(AD_User_ID));
	}

	/** Get Ansprechpartner.
		@return User within the system - Internal or Business Partner Contact
	  */
	@Override
	public int getAD_User_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_User_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_AD_User getAD_User_InCharge() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_AD_User_InCharge_ID, org.compiere.model.I_AD_User.class);
	}

	@Override
	public void setAD_User_InCharge(org.compiere.model.I_AD_User AD_User_InCharge)
	{
		set_ValueFromPO(COLUMNNAME_AD_User_InCharge_ID, org.compiere.model.I_AD_User.class, AD_User_InCharge);
	}

	/** Set Betreuer.
		@param AD_User_InCharge_ID 
		Person, die bei einem fachlichen Problem vom System informiert wird.
	  */
	@Override
	public void setAD_User_InCharge_ID (int AD_User_InCharge_ID)
	{
		if (AD_User_InCharge_ID < 1) 
			set_Value (COLUMNNAME_AD_User_InCharge_ID, null);
		else 
			set_Value (COLUMNNAME_AD_User_InCharge_ID, Integer.valueOf(AD_User_InCharge_ID));
	}

	/** Get Betreuer.
		@return Person, die bei einem fachlichen Problem vom System informiert wird.
	  */
	@Override
	public int getAD_User_InCharge_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_AD_User_InCharge_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Adresse.
		@param Address 
		Anschrift
	  */
	@Override
	public void setAddress (java.lang.String Address)
	{
		throw new IllegalArgumentException ("Address is virtual column");	}

	/** Get Adresse.
		@return Anschrift
	  */
	@Override
	public java.lang.String getAddress () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Address);
	}

	@Override
	public org.compiere.model.I_AD_Image getAvatar() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_Avatar_ID, org.compiere.model.I_AD_Image.class);
	}

	@Override
	public void setAvatar(org.compiere.model.I_AD_Image Avatar)
	{
		set_ValueFromPO(COLUMNNAME_Avatar_ID, org.compiere.model.I_AD_Image.class, Avatar);
	}

	/** Set Avatar.
		@param Avatar_ID Avatar	  */
	@Override
	public void setAvatar_ID (int Avatar_ID)
	{
		if (Avatar_ID < 1) 
			set_Value (COLUMNNAME_Avatar_ID, null);
		else 
			set_Value (COLUMNNAME_Avatar_ID, Integer.valueOf(Avatar_ID));
	}

	/** Get Avatar.
		@return Avatar	  */
	@Override
	public int getAvatar_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_Avatar_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Geburtstag.
		@param Birthday 
		Birthday or Anniversary day
	  */
	@Override
	public void setBirthday (java.sql.Timestamp Birthday)
	{
		set_Value (COLUMNNAME_Birthday, Birthday);
	}

	/** Get Geburtstag.
		@return Birthday or Anniversary day
	  */
	@Override
	public java.sql.Timestamp getBirthday () 
	{
		return (java.sql.Timestamp)get_Value(COLUMNNAME_Birthday);
	}

	@Override
	public org.compiere.model.I_C_BPartner getBPartner_Parent() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_BPartner_Parent_ID, org.compiere.model.I_C_BPartner.class);
	}

	@Override
	public void setBPartner_Parent(org.compiere.model.I_C_BPartner BPartner_Parent)
	{
		set_ValueFromPO(COLUMNNAME_BPartner_Parent_ID, org.compiere.model.I_C_BPartner.class, BPartner_Parent);
	}

	/** Set Partner Parent.
		@param BPartner_Parent_ID 
		Business Partner Parent
	  */
	@Override
	public void setBPartner_Parent_ID (int BPartner_Parent_ID)
	{
		throw new IllegalArgumentException ("BPartner_Parent_ID is virtual column");	}

	/** Get Partner Parent.
		@return Business Partner Parent
	  */
	@Override
	public int getBPartner_Parent_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_BPartner_Parent_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_C_BP_Group getC_BP_Group() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_C_BP_Group_ID, org.compiere.model.I_C_BP_Group.class);
	}

	@Override
	public void setC_BP_Group(org.compiere.model.I_C_BP_Group C_BP_Group)
	{
		set_ValueFromPO(COLUMNNAME_C_BP_Group_ID, org.compiere.model.I_C_BP_Group.class, C_BP_Group);
	}

	/** Set Geschäftspartnergruppe.
		@param C_BP_Group_ID 
		Geschäftspartnergruppe
	  */
	@Override
	public void setC_BP_Group_ID (int C_BP_Group_ID)
	{
		throw new IllegalArgumentException ("C_BP_Group_ID is virtual column");	}

	/** Get Geschäftspartnergruppe.
		@return Geschäftspartnergruppe
	  */
	@Override
	public int getC_BP_Group_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_C_BP_Group_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_C_BPartner getC_BPartner() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_C_BPartner_ID, org.compiere.model.I_C_BPartner.class);
	}

	@Override
	public void setC_BPartner(org.compiere.model.I_C_BPartner C_BPartner)
	{
		set_ValueFromPO(COLUMNNAME_C_BPartner_ID, org.compiere.model.I_C_BPartner.class, C_BPartner);
	}

	/** Set Geschäftspartner.
		@param C_BPartner_ID 
		Identifies a Business Partner
	  */
	@Override
	public void setC_BPartner_ID (int C_BPartner_ID)
	{
		if (C_BPartner_ID < 1) 
			set_ValueNoCheck (COLUMNNAME_C_BPartner_ID, null);
		else 
			set_ValueNoCheck (COLUMNNAME_C_BPartner_ID, Integer.valueOf(C_BPartner_ID));
	}

	/** Get Geschäftspartner.
		@return Identifies a Business Partner
	  */
	@Override
	public int getC_BPartner_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_C_BPartner_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_C_BPartner_Location getC_BPartner_Location() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_C_BPartner_Location_ID, org.compiere.model.I_C_BPartner_Location.class);
	}

	@Override
	public void setC_BPartner_Location(org.compiere.model.I_C_BPartner_Location C_BPartner_Location)
	{
		set_ValueFromPO(COLUMNNAME_C_BPartner_Location_ID, org.compiere.model.I_C_BPartner_Location.class, C_BPartner_Location);
	}

	/** Set Standort.
		@param C_BPartner_Location_ID 
		Identifies the (ship to) address for this Business Partner
	  */
	@Override
	public void setC_BPartner_Location_ID (int C_BPartner_Location_ID)
	{
		if (C_BPartner_Location_ID < 1) 
			set_Value (COLUMNNAME_C_BPartner_Location_ID, null);
		else 
			set_Value (COLUMNNAME_C_BPartner_Location_ID, Integer.valueOf(C_BPartner_Location_ID));
	}

	/** Get Standort.
		@return Identifies the (ship to) address for this Business Partner
	  */
	@Override
	public int getC_BPartner_Location_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_C_BPartner_Location_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_C_Country getC_Country() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_C_Country_ID, org.compiere.model.I_C_Country.class);
	}

	@Override
	public void setC_Country(org.compiere.model.I_C_Country C_Country)
	{
		set_ValueFromPO(COLUMNNAME_C_Country_ID, org.compiere.model.I_C_Country.class, C_Country);
	}

	/** Set Land.
		@param C_Country_ID 
		Land
	  */
	@Override
	public void setC_Country_ID (int C_Country_ID)
	{
		throw new IllegalArgumentException ("C_Country_ID is virtual column");	}

	/** Get Land.
		@return Land
	  */
	@Override
	public int getC_Country_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_C_Country_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_C_Greeting getC_Greeting() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_C_Greeting_ID, org.compiere.model.I_C_Greeting.class);
	}

	@Override
	public void setC_Greeting(org.compiere.model.I_C_Greeting C_Greeting)
	{
		set_ValueFromPO(COLUMNNAME_C_Greeting_ID, org.compiere.model.I_C_Greeting.class, C_Greeting);
	}

	/** Set Anrede (ID).
		@param C_Greeting_ID 
		Anrede zum Druck auf Korrespondenz
	  */
	@Override
	public void setC_Greeting_ID (int C_Greeting_ID)
	{
		if (C_Greeting_ID < 1) 
			set_Value (COLUMNNAME_C_Greeting_ID, null);
		else 
			set_Value (COLUMNNAME_C_Greeting_ID, Integer.valueOf(C_Greeting_ID));
	}

	/** Get Anrede (ID).
		@return Anrede zum Druck auf Korrespondenz
	  */
	@Override
	public int getC_Greeting_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_C_Greeting_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_C_Job getC_Job() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_C_Job_ID, org.compiere.model.I_C_Job.class);
	}

	@Override
	public void setC_Job(org.compiere.model.I_C_Job C_Job)
	{
		set_ValueFromPO(COLUMNNAME_C_Job_ID, org.compiere.model.I_C_Job.class, C_Job);
	}

	/** Set Position.
		@param C_Job_ID 
		Job Position
	  */
	@Override
	public void setC_Job_ID (int C_Job_ID)
	{
		if (C_Job_ID < 1) 
			set_Value (COLUMNNAME_C_Job_ID, null);
		else 
			set_Value (COLUMNNAME_C_Job_ID, Integer.valueOf(C_Job_ID));
	}

	/** Get Position.
		@return Job Position
	  */
	@Override
	public int getC_Job_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_C_Job_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_C_Region getC_Region() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_C_Region_ID, org.compiere.model.I_C_Region.class);
	}

	@Override
	public void setC_Region(org.compiere.model.I_C_Region C_Region)
	{
		set_ValueFromPO(COLUMNNAME_C_Region_ID, org.compiere.model.I_C_Region.class, C_Region);
	}

	/** Set Region.
		@param C_Region_ID 
		Identifiziert eine geographische Region
	  */
	@Override
	public void setC_Region_ID (int C_Region_ID)
	{
		throw new IllegalArgumentException ("C_Region_ID is virtual column");	}

	/** Get Region.
		@return Identifiziert eine geographische Region
	  */
	@Override
	public int getC_Region_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_C_Region_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Bemerkungen.
		@param Comments 
		Comments or additional information
	  */
	@Override
	public void setComments (java.lang.String Comments)
	{
		set_Value (COLUMNNAME_Comments, Comments);
	}

	/** Get Bemerkungen.
		@return Comments or additional information
	  */
	@Override
	public java.lang.String getComments () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Comments);
	}

	/** 
	 * ConnectionProfile AD_Reference_ID=364
	 * Reference name: AD_User ConnectionProfile
	 */
	public static final int CONNECTIONPROFILE_AD_Reference_ID=364;
	/** LAN = L */
	public static final String CONNECTIONPROFILE_LAN = "L";
	/** TerminalServer = T */
	public static final String CONNECTIONPROFILE_TerminalServer = "T";
	/** VPN = V */
	public static final String CONNECTIONPROFILE_VPN = "V";
	/** WAN = W */
	public static final String CONNECTIONPROFILE_WAN = "W";
	/** Set Verbindungsart.
		@param ConnectionProfile 
		How a Java Client connects to the server(s)
	  */
	@Override
	public void setConnectionProfile (java.lang.String ConnectionProfile)
	{

		set_Value (COLUMNNAME_ConnectionProfile, ConnectionProfile);
	}

	/** Get Verbindungsart.
		@return How a Java Client connects to the server(s)
	  */
	@Override
	public java.lang.String getConnectionProfile () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ConnectionProfile);
	}

	/** 
	 * ContactLimitation AD_Reference_ID=540089
	 * Reference name: ContactLimitation
	 */
	public static final int CONTACTLIMITATION_AD_Reference_ID=540089;
	/** keine Anrufe = B */
	public static final String CONTACTLIMITATION_KeineAnrufe = "B";
	/** keine Anrufe und eMails = C */
	public static final String CONTACTLIMITATION_KeineAnrufeUndEMails = "C";
	/** keine Anrufe, eMails und Post = D */
	public static final String CONTACTLIMITATION_KeineAnrufeEMailsUndPost = "D";
	/** keine eMail und Post = E */
	public static final String CONTACTLIMITATION_KeineEMailUndPost = "E";
	/** Set Kontakt Einschränkung.
		@param ContactLimitation Kontakt Einschränkung	  */
	@Override
	public void setContactLimitation (java.lang.String ContactLimitation)
	{

		set_Value (COLUMNNAME_ContactLimitation, ContactLimitation);
	}

	/** Get Kontakt Einschränkung.
		@return Kontakt Einschränkung	  */
	@Override
	public java.lang.String getContactLimitation () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ContactLimitation);
	}

	/** Set Begründung.
		@param ContactLimitationReason Begründung	  */
	@Override
	public void setContactLimitationReason (java.lang.String ContactLimitationReason)
	{
		set_Value (COLUMNNAME_ContactLimitationReason, ContactLimitationReason);
	}

	/** Get Begründung.
		@return Begründung	  */
	@Override
	public java.lang.String getContactLimitationReason () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ContactLimitationReason);
	}

	/** Set Löschdatum.
		@param DelDate Löschdatum	  */
	@Override
	public void setDelDate (java.sql.Timestamp DelDate)
	{
		set_Value (COLUMNNAME_DelDate, DelDate);
	}

	/** Get Löschdatum.
		@return Löschdatum	  */
	@Override
	public java.sql.Timestamp getDelDate () 
	{
		return (java.sql.Timestamp)get_Value(COLUMNNAME_DelDate);
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

	/** Set eMail.
		@param EMail 
		EMail-Adresse
	  */
	@Override
	public void setEMail (java.lang.String EMail)
	{
		set_Value (COLUMNNAME_EMail, EMail);
	}

	/** Get eMail.
		@return EMail-Adresse
	  */
	@Override
	public java.lang.String getEMail () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_EMail);
	}

	/** Set EMail Nutzer-ID.
		@param EMailUser 
		User Name (ID) in the Mail System
	  */
	@Override
	public void setEMailUser (java.lang.String EMailUser)
	{
		set_Value (COLUMNNAME_EMailUser, EMailUser);
	}

	/** Get EMail Nutzer-ID.
		@return User Name (ID) in the Mail System
	  */
	@Override
	public java.lang.String getEMailUser () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_EMailUser);
	}

	/** Set Passwort EMail-Nutzer.
		@param EMailUserPW 
		Passwort Ihrer EMail Nutzer-ID
	  */
	@Override
	public void setEMailUserPW (java.lang.String EMailUserPW)
	{
		set_Value (COLUMNNAME_EMailUserPW, EMailUserPW);
	}

	/** Get Passwort EMail-Nutzer.
		@return Passwort Ihrer EMail Nutzer-ID
	  */
	@Override
	public java.lang.String getEMailUserPW () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_EMailUserPW);
	}

	/** Set Überprüfung EMail.
		@param EMailVerify 
		Verification information of EMail Address
	  */
	@Override
	public void setEMailVerify (java.lang.String EMailVerify)
	{
		set_ValueNoCheck (COLUMNNAME_EMailVerify, EMailVerify);
	}

	/** Get Überprüfung EMail.
		@return Verification information of EMail Address
	  */
	@Override
	public java.lang.String getEMailVerify () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_EMailVerify);
	}

	/** Set EMail überprüft.
		@param EMailVerifyDate 
		Date Email was verified
	  */
	@Override
	public void setEMailVerifyDate (java.sql.Timestamp EMailVerifyDate)
	{
		set_ValueNoCheck (COLUMNNAME_EMailVerifyDate, EMailVerifyDate);
	}

	/** Get EMail überprüft.
		@return Date Email was verified
	  */
	@Override
	public java.sql.Timestamp getEMailVerifyDate () 
	{
		return (java.sql.Timestamp)get_Value(COLUMNNAME_EMailVerifyDate);
	}

	/** Set External ID.
		@param ExternalId External ID	  */
	@Override
	public void setExternalId (java.lang.String ExternalId)
	{
		set_Value (COLUMNNAME_ExternalId, ExternalId);
	}

	/** Get External ID.
		@return External ID	  */
	@Override
	public java.lang.String getExternalId () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_ExternalId);
	}

	/** Set Fax.
		@param Fax 
		Facsimile number
	  */
	@Override
	public void setFax (java.lang.String Fax)
	{
		set_Value (COLUMNNAME_Fax, Fax);
	}

	/** Get Fax.
		@return Facsimile number
	  */
	@Override
	public java.lang.String getFax () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Fax);
	}

	/** Set Vorname.
		@param Firstname 
		Vorname
	  */
	@Override
	public void setFirstname (java.lang.String Firstname)
	{
		set_Value (COLUMNNAME_Firstname, Firstname);
	}

	/** Get Vorname.
		@return Vorname
	  */
	@Override
	public java.lang.String getFirstname () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Firstname);
	}

	/** 
	 * Fresh_xmas_Gift AD_Reference_ID=540504
	 * Reference name: fresh_xmas_list
	 */
	public static final int FRESH_XMAS_GIFT_AD_Reference_ID=540504;
	/** Karte = K */
	public static final String FRESH_XMAS_GIFT_Karte = "K";
	/** Geschenk = G */
	public static final String FRESH_XMAS_GIFT_Geschenk = "G";
	/** Set Weihnachtsgeschenk.
		@param Fresh_xmas_Gift Weihnachtsgeschenk	  */
	@Override
	public void setFresh_xmas_Gift (java.lang.String Fresh_xmas_Gift)
	{

		set_Value (COLUMNNAME_Fresh_xmas_Gift, Fresh_xmas_Gift);
	}

	/** Get Weihnachtsgeschenk.
		@return Weihnachtsgeschenk	  */
	@Override
	public java.lang.String getFresh_xmas_Gift () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Fresh_xmas_Gift);
	}

	/** Set Anrede.
		@param Greeting 
		Für Briefe - z.B. "Sehr geehrter {0}" oder "Sehr geehrter Herr {0}" - Zur Laufzeit wird  "{0}" durch den Namen ersetzt
	  */
	@Override
	public void setGreeting (java.lang.String Greeting)
	{
		throw new IllegalArgumentException ("Greeting is virtual column");	}

	/** Get Anrede.
		@return Für Briefe - z.B. "Sehr geehrter {0}" oder "Sehr geehrter Herr {0}" - Zur Laufzeit wird  "{0}" durch den Namen ersetzt
	  */
	@Override
	public java.lang.String getGreeting () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Greeting);
	}

	/** Set Included Tab.
		@param Included_Tab_ID 
		Included Tab in this Tab (Master Dateail)
	  */
	@Override
	public void setIncluded_Tab_ID (java.lang.String Included_Tab_ID)
	{
		set_Value (COLUMNNAME_Included_Tab_ID, Included_Tab_ID);
	}

	/** Get Included Tab.
		@return Included Tab in this Tab (Master Dateail)
	  */
	@Override
	public java.lang.String getIncluded_Tab_ID () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Included_Tab_ID);
	}

	/** Set Rechnungskontakt.
		@param IsBillToContact_Default Rechnungskontakt	  */
	@Override
	public void setIsBillToContact_Default (boolean IsBillToContact_Default)
	{
		set_Value (COLUMNNAME_IsBillToContact_Default, Boolean.valueOf(IsBillToContact_Default));
	}

	/** Get Rechnungskontakt.
		@return Rechnungskontakt	  */
	@Override
	public boolean isBillToContact_Default () 
	{
		Object oo = get_Value(COLUMNNAME_IsBillToContact_Default);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Kunde.
		@param IsCustomer 
		Zeigt an, ob dieser Geschäftspartner ein Kunde ist
	  */
	@Override
	public void setIsCustomer (boolean IsCustomer)
	{
		throw new IllegalArgumentException ("IsCustomer is virtual column");	}

	/** Get Kunde.
		@return Zeigt an, ob dieser Geschäftspartner ein Kunde ist
	  */
	@Override
	public boolean isCustomer () 
	{
		Object oo = get_Value(COLUMNNAME_IsCustomer);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Bereinigt.
		@param IsDataClarified 
		Nur bereinigte Daten ausgeben
	  */
	@Override
	public void setIsDataClarified (boolean IsDataClarified)
	{
		set_Value (COLUMNNAME_IsDataClarified, Boolean.valueOf(IsDataClarified));
	}

	/** Get Bereinigt.
		@return Nur bereinigte Daten ausgeben
	  */
	@Override
	public boolean isDataClarified () 
	{
		Object oo = get_Value(COLUMNNAME_IsDataClarified);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Standard-Ansprechpartner.
		@param IsDefaultContact Standard-Ansprechpartner	  */
	@Override
	public void setIsDefaultContact (boolean IsDefaultContact)
	{
		set_Value (COLUMNNAME_IsDefaultContact, Boolean.valueOf(IsDefaultContact));
	}

	/** Get Standard-Ansprechpartner.
		@return Standard-Ansprechpartner	  */
	@Override
	public boolean isDefaultContact () 
	{
		Object oo = get_Value(COLUMNNAME_IsDefaultContact);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Full BP Access.
		@param IsFullBPAccess 
		The user/contact has full access to Business Partner information and resources
	  */
	@Override
	public void setIsFullBPAccess (boolean IsFullBPAccess)
	{
		set_Value (COLUMNNAME_IsFullBPAccess, Boolean.valueOf(IsFullBPAccess));
	}

	/** Get Full BP Access.
		@return The user/contact has full access to Business Partner information and resources
	  */
	@Override
	public boolean isFullBPAccess () 
	{
		Object oo = get_Value(COLUMNNAME_IsFullBPAccess);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set hat zentralen Partner.
		@param IsHavingParentPartner hat zentralen Partner	  */
	@Override
	public void setIsHavingParentPartner (boolean IsHavingParentPartner)
	{
		throw new IllegalArgumentException ("IsHavingParentPartner is virtual column");	}

	/** Get hat zentralen Partner.
		@return hat zentralen Partner	  */
	@Override
	public boolean isHavingParentPartner () 
	{
		Object oo = get_Value(COLUMNNAME_IsHavingParentPartner);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Is In Payroll.
		@param IsInPayroll 
		Defined if any User Contact will be used for Calculate Payroll
	  */
	@Override
	public void setIsInPayroll (boolean IsInPayroll)
	{
		set_Value (COLUMNNAME_IsInPayroll, Boolean.valueOf(IsInPayroll));
	}

	/** Get Is In Payroll.
		@return Defined if any User Contact will be used for Calculate Payroll
	  */
	@Override
	public boolean isInPayroll () 
	{
		Object oo = get_Value(COLUMNNAME_IsInPayroll);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Login As HostKey .
		@param IsLoginAsHostKey Login As HostKey 	  */
	@Override
	public void setIsLoginAsHostKey (boolean IsLoginAsHostKey)
	{
		set_Value (COLUMNNAME_IsLoginAsHostKey, Boolean.valueOf(IsLoginAsHostKey));
	}

	/** Get Login As HostKey .
		@return Login As HostKey 	  */
	@Override
	public boolean isLoginAsHostKey () 
	{
		Object oo = get_Value(COLUMNNAME_IsLoginAsHostKey);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Newsletter.
		@param IsNews 
		Template or container uses news channels
	  */
	@Override
	public void setIsNews (boolean IsNews)
	{
		set_Value (COLUMNNAME_IsNews, Boolean.valueOf(IsNews));
	}

	/** Get Newsletter.
		@return Template or container uses news channels
	  */
	@Override
	public boolean isNews () 
	{
		Object oo = get_Value(COLUMNNAME_IsNews);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Newsletter.
		@param IsNewsletter Newsletter	  */
	@Override
	public void setIsNewsletter (boolean IsNewsletter)
	{
		set_Value (COLUMNNAME_IsNewsletter, Boolean.valueOf(IsNewsletter));
	}

	/** Get Newsletter.
		@return Newsletter	  */
	@Override
	public boolean isNewsletter () 
	{
		Object oo = get_Value(COLUMNNAME_IsNewsletter);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Zielkunde.
		@param IsProspect 
		Kennzeichnet einen Interessenten oder Kunden
	  */
	@Override
	public void setIsProspect (boolean IsProspect)
	{
		throw new IllegalArgumentException ("IsProspect is virtual column");	}

	/** Get Zielkunde.
		@return Kennzeichnet einen Interessenten oder Kunden
	  */
	@Override
	public boolean isProspect () 
	{
		Object oo = get_Value(COLUMNNAME_IsProspect);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Einkaufskontakt.
		@param IsPurchaseContact Einkaufskontakt	  */
	@Override
	public void setIsPurchaseContact (boolean IsPurchaseContact)
	{
		set_Value (COLUMNNAME_IsPurchaseContact, Boolean.valueOf(IsPurchaseContact));
	}

	/** Get Einkaufskontakt.
		@return Einkaufskontakt	  */
	@Override
	public boolean isPurchaseContact () 
	{
		Object oo = get_Value(COLUMNNAME_IsPurchaseContact);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set IsPurchaseContact_Default.
		@param IsPurchaseContact_Default IsPurchaseContact_Default	  */
	@Override
	public void setIsPurchaseContact_Default (boolean IsPurchaseContact_Default)
	{
		set_Value (COLUMNNAME_IsPurchaseContact_Default, Boolean.valueOf(IsPurchaseContact_Default));
	}

	/** Get IsPurchaseContact_Default.
		@return IsPurchaseContact_Default	  */
	@Override
	public boolean isPurchaseContact_Default () 
	{
		Object oo = get_Value(COLUMNNAME_IsPurchaseContact_Default);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Verkaufskontakt.
		@param IsSalesContact Verkaufskontakt	  */
	@Override
	public void setIsSalesContact (boolean IsSalesContact)
	{
		set_Value (COLUMNNAME_IsSalesContact, Boolean.valueOf(IsSalesContact));
	}

	/** Get Verkaufskontakt.
		@return Verkaufskontakt	  */
	@Override
	public boolean isSalesContact () 
	{
		Object oo = get_Value(COLUMNNAME_IsSalesContact);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set IsSalesContact_Default.
		@param IsSalesContact_Default IsSalesContact_Default	  */
	@Override
	public void setIsSalesContact_Default (boolean IsSalesContact_Default)
	{
		set_Value (COLUMNNAME_IsSalesContact_Default, Boolean.valueOf(IsSalesContact_Default));
	}

	/** Get IsSalesContact_Default.
		@return IsSalesContact_Default	  */
	@Override
	public boolean isSalesContact_Default () 
	{
		Object oo = get_Value(COLUMNNAME_IsSalesContact_Default);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Selbstregistrierung.
		@param IsSelfService 
		This is a Self-Service entry or this entry can be changed via Self-Service
	  */
	@Override
	public void setIsSelfService (boolean IsSelfService)
	{
		throw new IllegalArgumentException ("IsSelfService is virtual column");	}

	/** Get Selbstregistrierung.
		@return This is a Self-Service entry or this entry can be changed via Self-Service
	  */
	@Override
	public boolean isSelfService () 
	{
		Object oo = get_Value(COLUMNNAME_IsSelfService);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Lieferkontakt.
		@param IsShipToContact_Default Lieferkontakt	  */
	@Override
	public void setIsShipToContact_Default (boolean IsShipToContact_Default)
	{
		set_Value (COLUMNNAME_IsShipToContact_Default, Boolean.valueOf(IsShipToContact_Default));
	}

	/** Get Lieferkontakt.
		@return Lieferkontakt	  */
	@Override
	public boolean isShipToContact_Default () 
	{
		Object oo = get_Value(COLUMNNAME_IsShipToContact_Default);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set IsSubjectMatterContact.
		@param IsSubjectMatterContact IsSubjectMatterContact	  */
	@Override
	public void setIsSubjectMatterContact (boolean IsSubjectMatterContact)
	{
		set_Value (COLUMNNAME_IsSubjectMatterContact, Boolean.valueOf(IsSubjectMatterContact));
	}

	/** Get IsSubjectMatterContact.
		@return IsSubjectMatterContact	  */
	@Override
	public boolean isSubjectMatterContact () 
	{
		Object oo = get_Value(COLUMNNAME_IsSubjectMatterContact);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Sprachregion.
		@param Language Sprachregion	  */
	@Override
	public void setLanguage (java.lang.String Language)
	{
		throw new IllegalArgumentException ("Language is virtual column");	}

	/** Get Sprachregion.
		@return Sprachregion	  */
	@Override
	public java.lang.String getLanguage () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Language);
	}

	/** Set Nachname.
		@param Lastname Nachname	  */
	@Override
	public void setLastname (java.lang.String Lastname)
	{
		set_Value (COLUMNNAME_Lastname, Lastname);
	}

	/** Get Nachname.
		@return Nachname	  */
	@Override
	public java.lang.String getLastname () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Lastname);
	}

	/** Set Memo.
		@param Memo 
		Memo Text
	  */
	@Override
	public void setMemo (java.lang.String Memo)
	{
		set_Value (COLUMNNAME_Memo, Memo);
	}

	/** Get Memo.
		@return Memo Text
	  */
	@Override
	public java.lang.String getMemo () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Memo);
	}

	/** Set Handynummer.
		@param MobilePhone Handynummer	  */
	@Override
	public void setMobilePhone (java.lang.String MobilePhone)
	{
		set_Value (COLUMNNAME_MobilePhone, MobilePhone);
	}

	/** Get Handynummer.
		@return Handynummer	  */
	@Override
	public java.lang.String getMobilePhone () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_MobilePhone);
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

	/** 
	 * NotificationType AD_Reference_ID=344
	 * Reference name: AD_User NotificationType
	 */
	public static final int NOTIFICATIONTYPE_AD_Reference_ID=344;
	/** EMail = E */
	public static final String NOTIFICATIONTYPE_EMail = "E";
	/** Notice = N */
	public static final String NOTIFICATIONTYPE_Notice = "N";
	/** None = X */
	public static final String NOTIFICATIONTYPE_None = "X";
	/** EMailPlusNotice = B */
	public static final String NOTIFICATIONTYPE_EMailPlusNotice = "B";
	/** NotifyUserInCharge = O */
	public static final String NOTIFICATIONTYPE_NotifyUserInCharge = "O";
	/** Set Benachrichtigungs-Art.
		@param NotificationType 
		Type of Notifications
	  */
	@Override
	public void setNotificationType (java.lang.String NotificationType)
	{

		set_Value (COLUMNNAME_NotificationType, NotificationType);
	}

	/** Get Benachrichtigungs-Art.
		@return Type of Notifications
	  */
	@Override
	public java.lang.String getNotificationType () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_NotificationType);
	}

	/** Set Kennwort.
		@param Password Kennwort	  */
	@Override
	public void setPassword (java.lang.String Password)
	{
		set_Value (COLUMNNAME_Password, Password);
	}

	/** Get Kennwort.
		@return Kennwort	  */
	@Override
	public java.lang.String getPassword () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Password);
	}

	/** Set Portalpasswort.
		@param passwordportal Portalpasswort	  */
	@Override
	public void setpasswordportal (java.lang.String passwordportal)
	{
		set_Value (COLUMNNAME_passwordportal, passwordportal);
	}

	/** Get Portalpasswort.
		@return Portalpasswort	  */
	@Override
	public java.lang.String getpasswordportal () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_passwordportal);
	}

	/** Set Code für Passwort-Änderung.
		@param PasswordResetCode Code für Passwort-Änderung	  */
	@Override
	public void setPasswordResetCode (java.lang.String PasswordResetCode)
	{
		set_Value (COLUMNNAME_PasswordResetCode, PasswordResetCode);
	}

	/** Get Code für Passwort-Änderung.
		@return Code für Passwort-Änderung	  */
	@Override
	public java.lang.String getPasswordResetCode () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_PasswordResetCode);
	}

	/** Set Telefon.
		@param Phone 
		Beschreibt eine Telefon Nummer
	  */
	@Override
	public void setPhone (java.lang.String Phone)
	{
		set_Value (COLUMNNAME_Phone, Phone);
	}

	/** Get Telefon.
		@return Beschreibt eine Telefon Nummer
	  */
	@Override
	public java.lang.String getPhone () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Phone);
	}

	/** Set Mobil.
		@param Phone2 
		Alternative Mobile Telefonnummer
	  */
	@Override
	public void setPhone2 (java.lang.String Phone2)
	{
		set_Value (COLUMNNAME_Phone2, Phone2);
	}

	/** Get Mobil.
		@return Alternative Mobile Telefonnummer
	  */
	@Override
	public java.lang.String getPhone2 () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Phone2);
	}

	/** Set Verarbeiten.
		@param Processing Verarbeiten	  */
	@Override
	public void setProcessing (boolean Processing)
	{
		set_Value (COLUMNNAME_Processing, Boolean.valueOf(Processing));
	}

	/** Get Verarbeiten.
		@return Verarbeiten	  */
	@Override
	public boolean isProcessing () 
	{
		Object oo = get_Value(COLUMNNAME_Processing);
		if (oo != null) 
		{
			 if (oo instanceof Boolean) 
				 return ((Boolean)oo).booleanValue(); 
			return "Y".equals(oo);
		}
		return false;
	}

	/** Set Handelsregister.
		@param Registry 
		Handelsregister
	  */
	@Override
	public void setRegistry (java.lang.String Registry)
	{
		set_Value (COLUMNNAME_Registry, Registry);
	}

	/** Get Handelsregister.
		@return Handelsregister
	  */
	@Override
	public java.lang.String getRegistry () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Registry);
	}

	@Override
	public org.compiere.model.I_AD_User getSalesRep() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_SalesRep_ID, org.compiere.model.I_AD_User.class);
	}

	@Override
	public void setSalesRep(org.compiere.model.I_AD_User SalesRep)
	{
		set_ValueFromPO(COLUMNNAME_SalesRep_ID, org.compiere.model.I_AD_User.class, SalesRep);
	}

	/** Set Beratung.
		@param SalesRep_ID Beratung	  */
	@Override
	public void setSalesRep_ID (int SalesRep_ID)
	{
		throw new IllegalArgumentException ("SalesRep_ID is virtual column");	}

	/** Get Beratung.
		@return Beratung	  */
	@Override
	public int getSalesRep_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_SalesRep_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	@Override
	public org.compiere.model.I_AD_User getSupervisor() throws RuntimeException
	{
		return get_ValueAsPO(COLUMNNAME_Supervisor_ID, org.compiere.model.I_AD_User.class);
	}

	@Override
	public void setSupervisor(org.compiere.model.I_AD_User Supervisor)
	{
		set_ValueFromPO(COLUMNNAME_Supervisor_ID, org.compiere.model.I_AD_User.class, Supervisor);
	}

	/** Set Vorgesetzter.
		@param Supervisor_ID 
		Supervisor for this user/organization - used for escalation and approval
	  */
	@Override
	public void setSupervisor_ID (int Supervisor_ID)
	{
		if (Supervisor_ID < 1) 
			set_Value (COLUMNNAME_Supervisor_ID, null);
		else 
			set_Value (COLUMNNAME_Supervisor_ID, Integer.valueOf(Supervisor_ID));
	}

	/** Get Vorgesetzter.
		@return Supervisor for this user/organization - used for escalation and approval
	  */
	@Override
	public int getSupervisor_ID () 
	{
		Integer ii = (Integer)get_Value(COLUMNNAME_Supervisor_ID);
		if (ii == null)
			 return 0;
		return ii.intValue();
	}

	/** Set Titel.
		@param Title 
		Name this entity is referred to as
	  */
	@Override
	public void setTitle (java.lang.String Title)
	{
		set_Value (COLUMNNAME_Title, Title);
	}

	/** Get Titel.
		@return Name this entity is referred to as
	  */
	@Override
	public java.lang.String getTitle () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Title);
	}

	/** Set Benutzer PIN.
		@param UserPIN Benutzer PIN	  */
	@Override
	public void setUserPIN (java.lang.String UserPIN)
	{
		set_Value (COLUMNNAME_UserPIN, UserPIN);
	}

	/** Get Benutzer PIN.
		@return Benutzer PIN	  */
	@Override
	public java.lang.String getUserPIN () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_UserPIN);
	}

	/** Set Suchschlüssel.
		@param Value 
		Search key for the record in the format required - must be unique
	  */
	@Override
	public void setValue (java.lang.String Value)
	{
		set_Value (COLUMNNAME_Value, Value);
	}

	/** Get Suchschlüssel.
		@return Search key for the record in the format required - must be unique
	  */
	@Override
	public java.lang.String getValue () 
	{
		return (java.lang.String)get_Value(COLUMNNAME_Value);
	}
}