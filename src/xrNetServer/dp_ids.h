#pragma once
#include "net_shared.h"

// {0218FA8B-515B-4bf2-9A5F-2F079D1759F3}
const GUID NET_GUID =
{ 0x218fa8b,  0x515b, 0x4bf2, { 0x9a, 0x5f, 0x2f, 0x7, 0x9d, 0x17, 0x59, 0xf3 } };

// {8D3F9E5E-A3BD-475b-9E49-B0E77139143C}
const GUID CLSID_NETWORKSIMULATOR_DP8SP_TCPIP =
{ 0x8d3f9e5e, 0xa3bd, 0x475b, { 0x9e, 0x49, 0xb0, 0xe7, 0x71, 0x39, 0x14, 0x3c } };

const GUID CLSID_DirectPlay8Client =
{ 0x743f1dc6, 0x5aba, 0x429f, { 0x8b, 0xdf, 0xc5, 0x4d, 0x03, 0x25, 0x3d, 0xc2 } };

// {DA825E1B-6830-43d7-835D-0B5AD82956A2}
const GUID CLSID_DirectPlay8Server =
{ 0xda825e1b, 0x6830, 0x43d7, { 0x83, 0x5d, 0x0b, 0x5a, 0xd8, 0x29, 0x56, 0xa2 } };

// {286F484D-375E-4458-A272-B138E2F80A6A}
const GUID CLSID_DirectPlay8Peer =
{ 0x286f484d, 0x375e, 0x4458, { 0xa2, 0x72, 0xb1, 0x38, 0xe2, 0xf8, 0x0a, 0x6a } };


// CLSIDs added for DirectX 9

// {FC47060E-6153-4b34-B975-8E4121EB7F3C}
const GUID CLSID_DirectPlay8ThreadPool =
{ 0xfc47060e, 0x6153, 0x4b34, { 0xb9, 0x75, 0x8e, 0x41, 0x21, 0xeb, 0x7f, 0x3c } };

// {E4C1D9A2-CBF7-48bd-9A69-34A55E0D8941}
const GUID CLSID_DirectPlay8NATResolver =
{ 0xe4c1d9a2, 0xcbf7, 0x48bd, { 0x9a, 0x69, 0x34, 0xa5, 0x5e, 0x0d, 0x89, 0x41 } };

/****************************************************************************
 *
 * DirectPlay8 Interface IIDs
 *
 ****************************************************************************/

typedef REFIID	DP8REFIID;


// {5102DACD-241B-11d3-AEA7-006097B01411}
const GUID IID_IDirectPlay8Client =
{ 0x5102dacd, 0x241b, 0x11d3, { 0xae, 0xa7, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };

// {5102DACE-241B-11d3-AEA7-006097B01411}
const GUID IID_IDirectPlay8Server =
{ 0x5102dace, 0x241b, 0x11d3, { 0xae, 0xa7, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };

// {5102DACF-241B-11d3-AEA7-006097B01411}
const GUID IID_IDirectPlay8Peer =
{ 0x5102dacf, 0x241b, 0x11d3, { 0xae, 0xa7, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };


// IIDs added for DirectX 9

// {0D22EE73-4A46-4a0d-89B2-045B4D666425}
const GUID IID_IDirectPlay8ThreadPool =
{ 0xd22ee73, 0x4a46, 0x4a0d,  { 0x89, 0xb2, 0x04, 0x5b, 0x4d, 0x66, 0x64, 0x25 } };

// {A9E213F2-9A60-486f-BF3B-53408B6D1CBB}
const GUID IID_IDirectPlay8NATResolver =
{ 0xa9e213f2, 0x9a60, 0x486f, { 0xbf, 0x3b, 0x53, 0x40, 0x8b, 0x6d, 0x1c, 0xbb } };

// {53934290-628D-11D2-AE0F-006097B01411}
const GUID CLSID_DP8SP_IPX =
{ 0x53934290, 0x628d, 0x11d2, { 0xae, 0x0f, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };


// {6D4A3650-628D-11D2-AE0F-006097B01411}
const GUID CLSID_DP8SP_MODEM =
{ 0x6d4a3650, 0x628d, 0x11d2, { 0xae, 0x0f, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };


// {743B5D60-628D-11D2-AE0F-006097B01411}
const GUID CLSID_DP8SP_SERIAL =
{ 0x743b5d60, 0x628d, 0x11d2, { 0xae, 0x0f, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };


// {EBFE7BA0-628D-11D2-AE0F-006097B01411}
const GUID CLSID_DP8SP_TCPIP =
{ 0xebfe7ba0, 0x628d, 0x11d2, { 0xae, 0x0f, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };


// Service providers added for DirectX 9

// {995513AF-3027-4b9a-956E-C772B3F78006}
const GUID CLSID_DP8SP_BLUETOOTH =
{ 0x995513af, 0x3027, 0x4b9a, { 0x95, 0x6e, 0xc7, 0x72, 0xb3, 0xf7, 0x80, 0x06 } };

const GUID CLSID_DirectPlay8Address =
{ 0x934a9523, 0xa3ca, 0x4bc5, { 0xad, 0xa0, 0xd6, 0xd9, 0x5d, 0x97, 0x94, 0x21 } };

const GUID IID_IDirectPlay8Address =
{ 0x83783300, 0x4063, 0x4c8a, { 0x9d, 0xb3, 0x82, 0x83, 0x0a, 0x7f, 0xeb, 0x31 } };

