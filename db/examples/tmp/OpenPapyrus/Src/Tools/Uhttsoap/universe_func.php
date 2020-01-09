<?php
   
    /*
        ������� ���������� XML ��������� � �������� ���������, ������������ ���������
        $root - �������� ������� � �������� XML
        
        ������������ ��� php ���������� SimpleXML
    */
    function AddXMLEntities($root)
    {
        $entities = "<!DOCTYPE ".$root." [<!ENTITY amp '&#38;#38;'><!ENTITY exclam '&#33;'><!ENTITY sharp '&#35;'><!ENTITY pct '&#37;'><!ENTITY apostr '&#39;'><!ENTITY comma '&#44;'><!ENTITY dot '&#46;'><!ENTITY fwsl '&#47;'><!ENTITY lt '&#38;#60;'><!ENTITY eq '&#61;'><!ENTITY gt '&#62;'><!ENTITY ques '&#63;'><!ENTITY lsq '&#91;'><!ENTITY bksl '&#92;'><!ENTITY rsq '&#93;'>]>";
        return "<?xml version='1.0' encoding='windows-1251'?>".$entities;
    }

    /*
        ������� �������� ������ �� UhttXML - XML ������������� ������ � ������
        UhttLocation
        
        ������������ ��� ���������� ��������� ����
    */
    function CopyAddressFromXML(&$UhttLocation, $UhttXML)
    {
        $UhttLocation->LocID = (int)$UhttXML->LocID;
        $UhttLocation->LocKind = (int)$UhttXML->LocKind;
        $UhttLocation->LocCode = (string)$UhttXML->LocCode;
        $UhttLocation->LocName = (string)$UhttXML->LocName;
        $UhttLocation->CountryID = (int)$UhttXML->CountryID;
        $UhttLocation->Country = (string)$UhttXML->Country;
        $UhttLocation->CityID = (int)$UhttXML->CityID;
        $UhttLocation->City = (string)$UhttXML->City;
        $UhttLocation->ZIP = (string)$UhttXML->ZIP;
        $UhttLocation->Address = (string)$UhttXML->Address;
        $UhttLocation->Latitude = (float)$UhttXML->Latitude;
        $UhttLocation->Longitude = (float)$UhttXML->Longitude;
    }
    
    /*
        ������� ��������� ���������� ��� ������� � �������� ������ �� ������������
        ��������. ��� ����������� ����������� �������� ������������� � �� ���������
        � ��������. ��������� ����������� ��������� ������ ����
        ��������(�������)
        � ������, ���� ��� ��������� �������� ��������� - �������� ��������
        ���������� ByCriteria, ������� ������ ���������� ��������.
        
        ������ ������ ���� � ��������� windows-1251
        ���������� true � ������ �������� ��������
    */
    function CheckSelectCriterion(&$ObjName, &$ByCriteria)
    {
        // ������ ����������� �������� � ��������������� �� ���������
        $UhttObjects = array
        (
            "goods" => array
            (
                "id" => "",
                "name" => "",
                "subname" => "",
                "parent.id" => "",
                "parent.code" => "",
                "parent.name" => "",
                "brand.id" => "",
                "brand.name" => "",
                "manuf.id" => "",
                "manuf.name" => "",
                "manuf.code" => "",
                "code" => "",
                "barcode" => "",
                "arcode" => ""
            ),
            "goodsgroup" => array
            (
                "id" => "",
                "name" => "",
                "subname" => "",
                "code" => "",
                "parent.id" => "",
                "parent.code" => "",
                "parent.name" => ""
            ),
            "brand" => array
            (
                "id" => "",
                "name" => "",
                "subname" => "",
                "owner.id" => "",
                "owner.code" => "",
                "owner.name" => ""
            ),
            "personkind" => array
            (
                "id" => "",
                "name" => "",
                "code" => ""
            ),
            "person" => array
            (
                "id" => "",
                "name" => "",
                "subname" => "",
                "kind.id" => "",
                "kind.name" => "",
                "kind.code" => "",
                "register.def" => "",
                "register.id" => "",
                "register.code" => ""
            ),
            "city" => array
            (
                "id" => "",
                "name" => "",
                "code" => "",
                "parent.id" => "",
                "parent.code" => ""
            ),
            "country" => array
            (
                "id" => "",
                "name" => "",
                "code" => "",
                "parent.id" => "",
                "parent.code" => ""
            )
        );
        
        // �������� �������
        $ObjName = trim($ObjName);
        $ByCriteria = trim($ByCriteria);
        
        $valid = true;
        if(!isset($UhttObjects[strtolower($ObjName)])) 
			$valid = false;
        elseif(strlen($ByCriteria) > 0) {
            // ��������� ��������
            $arr = array();
            preg_match_all("/(.+)\(.+\)/Ui", $ByCriteria, $arr, PREG_PATTERN_ORDER);
            $c = count($arr[1]);
            if($c == 0) 
				$valid = false;
            else {
                // ��������� ��� �������� �� ������������
                $ByNew = "";
                for($i = 0; $i < $c; $i++) {
                    $ByNew .= $arr[0][$i];
                    if(!isset($UhttObjects[$ObjName][strtolower(trim($arr[1][$i]))])) 
						$valid = false;
                }
                if($valid) $ByCriteria = $ByNew;
            }
        }
        return $valid;
    }
  
?>
