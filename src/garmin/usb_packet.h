/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#ifndef __GARMIN__USB_PACKET__H__
#define __GARMIN__USB_PACKET__H__

#ifdef __WXMSW__
	#include <windows.h>
	#include <dbt.h>
	#include <initguid.h>
#endif

#define GARMIN_USB_API_VERSION 1
#define GARMIN_USB_MAX_BUFFER_SIZE 4096
#define GARMIN_USB_INTERRUPT_DATA_SIZE 64

#define IOCTL_GARMIN_USB_API_VERSION CTL_CODE \
	(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GARMIN_USB_INTERRUPT_IN CTL_CODE \
	(FILE_DEVICE_UNKNOWN, 0x850, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GARMIN_USB_BULK_OUT_PACKET_SIZE CTL_CODE \
	(FILE_DEVICE_UNKNOWN, 0x851, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifdef __WXMSW__
// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
DEFINE_GUID(GARMIN_GUID, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0);
#endif


// New packet types in USB.
#define GUSB_SESSION_START 5    // We request units attention
#define GUSB_SESSION_ACK   6    // Unit responds that we have its attention
#define GUSB_REQUEST_BULK  2    // Unit requests we read from bulk pipe

#define GUSB_RESPONSE_PVT  51   // PVT Data Packet
#define GUSB_RESPONSE_SDR  114  // Satellite Data Record Packet

typedef
union {
	struct {
		unsigned char type;
		unsigned char reserved1;
		unsigned char reserved2;
		unsigned char reserved3;
		unsigned char pkt_id[2];
		unsigned char reserved6;
		unsigned char reserved7;
		unsigned char datasz[4];
		unsigned char databuf[5]; // actually a variable length array...
	} gusb_pkt;
	unsigned char dbuf[1024];
} garmin_usb_packet;


// Packet structure for Pkt_ID = 51 (PVT Data Record)
//#pragma pack(push)  /* push current alignment to stack */
//#pragma pack(1)     /* set alignment to 1 byte boundary */
#pragma pack(push,1) // push current alignment to stack, set alignment to 1 byte boundary

typedef struct {
	float   alt;
	float   epe;
	float   eph;
	float   epv;
	short   fix;
	double  tow;
	double  lat;
	double  lon;
	float   east;
	float   north;
	float   up;
	float   msl_hght;
	short   leap_scnds;
	long    wn_days;
} D800_Pvt_Data_Type;

#pragma pack(pop)   // restore original alignment from stack


enum
{
	rs_fromintr,
	rs_frombulk
};

#endif
