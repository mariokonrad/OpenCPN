/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#include "CM93Chart.h"

#include <chart/CM93Manager.h>
#include <chart/CM93Dictionary.h>
#include <chart/COVR_Set.h>
#include <chart/CM93Geometry.h>
#include <chart/CM93_attr_block.h>

#include <geo/ExtendedGeometry.h>
#include <geo/PolyTessGeo.h>
#include <geo/GeoRef.h>
#include <geo/Polygon.h>

#include <MicrosoftCompatibility.h>
#include <LogMessageOnce.h>

#include <algorithm>
#include <vector>

#ifdef USE_S57
	#include <chart/s52plib.h>
	extern chart::s52plib * ps52plib; // FIXME
#endif

extern bool g_bDebugCM93; // FIXME
static bool s_b_busy_shown; // FIXME

namespace chart {

typedef std::vector<M_COVR_Desc*> List_Of_M_COVR_Desc;

struct header_struct
{
	double                  lon_min;
	double                  lat_min;
	double                  lon_max;
	double                  lat_max;
	// Bounding Box, in Mercator transformed co-ordinates
	double                  easting_min;
	double                  northing_min;
	double                  easting_max;
	double                  northing_max;

	unsigned short          usn_vector_records;        //  number of spacial(vector) records
	int                     n_vector_record_points;    //  number of cm93 points in vector record block
	int                     m_46;
	int                     m_4a;
	unsigned short          usn_point3d_records;
	int                     m_50;
	int                     m_54;
	unsigned short          usn_point2d_records;             //m_58;
	unsigned short          m_5a;
	unsigned short          m_5c;
	unsigned short          usn_feature_records;            //m_5e, number of feature records

	int                     m_60;
	int                     m_64;
	unsigned short          m_68;
	unsigned short          m_6a;
	unsigned short          m_6c;
	int                     m_nrelated_object_pointers;

	int                     m_72;
	unsigned short          m_76;

	int                     m_78;
	int                     m_7c;
};

struct vector_record_descriptor
{
	geometry_descriptor * pGeom_Description;
	unsigned char segment_usage;
};

struct Object
{
	unsigned char otype;
	unsigned char geotype;
	unsigned short n_geom_elements;
	void * pGeometry; // may be a (cm93_point*) or other geom; FIXME
	unsigned char n_related_objects;
	void * p_related_object_pointer_array;
	unsigned char n_attributes;     // number of attributes
	unsigned char * attributes_block;      // encoded attributes
};

struct Cell_Info_Block
{
	// Georeferencing transform coefficients
	double transform_x_rate;
	double transform_y_rate;
	double transform_x_origin;
	double transform_y_origin;

	cm93_point * p2dpoint_array;
	Object ** pprelated_object_block;
	unsigned char * attribute_block_top;               // attributes block
	geometry_descriptor * edge_vector_descriptor_block;      // edge vector descriptor block
	geometry_descriptor * point3d_descriptor_block;
	cm93_point * pvector_record_block_top;
	cm93_point_3d * p3dpoint_array;

	int m_nvector_records;
	int m_nfeature_records;
	int m_n_point3d_records;
	int m_n_point2d_records;

	List_Of_M_COVR_Desc m_cell_mcovr_list;
	bool b_have_offsets;
	bool b_have_user_offsets;

	double user_xoff;
	double user_yoff;

	// Allocated working blocks
	vector_record_descriptor * object_vector_record_descriptor_block;
	Object * pobject_block;
};


static bool cm93_decode_table_created;

// Calculate the Lat/Lon of the lower left corner of a CM93 cell,
// given a CM93 CellIndex and scale
// Returned longitude value is always > 0
static void Get_CM93_Cell_Origin(int cellindex, int /*scale*/, double* lat, double* lon)
{
	// Longitude
	double idx1 = cellindex % 10000;
	double lont = ( idx1 / 3.0);

	*lon = lont;

	// Latitude
	int idx2 = cellindex / 10000;
	double lat1 = idx2 - 270.0;
	*lat = lat1 / 3.0;
}

// Calculate the CM93 CellIndex integer for a given Lat/Lon, at a given scale
int Get_CM93_CellIndex(double lat, double lon, int scale)
{
	int retval = 0;

	int dval;
	switch ( scale )
	{
		case 20000000: dval = 120; break;         // Z
		case  3000000: dval =  60; break;         // A
		case  1000000: dval =  30; break;         // B
		case   200000: dval =  12; break;         // C
		case   100000: dval =   3; break;         // D
		case    50000: dval =   1; break;         // E
		case    20000: dval =   1; break;         // F
		case     7500: dval =   1; break;         // G
		default: dval =   1; break;
	}

	// Longitude
	double lon1 = (lon + 360.0) * 3.0;                    // basic cell size is 20 minutes
	while ( lon1 >= 1080.0)
		lon1 -= 1080.0;
	unsigned short lon2 = ( unsigned short ) floor ( lon1 /dval );      // normalize
	unsigned short lon3 = lon2 * dval;

	retval = lon3;

	// Latitude
	double lat1 = ( lat * 3.0) + 270.0 - 30;
	unsigned short lat2 = ( unsigned short ) floor ( lat1 / dval );      // normalize
	unsigned short lat3 = lat2 * dval;


	retval += ( lat3 + 30 ) * 10000;

	return retval;
}


// FIXME: move chart decoding to separate class
static unsigned char Table_0[] =
{
	0x0CD,0x0EA,0x0DC,0x048,0x03E,0x06D,0x0CA,0x07B,0x052,0x0E1,0x0A4,0x08E,0x0AB,0x005,0x0A7,0x097,
	0x0B9,0x060,0x039,0x085,0x07C,0x056,0x07A,0x0BA,0x068,0x06E,0x0F5,0x05D,0x002,0x04E,0x00F,0x0A1,
	0x027,0x024,0x041,0x034,0x000,0x05A,0x0FE,0x0CB,0x0D0,0x0FA,0x0F8,0x06C,0x074,0x096,0x09E,0x00E,
	0x0C2,0x049,0x0E3,0x0E5,0x0C0,0x03B,0x059,0x018,0x0A9,0x086,0x08F,0x030,0x0C3,0x0A8,0x022,0x00A,
	0x014,0x01A,0x0B2,0x0C9,0x0C7,0x0ED,0x0AA,0x029,0x094,0x075,0x00D,0x0AC,0x00C,0x0F4,0x0BB,0x0C5,
	0x03F,0x0FD,0x0D9,0x09C,0x04F,0x0D5,0x084,0x01E,0x0B1,0x081,0x069,0x0B4,0x009,0x0B8,0x03C,0x0AF,
	0x0A3,0x008,0x0BF,0x0E0,0x09A,0x0D7,0x0F7,0x08C,0x067,0x066,0x0AE,0x0D4,0x04C,0x0A5,0x0EC,0x0F9,
	0x0B6,0x064,0x078,0x006,0x05B,0x09B,0x0F2,0x099,0x0CE,0x0DB,0x053,0x055,0x065,0x08D,0x007,0x033,
	0x004,0x037,0x092,0x026,0x023,0x0B5,0x058,0x0DA,0x02F,0x0B3,0x040,0x05E,0x07F,0x04B,0x062,0x080,
	0x0E4,0x06F,0x073,0x01D,0x0DF,0x017,0x0CC,0x028,0x025,0x02D,0x0EE,0x03A,0x098,0x0E2,0x001,0x0EB,
	0x0DD,0x0BC,0x090,0x0B0,0x0FC,0x095,0x076,0x093,0x046,0x057,0x02C,0x02B,0x050,0x011,0x00B,0x0C1,
	0x0F0,0x0E7,0x0D6,0x021,0x031,0x0DE,0x0FF,0x0D8,0x012,0x0A6,0x04D,0x08A,0x013,0x043,0x045,0x038,
	0x0D2,0x087,0x0A0,0x0EF,0x082,0x0F1,0x047,0x089,0x06A,0x0C8,0x054,0x01B,0x016,0x07E,0x079,0x0BD,
	0x06B,0x091,0x0A2,0x071,0x036,0x0B7,0x003,0x03D,0x072,0x0C6,0x044,0x08B,0x0CF,0x015,0x09F,0x032,
	0x0C4,0x077,0x083,0x063,0x020,0x088,0x0F6,0x0AD,0x0F3,0x0E8,0x04A,0x0E9,0x035,0x01C,0x05F,0x019,
	0x01F,0x07D,0x070,0x0FB,0x0D1,0x051,0x010,0x0D3,0x02E,0x061,0x09D,0x05C,0x02A,0x042,0x0BE,0x0E6
};

static unsigned char Encode_table[256];
static unsigned char Decode_table[256];

void CreateDecodeTable(void)
{
	for (unsigned int i = 0; i < 256; ++i) {
		Encode_table[i] = Table_0[i] ^ 8;
	}

	for (unsigned int i = 0; i < 256; ++i) {
		const unsigned char a = Encode_table[i];
		Decode_table[(int)a] = i;
	}
}

static int read_and_decode_bytes(FILE* stream, void* p, int nbytes)
{
	if (0 == nbytes) // declare victory if no bytes requested
		return 1;

	// read into callers buffer
	if (!fread(p, nbytes, 1, stream))
		return 0;

	// decode inplace
	unsigned char* q = (unsigned char*)p;

	for (int i = 0; i < nbytes; i++) {
		unsigned char a = *q;
		int b = a;
		unsigned char c = Decode_table[b];
		*q = c;

		q++;
	}
	return 1;
}

static int read_and_decode_double(FILE* stream, double* p)
{
	double t;
	// read into temp buffer
	if (!fread(&t, sizeof(double), 1, stream))
		return 0;

	// decode inplace
	unsigned char* q = (unsigned char*)&t;

	for (unsigned int i = 0; i < sizeof(double); i++) {
		unsigned char a = *q;
		int b = a;
		unsigned char c = Decode_table[b];
		*q = c;

		q++;
	}

	// copy to target
	*p = t;

	return 1;
}

static int read_and_decode_int(FILE* stream, int* p)
{
	int t;
	// read into temp buffer
	if (!fread(&t, sizeof(int), 1, stream))
		return 0;

	// decode inplace
	unsigned char* q = (unsigned char*)&t;

	for (unsigned int i = 0; i < sizeof(int); i++) {
		unsigned char a = *q;
		int b = a;
		unsigned char c = Decode_table[b];
		*q = c;

		q++;
	}

	// copy to target
	*p = t;

	return 1;
}

static int read_and_decode_ushort(FILE* stream, unsigned short* p)
{
	unsigned short t;
	// read into temp buffer
	if (!fread(&t, sizeof(unsigned short), 1, stream))
		return 0;

	// decode inplace
	unsigned char* q = (unsigned char*)&t;

	for (unsigned int i = 0; i < sizeof(unsigned short); i++) {
		unsigned char a = *q;
		int b = a;
		unsigned char c = Decode_table[b];
		*q = c;

		q++;
	}

	// copy to target
	*p = t;

	return 1;
}

static bool read_header_and_populate_cib ( FILE *stream, Cell_Info_Block *pCIB )
{
	//    Read header, populate Cell_Info_Block

	//    This 128 byte block is read element-by-element, to allow for
	//    endian-ness correction by element.
	//    Unused elements are read and, well, unused.

	header_struct header;

	memset ( ( void * ) &header, 0, sizeof ( header ) );

	read_and_decode_double ( stream,&header.lon_min );
	read_and_decode_double ( stream,&header.lat_min );
	read_and_decode_double ( stream,&header.lon_max );
	read_and_decode_double ( stream,&header.lat_max );

	read_and_decode_double ( stream,&header.easting_min );
	read_and_decode_double ( stream,&header.northing_min );
	read_and_decode_double ( stream,&header.easting_max );
	read_and_decode_double ( stream,&header.northing_max );

	read_and_decode_ushort ( stream,&header.usn_vector_records );
	read_and_decode_int ( stream,&header.n_vector_record_points );
	read_and_decode_int ( stream,&header.m_46 );
	read_and_decode_int ( stream,&header.m_4a );
	read_and_decode_ushort ( stream,&header.usn_point3d_records );
	read_and_decode_int ( stream,&header.m_50 );
	read_and_decode_int ( stream,&header.m_54 );
	read_and_decode_ushort ( stream,&header.usn_point2d_records );
	read_and_decode_ushort ( stream,&header.m_5a );
	read_and_decode_ushort ( stream,&header.m_5c );
	read_and_decode_ushort ( stream,&header.usn_feature_records );

	read_and_decode_int ( stream,&header.m_60 );
	read_and_decode_int ( stream,&header.m_64 );
	read_and_decode_ushort ( stream,&header.m_68 );
	read_and_decode_ushort ( stream,&header.m_6a );
	read_and_decode_ushort ( stream,&header.m_6c );
	read_and_decode_int ( stream,&header.m_nrelated_object_pointers );

	read_and_decode_int ( stream,&header.m_72 );
	read_and_decode_ushort ( stream,&header.m_76 );

	read_and_decode_int ( stream,&header.m_78 );
	read_and_decode_int ( stream,&header.m_7c );


	//    Calculate and record the cell coordinate transform coefficients


	double delta_x = header.easting_max - header.easting_min;
	if ( delta_x < 0 )
		delta_x += CM93_semimajor_axis_meters * 2.0 * M_PI;              // add one trip around

	pCIB->transform_x_rate = delta_x / 65535;
	pCIB->transform_y_rate = ( header.northing_max - header.northing_min ) / 65535;

	//    Force all transforms to produce positive longitude only
	pCIB->transform_x_origin = header.easting_min;
	if ( pCIB->transform_x_origin < 0 )
		pCIB->transform_x_origin += CM93_semimajor_axis_meters * 2.0 * M_PI;              // add one trip around
	pCIB->transform_y_origin = header.northing_min;

	//      pCIB->m_cell_mcovr_array.Empty();

	//    Extract some table sizes from the header, and pre-allocate the tables
	//    We do it this way to avoid incremental realloc() calls, which are expensive

	pCIB->m_nfeature_records = header.usn_feature_records;
	pCIB->pobject_block = ( Object * ) calloc ( pCIB->m_nfeature_records * sizeof ( Object ), 1 );

	pCIB->m_n_point2d_records = header.usn_point2d_records;
	pCIB->p2dpoint_array = ( cm93_point * ) malloc ( pCIB->m_n_point2d_records * sizeof ( cm93_point ) );

	pCIB->pprelated_object_block = ( Object ** ) malloc ( header.m_nrelated_object_pointers * sizeof ( Object * ) );

	pCIB->object_vector_record_descriptor_block = ( vector_record_descriptor * ) malloc ( ( header.m_4a + header.m_46 ) * sizeof ( vector_record_descriptor ) );

	pCIB->attribute_block_top = ( unsigned char * ) calloc ( header.m_78, 1 );

	pCIB->m_nvector_records = header.usn_vector_records;
	pCIB->edge_vector_descriptor_block = ( geometry_descriptor * ) malloc ( header.usn_vector_records * sizeof ( geometry_descriptor ) );

	pCIB->pvector_record_block_top = ( cm93_point * ) malloc ( header.n_vector_record_points * sizeof ( cm93_point ) );

	pCIB->m_n_point3d_records = header.usn_point3d_records;
	pCIB->point3d_descriptor_block = ( geometry_descriptor * ) malloc ( pCIB->m_n_point3d_records * sizeof ( geometry_descriptor ) );

	pCIB->p3dpoint_array = ( cm93_point_3d * ) malloc ( header.m_50 * sizeof ( cm93_point_3d ) );

	return true;
}

static bool read_vector_record_table ( FILE *stream, int count, Cell_Info_Block *pCIB )
{
	bool brv;

	geometry_descriptor *p = pCIB->edge_vector_descriptor_block;
	cm93_point *q = pCIB->pvector_record_block_top;

	for ( int iedge=0 ; iedge < count ; iedge++ )
	{

		p->index = iedge;

		unsigned short npoints;
		brv = ! ( read_and_decode_ushort ( stream, &npoints ) == 0 );
		if ( !brv )
			return false;

		p->n_points = npoints;
		p->p_points = q;

		//           brv = read_and_decode_bytes(stream, q, p->n_points * sizeof(cm93_point));
		//            if(!brv)
		//                  return false;

		unsigned short x, y;
		for ( int index = 0 ; index <  p->n_points ; index++ )
		{
			if ( !read_and_decode_ushort ( stream, &x ) )
				return false;
			if ( !read_and_decode_ushort ( stream, &y ) )
				return false;

			q[index].x = x;
			q[index].y = y;
		}


		//    Compute and store the min/max of this block of n_points
		cm93_point *t = p->p_points;

		p->x_max = t->x;
		p->x_min = t->x;
		p->y_max = t->y;
		p->y_min = t->y;

		t++;

		for ( int j=0 ; j < p->n_points-1 ; j++ )
		{
			if ( t->x >= p->x_max )
				p->x_max = t->x;

			if ( t->x <= p->x_min )
				p->x_min = t->x;

			if ( t->y >= p->y_max )
				p->y_max = t->y;

			if ( t->y <= p->y_max )
				p->y_min = t->y;

			t++;
		}

		//    Advance the block pointer
		q += p->n_points;

		//    Advance the geometry descriptor pointer
		p++;

	}

	return true;
}


static bool read_3dpoint_table ( FILE *stream, int count, Cell_Info_Block *pCIB )
{
	geometry_descriptor *p = pCIB->point3d_descriptor_block;
	cm93_point_3d *q = pCIB->p3dpoint_array;

	for ( int i = 0 ; i < count ; i++ )
	{
		unsigned short npoints;
		if ( !read_and_decode_ushort ( stream, &npoints ) )
			return false;

		p->n_points = npoints;
		p->p_points = ( cm93_point * ) q;       // might not be the right cast

		//            unsigned short t = p->n_points;

		//            if(!read_and_decode_bytes(stream, q, t*6))
		//                  return false;

		unsigned short x, y, z;
		for ( int index = 0 ; index < p->n_points ; index++ )
		{
			if ( !read_and_decode_ushort ( stream, &x ) )
				return false;
			if ( !read_and_decode_ushort ( stream, &y ) )
				return false;
			if ( !read_and_decode_ushort ( stream, &z ) )
				return false;

			q[index].x = x;
			q[index].y = y;
			q[index].z = z;
		}


		p++;
		q++;
	}

	return true;
}

static bool read_2dpoint_table(FILE* stream, int count, Cell_Info_Block* pCIB)
{
	unsigned short x;
	unsigned short y;
	for (int index = 0; index < count; index++) {
		if (!read_and_decode_ushort(stream, &x))
			return false;
		if (!read_and_decode_ushort(stream, &y))
			return false;

		pCIB->p2dpoint_array[index].x = x;
		pCIB->p2dpoint_array[index].y = y;
	}

	return true;
}

static bool read_feature_record_table ( FILE *stream, int n_features, Cell_Info_Block *pCIB )
{
	try
	{

		Object *pobj = pCIB->pobject_block;                // head of object array

		vector_record_descriptor *pobject_vector_collection = pCIB->object_vector_record_descriptor_block;

		Object **p_relob = pCIB->pprelated_object_block;             // head of previously allocated related object pointer block

		unsigned char *puc_var10 = pCIB->attribute_block_top;       //m_3a;
		int puc10count = 0;                 // should be same as header.m_78

		unsigned char object_type;
		unsigned char geom_prim;
		unsigned short obj_desc_bytes = 0;

		unsigned int t;
		unsigned short index;
		unsigned short n_elements;


		for ( int iobject = 0 ; iobject < n_features ; iobject++ )
		{

			// read the object definition
			read_and_decode_bytes ( stream, &object_type, 1 );           // read the object type
			read_and_decode_bytes ( stream, &geom_prim, 1 );             // read the object geometry primitive type
			read_and_decode_ushort ( stream, &obj_desc_bytes );          // read the object byte count

			pobj->otype = object_type;
			pobj->geotype = geom_prim;


			switch ( pobj->geotype & 0x0f )
			{
				case 4:              // AREA
					{

						if ( !read_and_decode_ushort ( stream, &n_elements ) )
							return false;

						pobj->n_geom_elements = n_elements;
						t = ( pobj->n_geom_elements * 2 ) + 2;
						obj_desc_bytes -= t;

						pobj->pGeometry = pobject_vector_collection;           // save pointer to created vector_record_descriptor in the object

						for ( unsigned short i = 0 ; i < pobj->n_geom_elements ; i++ )
						{
							if ( !read_and_decode_ushort ( stream, &index ) )
								return false;

							if ( ( index & 0x1fff ) > pCIB->m_nvector_records )
								return false;               // error in this cell, ignore all of it

							geometry_descriptor *u = &pCIB->edge_vector_descriptor_block[ ( index & 0x1fff ) ];   //point to the vector descriptor

							pobject_vector_collection->pGeom_Description = u;
							pobject_vector_collection->segment_usage = ( unsigned char ) ( index >> 13 );

							pobject_vector_collection++;
						}

						break;
					}                                   // AREA geom



				case 2:                                         // LINE geometry
					{

						if ( !read_and_decode_ushort ( stream, &n_elements ) )      // read geometry element count
							return false;

						pobj->n_geom_elements = n_elements;
						t = ( pobj->n_geom_elements * 2 ) + 2;
						obj_desc_bytes -= t;

						pobj->pGeometry = pobject_vector_collection;                     // save pointer to created vector_record_descriptor in the object

						for ( unsigned short i = 0 ; i < pobj->n_geom_elements ; i++ )
						{
							unsigned short geometry_index;

							if ( !read_and_decode_ushort ( stream, &geometry_index ) )
								return false;


							if ( ( geometry_index & 0x1fff ) > pCIB->m_nvector_records )
								//                                    *(int *)(0) = 0;                              // error
								return 0;               // error, bad pointer

							geometry_descriptor *u = &pCIB->edge_vector_descriptor_block[ ( geometry_index & 0x1fff ) ];  //point to the vector descriptor

							pobject_vector_collection->pGeom_Description = u;
							pobject_vector_collection->segment_usage = ( unsigned char ) ( geometry_index >> 13 );

							pobject_vector_collection++;
						}

						break;


					}

				case 1:
					{
						if ( !read_and_decode_ushort ( stream, &index ) )
							return false;

						obj_desc_bytes -= 2;

						pobj->n_geom_elements = 1;                 // one point

						pobj->pGeometry = &pCIB->p2dpoint_array[index];            //cm93_point *

						break;
					}

				case 8:
					{
						if ( !read_and_decode_ushort ( stream, &index ) )
							return false;
						obj_desc_bytes -= 2;

						pobj->n_geom_elements = 1;                 // one point

						pobj->pGeometry = &pCIB->point3d_descriptor_block[index];          //geometry_descriptor *

						break;
					}

			}           // switch



			if ( ( pobj->geotype & 0x10 ) == 0x10 )        // children/related
			{
				unsigned char nrelated;
				if ( !read_and_decode_bytes ( stream, &nrelated, 1 ) )
					return false;

				pobj->n_related_objects = nrelated;
				t = ( pobj->n_related_objects * 2 ) + 1;
				obj_desc_bytes -= t;

				pobj->p_related_object_pointer_array = p_relob;
				p_relob += pobj->n_related_objects;

				Object **w = ( Object ** ) pobj->p_related_object_pointer_array;
				for ( unsigned char j = 0 ; j < pobj->n_related_objects ; j++ )
				{
					if ( !read_and_decode_ushort ( stream, &index ) )
						return false;

					if ( index > pCIB->m_nfeature_records )
						//                              *(int *)(0) = 0;                              // error
						return false;

					Object *prelated_object = &pCIB->pobject_block[index];
					*w = prelated_object;                       // fwd link

					prelated_object->p_related_object_pointer_array = pobj;              // back link, array of 1 element
					w++;
				}
			}

			if ( ( pobj->geotype & 0x20 ) == 0x20 )
			{
				unsigned short nrelated;
				if ( !read_and_decode_ushort ( stream, &nrelated ) )
					return false;

				pobj->n_related_objects = ( unsigned char ) ( nrelated & 0xFF );
				obj_desc_bytes -= 2;
			}

			if ( ( pobj->geotype & 0x80 ) == 0x80 )        // attributes
			{

				unsigned char nattr;
				if ( !read_and_decode_bytes ( stream, &nattr, 1 ) )
					return false;        //m_od

				pobj->n_attributes = nattr;
				obj_desc_bytes -= 5;

				pobj->attributes_block = puc_var10;
				puc_var10 += obj_desc_bytes;

				puc10count += obj_desc_bytes;


				if ( !read_and_decode_bytes ( stream, pobj->attributes_block, obj_desc_bytes ) )
					return false;           // the attributes....
			}
			pobj++;                       // next object
		}
	}

	catch ( ... )
	{
		printf ( "catch on read_feature_record_table\n" );
	}

	return true;
}

static bool Ingest_CM93_Cell(const char* cell_file_name, Cell_Info_Block* pCIB) // FIXME
{
	try
	{
		int file_length;

		// Get the file length
		FILE* flstream = fopen(cell_file_name, "rb");
		if (!flstream)
			return false;

		fseek(flstream, 0, SEEK_END);
		file_length = ftell(flstream);
		fclose(flstream);

		// Open the file
		FILE* stream = fopen(cell_file_name, "rb");
		if (!stream)
			return false;

		// Validate the integrity of the cell file

		unsigned short word0 = 0;
		int int0 = 0;
		int int1 = 0;

		read_and_decode_ushort ( stream, &word0 );     // length of prolog + header (10 + 128)
		read_and_decode_int ( stream, &int0 );         // length of table 1
		read_and_decode_int ( stream, &int1 );         // length of table 2

		int test = word0 + int0 + int1;
		if (test != file_length) {
			fclose ( stream );
			return false;                           // file is corrupt
		}

		// Cell is OK, proceed to ingest

		if (!read_header_and_populate_cib(stream, pCIB)) {
			fclose(stream);
			return false;
		}

		if (!read_vector_record_table(stream, pCIB->m_nvector_records, pCIB)) {
			fclose(stream);
			return false;
		}

		if (!read_3dpoint_table(stream, pCIB->m_n_point3d_records, pCIB)) {
			fclose(stream);
			return false;
		}

		if (!read_2dpoint_table(stream, pCIB->m_n_point2d_records, pCIB)) {
			fclose(stream);
			return false;
		}

		if (!read_feature_record_table(stream, pCIB->m_nfeature_records, pCIB)) {
			fclose(stream);
			return false;
		}

		fclose(stream);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

cm93chart::cm93chart()
{
	m_CIB = new Cell_Info_Block;

	m_ChartType = CHART_TYPE_CM93;

	// Create the decode table once, if needed
	if (!cm93_decode_table_created) {
		CreateDecodeTable();
		cm93_decode_table_created = true;
	}

	m_pDict = NULL;
	m_pManager = NULL;

	m_current_cell_vearray_offset = 0;

	m_ncontour_alloc = 100; // allocate inital vertex count container array
	m_pcontour_array = (int*)malloc(m_ncontour_alloc * sizeof(int));

	// Establish a common reference point for the cell
	ref_lat = 0.0;
	ref_lon = 0.0;

	// Need a covr_set
	m_pcovr_set = new covr_set(this);

	// Make initial allocation of shared outline drawing buffer
	m_pDrawBuffer = (wxPoint*)malloc(4 * sizeof(wxPoint));
	m_nDrawBufferSize = 1;
}

cm93chart::~cm93chart()
{
	free(m_pcontour_array);
	delete m_pcovr_set;
	free(m_pDrawBuffer);
	if (m_CIB) {
		delete m_CIB;
		m_CIB = NULL;
	}
}

void cm93chart::Unload_CM93_Cell(void)
{
	free(m_CIB->pobject_block);
	free(m_CIB->p2dpoint_array);
	free(m_CIB->pprelated_object_block);
	free(m_CIB->object_vector_record_descriptor_block);
	free(m_CIB->attribute_block_top);
	free(m_CIB->edge_vector_descriptor_block);
	free(m_CIB->pvector_record_block_top);
	free(m_CIB->point3d_descriptor_block);
	free(m_CIB->p3dpoint_array);

	delete m_CIB;
	m_CIB = new Cell_Info_Block;
}


// The idea here is to suggest to upper layers the appropriate scale values to be used with this chart
// If max is too large, performance suffers, and the charts are very cluttered onscreen.
// If the min is too small, then the chart rendereding will be over-scaled, and accuracy suffers.
// In some ways, this is subjective.....

double cm93chart::GetNormalScaleMin(double /*canvas_scale_factor*/, bool /*b_allow_overzoom*/)
{
	switch ( GetNativeScale() )
	{
		case 20000000: return 3000000.0; // Z
		case  3000000: return 1000000.0; // A
		case  1000000: return 200000.0;  // B
		case   200000: return 40000.0;   // C
		case   100000: return 20000.0;   // D
		case    50000: return 10000.0;   // E
		case    20000: return 5000.0;    // F
		case     7500: return 3500.0;    // G
	}

	return 1.0;
}

double cm93chart::GetNormalScaleMax(double /*canvas_scale_factor*/)
{
	switch ( GetNativeScale() )
	{
		case 20000000: return 50000000.0; // Z
		case  3000000: return 6000000.0;  // A
		case  1000000: return 2000000.0;  // B
		case   200000: return 400000.0;   // C
		case   100000: return 200000.0;   // D
		case    50000: return 100000.0;   // E
		case    20000: return 40000.0;    // F
		case     7500: return 15000.0;    // G
	}

	return 1.0e7;
}

void cm93chart::GetPointPix(ObjRazRules* rzRules, float north, float east, wxPoint* r)
{
	using geo::BoundingBox;

	S57Obj* obj = rzRules->obj;

	double valx = (east * obj->x_rate) + obj->x_origin;
	double valy = (north * obj->y_rate) + obj->y_origin;

	//    Crossing Greenwich right
	if (m_vp_current.GetBBox().GetMaxX() > 360.0) {
		BoundingBox bbRight(0.0, m_vp_current.GetBBox().GetMinY(),
							m_vp_current.GetBBox().GetMaxX() - 360.,
							m_vp_current.GetBBox().GetMaxY());
		if (bbRight.Intersect(rzRules->obj->BBObj, 0) != BoundingBox::_OUT) {
			valx += geo::mercator_k0 * geo::WGS84_semimajor_axis_meters * 2.0 * M_PI; // 6375586.0;
		}
	}

	r->x = (int)wxRound(((valx - m_easting_vp_center) * m_view_scale_ppm) + m_pixx_vp_center);
	r->y = (int)wxRound(m_pixy_vp_center - ((valy - m_northing_vp_center) * m_view_scale_ppm));
}

void cm93chart::GetPointPix(ObjRazRules* rzRules, wxPoint2DDouble* en, wxPoint* r, int nPoints)
{
	using geo::BoundingBox;

	S57Obj* obj = rzRules->obj;

	double xr = obj->x_rate;
	double xo = obj->x_origin;
	double yr = obj->y_rate;
	double yo = obj->y_origin;

	//    Crossing Greenwich right
	if (m_vp_current.GetBBox().GetMaxX() > 360.0) {
		BoundingBox bbRight(0.0, m_vp_current.GetBBox().GetMinY(),
							m_vp_current.GetBBox().GetMaxX() - 360.0,
							m_vp_current.GetBBox().GetMaxY());
		if (bbRight.Intersect(rzRules->obj->BBObj, 0) != BoundingBox::_OUT) {
			xo += geo::mercator_k0 * geo::WGS84_semimajor_axis_meters * 2.0 * M_PI;
		}
	}

	for (int i = 0; i < nPoints; ++i) {
		double valx = (en[i].m_x * xr) + xo;
		double valy = (en[i].m_y * yr) + yo;
		r[i].x = (int)wxRound(((valx - m_easting_vp_center) * m_view_scale_ppm) + m_pixx_vp_center);
		r[i].y
			= (int)wxRound(m_pixy_vp_center - ((valy - m_northing_vp_center) * m_view_scale_ppm));
	}
}

void cm93chart::GetPixPoint ( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
	// Use Mercator estimator
	int dx = pixx - ( vpt->pix_width / 2 );
	int dy = ( vpt->pix_height / 2 ) - pixy;

	double xp = ( dx * cos ( vpt->skew ) ) - ( dy * sin ( vpt->skew ) );
	double yp = ( dy * cos ( vpt->skew ) ) + ( dx * sin ( vpt->skew ) );

	double d_east = xp / vpt->view_scale_ppm;
	double d_north = yp / vpt->view_scale_ppm;

	double slat, slon;
	geo::fromSM(d_east, d_north, vpt->clat, vpt->clon, &slat, &slon);

	if ( slon > 360.0)
		slon -= 360.0;

	*plat = slat;
	*plon = slon;
}

bool cm93chart::AdjustVP ( ViewPort &vp_last, ViewPort &vp_proposed )
{
	if (IsCacheValid()) {
		// If this viewpoint is same scale as last...
		if (vp_last.view_scale_ppm == vp_proposed.view_scale_ppm) {

			double prev_easting_c, prev_northing_c;
			geo::toSM(vp_last.clat, vp_last.clon, ref_lat, ref_lon, &prev_easting_c, &prev_northing_c);

			double easting_c, northing_c;
			geo::toSM(vp_proposed.clat, vp_proposed.clon,  ref_lat, ref_lon, &easting_c, &northing_c);

			//  then require this viewport to be exact integral pixel difference from last
			//  adjusting clat/clat and SM accordingly

			double delta_pix_x = ( easting_c - prev_easting_c ) * vp_proposed.view_scale_ppm;
			int dpix_x = ( int ) round(delta_pix_x);
			double dpx = dpix_x;

			double delta_pix_y = ( northing_c - prev_northing_c ) * vp_proposed.view_scale_ppm;
			int dpix_y = ( int ) round(delta_pix_y);
			double dpy = dpix_y;

			double c_east_d = ( dpx / vp_proposed.view_scale_ppm ) + prev_easting_c;
			double c_north_d = ( dpy / vp_proposed.view_scale_ppm ) + prev_northing_c;

			double xlat, xlon;
			geo::fromSM(c_east_d, c_north_d, ref_lat, ref_lon, &xlat, &xlon);

			vp_proposed.clon = xlon;
			vp_proposed.clat = xlat;

			return true;
		}
	}

	return false;
}

void cm93chart::SetVPParms ( const ViewPort &vpt )
{
	// Save a copy for later reference

	m_vp_current = vpt;

	//  Set up local SM rendering constants
	m_pixx_vp_center = vpt.pix_width / 2;
	m_pixy_vp_center = vpt.pix_height / 2;
	m_view_scale_ppm = vpt.view_scale_ppm;

	geo::toSM(vpt.clat, vpt.clon, ref_lat, ref_lon, &m_easting_vp_center, &m_northing_vp_center);


	if (g_bDebugCM93)
	{
		//    Fetch the lat/lon of the screen corner points
		const geo::LatLonBoundingBox & box = vpt.GetBBox();
		double ll_lon = box.GetMinX();
		double ll_lat = box.GetMinY();

		double ur_lon = box.GetMaxX();
		double ur_lat = box.GetMaxY();

		printf ( "cm93chart::SetVPParms   ll_lon: %g  ll_lat: %g   ur_lon: %g   ur_lat:  %g  m_dval: %g\n", ll_lon, ll_lat, ur_lon, ur_lat, m_dval );
	}


	//    Create an array of CellIndexes covering the current viewport
	std::vector<int> vpcells = GetVPCellArray ( vpt );

	//    Check the member array to see if all these viewport cells have been loaded
	bool bcell_is_in;

	for ( unsigned int i=0 ; i < vpcells.size() ; i++ )
	{
		bcell_is_in = false;
		for ( unsigned int j=0 ; j < m_cells_loaded_array.size() ; j++ )
		{
			if (vpcells[i] == m_cells_loaded_array[j]) {
				bcell_is_in = true;
				break;
			}
		}

		//    The cell is not in place, so go load it
		if ( !bcell_is_in )
		{
			int cell_index = vpcells[i];

			if (loadcell_in_sequence(cell_index, '0')) // Base cell
			{
				ProcessVectorEdges();
				CreateObjChain(cell_index, ( int ) '0');
				ForceEdgePriorityEvaluate();              // need to re-evaluate priorities
				BuildDepthContourArray();
				m_cells_loaded_array.push_back(cell_index);
				Unload_CM93_Cell();
			}

			char loadcell_key = 'A';               // starting

			//    Load any subcells in sequence
			//    On successful load, add it to the member list and process the cell
			while ( loadcell_in_sequence ( cell_index, loadcell_key ) )
			{
				ProcessVectorEdges();
				CreateObjChain(cell_index, ( int ) loadcell_key);

				ForceEdgePriorityEvaluate();              // need to re-evaluate priorities

				if (m_cells_loaded_array.end() == std::find(m_cells_loaded_array.begin(), m_cells_loaded_array.end(), cell_index))
					m_cells_loaded_array.push_back(cell_index);

				Unload_CM93_Cell();
				loadcell_key++;
			}
		}
	}
	if( s_b_busy_shown){
		::wxEndBusyCursor();
		s_b_busy_shown = false;
	}
}


std::vector<int> cm93chart::GetVPCellArray(const ViewPort &vpt)
{
	//    Fetch the lat/lon of the screen corner points
	const geo::LatLonBoundingBox & box = vpt.GetBBox();
	double ll_lon = box.GetMinX();
	double ll_lat = box.GetMinY();

	double ur_lon = box.GetMaxX();
	double ur_lat = box.GetMaxY();

	//    Adjust to always positive for easier cell calculations
	if ( ll_lon < 0 )
	{
		ll_lon += 360;
		ur_lon += 360;
	}

	//    Create an array of CellIndexes covering the current viewport
	std::vector<int> vpcells;

	int lower_left_cell = Get_CM93_CellIndex ( ll_lat, ll_lon, GetNativeScale() );
	vpcells.push_back(lower_left_cell);                // always add the lower left cell

	if ( g_bDebugCM93 )
		printf ( "cm93chart::GetVPCellArray   Adding %d\n", lower_left_cell );

	double rlat, rlon;
	Get_CM93_Cell_Origin ( lower_left_cell, GetNativeScale(), &rlat, &rlon );


	// Use exact integer math here
	//    It is more obtuse, but it removes dependency on FP rounding policy

	int loni_0 = ( int ) wxRound ( rlon * 3 );
	int loni_20 = loni_0 + ( int ) m_dval;            // already added the lower left cell
	int lati_20 = ( int ) wxRound ( rlat * 3 );


	while ( lati_20 < ( ur_lat * 3. ) )
	{
		while ( loni_20 < ( ur_lon * 3. ) )
		{
			unsigned int next_lon = loni_20 + 1080;
			while ( next_lon >= 1080 )
				next_lon -= 1080;

			unsigned int next_cell = next_lon;

			next_cell += ( lati_20 + 270 ) * 10000;

			vpcells.push_back(next_cell);
			if ( g_bDebugCM93 )
				printf ( "cm93chart::GetVPCellArray   Adding %d\n", next_cell );

			loni_20 += ( int ) m_dval;
		}
		lati_20 += ( int ) m_dval;
		loni_20 = loni_0;
	}

	return vpcells;
}



void cm93chart::ProcessVectorEdges(void)
{
	// Create the vector(edge) map for this cell, appending to the existing member hash map

	VE_Hash & vehash = Get_ve_hash();

	m_current_cell_vearray_offset = vehash.size(); // keys start at the current size
	geometry_descriptor *pgd = m_CIB->edge_vector_descriptor_block;

	for (int iedge = 0 ; iedge < m_CIB->m_nvector_records ; ++iedge) {
		VE_Element * vep = new VE_Element(iedge + m_current_cell_vearray_offset, pgd->n_points);
		if (pgd->n_points) {
			double * pPoints = new double[pgd->n_points * 2];
			vep->pPoints = pPoints;

			cm93_point *ppt = pgd->p_points;
			for (int ip = 0; ip < pgd->n_points; ++ip) {
				*pPoints++ = ppt->x;
				*pPoints++ = ppt->y;
				ppt++;
			}
		}

		vehash[vep->index] = vep;
		pgd++; // next geometry descriptor
	}
}

int cm93chart::CreateObjChain(int cell_index, int subcell)
{
	LUPrec* LUP;
	LUPname LUP_Name = PAPER_CHART;

	m_CIB->m_cell_mcovr_list.clear();

	Object *pobjectDef = m_CIB->pobject_block;           // head of object array
	m_CIB->b_have_offsets = false;                       // will be set if any M_COVRs in this cell have defined, non-zero WGS84 offsets
	m_CIB->b_have_user_offsets = false;                  // will be set if any M_COVRs in this cell have user defined offsets

	int iObj = 0;
	S57Obj *obj;

	while ( iObj < m_CIB->m_nfeature_records )
	{
		if ( ( pobjectDef != NULL ) )
		{
			geo::ExtendedGeometry *xgeom = BuildGeom ( pobjectDef, NULL, iObj );

			obj = NULL;
			if ( NULL != xgeom )
				obj = CreateS57Obj ( cell_index, iObj, subcell, pobjectDef, m_pDict, xgeom, ref_lat, ref_lon, GetNativeScale() );

			if ( obj )
			{

				//      Build/Maintain the ATON floating/rigid arrays
				if ( GEO_POINT == obj->Primitive_type )
				{

					// set floating platform
					if ( ( !strncmp ( obj->FeatureName, "LITFLT", 6 ) ) ||
							( !strncmp ( obj->FeatureName, "LITVES", 6 ) ) ||
							( !strncmp ( obj->FeatureName, "BOY",    3 ) ) )
					{
						pFloatingATONArray->Add ( obj );
					}

					// set rigid platform
					if ( !strncmp ( obj->FeatureName, "BCN",    3 ) )
						pRigidATONArray->Add ( obj );


					//    Mark the object as an ATON
					if ( ( !strncmp ( obj->FeatureName,   "LIT",    3 ) ) ||
							( !strncmp ( obj->FeatureName, "LIGHTS", 6 ) ) ||
							( !strncmp ( obj->FeatureName, "BCN",    3 ) ) ||
							( !strncmp ( obj->FeatureName, "_slgto", 6 ) ) ||
							( !strncmp ( obj->FeatureName, "_boygn", 6 ) ) ||
							( !strncmp ( obj->FeatureName, "_bcngn", 6 ) ) ||
							( !strncmp ( obj->FeatureName, "_extgn", 6 ) ) ||
							( !strncmp ( obj->FeatureName, "TOWERS", 6 ) ) ||
							( !strncmp ( obj->FeatureName, "BOY",    3 ) ) )
					{
						obj->bIsAton = true;
					}
				}

				//    Mark th object as an "associable depth area"
				//    Flag is used by conditional symbology
				if ( GEO_AREA == obj->Primitive_type )
				{
					if ( !strncmp ( obj->FeatureName, "DEPARE", 6 ) || !strncmp ( obj->FeatureName, "DRGARE", 6 ) )
						obj->bIsAssociable = true;
				}


				//      This is where Simplified or Paper-Type point features are selected
				//      In the case where the chart needs alternate LUPS loaded, do so.
				//      This case is triggered when the UpdateLUP() method has been called on a partially loaded chart.

				switch ( obj->Primitive_type )
				{
					case GEO_POINT:
					case GEO_META:
					case GEO_PRIM:
						if ( PAPER_CHART == ps52plib->m_nSymbolStyle )
							LUP_Name = PAPER_CHART;
						else
							LUP_Name = SIMPLIFIED;

						if(m_b2pointLUPS)
						{
							LUPname  LUPO_Name;
							if ( PAPER_CHART == ps52plib->m_nSymbolStyle )
								LUPO_Name = SIMPLIFIED;
							else
								LUPO_Name = PAPER_CHART;

							//  Load the alternate LUP
							LUPrec *LUPO = ps52plib->S52_LUPLookup ( LUPO_Name, obj->FeatureName, obj );
							if( LUPO ) {
								ps52plib->_LUP2rules ( LUPO, obj );
								_insertRules ( obj,LUPO, this );
							}
						}
						break;

					case GEO_LINE:
						LUP_Name = LINES;
						break;

					case GEO_AREA:
						if ( PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle )
							LUP_Name = PLAIN_BOUNDARIES;
						else
							LUP_Name = SYMBOLIZED_BOUNDARIES;

						if(m_b2lineLUPS)
						{
							LUPname  LUPO_Name;
							if ( PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle )
								LUPO_Name = SYMBOLIZED_BOUNDARIES;
							else
								LUPO_Name = PLAIN_BOUNDARIES;

							//  Load the alternate LUP
							LUPrec *LUPO = ps52plib->S52_LUPLookup ( LUPO_Name, obj->FeatureName, obj );
							if( LUPO ) {
								ps52plib->_LUP2rules ( LUPO, obj );
								_insertRules ( obj,LUPO, this );
							}
						}
						break;
				}

				LUP = ps52plib->S52_LUPLookup ( LUP_Name, obj->FeatureName, obj );

				if ( NULL == LUP )
				{
					if ( g_bDebugCM93 )
					{
						wxString msg ( obj->FeatureName, wxConvUTF8 );
						msg.Prepend ( _T ( "   CM93 could not find LUP for " ) );
						LogMessageOnce ( msg );
					}
					if(0 == obj->nRef)
						delete obj;
				}
				else
				{
					// Convert LUP to rules set
					ps52plib->_LUP2rules ( LUP, obj );

					// Add linked object/LUP to the working set
					_insertRules ( obj,LUP, this );

					// Establish Object's Display Category
					obj->m_DisplayCat = LUP->DISC;
				}
			}
		} else
			break;

		pobjectDef++;

		iObj++;
	}
	return 1;
}

InitReturn cm93chart::Init(const wxString& name, ChartInitFlag flags)
{
	m_FullPath = name;
	m_Description = m_FullPath;

	wxFileName fn(name);

	if (!m_prefix.Len())
		m_prefix = fn.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

	m_scalechar = fn.GetExt();

	// Figure out the scale from the file name

	int scale;
	switch ( ( m_scalechar.mb_str() ) [ ( size_t ) 0] )
	{
		case 'Z': scale = 20000000;  break;
		case 'A': scale =  3000000;  break;
		case 'B': scale =  1000000;  break;
		case 'C': scale =   200000;  break;
		case 'D': scale =   100000;  break;
		case 'E': scale =    50000;  break;
		case 'F': scale =    20000;  break;
		case 'G': scale =     7500;  break;
		default:  scale = 20000000;  break;
	}

	m_Chart_Scale = scale;


	switch ( GetNativeScale() )
	{
		case 20000000: m_dval = 120; break; // Z
		case  3000000: m_dval =  60; break; // A
		case  1000000: m_dval =  30; break; // B
		case   200000: m_dval =  12; break; // C
		case   100000: m_dval =   3; break; // D
		case    50000: m_dval =   1; break; // E
		case    20000: m_dval =   1; break; // F
		case     7500: m_dval =   1; break; // G
		default: m_dval =   1; break;
	}

	// Set the nice name
	wxString data = _T ( "CM93Chart " );
	data.Append(m_scalechar);
	wxString s;
	s.Printf(_T ( "  1/%d" ), m_Chart_Scale);
	data.Append(s);
	m_Name = data;

	// Initialize the covr_set
	if (scale != 20000000)
		m_pcovr_set->Init(m_scalechar.mb_str()[(size_t)0], m_prefix);

	if (flags == THUMB_ONLY) {
		//            SetColorScheme(cs, false);

		return INIT_OK;
	}

	if (!m_pManager)
		m_pManager = new chart::cm93manager;

	if (flags == HEADER_ONLY)
		return CreateHeaderDataFromCM93Cell();

	// Load the cm93 dictionary if necessary
	if (!m_pDict) {
		if (m_pManager) {
			if (m_pManager->Loadcm93Dictionary(name))
				m_pDict = m_pManager->m_pcm93Dict;
			else {
				wxLogMessage(_T ( "   CM93Chart Init cannot locate CM93 dictionary." ));
				return INIT_FAIL_REMOVE;
			}
		}
	}

	bReadyToRender = true;

	return INIT_OK;
}

geo::ExtendedGeometry* cm93chart::BuildGeom(Object* pobject, wxFileOutputStream* /*postream*/,
											int iobject)
{
	wxString s;
	int geomtype;

	int geom_type_maybe = pobject->geotype;

	switch ( geom_type_maybe )
	{
		case 1:    geomtype = 1; break;
		case 2:    geomtype = 2; break;
		case 4:    geomtype = 3; break;
		case 129:  geomtype = 1; break;
		case 130:  geomtype = 2; break;
		case 132:  geomtype = 3; break;
		case 8:    geomtype = 8; break;
		case 16:   geomtype = 16; break;
		case 161:  geomtype = 1; break;    // lighthouse first child
		case 33:   geomtype = 1; break;
		default:
				   geomtype = -1; break;
	}

	int iseg;

	geo::ExtendedGeometry *ret_ptr = new geo::ExtendedGeometry;

	int lon_max, lat_max, lon_min, lat_min;
	lon_max = 0; lon_min = 65536; lat_max = 0; lat_min = 65536;

	switch ( geomtype )
	{

		case 3:                               // Areas
			{
				vector_record_descriptor *psegs = ( vector_record_descriptor * ) pobject->pGeometry;

				int nsegs = pobject->n_geom_elements;

				ret_ptr->n_vector_indices = nsegs;
				ret_ptr->pvector_index = ( int * ) malloc ( nsegs * 3 * sizeof ( int ) );

				//Traverse the object once to get a maximum polygon vertex count
				int n_maxvertex = 0;
				for ( int i=0 ; i < nsegs ; i++ )
				{
					geometry_descriptor *pgd = ( geometry_descriptor* ) ( psegs[i].pGeom_Description );
					n_maxvertex += pgd->n_points;
				}

				//TODO  May not need this fluff adder....
				n_maxvertex += 1;       // fluff

				wxPoint2DDouble *pPoints = ( wxPoint2DDouble * ) calloc ( ( n_maxvertex ) * sizeof ( wxPoint2DDouble ), 1 );

				int ip = 1;
				int n_prev_vertex_index = 1;
				bool bnew_ring = true;
				int ncontours = 0;
				iseg = 0;

				cm93_point start_point;
				start_point.x = 0; start_point.y = 0;

				cm93_point cur_end_point;
				cur_end_point.x = 1; cur_end_point.y = 1;

				int n_max_points = -1;
				while ( iseg < nsegs )
				{
					int type_seg = psegs[iseg].segment_usage;

					geometry_descriptor *pgd = ( geometry_descriptor* ) ( psegs[iseg].pGeom_Description );

					int npoints = pgd->n_points;
					cm93_point *rseg = pgd->p_points;

					n_max_points = wxMax ( n_max_points, npoints );

					//    Establish ring starting conditions
					if ( bnew_ring )
					{
						bnew_ring = false;

						if ( ( type_seg & 4 ) == 0 )
							start_point = rseg[0];
						else
							start_point = rseg[npoints-1];
					}



					if ( ( ( type_seg & 4 ) == 0 ) )
					{
						cur_end_point = rseg[npoints-1];
						for ( int j=0 ; j<npoints  ; j++ )
						{
							//                                    if(ncontours == 0)                             // outer ring describes envelope
							{
								lon_max = wxMax ( lon_max, rseg[j].x );
								lon_min = wxMin ( lon_min, rseg[j].x );
								lat_max = wxMax ( lat_max, rseg[j].y );
								lat_min = wxMin ( lat_min, rseg[j].y );
							}

							pPoints[ip].m_x = rseg[j].x;
							pPoints[ip].m_y = rseg[j].y;
							ip++;
						}
					}
					else if ( ( type_seg & 4 ) == 4 )   // backwards
					{
						cur_end_point = rseg[0];
						for ( int j=npoints-1 ; j>= 0  ; j-- )
						{
							//                                    if(ncontours == 0)                             // outer ring describes envelope
							{
								lon_max = wxMax ( lon_max, rseg[j].x );
								lon_min = wxMin ( lon_min, rseg[j].x );
								lat_max = wxMax ( lat_max, rseg[j].y );
								lat_min = wxMin ( lat_min, rseg[j].y );
							}

							pPoints[ip].m_x = rseg[j].x;
							pPoints[ip].m_y = rseg[j].y;
							ip++;
						}
					}

					ip--;                                                 // skip the last point in each segment

					ret_ptr->pvector_index[iseg * 3 + 0] = -1;                 // first connected node
					ret_ptr->pvector_index[iseg * 3 + 1] = pgd->index + m_current_cell_vearray_offset;         // edge index
					ret_ptr->pvector_index[iseg * 3 + 2] = -2;                 // last connected node

					if ( ( cur_end_point.x == start_point.x ) && ( cur_end_point.y == start_point.y ) )
					{
						// done with a ring

						ip++;                                                 // leave in ring closure point

						int nRingVertex = ip - n_prev_vertex_index;

						//    possibly increase contour array size
						if ( ncontours > m_ncontour_alloc - 1 )
						{
							m_ncontour_alloc *= 2;
							int * tmp = m_pcontour_array;
							m_pcontour_array = ( int * ) realloc ( m_pcontour_array, m_ncontour_alloc * sizeof ( int ) );
							if (NULL == tmp)
							{
								free (tmp);
								tmp = NULL;
							}
						}
						m_pcontour_array[ncontours] = nRingVertex;               // store the vertex count

						bnew_ring = true;                                               // set for next ring
						n_prev_vertex_index = ip;
						ncontours++;

					}
					iseg++;
				}           // while iseg


				ret_ptr->n_max_edge_points = n_max_points;

				ret_ptr->n_contours = ncontours;                          // parameters passed to trapezoid tesselator

				ret_ptr->contour_array = ( int * ) malloc ( ncontours * sizeof ( int ) );
				memcpy ( ret_ptr->contour_array, m_pcontour_array, ncontours * sizeof ( int ) );

				ret_ptr->vertex_array = pPoints;
				ret_ptr->n_max_vertex = n_maxvertex;

				ret_ptr->pogrGeom = NULL;

				ret_ptr->xmin = lon_min;
				ret_ptr->xmax = lon_max;
				ret_ptr->ymin = lat_min;
				ret_ptr->ymax = lat_max;

				break;
			}           // case 3



		case 1:     //single points
			{
				cm93_point *pt = ( cm93_point * ) pobject->pGeometry;
				ret_ptr->pogrGeom = NULL; //t;

				ret_ptr->pointx = pt->x;
				ret_ptr->pointy = pt->y;
				break;
			}

		case 2:                                               // LINE geometry
			{
				vector_record_descriptor *psegs = ( vector_record_descriptor * ) pobject->pGeometry;

				int nsegs = pobject->n_geom_elements;

				ret_ptr->n_vector_indices = nsegs;
				ret_ptr->pvector_index = ( int * ) malloc ( nsegs * 3 * sizeof ( int ) );

				//    Calculate the number of points
				int n_maxvertex = 0;
				for ( int imseg = 0 ; imseg < nsegs ; imseg++ )
				{
					geometry_descriptor *pgd = ( geometry_descriptor* ) psegs->pGeom_Description;

					n_maxvertex += pgd->n_points;
					psegs++;
				}


				wxPoint2DDouble *pPoints = ( wxPoint2DDouble * ) malloc ( n_maxvertex * sizeof ( wxPoint2DDouble ) );

				psegs = ( vector_record_descriptor * ) pobject->pGeometry;

				int ip = 0;
				int lon_max, lat_max, lon_min, lat_min;
				lon_max = 0; lon_min = 65536; lat_max = 0; lat_min = 65536;
				int n_max_points = -1;

				for ( int iseg = 0 ; iseg < nsegs ; iseg++ )
				{

					int type_seg = psegs->segment_usage;

					geometry_descriptor *pgd = ( geometry_descriptor* ) psegs->pGeom_Description;

					psegs++;          // next segment

					int npoints = pgd->n_points;
					cm93_point *rseg = pgd->p_points;

					n_max_points = wxMax ( n_max_points, npoints );

					if ( ( ( type_seg & 4 ) != 4 ) )
					{
						for ( int j=0 ; j<npoints  ; j++ )
						{
							lon_max = wxMax ( lon_max, rseg[j].x );
							lon_min = wxMin ( lon_min, rseg[j].x );
							lat_max = wxMax ( lat_max, rseg[j].y );
							lat_min = wxMin ( lat_min, rseg[j].y );

							pPoints[ip].m_x = rseg[j].x;
							pPoints[ip].m_y = rseg[j].y;
							ip++;

						}
					}

					else if ( ( type_seg & 4 ) == 4 )   // backwards
					{
						for ( int j=npoints-1 ; j>= 0  ; j-- )
						{
							lon_max = wxMax ( lon_max, rseg[j].x );
							lon_min = wxMin ( lon_min, rseg[j].x );
							lat_max = wxMax ( lat_max, rseg[j].y );
							lat_min = wxMin ( lat_min, rseg[j].y );

							pPoints[ip].m_x = rseg[j].x;
							pPoints[ip].m_y = rseg[j].y;
							ip++;
						}
					}

					ret_ptr->pvector_index[iseg * 3 + 0] = -1;                 // first connected node
					ret_ptr->pvector_index[iseg * 3 + 1] = pgd->index + m_current_cell_vearray_offset;         // edge index
					ret_ptr->pvector_index[iseg * 3 + 2] = -2;                 // last connected node

				}           //for

				ret_ptr->n_max_edge_points = n_max_points;

				ret_ptr->vertex_array = pPoints;
				ret_ptr->n_max_vertex = n_maxvertex;

				ret_ptr->pogrGeom = NULL;

				ret_ptr->xmin = lon_min;
				ret_ptr->xmax = lon_max;
				ret_ptr->ymin = lat_min;
				ret_ptr->ymax = lat_max;

				break;
			}           //case 2  (lines)

		case 8:
			{
				geometry_descriptor *pgd = ( geometry_descriptor* ) pobject->pGeometry;

				int npoints = pgd->n_points;
				cm93_point_3d *rseg = ( cm93_point_3d * ) pgd->p_points;

				OGRMultiPoint *pSMP = new OGRMultiPoint;

				int z;
				double zp;
				for ( int ip=0 ; ip < npoints ; ip++ )
				{
					z = rseg[ip].z;

					//    This is a magic number if there ever was one.....
					if ( z >= 12000 )
						zp = double ( z - 12000 );
					else
						zp = z / 10.;

					OGRPoint *ppoint = new OGRPoint ( rseg[ip].x, rseg[ip].y, zp );
					pSMP->addGeometryDirectly ( ppoint );

					lon_max = wxMax ( lon_max, rseg[ip].x );
					lon_min = wxMin ( lon_min, rseg[ip].x );
					lat_max = wxMax ( lat_max, rseg[ip].y );
					lat_min = wxMin ( lat_min, rseg[ip].y );

				}

				ret_ptr->pogrGeom = pSMP;

				ret_ptr->xmin = lon_min;
				ret_ptr->xmax = lon_max;
				ret_ptr->ymin = lat_min;
				ret_ptr->ymax = lat_max;


				break;
			}


		case 16:
			break;                        // this is the case of objects with children
			// the parent has no geometry.....

		default:
			{
				wxPrintf ( _T ( "Unexpected geomtype %d for Feature %d\n" ), geomtype,iobject );
				break;
			}

	}     // switch


	return ret_ptr;
}

void cm93chart::Transform ( cm93_point *s, double trans_x, double trans_y, double *lat, double *lon )
{
	//    Simple linear transform
	double valx = ( s->x * m_CIB->transform_x_rate ) + m_CIB->transform_x_origin;
	double valy = ( s->y * m_CIB->transform_y_rate ) + m_CIB->transform_y_origin;

	//    Add in the WGS84 offset corrections
	valx -= trans_x;
	valy -= trans_y;

	//    Convert to lat/lon
	*lat = ( 2.0 * atan ( exp ( valy/CM93_semimajor_axis_meters ) ) - M_PI/2.0) / (M_PI / 180.0);
	*lon = ( valx / ( (M_PI / 180.0)* CM93_semimajor_axis_meters ) );

}

void cm93chart::translate_colmar(const wxString& WXUNUSED(sclass), S57attVal* pattValTmp)
{
	int *pcur_attr = ( int * ) pattValTmp->value;
	int cur_attr = *pcur_attr;

	wxString lstring;

	switch ( cur_attr )
	{
		case 1: lstring = _T ( "4" ); break;            // green
		case 2: lstring = _T ( "2" ); break;            // black
		case 3: lstring = _T ( "3" ); break;            // red
		case 4: lstring = _T ( "6" ); break;            // yellow
		case 5: lstring = _T ( "1" ); break;            // white
		case 6: lstring = _T ( "11" ); break;           // orange
		case 7: lstring = _T ( "2,6" ); break;          // black/yellow
		case 8: lstring = _T ( "2,6,2" ); break;        // black/yellow/black
		case 9: lstring = _T ( "6,2" ); break;           // yellow/black
		case 10: lstring = _T ( "6,2,6" ); break;        // yellow/black/yellow
		case 11: lstring = _T ( "3,1" ); break;          // red/white
		case 12: lstring = _T ( "4,3,4" ); break;        // green/red/green
		case 13: lstring = _T ( "3,4,3" ); break;        // red/green/red
		case 14: lstring = _T ( "2,3,2" ); break;        // black/red/black
		case 15: lstring = _T ( "6,3,6" ); break;        // yellow/red/yellow
		case 16: lstring = _T ( "4,3" ); break;          // green/red
		case 17: lstring = _T ( "3,4" ); break;          // red/green
		case 18: lstring = _T ( "4,1" ); break;          // green/white
		default: break;
	}

	if ( lstring.Len() )
	{
		free ( pattValTmp->value );                       // free the old int pointer

		pattValTmp->valType = OGR_STR;
		pattValTmp->value = ( char * ) malloc ( lstring.Len() + 1 );      // create a new Lstring attribute
		strcpy ( ( char * ) pattValTmp->value, lstring.mb_str() );

	}
}


S57Obj * cm93chart::CreateS57Obj(
		int cell_index,
		int iobject,
		int subcell,
		Object * pobject,
		cm93_dictionary * pDict,
		geo::ExtendedGeometry * xgeom,
		double WXUNUSED(ref_lat),
		double WXUNUSED(ref_lon),
		double WXUNUSED(scale))
{

#define MAX_HDR_LINE    4000

	int npub_year = 1993; // silly default

	int iclass = pobject->otype;
	int geomtype = pobject->geotype & 0x0f;

	double tmp_transform_x = 0.;
	double tmp_transform_y = 0.;

	//    Per object transfor offsets,
	double trans_WGS84_offset_x = 0.;
	double trans_WGS84_offset_y = 0.;

	wxString sclass = pDict->GetClassName ( iclass );
	if ( sclass == _T ( "Unknown" ) )
	{
		wxString msg;
		msg.Printf ( _T ( "   CM93 Error...object type %d not found in CM93OBJ.DIC" ), iclass );
		wxLogMessage ( msg );
		return NULL;
	}

	wxString sclass_sub = sclass;

	//  Going to make some substitutions here
	if ( sclass.IsSameAs ( _T ( "ITDARE" ) ) )
		sclass_sub = _T ( "DEPARE" );

	if ( sclass.IsSameAs ( _T ( "_m_sor" ) ) )
		sclass_sub = _T ( "M_COVR" );

	if ( sclass.IsSameAs ( _T ( "SPOGRD" ) ) )
		sclass_sub = _T ( "DMPGRD" );

	if ( sclass.IsSameAs ( _T ( "FSHHAV" ) ) )
		sclass_sub = _T ( "FSHFAC" );

	if ( sclass.IsSameAs ( _T ( "OFSPRD" ) ) )
		sclass_sub = _T ( "CTNARE" );



	//    Create the S57 Object
	S57Obj *pobj = new S57Obj();

	pobj->Index = iobject;

	char u[201];
	strncpy ( u, sclass_sub.mb_str(), 199 );
	u[200] = '\0';
	strncpy ( pobj->FeatureName, u, 7 );

	pobj->attVal =  new wxArrayOfS57attVal();


	cm93_attr_block *pab = new cm93_attr_block ( pobject->attributes_block, pDict );


	for ( int jattr = 0 ; jattr  < pobject->n_attributes ; jattr++ )
	{

		unsigned char *curr_attr = pab->GetNextAttr();

		unsigned char iattr = *curr_attr;

		wxString sattr = pDict->GetAttrName ( iattr );

		char vtype = pDict->GetAttrType ( iattr );

		unsigned char *aval = curr_attr + 1;

		char val[4000];
		int *pi;
		float *pf;
		unsigned short *pw;
		unsigned char *pb;
		int *pAVI;
		char *pAVS;
		double *pAVR;
		int nlen;
		double dival;
		int ival;

		S57attVal *pattValTmp = new S57attVal;

		switch ( vtype )
		{
			case 'I':                           // never seen?
				pi = ( int * ) aval;
				pAVI = ( int * ) malloc ( sizeof ( int ) );         //new int;
				*pAVI = *pi;
				pattValTmp->valType = OGR_INT;
				pattValTmp->value   = pAVI;
				break;
			case 'B':
				pb = ( unsigned char * ) aval;
				pAVI = ( int * ) malloc ( sizeof ( int ) );         //new int;
				*pAVI = ( int ) ( *pb );
				pattValTmp->valType = OGR_INT;
				pattValTmp->value   = pAVI;
				break;
			case 'W':                                       // aWORD10
				pw = ( unsigned short * ) aval;
				ival = ( int ) ( *pw );
				dival = ival;

				pAVR = ( double * ) malloc ( sizeof ( double ) );   //new double;
				*pAVR = dival/10.;
				pattValTmp->valType = OGR_REAL;
				pattValTmp->value   = pAVR;
				break;
			case 'G':
				pi = ( int * ) aval;
				pAVI = ( int * ) malloc ( sizeof ( int ) );         //new int;
				*pAVI = ( int ) ( *pi );
				pattValTmp->valType = OGR_INT;
				pattValTmp->value   = pAVI;
				break;

			case 'S':
				nlen = strlen ( ( const char * ) aval );
				pAVS = ( char * ) malloc ( nlen + 1 );          ;
				strcpy ( pAVS, ( char * ) aval );
				pattValTmp->valType = OGR_STR;
				pattValTmp->value   = pAVS;
				break;

			case 'C':
				nlen = strlen ( ( const char * ) &aval[3] );
				pAVS = ( char * ) malloc ( nlen + 1 );          ;
				strcpy ( pAVS, ( const char * ) &aval[3] );
				pattValTmp->valType = OGR_STR;
				pattValTmp->value   = pAVS;
				break;
			case 'L':
				{
					pb = ( unsigned char * ) aval;
					unsigned char nl = *pb++;
					char vi[20];
					val[0] = 0;
					for ( int i=0 ; i<nl ; i++ )
					{
						sprintf ( vi, "%d,", *pb++ );
						strcat ( val, vi );
					}
					if ( strlen ( val ) )
						val[strlen ( val )-1] = 0;      // strip last ","

					int nlen = strlen ( val );
					pAVS = ( char * ) malloc ( nlen + 1 );          ;
					strcpy ( pAVS, val );
					pattValTmp->valType = OGR_STR;
					pattValTmp->value   = pAVS;
					break;
				}
			case 'R':
				pAVR = ( double * ) malloc ( sizeof ( double ) );   //new double;
				pf = ( float * ) aval;
#ifdef ARMHF
				float tf1;
				memcpy(&tf1, pf, sizeof(float));
				*pAVR = tf1;
#else
				*pAVR = *pf;
#endif
				pattValTmp->valType = OGR_REAL;
				pattValTmp->value   = pAVR;
				break;
			default:
				sattr.Clear();               // Unknown, TODO track occasional case '?'
				break;
		}     // switch


		if ( sattr.IsSameAs ( _T ( "COLMAR" ) ) )
		{
			translate_colmar ( sclass, pattValTmp );
			sattr = _T ( "COLOUR" );
		}



		//    Do CM93 $SCODE attribute substitutions
		if ( sclass.IsSameAs ( _T ( "$AREAS" ) ) && ( vtype == 'S' ) && sattr.IsSameAs ( _T ( "$SCODE" ) ) )
		{
			if ( !strcmp ( ( char * ) pattValTmp->value, "II25" ) )
			{
				free ( pattValTmp->value );
				pattValTmp->value   = ( char * ) malloc ( strlen ( "BACKGROUND" ) + 1 );
				strcpy ( ( char * ) pattValTmp->value, "BACKGROUND" );
			}
		}


		//    Capture some attributes on the fly as needed
		if ( sattr.IsSameAs ( _T ( "RECDAT" ) ) || sattr.IsSameAs ( _T ( "_dgdat" ) ) )
		{
			if ( sclass_sub.IsSameAs ( _T ( "M_COVR" ) ) && ( vtype == 'S' ) )
			{
				wxString pub_date ( ( char * ) pattValTmp->value, wxConvUTF8 );

				wxDateTime upd;
				upd.ParseFormat ( pub_date, _T ( "%Y%m%d" ) );
				if ( !upd.IsValid() )
					upd.ParseFormat ( _T ( "20000101" ), _T ( "%Y%m%d" ) );
				m_EdDate = upd;

				pub_date.Truncate ( 4 );

				long nyear = 0;
				pub_date.ToLong ( &nyear );
				npub_year = nyear;

			}
		}


		//    Capture the potential WGS84 transform offset for later use
		if ( sclass_sub.IsSameAs ( _T ( "M_COVR" ) ) && ( vtype == 'R' ) )
		{
			if ( sattr.IsSameAs ( _T ( "_wgsox" ) ) )
			{
				tmp_transform_x = * ( double * ) pattValTmp->value;
				if ( fabs ( tmp_transform_x ) > 1.0 )                 // metres
					m_CIB->b_have_offsets = true;
			}
			else if ( sattr.IsSameAs ( _T ( "_wgsoy" ) ) )
			{
				tmp_transform_y = * ( double * ) pattValTmp->value;
				if ( fabs ( tmp_transform_x ) > 1.0 )
					m_CIB->b_have_offsets = true;
			}
		}


		if ( sattr.Len() )
		{
			wxASSERT( sattr.Len() == 6);
			wxCharBuffer dbuffer=sattr.ToUTF8();
			if(dbuffer.data()) {
				pobj->att_array = (char *)realloc(pobj->att_array, 6*(pobj->n_attr + 1));

				strncpy(pobj->att_array + (6 * sizeof(char) * pobj->n_attr), dbuffer.data(), 6);
				pobj->n_attr++;

				pobj->attVal->Add ( pattValTmp );
			}
			else
				delete pattValTmp;
		}
		else
			delete pattValTmp;


	}     //for

	delete pab;

	//    ATON label optimization:
	//    Some CM93 ATON objects do not contain OBJNAM attribute, which means that no label is shown
	//    for these objects when ATON labals are requested
	//    Look for these cases, and change the INFORM attribute label to OBJNAM, if present.


	if ( 1 == geomtype )
	{
		if ( ( !strncmp ( pobj->FeatureName,   "LIT",    3 ) ) ||
				( !strncmp ( pobj->FeatureName, "LIGHTS", 6 ) ) ||
				( !strncmp ( pobj->FeatureName, "BCN",    3 ) ) ||
				( !strncmp ( pobj->FeatureName, "_slgto", 6 ) ) ||
				( !strncmp ( pobj->FeatureName, "_boygn", 6 ) ) ||
				( !strncmp ( pobj->FeatureName, "_bcngn", 6 ) ) ||
				( !strncmp ( pobj->FeatureName, "_extgn", 6 ) ) ||
				( !strncmp ( pobj->FeatureName, "TOWERS", 6 ) ) ||
				( !strncmp ( pobj->FeatureName, "BOY",    3 ) ) )
		{

			bool bfound_OBJNAM =  ( pobj->GetAttributeIndex("OBJNAM") != -1 );
			bool bfound_INFORM =  ( pobj->GetAttributeIndex("INFORM") != -1 );


			if ( ( !bfound_OBJNAM ) && ( bfound_INFORM ) )        // can make substitution
			{
				char *patl = pobj->att_array;
				for(int i=0 ; i < pobj->n_attr ; i++) {           // find "INFORM"
					if(!strncmp(patl, "INFORM", 6)){
						memcpy ( patl, "OBJNAM", 6 );            // change to "OBJNAM"
						break;
					}

					patl += 6;
				}

			}
		}
	}



	switch ( geomtype )
	{
		case 4:
			{
				pobj->Primitive_type = GEO_AREA;

				//    Check for and maintain the class array of M_COVR objects
				if ( sclass_sub.IsSameAs ( _T ( "M_COVR" ) ) )
				{
					M_COVR_Desc *pmcd;

					M_COVR_Desc *pmcd_look = GetCoverSet()->Find_MCD ( cell_index, iobject, subcell );
					if ( NULL == pmcd_look )     // not found
					{
						double lat, lon;

						pmcd = new M_COVR_Desc;

						//    Record unique identifiers for this M_COVR object
						pmcd->m_cell_index = cell_index;
						pmcd->m_object_id = iobject;
						pmcd->m_subcell = subcell;

						//    User offsets start empty
						pmcd->user_xoff = 0;
						pmcd->user_yoff = 0;
						pmcd->m_buser_offsets = false;

						//    Record the Publication Year of this cell
						pmcd->m_npub_year = npub_year;

						//      Get number of exterior ring points(vertices)
						int npta  = xgeom->contour_array[0];
						geo::float_2Dpt *geoPt = new geo::float_2Dpt[npta + 2];     // vertex array
						geo::float_2Dpt *ppt = geoPt;

						pmcd->m_covr_lon_max = -1000.0;
						pmcd->m_covr_lon_min = 1000.0;
						pmcd->m_covr_lat_max = -1000.0;
						pmcd->m_covr_lat_min = 1000.0;

						// Transcribe exterior ring points to vertex array, in Lat/Lon coordinates
						for (int ip = 0; ip < npta; ip++) {
							cm93_point p;
							p.x = (int)xgeom->vertex_array[ip + 1].m_x;
							p.y = (int)xgeom->vertex_array[ip + 1].m_y;

							Transform(&p, 0, 0, /*tmp_transform_x, tmp_transform_y,*/ &lat, &lon);
							ppt->x = lon;
							ppt->y = lat;

							pmcd->m_covr_lon_max = wxMax(pmcd->m_covr_lon_max, lon);
							pmcd->m_covr_lon_min = wxMin(pmcd->m_covr_lon_min, lon);
							pmcd->m_covr_lat_max = wxMax(pmcd->m_covr_lat_max, lat);
							pmcd->m_covr_lat_min = wxMin(pmcd->m_covr_lat_min, lat);

							ppt++;
						}
						pmcd->m_nvertices = npta;
						pmcd->pvertices = geoPt;

						pmcd->m_covr_bbox
							= geo::BoundingBox(pmcd->m_covr_lon_min, pmcd->m_covr_lat_min,
											   pmcd->m_covr_lon_max, pmcd->m_covr_lat_max);

						// Capture and store the potential WGS transform offsets grabbed during
						// attribute decode
						pmcd->transform_WGS84_offset_x = tmp_transform_x;
						pmcd->transform_WGS84_offset_y = tmp_transform_y;

						pmcd->m_centerlat_cos = cos(
							((pmcd->m_covr_lat_min + pmcd->m_covr_lat_max) / 2.0) * M_PI / 180.0);

						// Add this MCD to the persistent class covr_set
						GetCoverSet()->Add_Update_MCD(pmcd);
					} else {
						// If already in the coverset, are there user offsets applied to this MCD?
						if (pmcd_look->m_buser_offsets) {
							m_CIB->b_have_user_offsets = true;

							m_CIB->user_xoff = pmcd_look->user_xoff;
							m_CIB->user_yoff = pmcd_look->user_yoff;
						}
						pmcd = pmcd_look;
					}

					// Add this geometry to the currently loaded class M_COVR array
					m_pcovr_array_loaded.push_back(pmcd);

					// Add the MCD it to the current (temporary) per cell list
					// This array is used only to quickly find the M_COVR object parameters which apply to other objects
					// loaded from this cell.
					// We do this so we don't have to search the entire (worldwide) coverset for this chart scale
					m_CIB->m_cell_mcovr_list.push_back(pmcd);
				}

				//  Declare x/y of the object to be average of all cm93points
				pobj->x = (xgeom->xmin + xgeom->xmax) / 2.;
				pobj->y = (xgeom->ymin + xgeom->ymax) / 2.;

				//    associate the vector(edge) index table
				pobj->m_n_lsindex = xgeom->n_vector_indices;
				pobj->m_lsindex_array = xgeom->pvector_index; // object now owns the array
				pobj->m_n_edge_max_points = xgeom->n_max_edge_points;

				// Find the proper WGS offset for this object
				if (m_CIB->b_have_offsets || m_CIB->b_have_user_offsets) {
					double latc, lonc;
					cm93_point pc;
					pc.x = (short unsigned int)pobj->x;
					pc.y = (short unsigned int)pobj->y;
					Transform(&pc, 0.0, 0.0, &latc, &lonc);

					M_COVR_Desc* pmcd = FindM_COVR_InWorkingSet(latc, lonc);
					if (pmcd) {
						trans_WGS84_offset_x = pmcd->user_xoff;
						trans_WGS84_offset_y = pmcd->user_yoff;
					}
				}

				//  Set the s57obj bounding box as lat/lon
				double lat;
				double lon;
				cm93_point p;

				p.x = ( int ) xgeom->xmin;
				p.y = ( int ) xgeom->ymin;
				Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );
				pobj->BBObj.SetMin ( lon, lat );

				p.x = ( int ) xgeom->xmax;
				p.y = ( int ) xgeom->ymax;
				Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );
				pobj->BBObj.SetMax ( lon, lat );

				pobj->bBBObj_valid = true;

				// Set the object base point
				p.x = ( int ) pobj->x;
				p.y = ( int ) pobj->y;
				Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );
				pobj->m_lon = lon;
				pobj->m_lat = lat;

				// Set up the conversion factors for use in the tesselator
				xgeom->x_rate = m_CIB->transform_x_rate;
				xgeom->x_offset = m_CIB->transform_x_origin - trans_WGS84_offset_x;
				xgeom->y_rate = m_CIB->transform_y_rate;
				xgeom->y_offset = m_CIB->transform_y_origin - trans_WGS84_offset_y;

				// Set up a deferred tesselation
				pobj->pPolyTessGeo = new geo::PolyTessGeo(xgeom);

				break;
			}


		case 1:
			{
				pobj->Primitive_type = GEO_POINT;
				pobj->npt = 1;

				pobj->x = xgeom->pointx;
				pobj->y = xgeom->pointy;

				double lat, lon;
				cm93_point p;
				p.x = xgeom->pointx;
				p.y = xgeom->pointy;
				Transform ( &p, 0.0, 0.0, &lat, &lon );

				//    Find the proper WGS offset for this object
				if ( m_CIB->b_have_offsets || m_CIB->b_have_user_offsets )
				{
					M_COVR_Desc *pmcd = FindM_COVR_InWorkingSet ( lat, lon );
					if ( pmcd )
					{
						trans_WGS84_offset_x = pmcd->user_xoff;
						trans_WGS84_offset_y = pmcd->user_yoff;
					}
				}

				//    Transform again to pick up offsets
				Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );

				pobj->m_lat = lat;
				pobj->m_lon = lon;

				pobj->BBObj.SetMin ( lon-0.25, lat-0.25 );
				pobj->BBObj.SetMax ( lon+0.25, lat+0.25 );


				break;
			}



		case 8:               //wkbMultiPoint25D:
			{


				pobj->Primitive_type = GEO_POINT;

				//  Set the s57obj bounding box as lat/lon
				double lat, lon;
				cm93_point p;

				p.x = ( int ) xgeom->xmin;
				p.y = ( int ) xgeom->ymin;
				Transform ( &p, 0.0, 0.0, &lat, &lon );
				pobj->BBObj.SetMin ( lon, lat );

				p.x = ( int ) xgeom->xmax;
				p.y = ( int ) xgeom->ymax;
				Transform ( &p, 0.0, 0.0, &lat, &lon );
				pobj->BBObj.SetMax ( lon, lat );

				//  and declare x/y of the object to be average of all cm93points
				pobj->x = ( xgeom->xmin + xgeom->xmax ) / 2.0;
				pobj->y = ( xgeom->ymin + xgeom->ymax ) / 2.0;



				OGRMultiPoint *pGeo = ( OGRMultiPoint * ) xgeom->pogrGeom;
				pobj->npt = pGeo->getNumGeometries();

				pobj->geoPtz = ( double * ) malloc ( pobj->npt * 3 * sizeof ( double ) );
				pobj->geoPtMulti = ( double * ) malloc ( pobj->npt * 2 * sizeof ( double ) );

				double *pdd = pobj->geoPtz;
				double *pdl = pobj->geoPtMulti;

				for ( int ip=0 ; ip<pobj->npt ; ip++ )
				{
					OGRPoint *ppt = ( OGRPoint * ) ( pGeo->getGeometryRef ( ip ) );

					cm93_point p;
					p.x = ( int ) ppt->getX();
					p.y = ( int ) ppt->getY();
					double depth = ppt->getZ();

					double east  = p.x;
					double north = p.y;

					double snd_trans_x = 0.0;
					double snd_trans_y = 0.0;

					//    Find the proper offset for this individual sounding
					if ( m_CIB->b_have_user_offsets )
					{
						double lats, lons;
						Transform ( &p, 0.0, 0.0, &lats, &lons );

						M_COVR_Desc *pmcd = FindM_COVR_InWorkingSet ( lats, lons );
						if ( pmcd )
						{
							// For lat/lon calculation below
							snd_trans_x = pmcd->user_xoff;
							snd_trans_y = pmcd->user_yoff;

							// Actual cm93 point of this sounding, back-converted from metres e/n
							east  -= pmcd->user_xoff / m_CIB->transform_x_rate;
							north -= pmcd->user_yoff / m_CIB->transform_y_rate;
						}
					}

					*pdd++ = east;
					*pdd++ = north;
					*pdd++ = depth;

					//  Save offset lat/lon of point in obj->geoPtMulti for later use in decomposed bboxes
					Transform ( &p, snd_trans_x, snd_trans_y, &lat, &lon );
					*pdl++ = lon;
					*pdl++ = lat;
				}

				//  Set the object base point
				p.x = ( int ) pobj->x;
				p.y = ( int ) pobj->y;
				Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );
				pobj->m_lon = lon;
				pobj->m_lat = lat;


				delete pGeo;

				break;
			}         // case 8





		case 2:
			{
				pobj->Primitive_type = GEO_LINE;

				pobj->npt = xgeom->n_max_vertex;
				pobj->geoPt = ( pt * ) xgeom->vertex_array;
				xgeom->vertex_array = NULL;               // object now owns the array

				//  Declare x/y of the object to be average of all cm93points
				pobj->x = ( xgeom->xmin + xgeom->xmax ) / 2.0;
				pobj->y = ( xgeom->ymin + xgeom->ymax ) / 2.0;

				//    associate the vector(edge) index table
				pobj->m_n_lsindex = xgeom->n_vector_indices;
				pobj->m_lsindex_array = xgeom->pvector_index;         // object now owns the array
				pobj->m_n_edge_max_points = xgeom->n_max_edge_points;


				//    Find the proper WGS offset for this object
				if ( m_CIB->b_have_offsets || m_CIB->b_have_user_offsets )
				{
					double latc, lonc;
					cm93_point pc;   pc.x = ( short unsigned int ) pobj->x;  pc.y = ( short unsigned int ) pobj->y;
					Transform ( &pc, 0., 0., &latc, &lonc );

					M_COVR_Desc *pmcd = FindM_COVR_InWorkingSet ( latc, lonc );
					if ( pmcd )
					{
						trans_WGS84_offset_x = pmcd->user_xoff;
						trans_WGS84_offset_y = pmcd->user_yoff;
					}
				}



				//  Set the s57obj bounding box as lat/lon
				double lat, lon;
				cm93_point p;

				p.x = ( int ) xgeom->xmin;
				p.y = ( int ) xgeom->ymin;
				Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );
				pobj->BBObj.SetMin ( lon, lat );

				p.x = ( int ) xgeom->xmax;
				p.y = ( int ) xgeom->ymax;
				Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );
				pobj->BBObj.SetMax ( lon, lat );

				pobj->bBBObj_valid = true;

				//  Set the object base point
				p.x = ( int ) pobj->x;
				p.y = ( int ) pobj->y;
				Transform ( &p, trans_WGS84_offset_x, trans_WGS84_offset_y, &lat, &lon );
				pobj->m_lon = lon;
				pobj->m_lat = lat;

				break;

			}                // case 2
		default:
			{
				//TODO GEO_PRIM here is a placeholder.  Trace this code....
				pobj->Primitive_type = GEO_PRIM;
				break;
			}

	} // geomtype switch


	// Build/Maintain a list of found OBJL types for later use
	// And back-reference the appropriate list index in S57Obj for Display Filtering


	if ( pobj )
	{
		pobj->iOBJL = -1; // deferred, done by OBJL filtering in the PLIB as needed
	}



	// Everything in Xgeom that is needed later has been given to the object
	// So, the xgeom object can be deleted
	// Except for area features, which will get deferred tesselation, and so need the Extended geometry point
	// Those features will own the xgeom...
	if ( geomtype != 4 )
		delete xgeom;

	//    Set the per-object transform coefficients
	pobj->x_rate   = m_CIB->transform_x_rate * ( geo::mercator_k0 * geo::WGS84_semimajor_axis_meters / CM93_semimajor_axis_meters );
	pobj->y_rate   = m_CIB->transform_y_rate * ( geo::mercator_k0 * geo::WGS84_semimajor_axis_meters / CM93_semimajor_axis_meters );
	pobj->x_origin = m_CIB->transform_x_origin * ( geo::mercator_k0 * geo::WGS84_semimajor_axis_meters / CM93_semimajor_axis_meters );
	pobj->y_origin = m_CIB->transform_y_origin * ( geo::mercator_k0 * geo::WGS84_semimajor_axis_meters / CM93_semimajor_axis_meters );

	//    Add in the possible offsets to WGS84 which come from the proper M_COVR containing this feature
	pobj->x_origin -= trans_WGS84_offset_x;
	pobj->y_origin -= trans_WGS84_offset_y;

	return pobj;
}

// Find the proper M_COVR record within this current cell for this lat/lon
M_COVR_Desc* cm93chart::FindM_COVR_InWorkingSet(double lat, double lon)
{
	// Default is to use the first M_COVR, the usual case
	if (m_CIB->m_cell_mcovr_list.size() == 1) {
		return *m_CIB->m_cell_mcovr_list.begin();
	}
	for (List_Of_M_COVR_Desc::iterator i = m_CIB->m_cell_mcovr_list.begin();
		 i != m_CIB->m_cell_mcovr_list.end(); ++i) {
		M_COVR_Desc* pmcd = *i;
		if (G_PtInPolygon_FL(pmcd->pvertices, pmcd->m_nvertices, lon, lat)) {
			return pmcd;
		}
	}
	return NULL;
}

// Find the proper M_COVR record within this current cell for this lat/lon
// And return the WGS84 offsets contained within
wxPoint2DDouble cm93chart::FindM_COVROffset(double lat, double lon)
{
	wxPoint2DDouble ret(0.0, 0.0);

	// Default is to use the first M_COVR, the usual case
	if (m_CIB->m_cell_mcovr_list.size() > 0) {
		M_COVR_Desc* pmcd0 = *m_CIB->m_cell_mcovr_list.begin();
		ret.m_x = pmcd0->transform_WGS84_offset_x;
		ret.m_y = pmcd0->transform_WGS84_offset_y;
	}

	// If there are more than one M_COVR in this cell, need to search
	if (m_CIB->m_cell_mcovr_list.size() > 1) {
		for (List_Of_M_COVR_Desc::const_iterator i = m_CIB->m_cell_mcovr_list.begin();
			 i != m_CIB->m_cell_mcovr_list.end(); ++i) {
			M_COVR_Desc* pmcd = *i;
			if (G_PtInPolygon_FL(pmcd->pvertices, pmcd->m_nvertices, lon, lat)) {
				ret.m_x = pmcd->transform_WGS84_offset_x;
				ret.m_y = pmcd->transform_WGS84_offset_y;
				break;
			}
		}
	}
	return ret;
}

// Read the cm93 cell file header and create required Chartbase data structures
InitReturn cm93chart::CreateHeaderDataFromCM93Cell(void)
{
	// Figure out the scale from the file name
	wxFileName fn ( m_FullPath );
	wxString ext = fn.GetExt();

	int scale;
	switch ( ( ext.mb_str() ) [ ( size_t ) 0] )
	{
		case 'Z': scale = 20000000;  break;
		case 'A': scale =  3000000;  break;
		case 'B': scale =  1000000;  break;
		case 'C': scale =   200000;  break;
		case 'D': scale =   100000;  break;
		case 'E': scale =    50000;  break;
		case 'F': scale =    20000;  break;
		case 'G': scale =     7500;  break;
		default:  scale = 20000000;  break;
	}

	m_Chart_Scale = scale;



	//    Check with the manager to see if a chart of this scale has been processed
	//    If there is no manager, punt and open the chart
	if(m_pManager)
	{
		bool bproc = false;
		switch ( m_Chart_Scale )
		{
			case 20000000: bproc = m_pManager->m_bfoundZ; break;
			case  3000000: bproc = m_pManager->m_bfoundA; break;
			case  1000000: bproc = m_pManager->m_bfoundB; break;
			case   200000: bproc = m_pManager->m_bfoundC; break;
			case   100000: bproc = m_pManager->m_bfoundD; break;
			case    50000: bproc = m_pManager->m_bfoundE; break;
			case    20000: bproc = m_pManager->m_bfoundF; break;
			case     7500: bproc = m_pManager->m_bfoundG; break;
		}


		if ( bproc )
			return INIT_FAIL_NOERROR;




		//    Inform the manager that a chart of this scale has been processed
		switch ( m_Chart_Scale )
		{
			case 20000000: m_pManager->m_bfoundZ = true; break;
			case  3000000: m_pManager->m_bfoundA = true; break;
			case  1000000: m_pManager->m_bfoundB = true; break;
			case   200000: m_pManager->m_bfoundC = true; break;
			case   100000: m_pManager->m_bfoundD = true; break;
			case    50000: m_pManager->m_bfoundE = true; break;
			case    20000: m_pManager->m_bfoundF = true; break;
			case     7500: m_pManager->m_bfoundG = true; break;
		}
	}

	//    Specify the whole world as chart coverage
	m_FullExtent.ELON = 179.0;
	m_FullExtent.WLON = -179.0;
	m_FullExtent.NLAT = 80.0;
	m_FullExtent.SLAT = -80.0;
	m_bExtentSet = true;


	//    Populate one (huge) M_COVR Entry
	m_nCOVREntries = 1;
	m_pCOVRTablePoints = ( int * ) malloc ( sizeof ( int ) );
	*m_pCOVRTablePoints = 4;
	m_pCOVRTable = ( float ** ) malloc ( sizeof ( float * ) );
	float *pf = ( float * ) malloc ( 2 * 4 * sizeof ( float ) );
	*m_pCOVRTable = pf;
	float *pfe = pf;

	*pfe++ = m_FullExtent.NLAT; //LatMax;
	*pfe++ = m_FullExtent.WLON; //LonMin;

	*pfe++ = m_FullExtent.NLAT; //LatMax;
	*pfe++ = m_FullExtent.ELON; //LonMax;

	*pfe++ = m_FullExtent.SLAT; //LatMin;
	*pfe++ = m_FullExtent.ELON; //LonMax;

	*pfe++ = m_FullExtent.SLAT; //LatMin;
	*pfe++ = m_FullExtent.WLON; //LonMin;



	return INIT_OK;
}

void cm93chart::ProcessMCOVRObjects(int cell_index, char subcell)
{
	// Extract the m_covr structures inline

	Object* pobject = m_CIB->pobject_block; // head of object array

	int iObj = 0;
	while (iObj < m_CIB->m_nfeature_records) {
		if ((pobject != NULL)) {
			// Look for and process m_covr object(s)
			int iclass = pobject->otype;

			wxString sclass = m_pDict->GetClassName(iclass);

			if (sclass.IsSameAs(_T ( "_m_sor" ))) {
				M_COVR_Desc* pmcd = m_pcovr_set->Find_MCD(cell_index, iObj, (int)subcell);
				if (NULL == pmcd) {
					geo::ExtendedGeometry* xgeom = BuildGeom(pobject, NULL, iObj);

					//    Decode the attributes, specifically looking for _wgsox, _wgsoy

					double tmp_transform_x = 0.;
					double tmp_transform_y = 0.;

					cm93_attr_block* pab = new cm93_attr_block(pobject->attributes_block, m_pDict);
					for (int jattr = 0; jattr < pobject->n_attributes; jattr++) {
						unsigned char* curr_attr = pab->GetNextAttr();
						unsigned char iattr = *curr_attr;
						wxString sattr = m_pDict->GetAttrName(iattr);
						char vtype = m_pDict->GetAttrType(iattr);
						unsigned char* aval = curr_attr + 1;

						if (vtype == 'R') {
							float* pf = (float*)aval;
#ifdef ARMHF
							float tf1;
							memcpy(&tf1, pf, sizeof(float));
							if (sattr.IsSameAs(_T ( "_wgsox" )))
								tmp_transform_x = tf1;
							else if (sattr.IsSameAs(_T ( "_wgsoy" )))
								tmp_transform_y = tf1;
#else
							if (sattr.IsSameAs(_T ( "_wgsox" )))
								tmp_transform_x = *pf;
							else if (sattr.IsSameAs(_T ( "_wgsoy" )))
								tmp_transform_y = *pf;
#endif
						}

					} // for all attributes

					delete pab;

					if (NULL != xgeom) {
						double lat, lon;

						pmcd = new M_COVR_Desc;

						//    Record unique identifiers for this M_COVR object
						pmcd->m_cell_index = cell_index;
						pmcd->m_object_id = iObj;
						pmcd->m_subcell = (int)subcell;

						//      Get number of exterior ring points(vertices)
						int npta = xgeom->contour_array[0];
						geo::float_2Dpt* geoPt = new geo::float_2Dpt[npta + 2]; // vertex array
						geo::float_2Dpt* ppt = geoPt;

						//  Transcribe exterior ring points to vertex array, in Lat/Lon coordinates
						pmcd->m_covr_lon_max = -1000.0;
						pmcd->m_covr_lon_min = 1000.0;
						pmcd->m_covr_lat_max = -1000.0;
						pmcd->m_covr_lat_min = 1000.0;

						for (int ip = 0; ip < npta; ip++) {
							cm93_point p;
							p.x = (int)xgeom->vertex_array[ip + 1].m_x;
							p.y = (int)xgeom->vertex_array[ip + 1].m_y;

							Transform(&p, 0.0, 0.0, &lat, &lon);
							ppt->x = lon;
							ppt->y = lat;

							pmcd->m_covr_lon_max = wxMax(pmcd->m_covr_lon_max, lon);
							pmcd->m_covr_lon_min = wxMin(pmcd->m_covr_lon_min, lon);
							pmcd->m_covr_lat_max = wxMax(pmcd->m_covr_lat_max, lat);
							pmcd->m_covr_lat_min = wxMin(pmcd->m_covr_lat_min, lat);

							ppt++;
						}
						pmcd->m_nvertices = npta;
						pmcd->pvertices = geoPt;

						pmcd->m_covr_bbox
							= geo::BoundingBox(pmcd->m_covr_lon_min, pmcd->m_covr_lat_min,
											   pmcd->m_covr_lon_max, pmcd->m_covr_lat_max);

						// Capture and store the potential WGS transform offsets grabbed during
						// attribute decode
						pmcd->transform_WGS84_offset_x = tmp_transform_x;
						pmcd->transform_WGS84_offset_y = tmp_transform_y;

						pmcd->m_centerlat_cos = cos(
							((pmcd->m_covr_lat_min + pmcd->m_covr_lat_max) / 2.0) * M_PI / 180.0);

						//     Add this object to the covr_set
						m_pcovr_set->Add_Update_MCD(pmcd);

						//    Clean up the xgeom
						free(xgeom->pvector_index);

						delete xgeom;
					}
				}
			}
		} else // objectdef == NULL
			break;

		pobject++;
		iObj++;
	}
}

bool cm93chart::UpdateCovrSet(ViewPort* vpt)
{
	// Create an array of CellIndexes covering the current viewport
	std::vector<int> vpcells = GetVPCellArray(*vpt);

	// Check the member covr_set to see if all these viewport cells have had their m_covr loaded
	for (unsigned int i = 0; i < vpcells.size(); i++) {
		// If the cell is not already in the master coverset, go load enough of it to get the
		// offsets and outlines.....
		if (!m_pcovr_set->IsCovrLoaded(vpcells[i])) {
			if (loadcell_in_sequence(vpcells[i], '0')) {
				ProcessMCOVRObjects(vpcells[i], '0');
				Unload_CM93_Cell(); // all done with this (sub)cell
			}

			char loadcell_key = 'A'; // starting subcells

			// Load the subcells in sequence
			// On successful load, add it to the covr set and process the cell
			while (loadcell_in_sequence(vpcells[i], loadcell_key)) {
				// Extract the m_covr structures inline

				ProcessMCOVRObjects(vpcells[i], loadcell_key);
				Unload_CM93_Cell(); // all done with this (sub)cell
				loadcell_key++;
			}
		}
	}

	return true;
}

bool cm93chart::IsPointInLoadedM_COVR(double xc, double yc)
{
	for (CovrDescContainer::iterator i = m_pcovr_array_loaded.begin();
		 i != m_pcovr_array_loaded.end(); ++i) {
		if (G_PtInPolygon_FL((*i)->pvertices, (*i)->m_nvertices, xc, yc))
			return true;
	}
	return false;
}

int cm93chart::loadcell_in_sequence(int cellindex, char subcell)
{
	return loadsubcell(cellindex, subcell);
}

int cm93chart::loadsubcell(int cellindex, wxChar sub_char)
{
	// Create the file name

	int ilat = cellindex / 10000;
	int ilon = cellindex % 10000;

	if (g_bDebugCM93) {
		double dlat = m_dval / 3.;
		double dlon = m_dval / 3.;
		double lat, lon;
		Get_CM93_Cell_Origin(cellindex, GetNativeScale(), &lat, &lon);
		printf("\n   Attempting loadcell %d scale %lc, sub_char %lc at lat: %g/%g lon:%g/%g\n",
			   cellindex, wxChar(m_scalechar[0]), sub_char, lat, lat + dlat, lon, lon + dlon);
	}

	int jlat = (int)(((ilat - 30) / m_dval) * m_dval) + 30; // normalize
	int jlon = (int)((ilon / m_dval) * m_dval);

	int ilatroot = (((ilat - 30) / 60) * 60) + 30;
	int ilonroot = (ilon / 60) * 60;

	wxString file;
	file.Printf(_T ( "%04d%04d." ), jlat, jlon);
	file += m_scalechar;

	wxString fileroot;
	fileroot.Printf(_T ( "%04d%04d/" ), ilatroot, ilonroot);
	fileroot += m_scalechar;
	fileroot += _T ( "/" );
	fileroot.Prepend(m_prefix);

	file[0] = sub_char;
	file.Prepend(fileroot);

	if (g_bDebugCM93) {
		char sfile[200];
		strncpy(sfile, file.mb_str(), 199);
		sfile[199] = 0;
		printf("    filename: %s\n", sfile);
	}

	if (!::wxFileExists(file)) {
		// Try with alternate case of m_scalechar
		wxString new_scalechar = m_scalechar.Lower();

		wxString file1;
		file1.Printf(_T ( "%04d%04d." ), jlat, jlon);
		file1 += new_scalechar;

		file1[0] = sub_char;

		fileroot.Printf(_T ( "%04d%04d/" ), ilatroot, ilonroot);
		fileroot += new_scalechar;
		fileroot += _T ( "/" );
		fileroot.Prepend(m_prefix);

		file1.Prepend(fileroot);

		if (g_bDebugCM93) {
			char sfile[200];
			strncpy(sfile, file1.mb_str(), 199);
			sfile[199] = 0;
			printf("    alternate filename: %s\n", sfile);
		}

		if (!::wxFileExists(file1)) {

			// This is not really an error if the sub_char is not '0'.  It just means there are no
			// more subcells....
			if (g_bDebugCM93) {
				if (sub_char == '0')
					printf("   Tried to load non-existent CM93 cell\n");
				else
					printf("   No sub_cells of scale(%lc) found\n", sub_char);
			}

			return 0;
		} else
			file = file1; // found the file as lowercase, substitute the name
	}

	// File is known to exist

	if (!s_b_busy_shown) {
		::wxBeginBusyCursor();
		s_b_busy_shown = true;
	}

	wxString msg = _T("Loading CM93 cell ") + file;
	wxLogMessage(msg);

	// Set the member variable to be the actual file name for use in single chart mode info display
	m_LastFileName = file;

	if (g_bDebugCM93) {
		char str[256];
		strncpy(str, msg.mb_str(), 255);
		str[255] = 0;
		printf("   %s\n", str);
	}

	// Ingest it
	if (!Ingest_CM93_Cell((const char*)file.mb_str(), m_CIB)) {
		wxLogMessage(_T("   cm93chart  Error ingesting ") + file);
		return 0;
	}

	return 1;
}

void cm93chart::SetUserOffsets(int cell_index, int object_id, int subcell, int xoff, int yoff)
{
	M_COVR_Desc* pmcd = GetCoverSet()->Find_MCD(cell_index, object_id, subcell);
	if (pmcd) {
		pmcd->user_xoff = xoff;
		pmcd->user_yoff = yoff;
		pmcd->m_buser_offsets = true;
	}
}

wxPoint* cm93chart::GetDrawBuffer(int nSize)
{
	// Reallocate the cm93chart DrawBuffer if it is currently too small
	if (nSize > m_nDrawBufferSize) {
		wxPoint* tmp = m_pDrawBuffer;
		m_pDrawBuffer = (wxPoint*)realloc(m_pDrawBuffer, sizeof(wxPoint) * (nSize + 1));
		if (NULL == m_pDrawBuffer) {
			free(tmp);
			tmp = NULL;
		} else
			m_nDrawBufferSize = nSize + 1;
	}
	return m_pDrawBuffer;
}

void cm93chart::ResetSubcellKey()
{
	m_loadcell_key = '0';
}

void cm93chart::SetCM93Dict(cm93_dictionary* pDict)
{
	m_pDict = pDict;
}

void cm93chart::SetCM93Prefix(const wxString& prefix)
{
	m_prefix = prefix;
}

void cm93chart::SetCM93Manager(chart::cm93manager* pManager)
{
	m_pManager = pManager;
}

const wxString& cm93chart::GetLastFileName(void) const
{
	return m_LastFileName;
}

wxString cm93chart::GetScaleChar() const
{
	return m_scalechar;
}

}

