/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   Copyright (C) 2000-2001  Sylvain Duclos                               *
 *   sylvain_duclos@yahoo.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include <wx/tokenzr.h>

#include <chart/S57Chart.h>
#include <s52plib.h>
#include <chart/s52utils.h>
#include <dychart.h>

bool GetDoubleAttr(S57Obj *obj, const char *AttrName, double &val);

#define UNKNOWN 1e6 //HUGE_VAL   // INFINITY/NAN


// size of attributes value list buffer
#define LISTSIZE   16   // list size

extern s52plib  *ps52plib;

wxString *CSQUAPNT01(S57Obj *obj);
wxString *CSQUALIN01(S57Obj *obj);



static void *CLRLIN01(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        printf("s52csny : CLRLIN01 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

static void *DATCVR01(void *param)
{

// Remarks: This conditional symbology procedure describes procedures for:
// - symbolizing the limit of ENC coverage;
// - symbolizing navigational purpose boundaries ("scale boundarie"); and
// - indicating overscale display.
           //
// Note that the mandatory meta object CATQUA is symbolized by the look-up table.
           //
// Because the methods adopted by an ECDIS to meet the IMO and IHO requirements
// listed on the next page will depend on the manufacturer's software, and cannot be
// described in terms of a flow chart in the same way as other conditional procedures,
// this procedure is in the form of written notes.

//    ObjRazRules *rzRules = (ObjRazRules *)param;
//    S57Obj *obj = rzRules->obj;

    wxString rule_str;
       ///////////////////////
    // 1- REQUIREMENT
    // (IMO/IHO specs. explenation)

       ///////////////////////
    // 2- ENC COVERAGE
       //
    // 2.1- Limit of ENC coverage
    //datcvr01 = g_string_new(";OP(3OD11060);LC(HODATA01)");
    rule_str.Append(_T("LC(HODATA01)"));
//    rule_str.Append("AC(DEPDW)");
    // FIXME: get cell extend

    // 2.2- No data areas
    // This can be done outside of CS (ie when clearing the screen in Mesa)
    // FIXME: ";OP(0---);AC(NODATA)"
    // FIXME: set geo to cover earth (!)

       //////////////////////
    // 3- SCALE BOUNDARIES
       //
    // 3.1- Chart scale boundaties
    // FIXME;
    //g_string_append(datcvr01, ";LS(SOLD,1,CHGRD)");
    // -OR- LC(SCLBDYnn) (?)
       //
    // ;OP(3OS21030)

    // 3.2- Graphical index of navigational purpose
    // FIXME: draw extent of available SENC in DB

       //////////////////////
    // 4- OVERSCALE
       //
    // FIXME: get meta date CSCL of DSPM field
    // FIXME: get object M_CSCL or CSCALE
       //
    // 4.1- Overscale indication
       // FIXME: compute, scale = [denominator of the compilation scale] /
    //                         [denominator of the display scale]
    // FIXME: draw overscale indication (ie TX("X%3.1f",scale))
       //
    // 4.2- Ovescale area at a chart scale boundary
    // FIXME: test if next chart is over scale (ie going from large scale chart
    //        to a small scale chart)
    // FIXME: draw AP(OVERSC01) on overscale part of display
    //g_string(";OP(3OS21030)");

       //
    // 4.3- Larger scale data available
    // FIXME: display indication of better scale available (?)




   wxString datcvr01;
   datcvr01.Append(rule_str);
   datcvr01.Append('\037');

   char *r = (char *)malloc(datcvr01.Len() + 1);
   strcpy(r, datcvr01.mb_str());

   return r;

}


bool GetIntAttr(S57Obj *obj, const char *AttrName, int &val)
{
    int idx = obj->GetAttributeIndex(AttrName);
    
    if(idx >= 0) {
        //      using idx to get the attribute value
         S57attVal *v = obj->attVal->Item(idx);
        val = *(int*)(v->value);
        
        return true;
    }
    else
        return false;
        
}

bool GetDoubleAttr(S57Obj *obj, const char *AttrName, double &val)
{
    int idx = obj->GetAttributeIndex(AttrName);
    
    if(idx >= 0) {
//      using idx to get the attribute value

        S57attVal *v = obj->attVal->Item(idx);
        val = *(double*)(v->value);

        return true;
    }
    else
        return false;
}


bool GetStringAttr(S57Obj *obj, const char *AttrName, char *pval, int nc)
{
    int idx = obj->GetAttributeIndex(AttrName);
    
    if(idx >= 0) {
        //      using idx to get the attribute value
        S57attVal *v = obj->attVal->Item(idx);

        char *val = (char *)(v->value);

        strncpy(pval, val, nc);

        return true;
    }
    else
        return false;
}

wxString *GetStringAttrWXS(S57Obj *obj, const char *AttrName)
{
    int idx = obj->GetAttributeIndex(AttrName);
    
    if(idx >= 0) {
        //      using idx to get the attribute value
        S57attVal *v = obj->attVal->Item(idx);
        
        char *val = (char *)(v->value);
        
        return new wxString(val,  wxConvUTF8);
    }
    else
        return NULL;
}

static int      _parseList(const char *str_in, char *buf, int buf_size)
// Put a string of comma delimited number in an array (buf).
// Return: the number of value in buf.
// Assume: - number < 256,
//         - list size less than buf_size .
// Note: buf is \0 terminated for strpbrk().
{
    char *str = (char *)str_in;
    int i = 0;

    if (NULL != str && *str != '\0') {
        do {
            if ( i>= LISTSIZE-1) {
                printf("OVERFLOW --value in list lost!!\n");
                break;
            }

            /*
            if (255 <  (unsigned char) atoi(str)) {
                PRINTF("value overflow (>255)\n");
                exit(0);
            }
            */

            buf[i++] = (unsigned char) atoi(str);

            while(isdigit(*str))
                  str++;   // next

        } while(*str++ != '\0');      // skip ',' or exit
    }

    buf[i] = '\0';

    return i;
}


static int      _atPtPos(S57Obj *objNew, wxArrayPtrVoid *curntList, int bSectorCheck)
// return TRUE if there is a light at this position
// or if its an extended arc radius else FALSE
{
    unsigned int i;

    if(NULL == curntList)
          return false;

    for (i=0; i<curntList->GetCount(); i++) {
        S57Obj *objOld = (S57Obj *)curntList->Item(i);

        if ((objOld->x == objNew->x) && (objOld->y == objNew->y)) {

            if (!bSectorCheck)
                return TRUE;
        }
    }

    return FALSE;
}

wxString _selSYcol(char *buf, bool bsectr, double valnmr)
{
    wxString sym;

    if(!bsectr)
    {

      sym = _T(";SY(LITDEF11");                 // default

    // max 1 color
      if ('\0' == buf[1])
      {
        if (strpbrk(buf, "\003"))
              sym = _T(";SY(LIGHTS11");
        else if (strpbrk(buf, "\004"))
              sym = _T(";SY(LIGHTS12");
        else if (strpbrk(buf, "\001\006\011"))
              sym = _T(";SY(LIGHTS13");
      }
      else
      {
        // max 2 color
        if ('\0' == buf[2]) {
            if (strpbrk(buf, "\001") && strpbrk(buf, "\003"))
                  sym = _T(";SY(LIGHTS11");
            else if (strpbrk(buf, "\001") && strpbrk(buf, "\004"))
                  sym = _T(";SY(LIGHTS12");
        }
      }
    }

    else                // all-round fixed light symbolized as a circle, radius depends on color
                        // This treatment is seen on SeeMyDenc by SevenCs
                        // This may not be S-52 compliant....
    {
          //      Another non-standard extension....
          //      All round light circle diameter is scaled if the light has a reasonable VALNMR attribute
          int radius = 3;
          if(valnmr > 0)
          {
                if(valnmr < 7.0)
                      radius = 3;
                else if(valnmr < 15.0)
                      radius = 10;
                else if(valnmr < 30.0)
                      radius = 15;
                else
                      radius = 20;
          }

       // max 1 color
      if ('\0' == buf[1])
      {
          if (strpbrk(buf, "\003"))
                sym.Printf(_T(",LITRD, 2,0,360,%d,0"), radius + 1);
          else if (strpbrk(buf, "\004"))
                sym.Printf(_T(",LITGN, 2,0,360,%d,0"), radius);
          else if (strpbrk(buf, "\001\006\011"))
                sym.Printf(_T(",LITYW, 2,0,360,%d,0"), radius + 2);
          else if (strpbrk(buf, "\014"))
                sym.Printf(_T(",CHMGD, 2,0,360,%d,0"), radius + 3);
          else
                sym.Printf(_T(",CHMGD, 2,0,360,%d,0"), radius + 5);           // default

      }
      else  if ('\0' == buf[2])       // or 2 color
      {
                if (strpbrk(buf, "\001") && strpbrk(buf, "\003"))
                      sym.Printf(_T(",LITRD, 2,0,360,%d,0"), radius + 1);
                else if (strpbrk(buf, "\001") && strpbrk(buf, "\004"))
                      sym.Printf(_T(",LITGN, 2,0,360,%d,0"), radius);
                else
                      sym.Printf(_T(",CHMGD, 2,0,360,%d,0"), radius + 5);           // default

      }
      else
            sym.Printf(_T(",CHMGD, 2,0,360,%d,0"), radius + 5);


      if(sym.Len())
            sym.Prepend(_T(";CA(OUTLW, 4"));
    }


    return sym;
}

static double   _DEPVAL01(S57Obj *obj, double least_depth)
// Remarks: S-57 Appendix B1 Annex A requires in Section 6 that areas of rocks be
// encoded as area obstruction, and that area OBSTRNs and area WRECKS
// be covered by either group 1 object DEPARE or group 1 object UNSARE.
// If the value of the attribute VALSOU for an area OBSTRN or WRECKS
// is missing, the DRVAL1 of an underlying DEPARE is the preferred default
// for establishing a depth value. This procedure either finds the shallowest
// DRVAL1 of the one or more underlying DEPAREs, or returns an
// "unknown"" depth value to the main procedure for the next default
// procedure.

// NOTE: UNSARE test is useless since least_depth is already UNKNOWN
{
    least_depth = UNKNOWN;

    return least_depth;
}

static wxString *_UDWHAZ03(S57Obj *obj, double depth_value, ObjRazRules *rzRules)
// Remarks: Obstructions or isolated underwater dangers of depths less than the safety
// contour which lie within the safe waters defined by the safety contour are
// to be presented by a specific isolated danger symbol as hazardous objects
// and put in IMO category DISPLAYBASE (see (3), App.2, 1.3). This task
// is performed by this conditional symbology procedure.
{
    wxString udwhaz03str;
    int      danger         = FALSE;
    double   safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);

    if(depth_value == UNKNOWN)
          danger = TRUE;

    else if (depth_value <= safety_contour) {
        // that intersect this point/line/area for OBSTRN04
        // that intersect this point/area      for WRECKS02

        // get area DEPARE & DRGARE that intersect this point/line/area

        ListOfS57Obj *pobj_list = rzRules->chart->GetAssociatedObjects(obj);

        wxListOfS57ObjNode *node = pobj_list->GetFirst();
        while(node)
        {
              S57Obj *ptest_obj = node->GetData();
              if(GEO_LINE == ptest_obj->Primitive_type)
              {
                    double drval2 = 0.0;
                    GetDoubleAttr(ptest_obj, "DRVAL2", drval2);

                    if(drval2 < safety_contour)
                    {
                          danger = TRUE;
                          break;
                    }
              }
              else
              {
                    double drval1 = 0.0;
                    GetDoubleAttr(ptest_obj, "DRVAL1", drval1);

                    if(drval1 >= safety_contour)
                    {
                          danger = TRUE;
                          break;
                    }
              }
              node = node->GetNext();
        }

        delete pobj_list;
    }

    if (TRUE == danger)
    {
              int watlev;
              GetIntAttr(obj, "WATLEV", watlev);

              if((1 == watlev) || (2 == watlev))
              {
              }
              else
              {
                    udwhaz03str = _T(";SY(ISODGR51)");     //_T(";OP(8OD14010);SY(ISODGR51)");
              }

              //  Move this object to DisplayBase category
              rzRules->obj->m_DisplayCat = DISPLAYBASE;

    }
    // This is an enhancement to the original PLIB spec
    //  It forces all obstructions (rocks/wrecks) to be in category "Standard", at least
    else{
        rzRules->obj->m_DisplayCat = STANDARD;
    }


    wxString *ret_str = new wxString(udwhaz03str);
    return ret_str;

}





// Remarks: An object of the class "depth area" is coloured and covered with fill patterns
// according to the mariners selections of shallow contour, safety contour and
// deep contour. This requires a decision making process based  on DRVAL1 and DRVAL2.
// Objects of the class "dredged area" are handled by this routine as well to
// ensure a consistent symbolization of areas that represent the surface of the
// seabed.

static void *DEPARE01(void *param)
{

   ObjRazRules *rzRules = (ObjRazRules *)param;
   S57Obj *obj = rzRules->obj;


   double drval1, drval2;
   bool drval1_found;

//      Determine the color based on mariner selections


   drval1 = -1.0;                                          // default values
   drval1_found = GetDoubleAttr(obj, "DRVAL1", drval1);
   drval2 = drval1 + 0.01;
   GetDoubleAttr(obj, "DRVAL2", drval2);




   //   Create a string of the proper color reference

    bool shallow  = TRUE;
    wxString rule_str =_T("AC(DEPIT)");


    if (drval1 >= 0.0 && drval2 > 0.0)
        rule_str  = _T("AC(DEPVS)");

    if (TRUE == S52_getMarinerParam(S52_MAR_TWO_SHADES))
    {
        if (drval1 >= S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR)  &&
            drval2 >  S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR))
        {
            rule_str  = _T("AC(DEPDW)");
            shallow = FALSE;
        }
    }
    else
    {
        if (drval1 >= S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR) &&
            drval2 >  S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR))
            rule_str  = _T("AC(DEPMS)");

        if (drval1 >= S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR)  &&
                drval2 >  S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR))
        {
            rule_str  = _T("AC(DEPMD)");
            shallow = FALSE;
        }

        if (drval1 >= S52_getMarinerParam(S52_MAR_DEEP_CONTOUR)  &&
                drval2 >  S52_getMarinerParam(S52_MAR_DEEP_CONTOUR))
        {
            rule_str  = _T("AC(DEPDW)");
            shallow = FALSE;
        }

    }


//  If object is DRGARE....

    if(!strncmp(rzRules->LUP->OBCL, "DRGARE", 6))
    {
        if (!drval1_found) //If DRVAL1 was not defined...
        {
            rule_str  = _T("AC(DEPMD)");
            shallow = FALSE;
        }
        rule_str.Append(_T(";AP(DRGARE01)"));
        rule_str.Append(_T(";LS(DASH,1,CHGRF)"));

    }


    rule_str.Append('\037');

    char *r = (char *)malloc(rule_str.Len() + 1);
    strcpy(r, rule_str.mb_str());
    return r;

}

static void *DEPCNT02 (void *param)
// Remarks: An object of the class "depth contour" or "line depth area" is highlighted and must
// be shown under all circumstances if it matches the safety contour depth value
// entered by the mariner (see IMO PS 3.6). But, while the mariner is free to enter any
// safety contour depth value that he thinks is suitable for the safety of his ship, the
// SENC only contains a limited choice of depth contours. This symbology procedure
// determines whether a contour matches the selected safety contour. If the selected
// safety contour does not exist in the data, the procedure will default to the next deeper
// contour. The contour selected is highlighted as the safety contour and put in
// DISPLAYBASE. The procedure also identifies any line segment of the spatial
// component of the object that has a "QUAPOS" value indicating unreliable
// positioning, and symbolizes it with a double dashed line.
            //
// Note: Depth contours are not normally labeled. The ECDIS may provide labels, on demand
// only as with other text, or provide the depth value on cursor picking
{
      double   depth_value;
      double drval1, drval2;
      bool safe = FALSE;
      wxString rule_str;
      double safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);

      ObjRazRules *rzRules = (ObjRazRules *)param;
      S57Obj *obj = rzRules->obj;

      if ((!strncmp(obj->FeatureName, "DEPARE", 6)) && GEO_LINE==obj->Primitive_type)
      {
            drval1 = 0.0;                                          // default values
            GetDoubleAttr(obj, "DRVAL1", drval1);
            drval2 = drval1;
            GetDoubleAttr(obj, "DRVAL2", drval2);

            if (drval1 <= safety_contour)
            {
                  if (drval2 >= safety_contour)
                        safe = TRUE;
            }
            else
            {
                  double next_safe_contour;
                  if(rzRules->chart->GetNearestSafeContour(safety_contour, next_safe_contour))
                  {
                        if (drval1 == next_safe_contour)
                              safe = TRUE;
                  }
            }

            depth_value = drval1;

      }
      else
      {
        // continuation A (DEPCNT)
            double valdco = 0;
            GetDoubleAttr(obj, "VALDCO", valdco);

            depth_value = valdco;

            if (valdco == safety_contour)
                  safe = TRUE;   // this is useless !?!?
            else
            {
                  double next_safe_contour;
                  if(rzRules->chart->GetNearestSafeContour(safety_contour, next_safe_contour))
                  {
                        if (valdco == next_safe_contour)
                              safe = TRUE;
                  }
            }
      }

    // Continuation B
      char quaposstr[20];
      quaposstr[0] = 0;
      GetStringAttr(obj, "QUAPOS", quaposstr, 19);
      int quapos = 0;
      GetIntAttr(obj, "QUAPOS", quapos);        // QUAPOS is an E (Enumerated) type attribute

      if (0 != quapos) {
            if ( 2 <= quapos && quapos < 10) {
                  if (safe) {
                      wxString safeCntr = _T("LS(DASH,2,DEPSC)");
                      S57Obj tempObj;
                      LUPrec* safelup = ps52plib->S52_LUPLookup( PLAIN_BOUNDARIES, "SAFECD", &tempObj, false );
                      if( safelup )
                          safeCntr = *safelup->INST;
                      rule_str = _T(";") + safeCntr;
                  }
                  else
                        rule_str = _T(";LS(DASH,1,DEPCN)");
            }
      } else {
            if (safe) {
                wxString safeCntr = _T("LS(SOLD,2,DEPSC)");
                S57Obj tempObj;
                LUPrec* safelup = ps52plib->S52_LUPLookup( PLAIN_BOUNDARIES, "SAFECN", &tempObj, false );
                if( safelup )
                    safeCntr = *safelup->INST;
                rule_str = _T(";") + safeCntr;
            }
            else
                  rule_str = _T(";LS(SOLD,1,DEPCN)");
      }

      if (safe) {
           //  Move this object to DisplayBase category
            rzRules->obj->m_DisplayCat = DISPLAYBASE;
            rzRules->LUP->DPRI = PRIO_HAZARDS;

      } else {
      }

    // debug
            rule_str.Append('\037');

            char *r = (char *)malloc(rule_str.Len() + 1);
            strcpy(r, rule_str.mb_str());
            return r;
}




static void *DEPVAL01(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        printf("s52csny : DEPVAL01 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

static void *LEGLIN02(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        printf("s52csny : LEGLIN02 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

static wxString _LITDSN01(S57Obj *obj);

static void *LIGHTS05 (void *param)
// Remarks: A light is one of the most complex S-57 objects. Its presentation depends on
// whether it is a light on a floating or fixed platform, its range, it's colour and
// so on. This conditional symbology procedure derives the correct
// presentation from these parameters and also generates an area that shows the
// coverage of the light.
//
// Notes on light sectors:
// 1.) The radial leg-lines defining the light sectors are normally drawn to only 25mm
// from the light to avoid clutter (see Part C). However, the mariner should be able to
// select "full light-sector lines" and have the leg-lines extended to the nominal range
// of the light (VALMAR).
//
// 2.) Part C of this procedure symbolizes the sectors at the light itself. In addition,
// it should be possible, upon request, for the mariner to be capable of identifying
// the colour and sector limit lines of the sectors affecting the ship even if the light
// itself is off the display.
// [ed. last sentence in bold]


{
#define UNKNOWN_DOUBLE -9;
    wxString lights05;

    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    double valnmr = UNKNOWN_DOUBLE;
    GetDoubleAttr(obj, "VALNMR", valnmr);


    char catlitstr[20] = {'\0'};
    GetStringAttr(obj, "CATLIT", catlitstr, 19);

    char litvisstr[20] = {'\0'};;
    GetStringAttr(obj, "LITVIS", litvisstr, 19);


    char     catlit[LISTSIZE]  = {'\0'};
    char     litvis[LISTSIZE]  = {'\0'};
    char     col_str[20] = {'\0'};
    
    bool     flare_at_45       = false;
    double   sectr1            = UNKNOWN_DOUBLE;
    double   sectr2            = UNKNOWN_DOUBLE;
    double   sweep = 0.;
    char     colist[LISTSIZE]  = {'\0'};   // colour list
    bool     b_isflare = false;

    wxString orientstr;

    if ( strlen(catlitstr))
    {
        _parseList(catlitstr, catlit, sizeof(colist));

        // FIXME: OR vs AND/OR
        if (strpbrk(catlit, "\010\013")) {
            lights05.Append(_T(";SY(LIGHTS82)"));
            goto l05_end;
        }

        if (strpbrk(catlit, "\011")) {
            lights05.Append(_T(";SY(LIGHTS81)"));
            goto l05_end;
        }

/*
        if (strpbrk(catlit, "\001\020")) {
            orientstr = S57_getAttVal(geo, "ORIENT");
            if (NULL != orientstr) {
                // FIXME: create a geo object (!?) LINE of lenght VALNMR
                // using ORIENT (from seaward) & POINT_T position
                g_string_append(lights05, ";LS(DASH,1,CHBLK)");
            }
        }
*/
    }

    // Continuation A

    GetStringAttr(obj, "COLOUR", col_str, 19);

    if (strlen(col_str))
          _parseList(col_str, colist, sizeof(colist));
    else
    {
        colist[0] = '\014';  // magenta (12)
        colist[1] = '\000';
    }

    GetDoubleAttr(obj, "SECTR1", sectr1);
    GetDoubleAttr(obj, "SECTR2", sectr2);


    if ((-9 == sectr1) || (-9 == sectr2))
    {
        // This is not a sector light

          //      What follows is one interpretation of the modern (3_3 +)
          //      Presentation Library CS flow chart, which I(dsr) have never seen verbatim.
          //      We will use flare light symbols for floating aids, and
          //      all round sector lights for fixed aids.

        wxString ssym;

        if(_atPtPos(obj, rzRules->chart->pFloatingATONArray, false))          // Is this LIGHTS feature colocated with ...ANY... floating aid?
        {
            flare_at_45 = false;

            //TODO create LightArray in s57chart.
            //  Then, if another LIGHT object is colocated here, set flare_at_45
/*            if(_atPtPos(obj, rzRules->chart->pLIGHTSArray, false))          // Is this LIGHTS feature colocated with another LIGHTS?


            //    If the light is white, yellow, or orange, make it a flare at 45 degrees
                  if(strpbrk(colist, "\001\005\011"))
                    flare_at_45 = true;
*/
            ssym = _selSYcol(colist, 0, valnmr);              // flare
            b_isflare = true;
        }
        else
        {
            ssym = _selSYcol(colist, 1, valnmr);              // all round light
            b_isflare = false;
        }


        //  Is the light a directional or moire?
        if (strpbrk(catlit, "\001\016"))
        {
            if (orientstr.Len())
            {
                lights05.Append(ssym);
                lights05.Append(orientstr);
                lights05.Append(_T(";TE('%03.0lf deg','ORIENT',3,3,3,'15110',3,1,CHBLK,23)" ));
            }
            else
                lights05.Append(_T(";SY(QUESMRK1)"));
        }
        else
        {
            lights05.Append(ssym);
            if(b_isflare)
            {
                if (flare_at_45)
                      lights05.Append(_T(",45)"));
                else
                      lights05.Append(_T(",135)"));
            }
        }


        goto l05_end;
    }

    // Continuation B --sector light
    if (-9 == sectr1)
    {
        sectr1 = 0.0;
        sectr2 = 0.0;
    }
    else
        sweep = (sectr1 > sectr2) ? sectr2-sectr1+360 : sectr2-sectr1;


    if (sweep<1.0 || sweep==360.0)
    {
        // handle all round light
      wxString ssym = _selSYcol(colist, 1, valnmr);           // all round light
      lights05.Append(ssym);
      goto l05_end;
    }

    // setup sector
    {
             //        Build the (opencpn private) command string like this:
            //        e.g.  ";CA(OUTLW, 4,LITRD, 2, sectr1, sectr2, radius)"


          double arc_radius = 20.;                // mm
          double sector_radius = 25.;

          //      Another non-standard extension....
          //      Sector light arc radius is scaled if the light has a reasonable VALNMR attribute
          if(valnmr > 0)
          {
                if(valnmr < 15.0)
                      arc_radius = 10.;
                else if(valnmr < 30.0)
                      arc_radius = 15.;
                else
                      arc_radius = 20.;
          }

          char sym[80];
          strcpy(sym,";CA(OUTLW, 4");


            // max 1 color
            if ('\0' == colist[1])
            {
                if (strpbrk(colist, "\003"))
                    strcat(sym, ",LITRD, 2");
                else if (strpbrk(colist, "\004"))
                      strcat(sym, ",LITGN, 2");
                else if (strpbrk(colist, "\001\006\013"))
                      strcat(sym, ",LITYW, 2");
                else
                      strcat(sym, ",CHMGD, 2");                 // default is magenta

            }
            else if ('\0' == colist[2])
            {
                    if (strpbrk(colist, "\001") && strpbrk(colist, "\003"))
                          strcat(sym, ",LITRD, 2");
                    else if (strpbrk(colist, "\001") && strpbrk(colist, "\004"))
                          strcat(sym, ",LITGN, 2");
                    else
                          strcat(sym, ",CHMGD, 2");                 // default is magenta
            }
            else
                  strcat(sym, ",CHMGD, 2");                 // default is magenta


            if ( strlen(litvisstr))               // Obscured/faint sector?
            {
                  _parseList(litvisstr, litvis, sizeof(litvis));

                if (strpbrk(litvis, "\003\007\010"))
                     strcpy(sym, ";CA(CHBLK, 1,CHBLK, 0");
            }

            if(sectr2 <= sectr1)
                  sectr2 += 360;

            //    Sectors are defined from seaward
            if(sectr1 > 180)
                  sectr1 -= 180;
            else
                  sectr1 += 180;

            if(sectr2 > 180)
                  sectr2 -= 180;
            else
                  sectr2 += 180;

            char arc_data[80];
            sprintf(arc_data, ",%5.1f, %5.1f, %5.1f, %5.1f", sectr1, sectr2, arc_radius, sector_radius);

            strcat(sym, arc_data);

            wxString ssym(sym, wxConvUTF8);
            lights05 = ssym;

            goto l05_end;


    }


l05_end:

      if( ps52plib->m_bShowLdisText )
      {
            // Only show Light in certain position once. Otherwise there will be clutter.
            static double lastLat, lastLon;
            static wxString lastDescription;
            bool isFirstSector = true;

            if( lastLat == obj->m_lat && lastLon == obj->m_lon ) isFirstSector = false;
            lastLat = obj->m_lat;
            lastLon = obj->m_lon;

            wxString litdsn01 = _LITDSN01( obj );

            if( litdsn01.Len() && isFirstSector ) {
                  lastDescription = litdsn01;
                  lights05.Append( _T(";TX('") );
                  lights05.Append( litdsn01 );

                  if( flare_at_45 )
                        lights05.Append( _T("',3,3,3,'15110',2,-1,CHBLK,23)" ) );
                  else
                        lights05.Append( _T("',3,2,3,'15110',2,0,CHBLK,23)" ) );
            }

            if( !isFirstSector && lastDescription != litdsn01 ) {
                  lastDescription = litdsn01;
                  lights05.Append( _T(";TX('") );
                  lights05.Append( litdsn01 );
                  lights05.Append( _T("',3,2,3,'15110',2,1,CHBLK,23)" ) );
            }
      }

      lights05.Append( '\037' );

      char *r = (char *) malloc( lights05.Len() + 1 );
      strcpy( r, lights05.mb_str() );

      return r;
}





static void *LITDSN01(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        printf("s52csny : LITDSN01 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

/*
static void *OBSTRN04a(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        static int f03;
        if(!f03)
            printf("s52csny : OBSTRN04 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
        f03++;
   return NULL;
}
*/

wxString *SNDFRM02(S57Obj *obj, double depth_value);

static void *OBSTRN04 (void *param)
// Remarks: Obstructions or isolated underwater dangers of depths less than the safety
// contour which lie within the safe waters defined by the safety contour are
// to be presented by a specific isolated danger symbol and put in IMO
// category DISPLAYBASE (see (3), App.2, 1.3). This task is performed
// by the sub-procedure "UDWHAZ03" which is called by this symbology
// procedure. Objects of the class "under water rock" are handled by this
// routine as well to ensure a consistent symbolization of isolated dangers on
// the seabed.
{
      wxString obstrn04str;
      wxString *udwhaz03str = NULL;

      ObjRazRules *rzRules = (ObjRazRules *)param;
      S57Obj *obj = rzRules->obj;

      double   valsou      = UNKNOWN;
      double   depth_value = UNKNOWN;
      double   least_depth = UNKNOWN;

      wxString *sndfrm02str = NULL;
      wxString *quapnt01str = NULL;

      GetDoubleAttr(obj, "VALSOU", valsou);

      if (valsou != UNKNOWN)
      {
            depth_value = valsou;
            sndfrm02str = SNDFRM02(obj, valsou);
      }
      else
      {
            if (GEO_AREA == obj->Primitive_type)
                  least_depth = _DEPVAL01(obj, least_depth);

            if (UNKNOWN == least_depth)
            {
                  char catobsstr[20];
                  catobsstr[0] = 0;
                  GetStringAttr(obj, "CATOBS", catobsstr, 19);
                  char watlevstr[20];
                  watlevstr[0] = 0;
                  GetStringAttr(obj, "WATLEV", watlevstr, 19);

                  if ('6' == catobsstr[0])
                        depth_value = 0.01;
                  else if (0 == watlevstr[0]) // default
                        depth_value = -15.0;
                  else
                  {
                        switch (watlevstr[0]){
                              case 5: depth_value =   0.0 ; break;
                              case 3: depth_value =   0.01; break;
                              case 4:
                              case 1:
                              case 2:
                              default : depth_value = -15.0 ; break;
                        }
/*
                        switch (watlevstr[0]){
                              case '5': depth_value =   0.0 ; break;
                              case '3': depth_value =   0.01; break;
                              case '4':
                              case '1':
                              case '2':
                                    default : depth_value = -15.0 ; break;
                        }
*/
                  }
            }
            else
                  depth_value = least_depth;
      }

      udwhaz03str = _UDWHAZ03(obj, depth_value, rzRules);


      if (GEO_POINT == obj->Primitive_type)
      {
        // Continuation A
            int      sounding    = FALSE;
            quapnt01str = CSQUAPNT01(obj);

            if (0 != udwhaz03str->Len())
            {
                  obstrn04str.Append(*udwhaz03str);
                  obstrn04str.Append(*quapnt01str);

                  goto end;
            }

            if (UNKNOWN != valsou)
            {
                   if (valsou <= 20.0)
                  {
                        int watlev = -9;
                        GetIntAttr(obj, "WATLEV", watlev);

                        if (!strncmp(obj->FeatureName, "UWTROC", 6))
                        {
                              if (-9 == watlev) {  // default
                                    obstrn04str.Append(_T(";SY(DANGER51)"));
                                    sounding = TRUE;
                              } else {
                                    switch (watlev){
                                          case 3: obstrn04str.Append(_T(";SY(DANGER51)")); sounding = TRUE ; break;
                                          case 4:
                                          case 5: obstrn04str.Append(_T(";SY(UWTROC04)")); sounding = FALSE; break;
                                          default : obstrn04str.Append(_T(";SY(DANGER51)")); sounding = TRUE ; break;
                                    }
                              }
                        }
                        else
                        { // OBSTRN
                              if (-9 == watlev) { // default
                                    obstrn04str.Append(_T(";SY(DANGER01)"));
                                    sounding = TRUE;
                              } else {
                                    switch (watlev) {
                                          case 1:
                                          case 2: obstrn04str.Append(_T(";SY(LNDARE01)")); sounding = FALSE; break;
                                          case 3: obstrn04str.Append(_T(";SY(DANGER52)")); sounding = TRUE;  break;
                                          case 4:
                                          case 5: obstrn04str.Append(_T(";SY(DANGER53)")); sounding = TRUE; break;
                                          default : obstrn04str.Append(_T(";SY(DANGER51)")); sounding = TRUE; break;
                                    }
                              }
                        }
                  }
                  else
                  {  // valsou > 20.0
                        obstrn04str.Append(_T(";SY(DANGER52)"));
                        sounding = TRUE;
                  }
            }
            else
            {  // NO valsou
                  int watlev = -9;
                  GetIntAttr(obj, "WATLEV", watlev);

                  if (!strncmp(obj->FeatureName, "UWTROC", 6))
                  {
                        if (watlev == -9)  // default
                              obstrn04str.Append(_T(";SY(UWTROC04)"));
                        else {
                              switch (watlev) {
                                    case 2: obstrn04str.Append(_T(";SY(LNDARE01)")); break;
                                    case 3: obstrn04str.Append(_T(";SY(UWTROC03)")); break;
                                    default: obstrn04str.Append(_T(";SY(UWTROC04)")); break;
                              }
                        }

                  }
                  else
                  { // OBSTRN
                        if ( -9 == watlev) // default
                              obstrn04str = _T(";SY(OBSTRN01)");
                        else
                        {
                              switch (watlev) {
                                    case 1: obstrn04str.Append(_T(";SY(OBSTRN11)")); break;
                                    case 2: obstrn04str.Append(_T(";SY(OBSTRN11)")); break;
                                    case 3: obstrn04str.Append(_T(";SY(OBSTRN01)")); break;
                                    case 4: obstrn04str.Append(_T(";SY(OBSTRN03)")); break;
                                    case 5: obstrn04str.Append(_T(";SY(OBSTRN03)")); break;
                                    default : obstrn04str.Append(_T(";SY(OBSTRN01)")); break;
                              }
                        }
                  }
             }

             if (sounding)
                  obstrn04str.Append(*sndfrm02str);

             obstrn04str.Append(*quapnt01str);

            goto end;

      }     // if geopoint
      else
      {
             if (GEO_LINE == obj->Primitive_type)
             {
                 // Continuation B
                 
                 quapnt01str = CSQUAPNT01(obj);
                 
                 if( quapnt01str->Len() > 1 ) {
                     long quapos;
                     quapnt01str->ToLong(&quapos);
                     if ( 2 <= quapos && quapos < 10){
                         if (udwhaz03str->Len())
                             obstrn04str.Append(_T(";LC(LOWACC41)"));
                         else
                             obstrn04str.Append(_T(";LC(LOWACC31)"));
                     }
                     goto end;
                 }
                 
                 if ( udwhaz03str->Len() )
                 {
                     obstrn04str.Append( _T("LS(DOTT,2,CHBLK)") );
                     goto end;
                 }

                 if (UNKNOWN != valsou){
                     if (valsou <= 20.0)
                         obstrn04str.Append( _T(";LS(DOTT,2,CHBLK)") );
                     else
                         obstrn04str.Append( _T(";LS(DASH,2,CHBLK)") );
                 }
                 else
                     obstrn04str.Append( _T(";LS(DOTT,2,CHBLK)") );

                 
                 if (udwhaz03str->Len()){
                        //  Show the isolated danger symbol at the midpoint of the line
                    }
                 else {
                    if (UNKNOWN != valsou)
                        if (valsou <= 20.0)
                            obstrn04str.Append(*sndfrm02str);
                 }
               }

            else                // Area feature
            {
                  quapnt01str = CSQUAPNT01(obj);

                  if (0 != udwhaz03str->Len())
                  {
                       obstrn04str.Append(_T(";AC(DEPVS);AP(FOULAR01)"));
                       obstrn04str.Append(_T(";LS(DOTT,2,CHBLK)"));
                       obstrn04str.Append(*udwhaz03str);
                       obstrn04str.Append(*quapnt01str);

                        goto end;
                  }

                  if (UNKNOWN != valsou) {
                // BUG in CA49995B.000 if we get here because there is no color
                // beside NODATA (ie there is a hole in group 1 area!)
                //g_string_append(obstrn04, ";AC(UINFR)");

                        if (valsou <= 20.0)
                              obstrn04str.Append(_T(";LS(DOTT,2,CHBLK)"));
                        else
                              obstrn04str.Append(_T(";LS(DASH,2,CHBLK)"));

                        obstrn04str.Append(*sndfrm02str);

                  } else {
                        int watlev = -9;
                        GetIntAttr(obj, "WATLEV", watlev);

                        if (watlev == -9)   // default
                              obstrn04str.Append(_T(";AC(DEPVS);LS(DOTT,2,CHBLK)"));
                        else {
                              if (3 == watlev) {
                                    int catobs = -9;
                                    GetIntAttr(obj, "CATOBS", catobs);
                                    if (6 == catobs)
                                          obstrn04str.Append(_T(";AC(DEPVS);AP(FOULAR01);LS(DOTT,2,CHBLK)"));
                              } else {
                                    switch (watlev) {
                                          case 1:
                                          case 2: obstrn04str.Append(_T(";AC(CHBRN);LS(SOLD,2,CSTLN)")); break;
                                          case 4: obstrn04str.Append(_T(";AC(DEPIT);LS(DASH,2,CSTLN)")); break;
                                          case 5:
                                          case 3:
                                                default : obstrn04str.Append(_T(";AC(DEPVS);LS(DOTT,2,CHBLK)"));  break;
                                    }
                              }
                        }
                  }

                  obstrn04str.Append(*quapnt01str);
                  goto end;
            }     // area
      }

end:
    obstrn04str.Append('\037');

    char *r = (char *)malloc(obstrn04str.Len() + 1);
    strcpy(r, obstrn04str.mb_str());

    delete udwhaz03str;
    delete sndfrm02str;
    delete quapnt01str;

    return r;
}



static void *OWNSHP02(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;

        printf("s52csny : OWNSHP02 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

static void *PASTRK01(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;

        printf("s52csny : PASTRK01 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

static void *QUALIN01(void *param);
static void *QUAPNT01(void *param);

static void *QUAPOS01(void *param)
// Remarks: The attribute QUAPOS, which identifies low positional accuracy, is attached
// to the spatial object, not the feature object.
// In OpenCPN implementation, QUAPOS of Point Objects has been converted to
// QUALTY attribute of object.
//
// This procedure passes the object to procedure QUALIN01 or QUAPNT01,
// which traces back to the spatial object, retrieves any QUAPOS attributes,
// and returns the appropriate symbolization to QUAPOS01.
{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    wxString *q = NULL;

    if (GEO_LINE == obj->Primitive_type)
          q = CSQUALIN01(obj);

    else
          q = CSQUAPNT01(obj);

    char *r = (char *)malloc(q->Len() + 1);
    strcpy(r, q->mb_str());

    delete q;

    return r;

}

static void *QUALIN01(void *param)
// Remarks: The attribute QUAPOS, which identifies low positional accuracy, is attached
// only to the spatial component(s) of an object.
//
// A line object may be composed of more than one spatial object.
//
// This procedure looks at each of the spatial
// objects, and symbolizes the line according to the positional accuracy.
{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    wxString *q = CSQUALIN01(obj);
    char *r = (char *)malloc(q->Len() + 1);
    strcpy(r, q->mb_str());

    delete q;
    return r;
}

wxString *CSQUALIN01(S57Obj *obj)
// Remarks: The attribute QUAPOS, which identifies low positional accuracy, is attached
// only to the spatial component(s) of an object.
//
// A line object may be composed of more than one spatial object.
//
// This procedure looks at each of the spatial
// objects, and symbolizes the line according to the positional accuracy.
{
    wxString qualino1;
    int quapos = 0;
    bool bquapos = GetIntAttr(obj, "QUAPOS", quapos);
    const char *line = NULL;

    if (bquapos) {
        if ( 2 <= quapos && quapos < 10)
            line = "LC(LOWACC21)";
    } else {
        if (!strncmp("COALNE", obj->FeatureName, 6)) {
            int conrad;
            bool bconrad = GetIntAttr(obj, "CONRAD", conrad);

            if (bconrad) {
                if (1 == conrad)
                    line = "LS(SOLD,3,CHMGF);LS(SOLD,1,CSTLN)";
                else
                    line = "LS(SOLD,1,CSTLN)";
            } else
                line = "LS(SOLD,1,CSTLN)";

        } else  //LNDARE
            line = "LS(SOLD,1,CSTLN)";
    }

    if (NULL != line)
        qualino1.Append(wxString(line,  wxConvUTF8));

    qualino1.Append('\037');

    wxString *r = new wxString(qualino1);

/*    char *r = (char *)malloc(qualino1.Len() + 1);
    strcpy(r, qualino1.mb_str());
*/
    return r;
}



static void *QUAPNT01(void *param)
// Remarks: The attribute QUAPOS, which identifies low positional accuracy, is attached
// only to the spatial component(s) of an object.
//
// This procedure retrieves any QUALTY (ne QUAPOS) attributes, and returns the
// appropriate symbols to the calling procedure.

{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    wxString *q = CSQUAPNT01(obj);

    char *r = (char *)malloc(q->Len() + 1);
    strcpy(r, q->mb_str());

    return r;
}

wxString *CSQUAPNT01(S57Obj *obj)
// Remarks: The attribute QUAPOS, which identifies low positional accuracy, is attached
// only to the spatial component(s) of an object.
//
// This procedure retrieves any QUALTY (ne QUAPOS) attributes, and returns the
// appropriate symbols to the calling procedure.

{
    wxString quapnt01;
    int accurate  = TRUE;
    int qualty = 10;
    bool bquapos = GetIntAttr(obj, "QUALTY", qualty);

    if (bquapos) {
        if ( 2 <= qualty && qualty < 10)
            accurate = FALSE;
    }

    if (!accurate)
    {
          switch(qualty)
          {
          case 4:
                quapnt01.Append(_T(";SY(QUAPOS01)")); break;      // "PA"
          case 5:
                quapnt01.Append(_T(";SY(QUAPOS02)")); break;      // "PD"
          case 7:
          case 8:
                quapnt01.Append(_T(";SY(QUAPOS03)")); break;      // "REP"
          default:
                quapnt01.Append(_T(";SY(LOWACC03)")); break;      // "?"
          }
    }

    quapnt01.Append('\037');

    wxString *r = new wxString;

    *r = quapnt01;

/*    char *r = (char *)malloc(quapnt01.Len() + 1);
    strcpy(r, quapnt01.mb_str());
*/
    return r;
}

static void *SLCONS03(void *param)

    // Remarks: Shoreline construction objects which have a QUAPOS attribute on their
// spatial component indicating that their position is unreliable are symbolized
// by a special linestyle in the place of the varied linestyles normally used.
// Otherwise this procedure applies the normal symbolization.
{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;


    wxString slcons03;

    bool bvalstr;
    int ival;

    const char    *cmdw      = NULL;   // command word

    int quapos;
    bool bquapos = GetIntAttr(obj, "QUAPOS", quapos);


    if (GEO_POINT == obj->Primitive_type) {
        if (bquapos) {
            if (2 <= quapos && quapos < 10)
                cmdw ="SY(LOWACC01)";
        }
    } else {
        
        // This instruction not found in PLIB 3.4, but seems to appear in later PLIB implementations
        // by commercial ECDIS providers, so.....
        if (GEO_AREA == obj->Primitive_type) {
            slcons03 = _T("AP(CROSSX01);");
        }
            
        // GEO_LINE and GEO_AREA are the same
        if (bquapos) {
            if (2 <= quapos && quapos < 10)
                cmdw ="LC(LOWACC01)";
        } else {
            bvalstr = GetIntAttr(obj, "CONDTN", ival);

            if (bvalstr && ( 1 == ival || 2 == ival))
                cmdw = "LS(DASH,1,CSTLN)";
            else {
                ival = 0;
                bvalstr  = GetIntAttr(obj, "CATSLC", ival);

                if (bvalstr && ( 4 == ival || 6  == ival || 8  == ival || 15 == ival || 16 == ival ))
                    cmdw = "LS(SOLD,4,CSTLN)";
                else {
                    bvalstr = GetIntAttr(obj, "WATLEV", ival);

                    if (bvalstr && 2 == ival)
                        cmdw = "LS(SOLD,2,CSTLN)";
                    else
                        if (bvalstr && (3 == ival || 4 == ival))
                            cmdw = "LS(DASH,2,CSTLN)";
                        else
                            cmdw = "LS(SOLD,2,CSTLN)";  // default

                }
            }
        }
    }



    // WARNING: not explicitly specified in S-52 !!
    // FIXME:this is to put AC(DEPIT) --intertidal area
    // Could this be bug in OGR ?

    if (NULL != cmdw)
        slcons03.Append(wxString(cmdw,  wxConvUTF8));

    //      Match CM93 CMAPECS presentation?

    slcons03.Append('\037');

    char *r = (char *)malloc(slcons03.Len() + 1);
    strcpy(r, slcons03.mb_str());


    return r;


}

static void *RESARE02(void *param)
// Remarks: A list-type attribute is used because an area of the object class RESARE may
// have more than one category (CATREA). For example an inshore traffic
// zone might also have fishing and anchoring prohibition and a prohibited
// area might also be a bird sanctuary or a mine field.
//
// This conditional procedure is set up to ensure that the categories of most
// importance to safe navigation are prominently symbolized, and to pass on
// all given information with minimum clutter. Only the most significant
// restriction is symbolized, and an indication of further limitations is given by
// a subscript "!" or "I". Further details are given under conditional
// symbology procedure RESTRN01
//
// Other object classes affected by attribute RESTRN are handled by
// conditional symbology procedure RESTRN01.
{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;


    wxString resare02;

    wxString *restrnstr = GetStringAttrWXS(obj, "RESTRN");


    char     restrn[LISTSIZE] = {'\0'};
    wxString *catreastr = GetStringAttrWXS(obj, "CATREA");

    char     catrea[LISTSIZE] = {'\0'};
    wxString symb;
    wxString line;
    wxString prio;

    if (NULL != catreastr)
          _parseList(catreastr->mb_str(), catrea, sizeof(catrea));

    if ( NULL != restrnstr) {
          _parseList(restrnstr->mb_str(), restrn, sizeof(restrn));


        if (strpbrk(restrn, "\007\010\016")) {                          // entry restrictions
            // Continuation A
            if (strpbrk(restrn, "\001\002\003\004\005\006"))            // anchoring, fishing, trawling
                symb = _T(";SY(ENTRES61)");
            else {
                if (NULL != catreastr && strpbrk(catrea, "\001\010\011\014\016\023\025\031"))
                    symb = _T(";SY(ENTRES61)");
                else {
                    if (strpbrk(restrn, "\011\012\013\014\015"))
                        symb = _T(";SY(ENTRES71)");
                    else {
                        if (NULL != catreastr && strpbrk(catrea, "\004\005\006\007\012\022\024\026\027\030"))
                            symb = _T(";SY(ENTRES71)");
                        else
                            symb = _T(";SY(ENTRES51)");
                    }
                }
            }

            if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
                line = _T(";LC(RESARE51)");
            else
                line =_T( ";LS(DASH,2,CHMGD)");

            prio = _T(";OP(6---)");  // display prio set to 6

        } else {
            if (strpbrk(restrn, "\001\002")) {                          // anchoring
                // Continuation B
                if (strpbrk(restrn, "\003\004\005\006"))
                    symb = _T(";SY(ACHRES61)");
                else {
                    if (NULL != catreastr && strpbrk(catrea, "\001\010\011\014\016\023\025\031"))
                        symb = _T(";SY(ACHRES61)");
                    else {
                        if (strpbrk(restrn, "\011\012\013\014\015"))
                            symb = _T(";SY(ACHRES71)");
                        else {
                            if (NULL != catreastr && strpbrk(catrea, "\004\005\006\007\012\022\024\026\027\030"))
                                symb =_T( ";SY(ACHRES71)");
                            else
                                symb = _T(";SY(RESTRN51)");
                        }
                    }
                }

                if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
                    line = _T(";LC(RESARE51)");                // could be ACHRES51 when _drawLC is implemented fully
                else
                    line = _T(";LS(DASH,2,CHMGD)");

                prio = _T(";OP(6---)");  // display prio set to 6

            } else {
                if (strpbrk(restrn, "\003\004\005\006")) {              // fishing/trawling
                    // Continuation C
                    if (NULL != catreastr && strpbrk(catrea, "\001\010\011\014\016\023\025\031"))
                        symb = _T(";SY(FSHRES51)");
                    else {
                        if (strpbrk(restrn, "\011\012\013\014\015"))
                            symb = _T(";SY(FSHRES71)");
                        else{
                            if (NULL != catreastr && strpbrk(catrea, "\004\005\006\007\012\022\024\026\027\030"))
                                symb = _T(";SY(FSHRES71)");
                            else
                                symb = _T(";SY(FSHRES51)");
                        }
                    }

                    if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
                        line = _T(";LC(FSHRES51)");
                    else
                        line = _T(";LS(DASH,2,CHMGD)");

                    prio = _T(";OP(6---)");  // display prio set to 6

                } else {
                    if (strpbrk(restrn, "\011\012\013\014\015"))        // diving, dredging, waking...
                        symb = _T(";SY(INFARE51)");
                    else
                        symb = _T(";SY(RSRDEF51)");

                    if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
                        line = _T(";LC(CTYARE51)");
                    else
                        line = _T(";LS(DASH,2,CHMGD)");

                }
                //  Todo more for s57 3.1  Look at caris catalog ATTR::RESARE

            }
        }

    } else {
        // Continuation D
        if (NULL != catreastr) {
            if (strpbrk(catrea, "\001\010\011\014\016\023\025\031")) {
                if (strpbrk(catrea, "\004\005\006\007\012\022\024\026\027\030"))
                    symb = _T(";SY(CTYARE71)");
                else
                    symb = _T(";SY(CTYARE51)");
            } else {
                if (strpbrk(catrea, "\004\005\006\007\012\022\024\026\027\030"))
                    symb = _T(";SY(INFARE51)");
                else
                    symb = _T(";SY(RSRDEF51)");
            }
        } else
            symb = _T(";SY(RSRDEF51)");

        if (TRUE == S52_getMarinerParam(S52_MAR_SYMBOLIZED_BND))
            line = _T(";LC(CTYARE51)");
        else
            line = _T(";LS(DASH,2,CHMGD)");
    }

    // create command word
    if (prio.Len())
        resare02.Append(prio);
    resare02.Append(line);
    resare02.Append(symb);

    resare02.Append('\037');

    char *r = (char *)malloc(resare02.Len() + 1);
    strcpy(r, resare02.mb_str());

    delete restrnstr;
    delete catreastr;

    return r;
}








static void *_RESCSP01(void *param);
static void *RESTRN01 (void *param)
// Remarks: Objects subject to RESTRN01 are actually symbolised in sub-process
// RESCSP01, since the latter can also be accessed from other conditional
// symbology procedures. RESTRN01 merely acts as a "signpost" for
// RESCSP01.
//
// Object class RESARE is symbolised for the effect of attribute RESTRN in a separate
// conditional symbology procedure called RESARE02.
//
// Since many of the areas concerned cover shipping channels, the number of symbols used
// is minimised to reduce clutter. To do this, values of RESTRN are ranked for significance
// as follows:
// "Traffic Restriction" values of RESTRN:
// (1) RESTRN 7,8: entry prohibited or restricted
//     RESTRN 14: IMO designated "area to be avoided" part of a TSS
// (2) RESTRN 1,2: anchoring prohibited or restricted
// (3) RESTRN 3,4,5,6: fishing or trawling prohibited or restricted
// (4) "Other Restriction" values of RESTRN are:
//     RESTRN 9, 10: dredging prohibited or restricted,
//     RESTRN 11,12: diving prohibited or restricted,
//     RESTRN 13   : no wake area.
{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    wxString *restrnstr = GetStringAttrWXS(obj, "RESTRN");

    char *restrn01    = NULL;

    if (NULL != restrnstr)
        restrn01 = (char *)_RESCSP01(param);
    else
        restrn01 = NULL;

    delete restrnstr;
    return restrn01;
}

static void *_RESCSP01(void *param)
// Remarks: See procedure RESTRN01
{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    wxString rescsp01;
    wxString *restrnstr = GetStringAttrWXS(obj, "RESTRN");
    char     restrn[LISTSIZE] = {'\0'};   // restriction list
    wxString symb;
    char    *r = NULL;

    if ( restrnstr->Len()) {
          _parseList(restrnstr->mb_str(), restrn, sizeof(restrn));

        if (strpbrk(restrn, "\007\010\016")) {
            // continuation A
            if (strpbrk(restrn, "\001\002\003\004\005\006"))
                symb = _T(";SY(ENTRES61)");
            else {
                if (strpbrk(restrn, "\011\012\013\014\015"))
                    symb = _T(";SY(ENTRES71)");
                else
                    symb = _T(";SY(ENTRES51)");

            }
        } else {
            if (strpbrk(restrn, "\001\002")) {
                // continuation B
                if (strpbrk(restrn, "\003\004\005\006"))
                    symb = _T(";SY(ACHRES61)");
                else {
                    if (strpbrk(restrn, "\011\012\013\014\015"))
                        symb =_T( ";SY(ACHRES71)");
                    else
                        symb = _T(";SY(ACHRES51)");
                }


            } else {
                if (strpbrk(restrn, "\003\004\005\006")) {
                    // continuation C
                    if (strpbrk(restrn, "\011\012\013\014\015"))
                        symb = _T(";SY(FSHRES71)");
                    else
                        symb = _T(";SY(FSHRES51)");


                } else {
                    if (strpbrk(restrn, "\011\012\013\014\015"))
                        symb = _T(";SY(INFARE51)");
                    else
                        symb = _T(";SY(RSRDEF51)");

                }
            }
        }

        rescsp01.Append(symb);
        rescsp01.Append('\037');

        r = (char *)malloc(rescsp01.Len() + 1);
        strcpy(r, rescsp01.mb_str());

        delete restrnstr;
    }

    return r;
}

static void *SEABED01(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        CPLError((CPLErr)0, 0,"s52csny : SEABED01 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

/*
static void *SNDFRM02(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        CPLError((CPLErr)0, 0,"s52csny : SNDFRM02 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}
*/

wxString *SNDFRM02(S57Obj *obj, double depth_value);

static void *SOUNDG02(void *param)
// Remarks: In S-57 soundings are elements of sounding arrays rather than individual
// objects. Thus the conditional symbology methodology must examine each
// sounding of a sounding array one by one. To symbolize the depth values it
// calls the procedure SNDFRM02 which in turn translates the depth values
// into a set of symbols to be shown at the soundings position.
{
    // Shortcut.  This CS method causes a branch to an S52plib method
    // which splits multi-point soundings into separate point objects,
    // and then calls CS(SOUNDG03) on successive points below.
      char *r = (char *)malloc(6);
      strcpy(r, "MP();");

      return r;

}


static void *SOUNDG03(void *param)
// Remarks:  SOUNDG03 is a private conditional symbology,
// called to render individual points of a multi-point sounding set.
{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    wxString *s = SNDFRM02(obj, obj->z);

    char *r = (char *)malloc(s->Len() + 1);
    strcpy(r, s->mb_str());

    delete s;
    return r;
}



wxString *SNDFRM02(S57Obj *obj, double depth_value_in)
// Remarks: Soundings differ from plain text because they have to be readable under all
// circumstances and their digits are placed according to special rules. This
// conditional symbology procedure accesses a set of carefully designed
// sounding symbols provided by the symbol library and composes them to
// sounding labels. It symbolizes swept depth and it also symbolizes for low
// reliability as indicated by attributes QUASOU and QUAPOS.
{
    wxString sndfrm02;
    char     temp_str[LISTSIZE] = {'\0'};

    wxString symbol_prefix;

    char symbol_prefix_a[200];

    wxString *tecsoustr = GetStringAttrWXS(obj, "TECSOU");
    char     tecsou[LISTSIZE] = {'\0'};

    wxString *quasoustr = GetStringAttrWXS(obj, "QUASOU");
    char     quasou[LISTSIZE] = {'\0'};

    wxString *statusstr = GetStringAttrWXS(obj, "STATUS");
    char     status[LISTSIZE] = {'\0'};

    double   leading_digit    = 0.0;

    double safety_depth = S52_getMarinerParam(S52_MAR_SAFETY_DEPTH);

    //      Do the math to convert soundings to ft/metres/fathoms on request
    double depth_value = depth_value_in;

    //      If the sounding value from the ENC is bogus, so state
    if(depth_value_in > 40000.)
      depth_value = 99999.;

    switch(ps52plib->m_nDepthUnitDisplay)
    {
          case 0:
                depth_value = depth_value   * 3 * 39.37 / 36;              // feet
                safety_depth = safety_depth * 3 * 39.37 / 36;
                break;
          case 2:
                depth_value = depth_value   * 3 * 39.37 / (36 * 6);        // fathoms
                safety_depth = safety_depth * 3 * 39.37 / (36 * 6);
                break;
          default:
                break;
    }


    // FIXME: test to fix the rounding error (!?)
    depth_value  += (depth_value > 0.0)? 0.01: -0.01;
    leading_digit = (int) depth_value;

    if (depth_value <= safety_depth)            //S52_getMarinerParam(S52_MAR_SAFETY_DEPTH)
        symbol_prefix = _T("SOUNDS");
    else
        symbol_prefix = _T("SOUNDG");

    strcpy(symbol_prefix_a,symbol_prefix.mb_str());

    if (NULL != tecsoustr)
    {
          _parseList(tecsoustr->mb_str(), tecsou, sizeof(tecsou));
        if (strpbrk(tecsou, "\006"))
        {
            sprintf(temp_str, ";SY(%sB1)", symbol_prefix_a);
            sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        }
    }

    if (NULL != quasoustr) _parseList(quasoustr->mb_str(), quasou, sizeof(quasou));
    if (NULL != statusstr) _parseList(statusstr->mb_str(), status, sizeof(status));

    if (strpbrk(quasou, "\003\004\005\010\011") || strpbrk(status, "\022"))
    {
        sprintf(temp_str, ";SY(%sC2)", symbol_prefix_a);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));

    }
    else
    {
        wxString *quaposstr = GetStringAttrWXS(obj, "QUAPOS");
        int quapos = (NULL == quaposstr)? 0 : atoi(quaposstr->mb_str());

        if (0 != quapos)
        {
            if (2 <= quapos && quapos < 10)
            {
                sprintf(temp_str, ";SY(%sC2)", symbol_prefix_a);
                sndfrm02.Append(wxString(temp_str, wxConvUTF8));
            }
        }
        delete quaposstr;
    }

    // Continuation A
    if (depth_value < 10.0) {
        // can be above water (negative)
        int fraction = (int)ABS((depth_value - leading_digit)*10);


        sprintf(temp_str, ";SY(%s1%1i)", symbol_prefix_a, (int)ABS(leading_digit));
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s5%1i)", symbol_prefix_a, fraction);
        if(fraction > 0)
            sndfrm02.Append(wxString(temp_str, wxConvUTF8));

        // above sea level (negative)
        if (depth_value < 0.0)
        {
            sprintf(temp_str, ";SY(%sA1)", symbol_prefix_a);
            sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        }
        goto return_point;
    }

    if (depth_value < 31.0) {
        double fraction = depth_value - floor(leading_digit);

        if (fraction != 0.0) {
            fraction = fraction * 10;
            if (leading_digit >= 10.0)
            {
                sprintf(temp_str, ";SY(%s2%1i)", symbol_prefix_a, (int)leading_digit/10);
                sndfrm02.Append(wxString(temp_str, wxConvUTF8));
            }

            double first_digit = floor(leading_digit / 10);
            int secnd_digit = (int)(floor(leading_digit - (first_digit * 10)));
            sprintf(temp_str, ";SY(%s1%1i)", symbol_prefix_a, secnd_digit/*(int)leading_digit*/);
            sndfrm02.Append(wxString(temp_str, wxConvUTF8));
            sprintf(temp_str, ";SY(%s5%1i)", symbol_prefix_a, (int)fraction);
            if((int)fraction > 0)
                sndfrm02.Append(wxString(temp_str, wxConvUTF8));

            goto return_point;
        }
    }

    // Continuation B
    depth_value = leading_digit;    // truncate to integer
    if (depth_value < 100.0)
    {
        double first_digit = floor(leading_digit / 10);
        double secnd_digit = floor(leading_digit - (first_digit * 10));

        sprintf(temp_str, ";SY(%s1%1i)", symbol_prefix_a, (int)first_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s0%1i)", symbol_prefix_a, (int)secnd_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));

        goto return_point;
    }

    if (depth_value < 1000.0)
    {
        double first_digit = floor(leading_digit / 100);
        double secnd_digit = floor((leading_digit - (first_digit * 100)) / 10);
        double third_digit = floor(leading_digit - (first_digit * 100) - (secnd_digit * 10));

        sprintf(temp_str, ";SY(%s2%1i)", symbol_prefix_a, (int)first_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s1%1i)", symbol_prefix_a, (int)secnd_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s0%1i)", symbol_prefix_a, (int)third_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));

        goto return_point;
    }

    if (depth_value < 10000.0)
    {
        double first_digit = floor(leading_digit / 1000);
        double secnd_digit = floor((leading_digit - (first_digit * 1000)) / 100);
        double third_digit = floor((leading_digit - (first_digit * 1000) - (secnd_digit * 100)) / 10);
        double last_digit  = floor(leading_digit - (first_digit * 1000) - (secnd_digit * 100) - (third_digit * 10)) ;

        sprintf(temp_str, ";SY(%s2%1i)", symbol_prefix_a, (int)first_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s1%1i)", symbol_prefix_a, (int)secnd_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s0%1i)", symbol_prefix_a, (int)third_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s4%1i)", symbol_prefix_a, (int)last_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));

        goto return_point;
    }

    // Continuation C
    {
        double first_digit  = floor(leading_digit / 10000);
        double secnd_digit  = floor((leading_digit - (first_digit * 10000)) / 1000);
        double third_digit  = floor((leading_digit - (first_digit * 10000) - (secnd_digit * 1000)) / 100 );
        double fourth_digit = floor((leading_digit - (first_digit * 10000) - (secnd_digit * 1000) - (third_digit * 100)) / 10 ) ;
        double last_digit   = floor(leading_digit - (first_digit * 10000) - (secnd_digit * 1000) - (third_digit * 100) - (fourth_digit * 10)) ;

        sprintf(temp_str, ";SY(%s3%1i)", symbol_prefix_a, (int)first_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s2%1i)", symbol_prefix_a, (int)secnd_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s1%1i)", symbol_prefix_a, (int)third_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s0%1i)", symbol_prefix_a, (int)fourth_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));
        sprintf(temp_str, ";SY(%s4%1i)", symbol_prefix_a, (int)last_digit);
        sndfrm02.Append(wxString(temp_str, wxConvUTF8));

        goto return_point;
    }

return_point:
        sndfrm02.Append('\037');

        wxString *r = new wxString(sndfrm02);

/*        char *r = (char *)malloc(sndfrm02.Len() + 1);
        strcpy(r, sndfrm02.mb_str());
*/
        delete tecsoustr;
        delete quasoustr;
        delete statusstr;

        return r;
}


static void *TOPMAR01 (void *param)
// Remarks: Topmark objects are to be symbolized through consideration of their
// platforms e.g. a buoy. Therefore this conditional symbology procedure
// searches for platforms by looking for other objects that are located at the
// same position.. Based on the finding whether the platform is rigid or
// floating, the respective upright or sloping symbol is selected and presented
// at the objects location. Buoyf symbols and topmark symbols have been
// carefully designed to fit to each other when combined at the same position.
// The result is a composed symbol that looks like the traditional symbols the
// mariner is used to.
{
    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    int top_int = 0;
    bool battr = GetIntAttr(obj, "TOPSHP", top_int);

    wxString sy;

    if (!battr)
        sy = _T(";SY(QUESMRK1)");
    else {
        int floating    = FALSE; // not a floating platform
        int topshp      = (!battr) ? 0 : top_int;


        if (TRUE == _atPtPos(obj, rzRules->chart->pFloatingATONArray, false))
            floating = TRUE;
        else
            // FIXME: this test is wierd since it doesn't affect 'floating'
            if (TRUE == _atPtPos(obj, rzRules->chart->pRigidATONArray, false))
                floating = FALSE;


        if (floating) {
            // floating platform
            switch (topshp) {
                case 1 : sy = _T(";SY(TOPMAR02)"); break;
                case 2 : sy = _T(";SY(TOPMAR04)"); break;
                case 3 : sy = _T(";SY(TOPMAR10)"); break;
                case 4 : sy = _T(";SY(TOPMAR12)"); break;

                case 5 : sy = _T(";SY(TOPMAR13)"); break;
                case 6 : sy = _T(";SY(TOPMAR14)"); break;
                case 7 : sy = _T(";SY(TOPMAR65)"); break;
                case 8 : sy = _T(";SY(TOPMAR17)"); break;

                case 9 : sy = _T(";SY(TOPMAR16)"); break;
                case 10: sy = _T(";SY(TOPMAR08)"); break;
                case 11: sy = _T(";SY(TOPMAR07)"); break;
                case 12: sy = _T(";SY(TOPMAR14)"); break;

                case 13: sy = _T(";SY(TOPMAR05)"); break;
                case 14: sy = _T(";SY(TOPMAR06)"); break;
                case 17: sy = _T(";SY(TMARDEF2)"); break;
                case 18: sy = _T(";SY(TOPMAR10)"); break;

                case 19: sy = _T(";SY(TOPMAR13)"); break;
                case 20: sy = _T(";SY(TOPMAR14)"); break;
                case 21: sy = _T(";SY(TOPMAR13)"); break;
                case 22: sy = _T(";SY(TOPMAR14)"); break;

                case 23: sy = _T(";SY(TOPMAR14)"); break;
                case 24: sy = _T(";SY(TOPMAR02)"); break;
                case 25: sy = _T(";SY(TOPMAR04)"); break;
                case 26: sy = _T(";SY(TOPMAR10)"); break;

                case 27: sy = _T(";SY(TOPMAR17)"); break;
                case 28: sy = _T(";SY(TOPMAR18)"); break;
                case 29: sy = _T(";SY(TOPMAR02)"); break;
                case 30: sy = _T(";SY(TOPMAR17)"); break;

                case 31: sy = _T(";SY(TOPMAR14)"); break;
                case 32: sy = _T(";SY(TOPMAR10)"); break;
                case 33: sy = _T(";SY(TMARDEF2)"); break;
                default: sy = _T(";SY(TMARDEF2)"); break;
            }
        } else {
            // not a floating platform
            switch (topshp) {
                case 1 : sy = _T(";SY(TOPMAR22)"); break;
                case 2 : sy = _T(";SY(TOPMAR24)"); break;
                case 3 : sy = _T(";SY(TOPMAR30)"); break;
                case 4 : sy = _T(";SY(TOPMAR32)"); break;

                case 5 : sy = _T(";SY(TOPMAR33)"); break;
                case 6 : sy = _T(";SY(TOPMAR34)"); break;
                case 7 : sy = _T(";SY(TOPMAR85)"); break;
                case 8 : sy = _T(";SY(TOPMAR86)"); break;

                case 9 : sy = _T(";SY(TOPMAR36)"); break;
                case 10: sy = _T(";SY(TOPMAR28)"); break;
                case 11: sy = _T(";SY(TOPMAR27)"); break;
                case 12: sy = _T(";SY(TOPMAR14)"); break;

                case 13: sy = _T(";SY(TOPMAR25)"); break;
                case 14: sy = _T(";SY(TOPMAR26)"); break;
                case 15: sy = _T(";SY(TOPMAR88)"); break;
                case 16: sy = _T(";SY(TOPMAR87)"); break;

                case 17: sy = _T(";SY(TMARDEF1)"); break;
                case 18: sy = _T(";SY(TOPMAR30)"); break;
                case 19: sy = _T(";SY(TOPMAR33)"); break;
                case 20: sy = _T(";SY(TOPMAR34)"); break;

                case 21: sy = _T(";SY(TOPMAR33)"); break;
                case 22: sy = _T(";SY(TOPMAR34)"); break;
                case 23: sy = _T(";SY(TOPMAR34)"); break;
                case 24: sy = _T(";SY(TOPMAR22)"); break;

                case 25: sy = _T(";SY(TOPMAR24)"); break;
                case 26: sy = _T(";SY(TOPMAR30)"); break;
                case 27: sy = _T(";SY(TOPMAR86)"); break;
                case 28: sy = _T(";SY(TOPMAR89)"); break;

                case 29: sy = _T(";SY(TOPMAR22)"); break;
                case 30: sy = _T(";SY(TOPMAR86)"); break;
                case 31: sy = _T(";SY(TOPMAR14)"); break;
                case 32: sy = _T(";SY(TOPMAR30)"); break;
                case 33: sy = _T(";SY(TMARDEF1)"); break;
                default: sy = _T(";SY(TMARDEF1)"); break;
            }
        }

    }

    wxString topmar;
    topmar.Append(sy);
    topmar.Append('\037');

    char *r = (char *)malloc(topmar.Len() + 1);
    strcpy(r, topmar.mb_str());

    return r;
}


static void *UDWHAZ03(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        CPLError((CPLErr)0, 0,"s52csny : UDWHAZ03 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

static void *VESSEL01(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        CPLError((CPLErr)0, 0,"s52csny : VESSEL01 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

static void *VRMEBL01(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        CPLError((CPLErr)0, 0,"s52csny : VRMEBL01 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
   return NULL;
}

/*
static void *WRECKS02a(void *param)
{
        ObjRazRules *rzRules = (ObjRazRules *)param;
//      S57Obj *obj = rzRules->obj;

        static int f07;
        if(!f07)
                CPLError((CPLErr)0, 0,"s52csny : WRECKS02 ERROR no conditional symbology for: %s\n",rzRules->LUP->OBCL);
        f07++;
   return NULL;
}
*/

static void *WRECKS02 (void *param)
// Remarks: Wrecks of depths less than the safety contour which lie within the safe waters
// defined by the safety contour are to be presented by a specific isolated
// danger symbol and put in IMO category DISPLAYBASE (see (3), App.2,
// 1.3). This task is performed by the sub-procedure "UDWHAZ03" which is
// called by this symbology procedure.
{
    wxString wrecks02str;
    wxString *sndfrm02str = NULL;
    wxString *udwhaz03str = NULL;
    wxString *quapnt01str = NULL;
    double   least_depth = UNKNOWN;
    double   depth_value = UNKNOWN;
    double   valsou      = UNKNOWN;

    ObjRazRules *rzRules = (ObjRazRules *)param;
    S57Obj *obj = rzRules->obj;

    GetDoubleAttr(obj, "VALSOU", valsou);

    int watlev = -9;
    GetIntAttr(obj, "WATLEV", watlev);
    int catwrk = -9;
    GetIntAttr(obj, "CATWRK", catwrk);
	int quasou = -9;
    GetIntAttr(obj, "QUASOU", quasou);

    double safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);

    if (UNKNOWN != valsou)
    {
        depth_value = valsou;
        sndfrm02str = SNDFRM02(obj, depth_value);
    }
    else
    {
        if (GEO_AREA == obj->Primitive_type)
            least_depth = _DEPVAL01(obj, least_depth);

        if (least_depth == UNKNOWN)
/*
        {
            // WARNING: ambiguity removed in WRECKS03 (see update)

            if (-9 == watlev) // default
                depth_value = -15.0;
            else
                switch (watlev)
                  { // ambiguous
                    case 1:
                    case 2: depth_value = -15.0 ; break;
                    case 3: depth_value =   0.01; break;
                    case 4: depth_value = -15.0 ; break;
                    case 5: depth_value =   0.0 ; break;
                    case 6: depth_value = -15.0 ; break;
                    default :
                          {
                              if (-9 != catwrk)
                              {
                                    switch (catwrk)
                                    {
                                          case 1: depth_value =  20.0; break;
                                          case 2: depth_value =   0.0; break;
                                          case 4:
                                          case 5: depth_value = -15.0; break;
                                    }
                               }
                          }
                  }
        }
*/
////////////////////////////////////////////////
//    DSR New Logic Here  (FIXME)
        {
           if (-9 != catwrk)
           {
                  switch (catwrk)
                  {
                        case 1: depth_value =  20.0; break;       // safe
                        case 2: depth_value =   0.0; break;       // dangerous
                        case 4:
                        case 5: depth_value = -15.0; break;
                  }
            }
            else
            {
                if (-9 == watlev) // default
                      depth_value = -15.0;
                else
                  switch (watlev)
                  {
                    case 1:
                    case 2: depth_value = -15.0 ; break;
                    case 3: depth_value =   0.01; break;
                    case 4: depth_value = -15.0 ; break;
                    case 5: depth_value =   0.0 ; break;
                    case 6: depth_value = -15.0 ; break;
                  }
            }

        }

        else
            depth_value = least_depth;


    }
	if (7 != quasou) //Fixes FS 165
		udwhaz03str = _UDWHAZ03(obj, depth_value, rzRules);
	else
		udwhaz03str = new wxString();
    quapnt01str = CSQUAPNT01(obj);

    if (GEO_POINT == obj->Primitive_type) {
          if (0 != udwhaz03str->Len()) {
            wrecks02str = wxString(*udwhaz03str);

          wrecks02str.Append(*quapnt01str);

        } else {
            // Continuation A (POINT_T)
            if (UNKNOWN != valsou) {
///////////////////////////////////////////
//    DSR New logic here, FIXME check s52 specs

/*
                if (valsou <= 20.0)
                {
                    wrecks02str = wxString(";SY(DANGER51)");
                    if (NULL != sndfrm02str)
                        wrecks02str.Append(sndfrm02str);
                }
                else
                    wrecks02str = wxString(";SY(DANGER52)");
*/
                if((valsou < safety_contour)/* || (2 == catwrk)*/)    // maybe redundant, seems like wrecks with valsou < 20
                                                                  // are always coded as "dangerous wrecks"
                                                                  // Excluding (2 == catwrk) matches Caris logic
                      wrecks02str = wxString(_T(";SY(DANGER51)"));
                else
                      wrecks02str = wxString(_T(";SY(DANGER52)"));
				wrecks02str.Append(_T(";TX('Wk',2,1,2,'15110',1,0,CHBLK,21)"));
				if ( 7 == quasou ) //Fixes FS 165
					wrecks02str.Append(_T(";SY(WRECKS07)"));

                if (NULL != sndfrm02str)                          // always show valsou depth
                        wrecks02str.Append(*sndfrm02str);
///////////////////////////////////////////

                wrecks02str.Append(*udwhaz03str);
                wrecks02str.Append(*quapnt01str);

            } else {
                wxString sym;

                if (-9 != catwrk && -9 != watlev) {
                    if (1 == catwrk && 3 == watlev)
                          sym =_T(";SY(WRECKS04)");
                    else {
                        if (2 == catwrk && 3 == watlev)
                              sym = _T(";SY(WRECKS05)");
                        else {
                            if (4 == catwrk || 5 == catwrk)
                                  sym = _T(";SY(WRECKS01)");
                            else {
                                if (1 == watlev ||
                                    2 == watlev ||
                                    5 == watlev ||
                                    4 == watlev ){
                                      sym = _T(";SY(WRECKS01)");
                                } else
                                      sym = _T(";SY(WRECKS05)"); // default

                            }
                        }
                    }
                }

                wrecks02str = sym;
                if (NULL != quapnt01str)
                    wrecks02str.Append(*quapnt01str);

            }

        }


    } else {
        // Continuation B (AREAS_T)
        int quapos = 0;
        GetIntAttr(obj, "QUAPOS", quapos);

        wxString line;

        if (2 <= quapos && quapos < 10)
              line = _T(";LC(LOWACC41)");
        else {
              if ( 0 != udwhaz03str->Len())
                  line = _T(";LS(DOTT,2,CHBLK)");
            else {
                 if (UNKNOWN != valsou){
                     if (valsou <= 20)
                           line = _T(";LS(DOTT,2,CHBLK)");
                     else
                           line = _T(";LS(DASH,2,CHBLK)");
                 } else {

                     if (-9 == watlev)
                           line = _T(";LS(DOTT,2,CSTLN)");
                     else {
                         switch (watlev){
                             case 1:
                             case 2: line = _T(";LS(SOLD,2,CSTLN)"); break;
                             case 4: line = _T(";LS(DASH,2,CSTLN)"); break;
                             case 3:
                             case 5:

                             default : line = _T(";LS(DOTT,2,CSTLN)"); break;
                         }
                     }

                 }
            }
        }
        wrecks02str = wxString(line);

        if (UNKNOWN != valsou) {
            if (valsou <= 20) {
                    wrecks02str.Append(*udwhaz03str);
                    wrecks02str.Append(*quapnt01str);
                    wrecks02str.Append(*sndfrm02str);

            } else {
                // NOTE: ??? same as above ???
                    wrecks02str.Append(*udwhaz03str);
                    wrecks02str.Append(*quapnt01str);
            }
        } else {
            wxString ac;

            if (-9 == watlev)
                  ac = _T(";AC(DEPVS)");
            else
                switch (watlev) {
                          case 1:
                          case 2: ac = _T(";AC(CHBRN)"); break;
                          case 4: ac = _T(";AC(DEPIT)"); break;
                          case 5:
                          case 3:
                          default : ac = _T(";AC(DEPVS)"); break;
                }

            wrecks02str.Append(ac);

            wrecks02str.Append(*udwhaz03str);
            wrecks02str.Append(*quapnt01str);
        }
    }

    wrecks02str.Append('\037');

    char *r = (char *)malloc(wrecks02str.Len() + 1);
    strcpy(r, wrecks02str.mb_str());

    delete sndfrm02str;
    delete udwhaz03str;
    delete quapnt01str;

    return r;
}


static wxString _LITDSN01(S57Obj *obj)
// Remarks: In S-57 the light characteristics are held as a series of attributes values. The
// mariner may wish to see a light description text string displayed on the
// screen similar to the string commonly found on a paper chart. This
// conditional procedure, reads the attribute values from the above list of
// attributes and composes a light description string which can be displayed.
// This procedure is provided as a C function which has as input, the above
// listed attribute values and as output, the light description.
{
      // CATLIT, LITCHR, COLOUR, HEIGHT, LITCHR, SIGGRP, SIGPER, STATUS, VALNMR

      char colist[20];
      wxString return_value;

      // CATLIT
      int catlit = -9;
      GetIntAttr(obj, "CATLIT", catlit);

      if(-9 != catlit)
      {
      }


    /*
      1: directional function  IP 30.1-3;  475.7;
      2: rear/upper light
      3: front/lower light
      4: leading light           IP 20.1-3;      475.6;
      5: aero light                  IP 60;      476.1;
      6: air obstruction light IP 61;      476.2;
      7: fog detector light        IP 62;  477;
      8: flood light                 IP 63;      478.2;
      9: strip light                 IP 64;      478.5;
      10: subsidiary light          IP 42;  471.8;
      11: spotlight
      12: front
      13: rear
      14: lower
      15: upper
      16: moire' effect           IP 31;    475.8;
      17: emergency
      18: bearing light                   478.1;
      19: horizontally disposed
      20: vertically disposed
    */

    // LITCHR
      int litchr = -9;
      wxString spost(_T(""));
      GetIntAttr(obj, "LITCHR", litchr);

      bool b_grp2 = false;                      // 2 GRP attributes expected
      if(-9 != litchr)
      {
            switch (litchr)
            {
/*
                  case 1:   return_value.Append(_T("F"));    break;
                  case 2:   return_value.Append(_T("Fl"));   break;
                  case 3:   return_value.Append(_T("Fl"));   break;
                  case 4:   return_value.Append(_T("Q"));    break;
                  case 7:   return_value.Append(_T("Iso"));  break;
                  case 8:   return_value.Append(_T("Occ"));  break;
                  case 12:  return_value.Append(_T("Mo"));   break;
*/

                  case 1: return_value.Append(_T("F"));    break;                   //fixed     IP 10.1;
                  case 2: return_value.Append(_T("Fl"));   break;                   //flashing  IP 10.4;
                  case 3: return_value.Append(_T("LFl"));  break;                   //long-flashing   IP 10.5;
                  case 4: return_value.Append(_T("Q"));    break;                   //quick-flashing  IP 10.6;
                  case 5: return_value.Append(_T("VQ"));   break;                   //very quick-flashing   IP 10.7;
                  case 6: return_value.Append(_T("UQ"));   break;                   //ultra quick-flashing  IP 10.8;
                  case 7: return_value.Append(_T("Iso"));  break;                   //isophased IP 10.3;
                  case 8: return_value.Append(_T("Occ"));  break;                   //occulting IP 10.2;
                  case 9: return_value.Append(_T("IQ"));   break;                   //interrupted quick-flashing  IP 10.6;
                  case 10: return_value.Append(_T("IVQ")); break;                   //interrupted very quick-flashing   IP 10.7;
                  case 11: return_value.Append(_T("IUQ")); break;                   //interrupted ultra quick-flashing  IP 10.8;
                  case 12: return_value.Append(_T("Mo"));  break;                   //morse     IP 10.9;
                  case 13: return_value.Append(_T("F + Fl"));   b_grp2 = true; break;                   //fixed/flash     IP 10.10;
                  case 14: return_value.Append(_T("Fl + LFl")); b_grp2 = true; break;                   //flash/long-flash
                  case 15: return_value.Append(_T("Occ + Fl")); b_grp2 = true; break;                   //occulting/flash
                  case 16: return_value.Append(_T("F + LFl"));  b_grp2 = true;  break;                   //fixed/long-flash
                  case 17: return_value.Append(_T("Al Occ"));    break;                   //occulting alternating
                  case 18: return_value.Append(_T("Al LFl"));    break;                   //long-flash alternating
                  case 19: return_value.Append(_T("Al Fl"));    break;                   //flash alternating
                  case 20: return_value.Append(_T("Al Grp"));    break;                   //group alternating
                  case 21: return_value.Append(_T("F")); spost = _T(" (vert)");    break;                   //2 fixed (vertical)
                  case 22: return_value.Append(_T("F")); spost = _T(" (horz)");    break;                   //2 fixed (horizontal)
                  case 23: return_value.Append(_T("F")); spost = _T(" (vert)");    break;                   //3 fixed (vertical)
                  case 24: return_value.Append(_T("F")); spost = _T(" (horz)");    break;                   //3 fixed (horizontal)
                  case 25: return_value.Append(_T("Q + LFl"));  b_grp2 = true;    break;                   //quick-flash plus long-flash
                  case 26: return_value.Append(_T("VQ + LFl")); b_grp2 = true;    break;                   //very quick-flash plus long-flash
                  case 27: return_value.Append(_T("UQ + LFl")); b_grp2 = true;    break;                   //ultra quick-flash plus long-flash
                  case 28: return_value.Append(_T("Alt"));                        break;                   //alternating
                  case 29: return_value.Append(_T("F + Alt")); b_grp2 = true;     break;                   //fixed and alternating flashing

                  default: break;
            }
      }

      int nfirst_grp = -1;
      if(b_grp2)
      {
            wxString ret_new;
            nfirst_grp = return_value.Find(_T(" "));
            if( wxNOT_FOUND != nfirst_grp)
            {
                  ret_new = return_value.Mid(0, nfirst_grp);
                  ret_new.Append(_T("(?)"));
                  ret_new.Append(return_value.Mid(nfirst_grp));
                  return_value = ret_new;
                  nfirst_grp += 1;
            }
      }



     // SIGGRP, (c)(c) ...
      char grp_str[20] = {'\0'};
      GetStringAttr(obj, "SIGGRP", grp_str, 19);
      if(strlen(grp_str))
      {
            wxString ss(grp_str, wxConvUTF8);

            if(b_grp2)
            {
                  wxStringTokenizer tkz(ss, _T("()"));

                  int n_tok = 0;
                  while ( tkz.HasMoreTokens() && (n_tok <2))
                  {
                        wxString s = tkz.GetNextToken();
                        if(s.Len())
                        {
                              if((n_tok == 0) && (nfirst_grp > 0))
                              {
                                    return_value[nfirst_grp] = s[0];
                              }
                              else
                              {
                                    if(s != _T("1"))
                                    {
                                          return_value.Append(_T("("));
                                          return_value.Append(s);
                                          return_value.Append(_T(")"));
                                    }
                              }

                              n_tok++;
                        }
                  }
            }
            else
            {
                  if(ss != _T("(1)"))
                        return_value.Append(ss);
            }
      }

      // COLOUR,
      char col_str[20] = {'\0'};

      // Don't show for sectored lights since we are only showing one of the sectors.
      double sectrTest;
      bool hasSectors = GetDoubleAttr( obj, "SECTR1", sectrTest );

      if( ! hasSectors ) {
            GetStringAttr(obj, "COLOUR", col_str, 19);

            int n_cols = 0;
            if (strlen(col_str))
                  n_cols = _parseList(col_str, colist, sizeof(colist));

            if(n_cols)
                  return_value.Append(_T(" "));

            for(int i=0 ; i < n_cols ; i++)
            {
                  switch (colist[i])
                  {
                        case 1:  return_value.Append(_T("W")); break;
                        case 3:  return_value.Append(_T("R")); break;
                        case 4:  return_value.Append(_T("G")); break;
                        case 6:  return_value.Append(_T("Y")); break;
                        default:  break;
                  }
            }
      }



    /*
      1: white     IP 11.1;    450.2-3;
      2: black
      3: red IP 11.2;    450.2-3;
      4: green     IP 11.3;    450.2-3;
      5: blue      IP 11.4;    450.2-3;
      6: yellow    IP 11.6;    450.2-3;
      7: grey
      8: brown
      9: amber     IP 11.8;    450.2-3;
      10: violet    IP 11.5;    450.2-3;
      11: orange    IP 11.7;    450.2-3;
      12: magenta
      13: pink
    */

    // SIGPER, xx.xx
      double   sigper      = UNKNOWN;
      GetDoubleAttr(obj, "SIGPER", sigper);

      if(UNKNOWN != sigper)
      {
            wxString s;
            if(fabs(wxRound(sigper) - sigper) > 0.01)
                  s.Printf(_T("%4.1fs"), sigper);
            else
                  s.Printf(_T("%2.0fs"), sigper);

            s.Trim(false);          // remove leading spaces
            s.Prepend(_T(" "));
            return_value.Append(s);
      }


    // HEIGHT, xxx.x
      double   height      = UNKNOWN;
      GetDoubleAttr(obj, "HEIGHT", height);

      if(UNKNOWN != height)
      {
            wxString s;
            switch(ps52plib->m_nDepthUnitDisplay)
            {
                  case 0:                       // feet
                  case 2:                       // fathoms
                        s.Printf(_T("%3.0fft"), height* 3 * 39.37 / 36);
                        break;
                  default:
                        s.Printf(_T("%3.0fm"), height);
                        break;
            }

            s.Trim(false);          // remove leading spaces
            s.Prepend(_T(" "));
            return_value.Append(s);
      }


    // VALNMR, xx.x
      double   valnmr      = UNKNOWN;
      GetDoubleAttr(obj, "VALNMR", valnmr);

      if( UNKNOWN != valnmr && ! hasSectors )
      {
            wxString s;
            s.Printf(_T("%2.0fNm"), valnmr);
            s.Trim(false);          // remove leading spaces
            s.Prepend(_T(" "));
            return_value.Append(s);
      }

#if 0

    // STATUS,
      gstr = S57_getAttVal(geo, "STATUS");
      if (NULL != gstr)
            g_string_append(litdsn01, gstr->str);

    /*
      1: permanent
      2: occasional      IP 50;      473.2;
      3: recommended     IN 10;      431.1;
      4: not in use      IL 14, 44;  444.7;
      5: periodic/intermittent IC 21; IQ 71;     353.3; 460.5;
      6: reserved  IN 12.9;
      7: temporary IP 54;
      8: private   IQ 70;
      9: mandatory
      10: destroyed/ruined
      11: extinguished
      12: illuminated
      13: historic
      14: public
      15: synchronized
      16: watched
      17: un-watched
      18: existence doubtful
    */


#endif

      return_value.Append(spost);                     // add any final modifiers

      return return_value;
}


//--------------------------------
//
// JUMP TABLE SECTION
//
//--------------------------------
Cond condTable[] = {
   {"CLRLIN01",CLRLIN01},
   {"DATCVR01",DATCVR01},
   {"DATCVR01",DATCVR01},
   {"DEPARE01",DEPARE01},
   {"DEPARE02",DEPARE01},                 // new in PLIB 3_3, opencpn defaults to DEPARE01
   {"DEPCNT02",DEPCNT02},
   {"DEPVAL01",DEPVAL01},
   {"LEGLIN02",LEGLIN02},
   {"LIGHTS05",LIGHTS05},                 // new in PLIB 3_3, replaces LIGHTS04
   {"LITDSN01",LITDSN01},
   {"OBSTRN04",OBSTRN04},
   {"OWNSHP02",OWNSHP02},
   {"PASTRK01",PASTRK01},
   {"QUAPOS01",QUAPOS01},
   {"QUALIN01",QUALIN01},
   {"QUAPNT01",QUAPNT01},
   {"SLCONS03",SLCONS03},
   {"RESARE02",RESARE02},
   {"RESTRN01",RESTRN01},
//   {"RESCSP01",RESCSP01},
   {"SEABED01",SEABED01},
//   {"SNDFRM02",SNDFRM02},
   {"SOUNDG02",SOUNDG02},
   {"TOPMAR01",TOPMAR01},
   {"UDWHAZ03",UDWHAZ03},
   {"VESSEL01",VESSEL01},
   {"VRMEBL01",VRMEBL01},
   {"WRECKS02",WRECKS02},
   {"SOUNDG03",SOUNDG03},                   // special case for MPS
   {"########",NULL}
};

