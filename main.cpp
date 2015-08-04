#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "rapidxml.hpp"

using namespace rapidxml;
/* namespace std
{
    typedef basic_string<char> string;
    typedef basic_string<wchar_t> wstring;
} */

using namespace std;


void * print_all_AhpUid(string target, xml_node<> *axim_node, ofstream &outfile);
void search_all_nodes(string target, xml_node<> *start, ofstream &out);
void search_in(string target, xml_node<> *start,int l=-1);
bool check_ID(xml_node<> *node,int id);
int return_ID(xml_node<> *node);
int surface_code(xml_node<> *node);
void write_coordinates(xml_node<> *source, ofstream &out,int line=0, int light=0);
void write_coordinatesC(xml_node<> *source, ofstream &out,int line=0, int light=0);
void write_center(xml_node <> *source, ofstream &out);
void dimension(xml_node<> *source, double &HDG, double &len, double &wid);


xml_node<> *  complete_Ahp(const xml_node<> *Ahp, ofstream &outfile);
double my_stod(string s, int d=0);
float my_stof(string s, int d=0);
int my_stoi(string s);
string my_itos(int i,int n=0);
long double my_stold(string s, int d=0);

int non_ICAO=0;
xml_node<> *def;

 
int main(int argc, char* argv[])
{
    if(argc<=1)
    {
        cout << "Usage: " << argv[0] << "INPUT.xml" << endl;
        return EXIT_FAILURE;
    }
    cout << "Parsing OfmData..." << endl;
	xml_document<> doc, doc_def;
	xml_node<> * aixm_node;
		
	// Read the xml file into a vector
	ifstream theFile (argv[1]);
	if (!theFile.is_open())
	{
        cout << "Cannot open input file!" << endl;
        return EXIT_FAILURE;
    }
	vector<char> buffer((istreambuf_iterator<char>(theFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	doc.parse<0>(&buffer[0]);
	
	 ifstream convFile("configuration.xml");
	if(!convFile.is_open())
	{
        cout << "Cannot open configuration file!" << endl;
        return EXIT_FAILURE;
    }
    vector<char> buffer2((istreambuf_iterator<char>(convFile)), istreambuf_iterator<char>());
    buffer2.push_back('\0');
    doc_def.parse<0>(&buffer2[0]); 
	//----------------------------------HIER IST ALLES EINGELESEN
	/* erkannte struktur
	1. Ebene. ofm_webServiceResponse
	2. Ebene: "", "AIXM-Snapshot", "", "OFM-AixmPlugin"
	3. Ebene: "Ahp", "Rwy",...
	*/
	
	ofstream out;
	out.open("Apt.dat",ios::trunc);
	out << "I" << endl;
    out << "1000 Version - data cycle 2013.10, build 20131335, metadata AptXP1000.  Copyright © 2013, Robin A. Peel (robin@x-plane.com).   This data is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.  You should have received a copy of the GNU General Public License along with this program (""AptNavGNULicence.txt""); if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA." << endl << endl << endl;
    
    cout.precision(10);
    out.precision(10);
    cout << std::setfill('0') << std::setw(2);
    out << std::setfill('0') << std::setw(2);
    
	
	//Searching words
	def=doc_def.first_node("default_values");

	aixm_node = doc.first_node()->first_node("AIXM-Snapshot");

	if(aixm_node == NULL)
	{ 
		cout << "ERROR: ROOT Node 'AIXM-Snapshot' not found." << endl;
		return 1;	
	}
	
	
	// Searching all Ahps
	cout << "Searching all Ahps... " << endl;

	xml_node<> * current_Ahp=aixm_node->first_node("Ahp");
    complete_Ahp(current_Ahp,out);
	while(true)
	{
        if(current_Ahp->next_sibling("Ahp")==NULL)
            break;
        
        current_Ahp=current_Ahp->next_sibling("Ahp");
        complete_Ahp(current_Ahp,out);
    }
	out << "99" << endl;
    out.close();
    return 0;
}

xml_node<> * complete_Ahp(const xml_node<> *Ahp, ofstream &out)
{
    xml_node<> * current;
    xml_node<> * Rwy, *Rdn1, *Rdn2;
    xml_node<> * Twy, *Tly, *Apn, *misc;
    xml_node<> * to_return;
    string s;
    
    int heli_count=0;
    double hlen, hwid, hhdg;
    double ahphdg=-1;

    int ID=my_stoi(string(Ahp->first_node("AhpUid")->first_attribute("mid")->value()));
    out << endl << "1 ";
    string buf(Ahp->first_node("uomElev")->value());
    if(Ahp->first_node("valElev")!=NULL)
    {
        if(buf.compare("FT")==0)
            out << Ahp->first_node("valElev")->value() << " ";
        else
        {
            if(buf.compare("M")==0)
            {
                string buf2(Ahp->first_node("valElev")->value());
                out << int(my_stof(buf2)*3.28083) << " ";
            }else
            {
                cout << "Attention: unknown unit uomElev: " << buf << endl;
                string buf2(Ahp->first_node("valElev")->value());
                out << int(my_stof(buf2)) << " ";
            }
        }
    }else
    {
        cout << "Warning: default field elevation given!" << endl;
        out << def->first_node("Ahp")->first_node("altitude")->value() << " ";
    }
    out << "0 0 ";
    string buf2(Ahp->first_node("codeType")->value());

    if(buf2.compare("AD")==0)
    {
        if(Ahp->first_node("codeIcao")!=NULL)
            out << Ahp->first_node("codeIcao")->value() << " ";
        else
            out << Ahp->first_node("xt_GpsIdent")->value() << " ";
    }
    if(buf2.compare("ELS")==0)
    {
        non_ICAO++;
        s=string(Ahp->first_attribute("xt_fir")->value()).substr(0,2);
        s=s.append(my_itos(non_ICAO,2));
        out << s << " ";
        cout << "non-ICAO code given: " << s << endl;
    }
    
    out << Ahp->first_node("txtName")->value() << endl;
    //--------AHP line finished-----------

    if(Ahp->next_sibling()!=NULL)
        current=Ahp->next_sibling();
    else
        return NULL;
    while(true)
    {
        if(check_ID(current,ID)==false)
            goto cont;

        // ----------------------------- Making runway ----------------
        if(string(current->name()).compare("Rwy")==0)
        {
            Rwy=current;
            Rdn1=Rwy->next_sibling("Rdn");
            Rdn2=Rdn1->next_sibling("Rdn");
            
            out << "100 ";
            if(string(Rwy->first_node("uomDimRwy")->value()).compare("M")==0)
                out << Rwy->first_node("valWid")->value() << " ";
            if(string(Rwy->first_node("uomDimRwy")->value()).compare("FT")==0)
                out << my_stof(string(Rwy->first_node("valWid")->value()))*3.28083 << " ";
            
            out << surface_code(Rwy) << " ";
            out << def->first_node("Rwy")->first_node("shoulder_surface")->value() << " ";
            out << def->first_node("Rwy")->first_node("smoothness")->value() << " ";
            
            
            
            if(Rwy->first_node("xt_lighting")->first_node()!=NULL)
            {
                if(Rwy->first_node("xt_lighting")->first_node("CENTER_LIGHTS")!=NULL)
                    out << "1 ";
                else
                    out << "0 ";
                if(Rwy->first_node("xt_lighting")->first_node("EDGE_LIGTHS")!=NULL)
                    out << "2 ";
                else
                    out << "0 ";
            }else
            {
                out << def->first_node("Rwy")->first_node("center_lights")->value() << " ";
                out << def->first_node("Rwy")->first_node("edge_lights")->value() << " ";
            }
            
            out << def->first_node("Rwy")->first_node("generate_signs")->value() << " ";
            
            
            // --- Rdn1 --- //
            if(ahphdg==-1)
                ahphdg=my_stod(string(Rdn1->first_node("valueTruBrg")->value()));
            if(string(Rdn1->first_node("RdnUid")->first_node("txtDesig")->value()).length()!=0)
                out << Rdn1->first_node("RdnUid")->first_node("txtDesig")->value() << " ";
            else
                out << def->first_node("Rdn")->first_node("desig_1")->value() << " ";
            out << Rdn1->first_node("geoLat")->value() << " ";
            out << Rdn1->first_node("geoLon")->value() << " ";
            
            out << def->first_node("Rdn")->first_node("disptr")->value() << " ";
            if(Rdn1->first_node("xt_valDispTres")!=NULL)
            {
                if(string(Rdn1->first_node("xt_uomDispTres")->value()).compare("FT")==0)
                    out << my_stof(string(Rdn1->first_node("xt_valDispTres")->value()))*0.3048 << " ";
                else
                    out << Rdn1->first_node("xt_valDispTres")->value() << " ";
            }else
                out << "0.00 ";
            
            
            if(Ahp->first_node("xt_TypeOfTraffic")->first_node("Ifr")!=0)
                out << "3 ";
            else
                out << "1 ";
            
            out << def->first_node("Rdn")->first_node("Apr_light")->value() << " ";
            out << def->first_node("Rdn")->first_node("TDZ_light")->value() << " ";
            out << def->first_node("Rdn")->first_node("REIL")->value() << " ";
            
            // --- Rdn2 --- //
            if(!string(Rdn2->first_node("RdnUid")->first_node("txtDesig")->value()).empty())
                out << Rdn2->first_node("RdnUid")->first_node("txtDesig")->value() << " ";
            else
                out << def->first_node("Rdn")->first_node("desig_2")->value() << " ";
            out << Rdn2->first_node("geoLat")->value() << " ";
            out << Rdn2->first_node("geoLon")->value() << " ";
            
            out << def->first_node("Rdn")->first_node("disptr")->value() << " ";
            if(Rdn2->first_node("xt_valDispTres")!=NULL)
            {
                if(string(Rdn2->first_node("xt_uomDispTres")->value()).compare("FT")==0)
                    out << my_stof(string(Rdn2->first_node("xt_valDispTres")->value()))*0.3048 << " ";
                else
                    out << Rdn2->first_node("xt_valDispTres")->value() << " ";
            }else
                out << "0.00 ";
            
            
            if(Ahp->first_node("xt_TypeOfTraffic")->first_node("Ifr")!=0)
                out << "3 ";
            else
                out << "1 ";
            
            out << def->first_node("Rdn")->first_node("Apr_light")->value() << " ";
            out << def->first_node("Rdn")->first_node("TDZ_light")->value() << " ";
            out << def->first_node("Rdn")->first_node("REIL")->value() << " " << endl;
            goto cont;
            
        } // -------------- End of making runway ----------------
        
        
        // ---------------- Making taxiway --------------
        if(string(current->name()).compare("Twy")==0)
        {
            Twy=current;
            out << "110 ";
            out << surface_code(Twy) << " ";
            out << def->first_node("Twy")->first_node("smoothness")->value() << " ";
            if(ahphdg==-1)
                out << def->first_node("Twy")->first_node("orientation")->value() << " ";
            else
                out << ahphdg << " ";
            out << Twy->first_node("TwyUid")->first_node("txtDesig")->value() << " " << endl;
            if(surface_code(Twy)==2)
                write_coordinatesC(Twy->first_node("xt_surface")->first_node("gmlPosList"),out); // 53,102
            else
                write_coordinatesC(Twy->first_node("xt_surface")->first_node("gmlPosList"),out); // 3,102
            /* if (string(Ahp->first_node()->first_attribute()->value()).compare("7459")==0)
                system("PAUSE"); */
            goto cont;
        }
        
        // -------------- Making taxiway lines -----------
        if(string(current->name()).compare("Tly")==0)
        {
            Tly=current;
            if(Tly->first_node("xt_surface")!=NULL)
            {    
                out << "120 ";
                out << Tly->first_node("TlyUid")->first_node("TwyUid")->first_node("txtDesig")->value() << endl;
                if(surface_code(Twy)==2)
                    write_coordinates(Tly->first_node("xt_surface")->first_node("gmlPosList"),out,51,101);
                else
                    write_coordinates(Tly->first_node("xt_surface")->first_node("gmlPosList"),out,1,101);
            }
            goto cont;
        }
        
        // --------------- Making apron ------------------
        if(string(current->name()).compare("Apn")==0)
        {
            Apn=current;
            out << "110 ";
            out << surface_code(Apn) << " ";
            out << def->first_node("Apn")->first_node("smoothness")->value() << " ";
            if(ahphdg==-1)
                out << def->first_node("Apn")->first_node("orientation")->value() << " ";
            else
                out << ahphdg << " ";
            out << Apn->first_node("ApnUid")->first_node("txtName")->value() << " " << endl;
            if(surface_code(Apn)==2)
                write_coordinatesC(Apn->first_node("xt_surface")->first_node("gmlPosList"),out);  //   52           
            else
                write_coordinatesC(Apn->first_node("xt_surface")->first_node("gmlPosList"),out); // 2
            goto cont;
        }
        
        
        // --------------- Checking misc -----------------
        if(string(current->name()).compare("misc")==0)
        {
            misc=current;
            s=string(misc->first_node("type")->value());
            // ------------ Windsock -------------
            if(s.compare("Windsock")==0)
            {
                out << "19 ";
                write_center(misc->first_node("xt_surface")->first_node("gmlPosList"),out);
                out << def->first_node("misc")->first_node("WS_light")->value() << endl;
                goto cont;
            }
            
            // ------------ Helipad --------------
            if(s.compare("Helipad")==0)
            {
                heli_count++;
                out << "102 ";
                out << "H" << heli_count << " ";
                write_center(misc->first_node("xt_surface")->first_node("gmlPosList"),out);
                dimension(misc->first_node("xt_surface")->first_node("gmlPosList"),hhdg,hlen,hwid);
                out << hhdg << " ";
                out << hlen << " ";
                out << hwid << " ";
                out << surface_code(Rwy) << " ";
                out << "0 ";
                out << surface_code(Rwy) << " ";
                out << def->first_node("misc")->first_node("heli_smooth")->value() << " ";
                out << def->first_node("misc")->first_node("heli_lights")->value() << endl;
                goto cont;
            }
            
            // -------------- Ramps --------------
            if(s.find("ramp")!=s.npos)
            {
                if(misc->first_node("xt_label")!=NULL && false)
                {
                    out << "1300 ";
                    out << def->first_node("misc")->first_node("ramp")->first_node("zone_class")->value() << " ";
                    
                    s=string(misc->first_node("xt_label")->first_node("geoLat")->value());
                    if(s[s.length()-1]=='N')
                        out << my_stold(s) << " ";
                    else
                        out << "-" << my_stold(s) << " ";
                    
                    s=string(misc->first_node("xt_label")->first_node("geoLon")->value());
                    if(s[s.length()-1]=='E')
                        out << my_stold(s) << " ";
                    else
                        out << "-" << my_stold(s) << " ";
                    
                    if(string(misc->first_node("xt_label")->first_node("angle")->value()).length()!=0)
                        out << misc->first_node("xt_label")->first_node("angle")->value() << " ";
                    else
                        out << def->first_node("misc")->first_node("ramp")->first_node("angle")->value() << " ";
                    out << def->first_node("misc")->first_node("ramp")->first_node("type")->value() << " ";
                    out << def->first_node("misc")->first_node("ramp")->first_node("airplane")->value() << endl;
                }
                out << "120 ";
                out << misc->first_node("miscUid")->first_node("txtName")->value() << endl;
                if(surface_code(Apn)==2)
                    write_coordinates(misc->first_node("xt_surface")->first_node("gmlPosList"),out,51);  //   52           
                else
                    write_coordinates(misc->first_node("xt_surface")->first_node("gmlPosList"),out,1);
                goto cont;
            }
            
            // Airport boundary
            if(s.compare("airport zone")==0)
            {
                //cout << "Zone written in " << Ahp->first_node("txtName")->value() << endl;
                out << "130 ";
                out << misc->first_node("miscUid")->first_node("txtName")->value() << endl;
                write_coordinatesC(misc->first_node("xt_surface")->first_node("gmlPosList"),out,2);
                goto cont;
            }
        }
        
            
        cont:
        if(current->next_sibling()!=NULL)
            current=current->next_sibling();
        else
        {
            out << endl;
            return NULL;
        }
    }
    
}

//--------------Auxiliary functions------------------------------------------------------

void search_all_nodes(string target, xml_node<> *start, ofstream &out)
{
    xml_node<> * current=start;
	int ebene=0;
	out << current->name() << endl << endl;
	while(true)
	{
	    if(current->first_node()!=NULL && current->first_node()->name()[0] != '\0')
		{
			ebene++;
			current=current->first_node();
			goto check;
		}
		sibparent:
		if(current->next_sibling()!=NULL)
		{
			current=current->next_sibling();
			goto check;
		}
		if(current->parent()!=NULL && ebene > 0)
		{
			ebene--;
			current=current->parent();
			goto sibparent;
		}
		break;
		
		check:
		string s(current->name());
		if(s.compare(target)==0)
			out << current->value() << endl;
	}
}

void search_in(string target, xml_node<> *start,int l)
{
    xml_node<> * current;
    char* my_type=start->name();
	int ebene=0;
	while(true)
	{
        if(ebene==0)
        {
    	    if(string(start->name()).compare(my_type)==0 && start->first_node()!=NULL)
    		{
    			ebene++;
    			current=start->first_node();
    			goto check;
    		}else
    		{
                if(start->next_sibling(my_type)!=NULL)
                {
                    start=start->next_sibling(my_type);
                    goto check;
                }
            }
        }
        else
        {
            if(current->first_node()!=NULL)
            {
                ebene++;
                current=current->first_node();
                goto check;
            }
    		sibparent:
    		if(current->next_sibling()!=NULL)
    		{
    			current=current->next_sibling();
    			goto check;
    		}
    		if(current->parent()!=NULL && ebene > 0)
    		{
    			ebene--;
    			if(ebene==0)
    			{
    			    start=current->parent();
    			    if(start->next_sibling(my_type)!=NULL)
    			        start=start->next_sibling(my_type);
    			    else
    			        break;
    			    continue;
                }
    			else
    			{
    			    current=current->parent();
    			    goto sibparent;
                }
    		}
        }
		break;
		
		check:
		if(string(current->name()).compare(target)==0 && (l==-1 || l==ebene))
			cout << current->name() << "\t" << start->first_node("txtName")->value() << "\t" <<  current->value() << endl;
	}
}

bool check_ID(xml_node<> *node,int id)
{
    for(int i=0;i<4;i++)
    {
        if(node->first_attribute("mid")==NULL)
        {
            if(node->first_node()==NULL)
                return false;
            else
            {
                node=node->first_node();
                continue;
            }
        }else
        {
            if(my_stoi(string(node->first_attribute("mid")->value()))==id)
                return true;
            else
                return false;     
        }
    }
}

int return_ID(xml_node<> *node)
{
    for(int i=0;i<4;i++)
    {
        if(node->first_attribute("mid")==NULL)
        {
            if(node->first_node()==NULL)
                return -1;
            else
            {
                node=node->first_node();
                continue;
            }
        }else
            return my_stoi(string(node->first_attribute("mid")->value()));
    }
}

int surface_code(xml_node<> *node)
{
    string s(node->first_node("codeComposition")->value());
    if(s.length()==0)
        s=string(def->first_node("others")->first_node("codeComposition")->value());
    if(s.compare("ASPH")==0)
        return 1;
    if(s.compare("CONC")==0)
        return 2;
    if(s.compare("GRASS")==0)
        return 3;
}

void write_coordinates(xml_node<> *source, ofstream &out, int line, int light)
{
    // Extracting coordinates 
    string s(source->value());
    size_t l=0,r=-1, ll, rr;
    int c=0;
    string x,y,bx,by;
    
    while(true)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        c++;
        if(r==s.npos-1)
            break;
    }
    c--;

    long double *coor=new long double[c];
    l=0;
    r=-1;
    
    for(int i=0;i<c;i++)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("11234567890.",l)-1;
        coor[i]=my_stold(s.substr(l,r-l+1));
    }
    delete [] coor;
    c=c/2;
        
    r=-1;
    l=0;
    for(int i=0;i<c-1;i=i+3)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        x=s.substr(l,r-l+1);
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        y=s.substr(l,r-l+1);     
        
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        bx=s.substr(l,r-l+1);
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        by=s.substr(l,r-l+1);    
        
        //out << "112 " << y << " " << x << " " << by << " " << bx << " ";
        out << "112 " << y << " " << x << " ";
        out << by << " " << bx << " ";
        //out << 2*my_stold(y)-my_stold(by) << " " << 2*my_stold(x)-my_stold(bx) << " ";
        if(line!=0)
            out << line << " ";
        if(light!=0)
            out << light << " ";
        out << endl;
        
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        bx=s.substr(l,r-l+1);
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        by=s.substr(l,r-l+1);
        
        ll=l;
        rr=r;     
        
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        x=s.substr(l,r-l+1);
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        y=s.substr(l,r-l+1);  
        
        out << "116 " << y << " " << x << " ";
        //out << by << " " << bx << " ";
        out << 2*my_stold(y)-my_stold(by) << " " << 2*my_stold(x)-my_stold(bx) << " ";
        
        if(line!=0)
            out << line << " "; // line
        if(light!=0)
            out << light << " "; // light
        out << endl;
        r=rr;
        l=ll;
    }
}


void write_coordinatesC(xml_node<> *source, ofstream &out, int line, int light)
{
    // Same as write_coordinates(), but ending with a closed loop.
    // Extracting coordinates 
    string s(source->value());
    size_t l=0,r=-1, ll, rr;
    int c=0;
    long double x,y,bx,by,n;
    long double shift, factor, exponent;
    int last;
    int ex;
    
    long double area=0;
    
    long double x1, y1, x2, y2, x3, y3, x4, y4;
    long double t, c1, c2;
    
    shift=my_stold(string(def->first_node("others")->first_node("Bezier")->first_node("Shift")->value()));
    factor=my_stold(string(def->first_node("others")->first_node("Bezier")->first_node("Factor")->value()));
    exponent=my_stold(string(def->first_node("others")->first_node("Bezier")->first_node("Exp")->value()));
    
    while(true)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        c++;
        if(r==s.npos-1)
            break;
    }
    c--;

    long double *coor=new long double[c];
    l=0;
    r=-1;
    
    for(int i=0;i<c;i++)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("11234567890.",l)-1;
        coor[i]=my_stold(s.substr(l,r-l+1));
    }
    for(int i=0;i<c-2;i=i+6)
        area+=coor[i]*coor[i+7]-coor[i+6]*coor[i+1];
    area-=coor[0]*coor[c-1] - coor[1]*coor[c-2];
    
    
    if(area<0)
    {
        last=6;
        while(coor[last]==coor[last-6] && coor[last+1]==coor[last-5])
            last+=6;
        //cout << "Correction of direction!" << endl;
        for(int i=c-2;i>0;i=i-6)
        {
            /*x1=coor[i], y1=coor[i+1], 
            x2=coor[i-2], y2=coor[i-1],
            x3=coor[i-4], y3=coor[i-3], 
            x4=coor[i-6], y4=coor[i-5];*/
            if(coor[i]==coor[i-6] && coor[i+1]==coor[i-5])
                continue;

            area=coor[i]*(coor[i-1]-coor[i-5])+coor[i-2]*(coor[i-3]-coor[i+1])+coor[i-4]*(coor[i-5]-coor[i-1])+coor[i-6]*(coor[i+1]-coor[i-3]);
            
            if(area==0 || isnan(area))
                n=1.0;
            else
            {
                frexpl(area,&ex);
                n=int(pow((ex+shift)*factor, exponent))+1.0;
            }
            for(long double j=0.0; j<=n; j=j+1.0)
            {
                /* if(i!=c-2 && j==0)
                    continue; */
                t=j/n;
                x=(1-t)*(1-t)*(1-t)*coor[i] + 
                3*(1-t)*(1-t)*t*coor[i-2] +
                3*(1-t)*t*t*coor[i-4] +
                t*t*t*coor[i-6];
                
                y=(1-t)*(1-t)*(1-t)*coor[i+1] +
                3*(1-t)*(1-t)*t*coor[i-1] + 
                3*(1-t)*t*t*coor[i-3] +
                t*t*t*coor[i-5];
                if(i>last || j<n)
                    out << "111 " << y << " " << x << " ";    
                else
                    out << "113 " << y << " " << x << " ";
                if(line!=0)
                    out << line << " ";
                if(light!=0)
                    out << light << " ";
                out << endl;
            }
        }
    }
    else
    {
        //cout << "Good direction!" << endl;
        last=c-8;
        while(coor[last]==coor[last+6] && coor[last+1]==coor[last+7])
            last-=6;
        for(int i=0;i<c-2;i=i+6)
        {
            /*x1=coor[i], y1=coor[i+1], 
            x2=coor[i+2], y2=coor[i+3],
            x3=coor[i+4], y3=coor[i+5], 
            x4=coor[i+6], y4=coor[i+7];*/
            if(coor[i]==coor[i+6] && coor[i+1]==coor[i+7])
                continue;
            
            area=coor[i]*(coor[i+3]-coor[i+7])+coor[i+2]*(coor[i+5]-coor[i+1])+coor[i+4]*(coor[i+7]-coor[i+3])+coor[i+6]*(coor[i+1]-coor[i+5]);

            if(area==0 || isnan(area))
                n=1;
            else
            {
                frexpl(area,&ex);
                n=int(pow((ex+shift)*factor, exponent))+1.0;
            }
            
            for(long double j=0.0; j<=n; j=j+1)
            {
                /* if(j==0 && i!=0)
                    continue; */
                t=j/n;
                
                x=(1-t)*(1-t)*(1-t)*coor[i] + 
                3*(1-t)*(1-t)*t*coor[i+2] +
                3*(1-t)*t*t*coor[i+4] +
                t*t*t*coor[i+6];
                
                y=(1-t)*(1-t)*(1-t)*coor[i+1] +
                3*(1-t)*(1-t)*t*coor[i+3] + 
                3*(1-t)*t*t*coor[i+5] +
                t*t*t*coor[i+7];
                
                if(i<last || j<n)
                    out << "111 " << y << " " << x << " ";
                else
                    out << "113 " << y << " " << x << " ";
                if(line!=0)
                    out << line << " ";
                if(light!=0)
                    out << light << " ";
                out << endl;
            }
        }
    }
}

void write_center(xml_node <> *source, ofstream &out)
{
    // Extracting coordinates 
    string s(source->value());
    size_t l=0,r=-1;
    int c=0;
    double x=0.0, y=0.0;
    
    while(true)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        c++;
        if(r==s.npos-1)
            break;
    }
    c--;

    double *coor=new double[c];
    l=0;
    r=-1;
    
    // Saving the numbers in double array
    for(int i=0;i<c;i++)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("11234567890.",l)-1;
        coor[i]=my_stod(s.substr(l,r-l+1));
    }
    
    for(int i=0;i<c;i=i+2)
    {
        x+=coor[i];
        y+=coor[i+1];
    }
    c=c/2;
    x=x/c;
    y=y/c;
    out << fixed << y << " " << x << " ";
    delete [] coor;
}

void dimension(xml_node<> *source, double &HDG, double &len, double &wid)
{
    // Extracting coordinates 
    long double pi=3.14159265358;
    string s(source->value());
//    string s("16.2514907056155,47.8451013849285 16.2514907056155,47.8451013849285 16.2514907056155,47.8451013849285 16.2514907056155,47.8451013849285");
    size_t l=0,r=-1;
    int c=0;
    long double rad=my_stod(string(def->first_node("others")->first_node("earthRadius")->value()));
    
    while(true)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("1234567890.",l)-1;
        c++;
        if(r==s.npos-1)
            break;
    }
    c--;

    long double *coor=new long double[c];
    long double *x=new long double[c/2];
    long double *y=new long double[c/2];
    l=0;
    r=-1;
    
    // Saving the numbers in double array
    for(int i=0;i<c;i++)
    {
        l=s.find_first_of("1234567890.",r+1);
        r=s.find_first_not_of("11234567890.",l)-1;
        coor[i]=my_stod(s.substr(l,r-l+1));
    }
    
    // Converting to local x and y
    c=c/2;
    for (int i=0;i<c;i++)
    {
        y[i]=coor[2*i+1]*180/pi*rad;
        x[i]=coor[2*i]*180/pi*rad * cos(coor[2*i+1]*pi/180);
    }
    
    HDG=atan2(x[0]-x[1],y[0]-y[1]) * 180/pi;
    len=sqrt( (x[0]-x[1])*(x[0]-x[1]) + (y[0]-y[1])*(y[0]-y[1]));
    wid=sqrt( (x[2]-x[1])*(x[2]-x[1]) + (y[2]-y[1])*(y[2]-y[1]));
    if(len==0)
        len=my_stod(string(def->first_node("misc")->first_node("heli_len")->value()));
    if(wid==0)
        wid=my_stod(string(def->first_node("misc")->first_node("heli_wid")->value()));
    
    delete [] x;
    delete [] y;
    delete [] coor;
}

void * print_all_AhpUid(string target, xml_node<> *axim_node, ofstream &outfile)
{
//string target="4693"; // St. Johann in Tirol
	
	//ALLE nodes durchgehen AhpUid 
	xml_node<> * current=axim_node;
	int ebene=0;
	while(true)
	{
	if(current->first_node()!=NULL && current->first_node()->name()[0] != '\0')
		{
			ebene++;
			// cout << "-going deeper. ebene: "<<ebene <<" "<<endl;	
			//printf("first node pointer: %p \n", current->first_node());
			current=current->first_node();
			goto check;
		}
		sibparent:
		if(current->next_sibling()!=NULL)
		{
			// cout << "-going sibling. ebene: "<<ebene <<" "<<endl;
			current=current->next_sibling();
			goto check;
		}
		if(current->parent()!=NULL && ebene > 0)
		{
			ebene--;
			// cout << "-becoming parent. ebene: "<<ebene <<" "<<endl;
			current=current->parent();
			goto sibparent;
		}
		
		// cout << "-no if matched, break."<<endl;
		//printf("my p: %p,  parent p: %p, sibling p: %p, parent->sibling p: %p\n", 
			// current, current->parent(), current->next_sibling(), current->parent()->next_sibling() );
		break;
		
		
		check:
		// cout << "--check: i am '" << current->name() << "'"<<endl;
		//printf(" my p: %p \n", current);
		string s(current->name());
		if(s.compare("AhpUid")==0 && current->first_attribute("mid")->value()[0] != '\0')
		{
			string t(current->first_attribute("mid")->value());
			if(t.compare(target)==0)
			{
				outfile << "--AhpUid: " << current->first_attribute("mid")->value() << " " << ebene << " " << current->parent()->name() << endl;
			}
		}
	}
	//cout << "Rekursive Suche beendet." << endl;
}



int my_stoi(string s)
{
    if(s.length()==0)
        return 0;
    int out=0;
    int add=1;
    size_t p1=s.find_first_of("1234567890");
    s=s.substr(p1);
    p1=s.find_first_not_of("1234567890");
    s=s.substr(0,p1);
    int m=s.length();
    for (int i=m-1; i>=0; i--)
    {
        out+=add * (int(s.at(i))-48);
        add=add*10;
    }
    return out;
}

float my_stof(string s, int d)
{
    if(s.length()==0)
        return 0.0;
    float out=0.0;
    float add=1.0;
    int l1;
    int l2;
    size_t p1=s.find_first_of("1234567890.");
    s=s.substr(p1);
    p1=s.find_first_not_of("1234567890.");
    s=s.substr(0,p1);
    
    l1=s.find(".");
    if(l1==s.npos)
        l1=s.length();
    l2=s.length();
    
    for(int i=l1-1;i>=0;i--)
    {
        out+=add*(int(s.at(i))-48);
        add=add*10;
    }
    
    add=0.1;
    int j=0;
    for(int i=l1+1;i<l2;i++)
    {
        out+=add*(int(s.at(i))-48);
        add=add*0.1;
        j++;
        if(j==d)
            break;
    }
    return out;
}

double my_stod(string s, int d)
{
    if(s.length()==0)
        return 0.0;
    double out=0.0;
    double add=1.0;
    int l1;
    int l2;
    size_t p1=s.find_first_of("1234567890.");
    s=s.substr(p1);
    p1=s.find_first_not_of("1234567890.");
    s=s.substr(0,p1);
    
    l1=s.find(".");
    if(l1==s.npos)
        l1=s.length();
    l2=s.length();
    
    for(int i=l1-1;i>=0;i--)
    {
        out+=add*(int(s.at(i))-48);
        add=add*10;
    }
    int j=0;
    add=0.1;
    for(int i=l1+1;i<l2;i++)
    {
        out+=add*(int(s.at(i))-48);
        add=add*0.1;
        j++;
/*         cout << "add: " << add << endl;
        cout << "out: " << out << endl << endl;
        system("pause"); */
        if(j==d)
            break;
    }
    return out;
}

long double my_stold(string s, int d)
{
    if(s.length()==0)
        return 0.0;
    long double out=0.0;
    long double add=1.0;
    int l1;
    int l2;
    size_t p1=s.find_first_of("1234567890.");
    s=s.substr(p1);
    p1=s.find_first_not_of("1234567890.");
    s=s.substr(0,p1);
    
    l1=s.find(".");
    if(l1==s.npos)
        l1=s.length();
    l2=s.length();
    
    for(int i=l1-1;i>=0;i--)
    {
        out+=add*(int(s.at(i))-48);
        add=add*10;
    }
    int j=0;
    add=0.1;
    for(int i=l1+1;i<l2;i++)
    {
        out+=add*(int(s.at(i))-48);
        add=add*0.1;
        j++;
/*         cout << "add: " << add << endl;
        cout << "out: " << out << endl << endl;
        system("pause"); */
        if(j==d)
            break;
    }
    return out;
}

string my_itos(int i,int n)
{
    int j=i, d=0;
    string s;
    while(j!=0)
    {
        j=j/10;
        d++;
    }
    if(d>n)
        n=d;
    
    s.assign(n,'0');
    j=s.length();
    for(int k=0;k<d;k++)
    {
        s[j-1-k]=i%10+48;
        i=i/10;
    }
    return s;
}

