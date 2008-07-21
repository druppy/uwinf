/****************************************************************************
MODULE
	FPaint.CPP
VERSION
	0.00.008
HOST
	PC AT 486,33 Borland C++ 3.1
	PC AT 486,66, OS/2, Watcom 10.6 C/C++
AUTHOR
	Carsten Mouritzen 2:238/10.17@FidoNet
STATE
	develop
DESCRIPTION
	Painter modul.
		Skal indkapsle alle OS afh‘ngelige aspekter, samtidigt med at det skal
		give mulighed for at bruge "native" funktioner.

		Der opstilles f›lgende design krav :
			1. Skal kunne bruges p† alle enheder der er i stand til at
				 repr‘sentere grafiske data.

			2. Alle funktioner som kan arbejde p† ovenst†ende skal v‘re
				 implementeret.

			3. Skal kunne nedarves i et vindue og derved optr‘de som en udvidelse
				 af funktionaliteten for det p†g‘ldende vindue.

			4. Skal kunne inds‘ttes som datamember i et vindue for at kunne bruges
				 til redirigering af output ( eks. til printer ).

HISTORY
	16.01.95, 0.00.001
		First shot
	29.03.95, 0.00.002
		Adjusted to Hungarian notation
		Added a few OS/2 functions
	30.03.95, 0.00.003
		Added all mapping functions to FPaintSpace
		Added FMapMode enum to FPaintSpace
	07.04.95, 0.00.004
		Moved HDC var to base class ( again )
		Added SetPS in Paintbox, to allow for paintbox inheritance ( idea by
		BL )
	10.04.95, 0.00.005
		- Changed StartDoc to take a documentname parameter,
			this was done because windows will not accept a document without a name
			( at least not in my setup, that is ).
		- Changed StartDoc work corectly when called multiple times ( now frees
			the previous name, if given )
		- Changed SetOutFile to RedirectOutput, bacause this gives me a better
			idea of the functions real intention.
		- Changed RedirectOutput work corectly when called multiple times
			( now frees the previous name, if given )
		- added OpenDefaultDevice at BL's recomendation
	11.04.95, 0.00.006
		- Clean'ed up EnumDevices, to remove LocalAlloc's ( replaced with
			new char[]'s ). No change in interface
		- Started working on mapping.
			- SetWindowOrg renamed to SetLogicalOrigin
			- Crunch, i'll need to work a little more with this
	12.04.95, 0.00.007
		- Encapsulated the driver, port and device parameters used in printer
			handling, into a single structure called FDeviceInfo ( also because
			of OS/2 portability ).
	19.04.95, 0.00.008, 11:03:19am
		- StartDoc, EndDoc, AbortDoc renamed to xxxJob to enforce a network/queue
			orientated abstraction.
		- OpenDialog in FDeviceEnumerator renamed to OpenViaStdDialog to enforce
			the meaning of the call.
	27.08.96, 0.00.008
		- Convertet to a more BIF like stucture, by separating the source
		to be a header and a implimentation file, and renamed it from
 		DEVCON2 to FPaint ! This was done by BL.
****************************************************************************/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define INCL_PM

#define INCL_DEV
#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_WINERRORS
#define INCL_DOSPROCESS

#include <bif.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#if( BIFSYSTEM == BIFWIN16 )
#include <commdlg.h>
#include <print.h>
#endif

#include "FPaint.h"
/*--------------------------------------------------------------------------*/
#ifdef BIFINCL_DEFAULTMAPMODES
#if( BIFSYSTEM == BIFWIN16 )
	FAbsoluteMapper HiEnglishMapper( MM_HIENGLISH );
	FAbsoluteMapper LoEnglishMapper( MM_LOENGLISH );
	FAbsoluteMapper HiMetricMapper( MM_HIMETRIC );
	FAbsoluteMapper LoMetricMapper( MM_LOMETRIC );
	FAbsoluteMapper PixelMapper( MM_TEXT );
#endif
#if( BIFSYSTEM == BIFOS2 )
	FAbsoluteMapper HiEnglishMapper( PU_HIENGLISH );
	FAbsoluteMapper LoEnglishMapper( PU_LOENGLISH );
	FAbsoluteMapper HiMetricMapper( PU_HIMETRIC );
	FAbsoluteMapper LoMetricMapper( PU_LOMETRIC );
	FAbsoluteMapper PixelMapper( PU_PELS );
#endif
  FRelativeMapper IsotropicMapper( FPoint( 1000, 1000 ), pm_isotropic );
  FRelativeMapper AnIsotropicMapper( FPoint( 1000, 1000 ), pm_anisotropic );
#endif

FColorRef FColors::Yellow        = RGBPct2Ref( 100, 100,   0 );
FColorRef FColors::Warm_Red      = RGBPct2Ref( 100,  21,   9 );
FColorRef FColors::Rubine_Red    = RGBPct2Ref( 100,   0,  85 );
FColorRef FColors::Rhodamine_Red = RGBPct2Ref(  94,   0, 100 );
FColorRef FColors::Purple        = RGBPct2Ref(  57,   9, 100 );
FColorRef FColors::Reflex_Blue   = RGBPct2Ref(   0,  22,  94 );
FColorRef FColors::Process_Blue  = RGBPct2Ref(   0,  85,  94 );
FColorRef FColors::Green         = RGBPct2Ref(   0, 100,  35 );
FColorRef FColors::Black         = RGBPct2Ref(   0,   0,   0 );
FColorRef FColors::Violet        = RGBPct2Ref(   6,   9, 100 );

FColorRef FColors::RGB_Red       = RGBPct2Ref( 100,   0,   0 );
FColorRef FColors::RGB_Green     = RGBPct2Ref(   0, 100,   0 );
FColorRef FColors::RGB_Blue      = RGBPct2Ref(   0,   0, 100 );
FColorRef FColors::RGB_Cyan      = RGBPct2Ref(   0, 100, 100 );
FColorRef FColors::RGB_Pink      = RGBPct2Ref( 100,   0, 100 );
FColorRef FColors::RGB_Yellow    = RGBPct2Ref( 100, 100,   0 );
FColorRef FColors::RGB_White     = RGBPct2Ref( 100, 100, 100 );
FColorRef FColors::RGB_LtGrey    = RGBPct2Ref(  70,  70,  70 );

Bool FDeviceEnumerator::EnumDevices( int nDeviceIndex, FDeviceInfo* pDeviceInfo )
{
#if( BIFSYSTEM == BIFWIN16 )
	int count = 0;
	Bool res = False;

	char* l_pszAllEntrys = new char[1024];
	char* l_pszSelectedDevice = l_pszAllEntrys;

	//Get a NULL separated list of all posible devices
	GetProfileString( "devices", NULL, "", l_pszAllEntrys, 1023 );

	//Find the device corresponding to nDeviceIndex
	while( strlen( l_pszSelectedDevice ) > 0 ) {
		if( count == nDeviceIndex ) {
			//Reached target count, break out
			strcpy( pDeviceInfo->pszDevice, l_pszSelectedDevice );
			break;
		}
		//Target count not reached, skip this entry
		while( *l_pszSelectedDevice != 0 )
			l_pszSelectedDevice++;

		//Skip the zero terminator
		l_pszSelectedDevice++;

		//Keep track of the device number
		count++;
	}

	if( strlen( pDeviceInfo->pszDevice ) > 0 ) {
		//Device found, get the correct string
		char* l_pszSelectedEntry = new char[1024];


		GetProfileString( "devices", pDeviceInfo->pszDevice, "", l_pszSelectedEntry, 1023 );
		sscanf( l_pszSelectedEntry, "%[^,]%*c%[^,]%*c", pDeviceInfo->pszDriver, pDeviceInfo->pszPort );
		//The entry is now copied to driver and port, that is we don't need it
		//anymore
		delete[] l_pszSelectedEntry;

		res = True;
	}
	delete[] l_pszAllEntrys;

	return Bool( res );
#endif

#if( BIFSYSTEM == BIFOS2FIN )
	PSZ pCompName = NULL; //Name of computer to query, NULL means local
	ULONG ulLevel = 3;    //Detail level, we use 3 at the moment
	PVOID pBuffer = NULL; //Pointer to buffer to store level data in
	ULONG ulBuffSize = 0L;//Size of above buffer
	ULONG cReturned;      //After the call this returns the number of entrys
	ULONG cTotal;         //
	ULONG ulSize;         //After the call this contains the number of bytes used

	SplEnumQueue( pCompName, ulLevel,
								pBuffer,
								ulBuffSize,
								(PULONG)&cReturned,
								(PULONG)&cTotal,
								(PULONG)&ulSize,
								NULL );

	if( cTotal < nDeviceIndex )	// Entrys not available
		return False;

	ulBuffSize = ulSize;
	pBuffer = new char[ ulBuffSize ];

	SplEnumQueue( pCompName, ulLevel,
								pBuffer,
								ulBuffSize,
								(PULONG)&cReturned,
								(PULONG)&cTotal,
								(PULONG)&ulSize,
								NULL );

	strcpy( devnam, ((PPRQINFO3)pBuffer)[nDeviceIndex] );
	delete[] pBuffer;
	return True;
#endif
	return False;	// No legal OS defined !!!
}

FDevice* FDeviceEnumerator::OpenDevice( int nDeviceIndex )
{
#if( BIFSYSTEM == BIFWIN16 )

	char device[100];
	char driver[100];
	char port[100];
	FDeviceInfo di;
	di.pszDevice = device;
	di.pszDriver = driver;
	di.pszPort = port;

	if( FDeviceEnumerator::EnumDevices( nDeviceIndex, &di ) ) {
		return new FDevice( &di );
	}
	return NULL;
#endif

#if( BIFSYSTEM == BIFOS2 )
	return NULL;
#endif
}

// Opens default printer device
// NOT FINISHED YET
FDevice* FDeviceEnumerator::OpenDefaultDevice( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	char device[100];
	char driver[100];
	char port[100];
	char t[200];
	FDeviceInfo di;
	di.pszDevice = device;
	di.pszDriver = driver;
	di.pszPort = port;
	//Get all posible devices
	GetProfileString( "windows", "device", "", t, 1023 );

	sscanf( t, "%[^,]%*c%[^,]%*c%[^,]%*c", device, driver, port );

	return new FDevice( &di );
#endif
#if( BIFSYSTEM == BIFOS2 )
	unsigned long ulSize;
	char szTemp[300];
	PPRQINFO3 pPrq3;
	unsigned long ulNeeded;

	ulSize = PrfQueryProfileString( HINI_PROFILE, SPL_INI_SPOOLER, "QUEUE", 0, szTemp, 300 );
	if( !ulSize )
		return NULL;

	//remove everything after the ;
	char* pStr;
	pStr = (PSZ)strchr(szTemp,';');
	*pStr = 0;

	SplQueryQueue( 0, szTemp, 3, NULL, 0L, &ulNeeded );
	pPrq3 = (PPRQINFO3)new char[ulNeeded];
	SplQueryQueue( 0, szTemp,3, pPrq3, ulNeeded, &ulNeeded );
	return NULL;
#endif
}

Bool FDeviceEnumerator::CloseDevice( FDevice* pDevice )
{
	if( !pDevice )
		return False;

	delete pDevice;
	return True;
}

//Displays a dialogbox containing the posible selections, and creates a device
//returns NULL if the user did not select anything or has aborted.
FDevice* FDeviceEnumerator::OpenViaStdDialog( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	FDevice* newdevice;
	LPDEVNAMES lpDevNames;
	char* pszDriverName;
	char* pszDeviceName;
	char* pszPortName;
	FDeviceInfo di;
	PRINTDLG   pd;


	//PrintDlg displays the common dialog box for printing. The
	//PRINTDLG structure should be initialized with appropriate values.
	memset( &pd, 0, sizeof( PRINTDLG ) );

	//Initialize the necessary PRINTDLG structure members.

	pd.lStructSize = sizeof( PRINTDLG );
	pd.hwndOwner = NULL;
	pd.Flags = PD_PRINTSETUP;

	if( !PrintDlg( &pd ) )
		return NULL;

	lpDevNames = (LPDEVNAMES)GlobalLock( pd.hDevNames );
	pszDriverName = (char*)lpDevNames + lpDevNames->wDriverOffset;
	pszDeviceName = (char*)lpDevNames + lpDevNames->wDeviceOffset;
	pszPortName   = (char*)lpDevNames + lpDevNames->wOutputOffset;
	//The were a bug here as i Unlock'ed before creating the device,
	//FDevice strdup's the strings and they can therefore be discarded after
	//use.
	di.pszDevice = pszDeviceName;
	di.pszDriver = pszDriverName;
	di.pszPort = pszPortName;

	newdevice = new FDevice( &di );
	GlobalUnlock( pd.hDevNames );
	return newdevice;
#endif
#if( BIFSYSTEM == BIFOS2 )
	return NULL;
#endif
}
/*--------------------------------------------------------------------------*/
FDevice::FDevice( FDeviceInfo * pDeviceInfo )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_DevInfo.pszDevice = strdup( pDeviceInfo->pszDevice );
	m_DevInfo.pszDriver = strdup( pDeviceInfo->pszDriver );;
	m_DevInfo.pszPort = strdup( pDeviceInfo->pszPort );
#endif
}

FDevice::~FDevice()
{
#if( BIFSYSTEM == BIFWIN16 )
	if( m_DevInfo.pszDevice )
		free( m_DevInfo.pszDevice );

	if( m_DevInfo.pszDriver )
		free( m_DevInfo.pszDriver );

	if( m_DevInfo.pszPort )
		free( m_DevInfo.pszPort );
#endif
}

Bool FDevice::SetupDialog( void )
{
	return False;
};

int FDevice::GetBinCount( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	return((int)Capabilities( DC_BINNAMES, NULL ));
#endif

#if( BIFSYSTEM == BIFOS2 )
	return 0;
#endif
}

int FDevice::GetPaperCount( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	return((int)Capabilities( DC_PAPERNAMES, NULL ));
#endif

#if( BIFSYSTEM == BIFOS2 )
	return 0;
#endif
}

int FDevice::GetMaxNumberOfCopies( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	char t[200];
	return((int)Capabilities( DC_COPIES, t ));
#endif

#if( BIFSYSTEM == BIFOS2 )
	return 0;
#endif
}

Bool FDevice::GetBinName( int nBinNumber, char* pszBuffer )
{
#if( BIFSYSTEM == BIFWIN16 )
	char t[20][24];

	if( nBinNumber >= Capabilities( DC_BINNAMES, (char*)t ) )
		return False;
	strcpy( pszBuffer, t[nBinNumber] );
	return True;
#endif

#if( BIFSYSTEM == BIFOS2 )
	return False;
#endif
}

Bool FDevice::GetPaperName( int nBinNumber, char* pszBuffer )
{
#if( BIFSYSTEM == BIFWIN16 )
	char t[20][64];

	if( nBinNumber >= Capabilities( DC_PAPERNAMES, (char*)t ) )
		return False;
	strcpy( pszBuffer, t[nBinNumber] );
	return True;
#endif

#if( BIFSYSTEM == BIFOS2 )
	return False;
#endif
}

unsigned long FDevice::Capabilities( int fwCapability, char* pszBuffer )
{
#if( BIFSYSTEM == BIFWIN16 )
	HINSTANCE inst;
	unsigned long res = 0;
	LPFNDEVCAPS DeviceCapabilities;
	char libname[20];

	wsprintf( libname, "%s.drv", m_DevInfo.pszDriver );

	strcpy( pszBuffer, libname );

	inst = LoadLibrary( libname );

	if( (UINT)inst < 32 )
		return 0;

	if( ( DeviceCapabilities = (LPFNDEVCAPS)GetProcAddress( inst, PROC_DEVICECAPABILITIES ) ) != NULL )
		res = DeviceCapabilities( m_DevInfo.pszDevice, m_DevInfo.pszPort, fwCapability, pszBuffer, NULL );
	else
		strcpy( pszBuffer, "LOAD FAULT" );

	if( res == -1 )
		strcpy( pszBuffer, " res fault ");
	FreeLibrary( inst );
	return res;
#endif
#if( BIFSYSTEM == BIFOS2 )
		//DevQueryCaps, hInfoDC, fwCapability, 1, buffer );
	return 0L;
#endif
};

/*--------------------------------------------------------------------------*/
FPaintSpace::FPaintSpace( void )
{
	m_hPS = NULL;
	m_pMapper = NULL;
}

FPaintSpace::~FPaintSpace()
{
}

FPSHandle FPaintSpace::PspGetHandle( void )
{
	return m_hPS;
}

//Now initializes the origin to bottom left at all times.
void FPaintSpace::PspSetMapper( FMapper* pMapper )
{
	m_pMapper = pMapper;
	if( m_pMapper ) {
		m_pMapper->Use( this );
	}
}

void FPaintSpace::PspPix2Log( FPoint* pPixPoint, int nPointCount, Bool fInternal )
{
	if( !m_pMapper )
		return;
	m_pMapper->Pix2Log( this, pPixPoint, nPointCount, fInternal );
}

void FPaintSpace::PspLog2Pix( FPoint* pLogPoint, int nPointCount, Bool fInternal )
{
	if( !m_pMapper )
		return;
	m_pMapper->Log2Pix( this, pLogPoint, nPointCount, fInternal );
}

void FPaintSpace::PspSetAxisOrigin( FAxisOrigin nOrigin )
{
	if( !m_pMapper )
		return;
	m_pMapper->SetAxisOrigin( this, nOrigin );
}

void FPaintSpace::PspSetTranslaion( FPoint* pOffset )
{
	if( !m_pMapper )
		return;
	m_pMapper->SetTranslaion( this, pOffset );
}

void FPaintSpace::PspSetScalation( FPoint* pScale )
{
	if( !m_pMapper )
		return;
	m_pMapper->SetScalation( this, pScale );
}

FMapper::FMapper( void )
{
}

FMapper::~FMapper()
{
}

FDefaultMapper::FDefaultMapper( void )
{
}

FDefaultMapper::~FDefaultMapper()
{
}

void FDefaultMapper::Pix2Log( FPaintSpace* pPS, FPoint* pPixPoint, int nPointCount, Bool fInternal )
{
	if( fInternal )
		return;

#if( BIFSYSTEM == BIFWIN16 )
	DPtoLP( pPS->PspGetHandle(), pPixPoint, nPointCount );
#endif

#if( BIFSYSTEM == BIFOS2 )
	GpiConvert( pPS->PspGetHandle(), CVTC_DEVICE, CVTC_WORLD, nPointCount, pPixPoint );
#endif
}

void FDefaultMapper::Log2Pix( FPaintSpace* pPS, FPoint* pLogPoint, int nPointCount, Bool fInternal )
{
	if( fInternal )
		return;

#if( BIFSYSTEM == BIFWIN16 )
	LPtoDP( pPS->PspGetHandle(), pLogPoint, nPointCount );
#endif

#if( BIFSYSTEM == BIFOS2 )
	GpiConvert( pPS->PspGetHandle(), CVTC_WORLD, CVTC_DEVICE, nPointCount, pLogPoint );
#endif
}

void FDefaultMapper::SetTranslaion( FPaintSpace* pPS, FPoint* pOffset )
{
#if( BIFSYSTEM == BIFWIN16 )
	::SetWindowOrg( pPS->PspGetHandle(), -pOffset->GetX(), -pOffset->GetY() );
#endif
}

void FDefaultMapper::SetScalation( FPaintSpace* pPS, FPoint* pScale )
{
#if( BIFSYSTEM == BIFWIN16 )
	::SetWindowExt(
		pPS->PspGetHandle(),
		LOWORD( ::GetWindowExt( pPS->PspGetHandle() ) / pScale->GetX() ),
		HIWORD( ::GetWindowExt( pPS->PspGetHandle() ) / pScale->GetY() )
	);
#endif
}

void FDefaultMapper::FlipXY( FPaintSpace* pPS )
{
#if( BIFSYSTEM == BIFOS2 )
	MATRIXLF mat;

	::GpiQueryModelTransformMatrix( pPS->PspGetHandle(), 9, &mat );
	FIXED t1, t2;

	t1 = mat.fxM11;
	t2 = mat.fxM21;

	mat.fxM11 = -mat.fxM12;
	mat.fxM21 = -mat.fxM22;

	mat.fxM12 = t1;
	mat.fxM22 = t2;

	::GpiSetModelTransformMatrix( pPS->PspGetHandle(), 9, &mat, TRANSFORM_REPLACE );
#endif
}

void FDefaultMapper::SetPhysicalDimensions( FPaintSpace* pPS, FAxisOrigin org, FPoint v, int ymult )
{
#if( BIFSYSTEM == BIFWIN16 )
	switch( org ) {
		case ao_center:
			::SetViewportExt( pPS->PspGetHandle(), v.GetX(), v.GetY() * ymult );
			::SetViewportOrg( pPS->PspGetHandle(), v.x / 2, v.y / 2 );
		break;
		case ao_top_left:
			::SetViewportExt( pPS->PspGetHandle(), v.GetX(), -v.GetY()* ymult );
			::SetViewportOrg( pPS->PspGetHandle(),       0,       0 );
		break;
		case ao_top_right:
			::SetViewportExt( pPS->PspGetHandle(), -v.GetX(), -v.GetY()* ymult );
			::SetViewportOrg( pPS->PspGetHandle(),      v.x,       0 );
		break;
		case ao_bottom_left:
			::SetViewportExt( pPS->PspGetHandle(), v.GetX(), v.GetY() * ymult);
			::SetViewportOrg( pPS->PspGetHandle(),       0,     v.y );
		break;
		case ao_bottom_right:
			::SetViewportExt( pPS->PspGetHandle(), -v.GetX(), v.GetY()* ymult );
			::SetViewportOrg( pPS->PspGetHandle(),      v.x,     v.y );
		break;
	}
#endif
#if( BIFSYSTEM == BIFOS2 )
	MATRIXLF mat = {
		MAKEFIXED( 1,0 ), 0               , 0,
		0               , MAKEFIXED( 1,0 ), 0,
		0               , 0               , 1
	};

	switch( org ) {
		case ao_center:
			mat.lM31 = v.x / 2;
			mat.lM32 = v.y / 2;
		break;
		case ao_top_left:
			mat.lM32 = v.y;
			mat.fxM22 = MAKEFIXED( -1, 0 );
		break;
		case ao_top_right:
			mat.lM32 = v.y;
			mat.fxM22 = MAKEFIXED( -1, 0 );
			mat.lM31 = v.x;
			mat.fxM11 = MAKEFIXED( -1, 0 );
		break;
		case ao_bottom_left:
		break;
		case ao_bottom_right:
			mat.lM31 = v.x;
			mat.fxM11 = MAKEFIXED( -1, 0 );
		break;
	}
	::GpiSetModelTransformMatrix( pPS->PspGetHandle(), 9, &mat, TRANSFORM_ADD);
#endif
}

FAbsoluteMapper::FAbsoluteMapper( int nMapMode )
{
	m_MapMode = nMapMode;
}

FAbsoluteMapper::~FAbsoluteMapper()
{
}

void FAbsoluteMapper::Use( FPaintSpace* pPS )
{
#if( BIFSYSTEM == BIFWIN16 )
	::SetMapMode( pPS->PspGetHandle(), m_MapMode );
#endif
#if( BIFSYSTEM == BIFOS2 )
	SIZEL p = { 0, 0 };
	::GpiSetPS( pPS->PspGetHandle(), &p, m_MapMode | GPIF_DEFAULT | PS_NORESET );
#endif
}

void FAbsoluteMapper::SetAxisOrigin( FPaintSpace* pPS, FAxisOrigin nOrigin )
{
#if( BIFSYSTEM == BIFWIN16 )

	FPoint v; //Device coordinates
	FPoint w; //Logical coordinates

	//Reset mapping to something that windows can use
	Use( pPS );
	::SetViewportOrg( pPS->PspGetHandle(), 0, 0 );

	pPS->PspGetSize( &v );

	//Get size of the paintspace in pixels
	w = v;
	//Convert to logical units in the prefered mapping mode
	Pix2Log( pPS, &w, 1 );

	::SetMapMode( pPS->PspGetHandle(), MM_ISOTROPIC );
	::SetWindowExt( pPS->PspGetHandle(), w.GetX(), w.GetY() );

	if( m_MapMode == MM_TEXT )
		SetPhysicalDimensions( pPS, nOrigin, v, -1 );
	else
		SetPhysicalDimensions( pPS, nOrigin, v, 1 );
#endif
#if( BIFSYSTEM == BIFOS2 )
	FPoint v; //Device coordinates

	Use( pPS );

	pPS->PspGetSize( &v );

	SetPhysicalDimensions( pPS, nOrigin, v, 1 );
#endif
}

FRelativeMapper::FRelativeMapper( FPoint extent, FProjectionMethods nProjectionMethod )
{
	m_RelativeSize = extent;
	m_ProjectionMethod = nProjectionMethod;
}

FRelativeMapper::~FRelativeMapper()
{
}

void FRelativeMapper::Use( FPaintSpace* pPS )
{
#if( BIFSYSTEM == BIFWIN16 )
	::SetMapMode( pPS->PspGetHandle(), m_ProjectionMethod );
#endif
#if( BIFSYSTEM == BIFOS2 )
	SIZEL p;
	p.cx = m_RelativeSize.GetX();
	p.cy = m_RelativeSize.GetY();
	::GpiSetPS( pPS->PspGetHandle(), &p, m_ProjectionMethod | GPIF_DEFAULT | PS_NORESET );
#endif
}

void FRelativeMapper::SetAxisOrigin( FPaintSpace* pPS, FAxisOrigin nOrigin )
{
#if( BIFSYSTEM == BIFWIN16 )

	FPoint v; //Device coordinates

	//Reset mapping to something that windows can use
	Use( pPS );
	::SetViewportOrg( pPS->PspGetHandle(), 0, 0 );

	pPS->PspGetSize( &v );

	::SetMapMode( pPS->PspGetHandle(), m_ProjectionMethod );
	::SetWindowExt( pPS->PspGetHandle(), m_RelativeSize.GetX(), m_RelativeSize.GetY() );
	SetPhysicalDimensions( pPS, nOrigin, v, -1 );
#endif
#if( BIFSYSTEM == BIFOS2 )
	FPoint v; //Device coordinates
	SIZEL p;

	Use( pPS );

	pPS->PspGetSize( &v );

	p.cx = m_RelativeSize.GetX();
	p.cy = m_RelativeSize.GetY();

	MATRIXLF mat;
	RECTL rec = {
		0,
		0,
		v.GetX(),
		v.GetY()
	};
	::GpiSetPS( pPS->PspGetHandle(), &p, m_ProjectionMethod | GPIF_DEFAULT | PS_NORESET );
	::GpiSetPageViewport( pPS->PspGetHandle(), &rec );
	SetPhysicalDimensions( pPS, nOrigin, m_RelativeSize, 1 );
#endif
}

/*--------------------------------------------------------------------------*/

FDevicePaintSpace::FDevicePaintSpace( FDevice* pDevice )
{
	m_pDevice = pDevice;
#if( BIFSYSTEM == BIFWIN16 )
	m_JobInfo.cbSize = sizeof( m_JobInfo );
	m_JobInfo.lpszDocName = NULL;
	m_JobInfo.lpszOutput = NULL;
#endif
#if( BIFSYSTEM == BIFOS2 )
#endif
}

FDevicePaintSpace::~FDevicePaintSpace()
{
#if( BIFSYSTEM == BIFWIN16 )
	if( m_JobInfo.lpszDocName )
		free( (void*)m_JobInfo.lpszDocName );
	if( m_JobInfo.lpszOutput )
		free( (void*)m_JobInfo.lpszOutput );
#endif
#if( BIFSYSTEM == BIFOS2 )
#endif
}

void FDevicePaintSpace::PspUse( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hPS = ::CreateDC( m_pDevice->m_DevInfo.pszDriver, m_pDevice->m_DevInfo.pszDevice, m_pDevice->m_DevInfo.pszPort, NULL );
#endif
#if( BIFSYSTEM == BIFOS2 )
	SIZEL p = { 0, 0 };
	m_JobInfo.OS2PrintDC = ::DevOpenDC(
										 12345L,
										 OD_QUEUED,
										 "*",
										 9,
										 (PDEVOPENDATA)&(m_pDevice->m_DevInfo),
										 (HDC)0
									);
	m_hPS = ::GpiCreatePS( 12345L, m_JobInfo.OS2PrintDC, &p, PU_LOMETRIC | GPIA_ASSOC );
#endif
}

void FDevicePaintSpace::PspRelease( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	::DeleteDC( m_hPS );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::GpiDestroyPS( m_hPS );
	::DevCloseDC( m_JobInfo.OS2PrintDC );
#endif
	m_hPS = NULL;
}

void FDevicePaintSpace::PspGetSize( FPoint* pPoint )
{
#if( BIFSYSTEM == BIFWIN16 )
	*pPoint = FPoint( ::GetDeviceCaps( PspGetHandle(), HORZRES ), ::GetDeviceCaps( PspGetHandle(), VERTRES ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
#endif
}

int FDevicePaintSpace::PspAbortJob( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	return ::AbortDoc( m_hPS );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return ::DevEscape( m_JobInfo.OS2PrintDC, DEVESC_ABORTDOC, 0, 0, 0, 0 );
#endif
}

int FDevicePaintSpace::PspEndJob( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	return ::EndDoc( m_hPS );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return ::DevEscape( m_JobInfo.OS2PrintDC, DEVESC_ENDDOC, 0, 0, 0, 0 );
#endif
}

int FDevicePaintSpace::PspEndPage( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	return ::EndPage( m_hPS );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return ::DevEscape( m_JobInfo.OS2PrintDC, DEVESC_NEWFRAME, 0, 0, 0, 0 );
#endif
}

#if( BIFSYSTEM == BIFWIN16 )
int FDevicePaintSpace::PspSetAbortProc( ABORTPROC pfnAbortProc )
{
	return ::SetAbortProc( m_hPS, pfnAbortProc );
}
#endif

//If docname is null the document name remains unchanged, otherwise
//the previous name is free'd and a copy of docname becomes the new name
int FDevicePaintSpace::PspStartJob( const char* pszDocumentName )
{
#if( BIFSYSTEM == BIFWIN16 )
	if( pszDocumentName ) {
		//If there was already a document name then delete it.
		if( m_JobInfo.lpszDocName )
			free( (void*)m_JobInfo.lpszDocName );

		//Set the new name
		m_JobInfo.lpszDocName = strdup( pszDocumentName );
	}
	//If the documentname is NULL then nothing will hapen when you call this
	return ::StartDoc( m_hPS, &m_JobInfo );
#endif

#if( BIFSYSTEM == BIFOS2 )
	if( !pszDocumentName )
		pszDocumentName = "BIF Job";

	return ::DevEscape( m_JobInfo.OS2PrintDC, DEVESC_STARTDOC, strlen( pszDocumentName ), (char*)pszDocumentName, 0, 0 );
#endif
}

int FDevicePaintSpace::PspStartPage( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	return ::StartPage( m_hPS );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return 0;
#endif
}

//If filename is NULL, redirection will be canceled
void FDevicePaintSpace::PspRedirectOutput( const char* pszFileName )
{
#if( BIFSYSTEM == BIFWIN16 )
	if( m_JobInfo.lpszOutput )
		delete (void*)m_JobInfo.lpszOutput;

	m_JobInfo.lpszOutput = NULL;

	if( pszFileName )
		m_JobInfo.lpszOutput = strdup( pszFileName );
#endif
#if( BIFSYSTEM == BIFOS2 )
#endif
}
/*--------------------------------------------------------------------------*/
FClientPaintSpace::FClientPaintSpace( FWnd* pWnd )
{
	m_pWnd = pWnd;
}

FClientPaintSpace::~FClientPaintSpace()
{
}

void FClientPaintSpace::PspUse( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hPS = ::GetDC( m_pWnd->GetHwnd() );
#endif
#if( BIFSYSTEM == BIFOS2 )
	m_hPS = ::WinGetPS( m_pWnd->GetHwnd() );
	//Switch the paintspace into real color mode
	GpiCreateLogColorTable( m_hPS, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );
#endif
}

void FClientPaintSpace::PspRelease( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	::ReleaseDC( m_pWnd->GetHwnd(), m_hPS );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::WinReleasePS( m_hPS );
#endif
	m_hPS = NULL;
}

void FClientPaintSpace::PspGetSize( FPoint* pPoint )
{
	FRect r;
#if( BIFSYSTEM == BIFWIN16 )
	::GetClientRect( m_pWnd->GetHwnd(), &r );
	*pPoint = FPoint( r.GetRight(), r.GetBottom() );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::WinQueryWindowRect( m_pWnd->GetHwnd(), &r );
	*pPoint = FPoint( r.GetRight(), r.GetTop() );
#endif
}

FEventPaintSpace::FEventPaintSpace( FWnd* pWnd ) : FClientPaintSpace( pWnd )
{
}

FWindowPaintSpace::FWindowPaintSpace( FWnd* pWnd ) : FClientPaintSpace( pWnd )
{
}

FWindowPaintSpace::~FWindowPaintSpace()
{
}

void FWindowPaintSpace::PspUse()
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hPS = ::GetWindowDC( m_pWnd->GetHwnd() );
#endif
#if( BIFSYSTEM == BIFOS2 )
	GpiCreateLogColorTable( m_hPS, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );
#endif
}

void FWindowPaintSpace::PspRelease()
{
#if( BIFSYSTEM == BIFWIN16 )
	::ReleaseDC( m_pWnd->GetHwnd(), m_hPS );
#endif
#if( BIFSYSTEM == BIFOS2 )
#endif
	m_hPS = NULL;
}

void FWindowPaintSpace::PspGetSize( FPoint* pPoint )
{
#if( BIFSYSTEM == BIFWIN16 )
	FRect r;
	GetWindowRect( m_pWnd->GetHwnd(), &r );
	*pPoint = FPoint( r.GetRight() - r.GetLeft(), r.GetBottom() - r.GetTop() );
#endif
#if( BIFSYSTEM == BIFOS2 )
#endif
}

FEventPaintSpace::~FEventPaintSpace()
{
}

void FEventPaintSpace::PspUse( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hPS = ::BeginPaint( m_pWnd->GetHwnd(), &m_PaintStruct );
#endif
#if( BIFSYSTEM == BIFOS2 )
	m_hPS = ::WinBeginPaint( m_pWnd->GetHwnd(), NULLHANDLE, NULL );
	GpiCreateLogColorTable( m_hPS, LCOL_RESET, LCOLF_RGB, 0, 0, NULL );
#endif
}

void FEventPaintSpace::PspRelease( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	::EndPaint( m_pWnd->GetHwnd(), &m_PaintStruct );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::WinEndPaint( m_hPS );
#endif
	m_hPS = NULL;
}

/*--------------------------------------------------------------------------*/
FCompatiblePaintSpace::FCompatiblePaintSpace( FPaintSpace* pTemplatePS )
{
	m_TemplatePaintSpace = pTemplatePS;
}

FCompatiblePaintSpace::~FCompatiblePaintSpace()
{
}

void FCompatiblePaintSpace::PspUse( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hPS = ::CreateCompatibleDC( m_TemplatePaintSpace->PspGetHandle() );
#endif

#if( BIFSYSTEM == BIFOS2 )
	HDC hdc;
	SIZEL s = { 0, 0 };
	m_hOS2DC = ::DevOpenDC( 12345L, OD_MEMORY, "*", 5, NULL, m_TemplatePaintSpace->PspGetHandle() );
	m_hPS = ::GpiCreatePS( 12345L, m_hOS2DC, &s, GPIA_ASSOC | PU_PELS );
#endif
}

void FCompatiblePaintSpace::PspRelease( void )
{
#if( BIFSYSTEM == BIFWIN16 )
	::DeleteDC( m_hPS );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::GpiDestroyPS( m_hPS );
	::DevCloseDC( m_hOS2DC );
#endif
	m_hPS = NULL;
}

void FCompatiblePaintSpace::PspGetSize( FPoint* pPoint )
{
	*pPoint = FPoint( 0, 0 );
}

/*--------------------------------------------------------------------------*/
FBitmap::FBitmap( FBitmapHandle hTemplate )
{
	m_hBitmap = hTemplate;

#if( BIFSYSTEM == BIFWIN16 )
	if( m_hBitmap )
		::GetObject( m_hBitmap, sizeof( m_Bitmap ), &m_Bitmap );
#endif
}

FBitmap::~FBitmap()
{
	if( m_hBitmap )
		Delete();
}

FGdiObjHandle FBitmap::GetHandle( void )
{
	return (FGdiObjHandle)m_hBitmap;
}

void FBitmap::GetSize( FPoint* pPoint )
{
#if( BIFSYSTEM == BIFWIN16 )
	*pPoint = FPoint( m_Bitmap.bmWidth, m_Bitmap.bmHeight );
#endif
}

Bool FBitmap::Create( FPoint size, int cbPlanes, int cbBits, void* lpvBits )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_Bitmap.bmType = 0;
	m_Bitmap.bmWidth = size.GetX();
	m_Bitmap.bmHeight = size.GetY();
	long x = m_Bitmap.bmWidth;
	x *= m_Bitmap.bmPlanes;
	x *= m_Bitmap.bmBitsPixel;
	x /= 8;
	if( x % 2 == 1 )
		x++;

	m_Bitmap.bmWidthBytes = x;
	m_Bitmap.bmPlanes = cbPlanes;
	m_Bitmap.bmBitsPixel = cbBits;
	m_Bitmap.bmBits = lpvBits;

	m_hBitmap = CreateBitmapIndirect( &m_Bitmap );

	return Bool( m_hBitmap != NULL );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return False;
#endif
}

Bool FBitmap::Create( const char* pszResourceName )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hBitmap = ::LoadBitmap( GetCurrentApp()->GetHINSTANCE(), pszResourceName );
	if( m_hBitmap )
		::GetObject( m_hBitmap, sizeof( m_Bitmap ), &m_Bitmap );
	return Bool( m_hBitmap != NULL );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return False;
#endif
}

Bool FBitmap::Delete( void )
{
	if( !m_hBitmap )
		return False;

	#if( BIFSYSTEM == BIFWIN16 )
	if( DeleteObject( m_hBitmap ) ) {
		m_hBitmap = NULL;
		return True;
	}
	return False;
#endif
#if( BIFSYSTEM == BIFOS2 )
	return False;
#endif
}

FPen::FPen( FPenHandle hTemplate )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hPen = hTemplate;
	if( m_hPen )
		::GetObject( m_hPen, sizeof( m_LogPen ), &m_LogPen );
#endif
#if( BIFSYSTEM == BIFOS2 )
	if( hTemplate ) {
		m_LogPen = *hTemplate;
		m_hPen = &m_LogPen;
	}
	else
		m_hPen = NULL;
#endif
}

FPen::FPen( FPenStyle nStyle, FColorRef nBaseColor, FPoint size )
{
	m_hPen = NULL;
	Create( nStyle, nBaseColor, size );
}

FPen::~FPen()
{
	if( m_hPen )
		Delete();
}

FGdiObjHandle FPen::GetHandle( void )
{
	return m_hPen;
}

Bool FPen::Create( FPenStyle nStyle, FColorRef nBaseColor, FPoint size )
{
	if( m_hPen )
		return False;

#if( BIFSYSTEM == BIFWIN16 )
	m_LogPen.lopnStyle = nStyle;
	m_LogPen.lopnWidth = size;
	m_LogPen.lopnColor = nBaseColor;

	m_hPen = CreatePenIndirect( &m_LogPen );
#endif
#if( BIFSYSTEM == BIFOS2 )
	//col = CLR_BLUE;
	m_LogPen.lColor = nBaseColor;
	m_LogPen.lBackColor = nBaseColor;
	m_LogPen.usMixMode = FM_OVERPAINT;
	m_LogPen.usBackMixMode = BM_LEAVEALONE;
	m_LogPen.fxWidth = 1;
	m_LogPen.lGeomWidth = 1;
	m_LogPen.usType = nStyle;
	m_LogPen.usEnd = LINEEND_FLAT;
	m_LogPen.usJoin = LINEJOIN_BEVEL;

	m_hPen = &m_LogPen;
#endif
	return Bool( m_hPen != NULL );
}

Bool FPen::Delete( void )
{
	if( !m_hPen )
		return False;

#if( BIFSYSTEM == BIFWIN16 )
	if( DeleteObject( m_hPen ) ) {
		m_hPen = NULL;
		return True;
	}
	return False;
#endif
#if( BIFSYSTEM == BIFOS2 )
	m_hPen = NULL;
	return True;
#endif
}


FSolidPen::FSolidPen( FPenHandle hTemplate ) : FPen( hTemplate )
{
}

FSolidPen::FSolidPen( FColorRef nBaseColor ) : FPen( NULL )
{
	Create( nBaseColor );
}

FSolidPen::~FSolidPen()
{
}

FSolidPen::Create( FColorRef nBaseColor )
{
	return FPen::Create( ps_solid, nBaseColor, FPoint( 1, 1 ) );
}

FBrush::FBrush( FBrushHandle hTemplate )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hBrush = hTemplate;
	if( m_hBrush )
		::GetObject( m_hBrush, sizeof( m_LogBrush ), &m_LogBrush );
#endif
#if( BIFSYSTEM == BIFOS2 )
	if( hTemplate ) {
		m_LogBrush = *hTemplate;
		m_hBrush = &m_LogBrush;
	}
	else
		m_hBrush = NULL;
#endif
}

FBrush::~FBrush()
{
	if( m_hBrush )
		Delete();
}

FGdiObjHandle FBrush::GetHandle()
{
	return m_hBrush;
}

Bool FBrush::Create( FBrushStyle nStyle, FColorRef nBaseColor, FHatchMethod hatchmethod )
{
	if( m_hBrush )
		return False;

#if( BIFSYSTEM == BIFWIN16 )
	m_LogBrush.lbStyle = nStyle;
	m_LogBrush.lbColor = nBaseColor;
	m_LogBrush.lbHatch = hatchmethod;
	m_hBrush = CreateBrushIndirect( &m_LogBrush );
#endif
#if( BIFSYSTEM == BIFOS2 )
	m_LogBrush.lColor = nBaseColor;
	m_LogBrush.lBackColor = nBaseColor;
	m_LogBrush.usMixMode = FM_OVERPAINT;
	m_LogBrush.usBackMixMode = BM_LEAVEALONE;
	m_LogBrush.usSet = LCID_DEFAULT;
	m_LogBrush.usSymbol = PATSYM_SOLID;
	m_LogBrush.ptlRefPoint.x = 0;
	m_LogBrush.ptlRefPoint.y = 0;
	m_hBrush = &m_LogBrush;
#endif

	return Bool( m_hBrush != NULL );
}

Bool FBrush::Delete( void )
{
	if( !m_hBrush )
		return False;

#if( BIFSYSTEM == BIFWIN16 )
	if( DeleteObject( m_hBrush ) ) {
		m_hBrush = NULL;
		return True;
	}
	return False;
#endif
#if( BIFSYSTEM == BIFOS2 )
	m_hBrush = NULL;
	return True;
#endif
}

FSolidBrush::FSolidBrush( FBrushHandle hTemplate ) : FBrush( hTemplate )
{
}

FSolidBrush::FSolidBrush( FColorRef nBaseColor ) : FBrush( NULL )
{
	Create( nBaseColor );
}

FSolidBrush::~FSolidBrush() {
}

Bool FSolidBrush::Create( FColorRef nBaseColor )
{
	if( m_hBrush )
		return False;

	return FBrush::Create( bs_solid, nBaseColor, hs_none );
}

/*--------------------------------------------------------------------------*/
FFont::FFont( FFontHandle hTemplate )
{
#if( BIFSYSTEM == BIFWIN16 )
	m_hFont = hTemplate;
	if( m_hFont )
		::GetObject( m_hFont, sizeof( m_LogFont ), &m_LogFont );
#endif
#if( BIFSYSTEM == BIFOS2 )
	if( hTemplate ) {
		m_LogFont = *hTemplate;
		m_hFont = &m_LogFont;
	}
	else
		m_hFont = NULL;
#endif
};

FFont::FFont( int nHeight, const char* pszFontName )
{
	m_hFont = NULL;
	Create( nHeight, pszFontName );
}

FFont::~FFont()
{
	if( m_hFont )
		Delete();
};

Bool FFont::Create( int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight,
						BYTE fbItalic, BYTE fbUnderline, BYTE fbStrikeOut, BYTE fbCharSet, BYTE fbOutputPrecision,
						BYTE fbClipPrecision, BYTE fbQuality, BYTE fbPitchAndFamily, const char* pszFace )
{

	if( m_hFont )
		return False;

#if( BIFSYSTEM == BIFWIN16 )
	m_LogFont.lfHeight = nHeight;
	m_LogFont.lfWidth = nWidth;
	m_LogFont.lfEscapement = nEscapement;
	m_LogFont.lfOrientation= nOrientation;
	m_LogFont.lfWeight = fnWeight;
	m_LogFont.lfItalic = fbItalic;
	m_LogFont.lfUnderline = fbUnderline;
	m_LogFont.lfStrikeOut = fbStrikeOut;
	m_LogFont.lfCharSet = fbCharSet;
	m_LogFont.lfOutPrecision = fbOutputPrecision;
	m_LogFont.lfClipPrecision = fbClipPrecision;
	m_LogFont.lfQuality = fbQuality;
	m_LogFont.lfPitchAndFamily = fbPitchAndFamily;
	memcpy( m_LogFont.lfFaceName, pszFace, strlen( pszFace ) );

	m_hFont = ::CreateFontIndirect( &m_LogFont );
#endif
#if( BIFSYSTEM == BIFOS2 )
	m_LogFont.usRecordLength  = sizeof(FATTRS);
	m_LogFont.fsSelection     = 0;
	m_LogFont.lMatch          = 0;
	m_LogFont.idRegistry      = 0;
	m_LogFont.usCodePage      = 0;
	m_LogFont.lMaxBaselineExt = /*nHeight * 2;*/0;
	m_LogFont.lAveCharWidth   = /*nHeight;*/0;
	m_LogFont.fsType          = 0;
	m_LogFont.fsFontUse       = 0;
	strcpy( m_LogFont.szFacename, pszFace );
	m_hFont = &m_LogFont;
#endif
	return Bool( m_hFont != NULL );
}

Bool FFont::Create( int nHeight, const char* pszFontName )
{
	return Create( nHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, pszFontName );
}

Bool FFont::Delete( void )
{
	if( !m_hFont )
		return False;

#if( BIFSYSTEM == BIFWIN16 )
	if( DeleteObject( m_hFont ) ) {
		m_hFont = NULL;
		return True;
	}
#endif
#if( BIFSYSTEM == BIFOS2 )
	m_hFont = NULL;
#endif
	return False;
}

FGdiObjHandle FFont::GetHandle( void )
{
	return m_hFont;
}

/*--------------------------------------------------------------------------*/
Bool FCaret::m_bEnabled = False;
int FCaret::m_nCsrWidth = 0;
int FCaret::m_nCsrHeight = 0;

Bool FCaret::Create( FClientPaintSpace* pPs, FPoint Size )
{
	FPoint l_Org( 0, 0 );

	pPs->PspLog2Pix( &l_Org, 1 );
	pPs->PspLog2Pix( &Size, 1 );

	m_nCsrWidth = Size.GetX() - l_Org.GetX();
	m_nCsrHeight = Size.GetY() - l_Org.GetX();

	if( m_nCsrWidth < 0 ) m_nCsrWidth = -m_nCsrWidth;
	if( m_nCsrHeight < 0 ) m_nCsrHeight = -m_nCsrHeight;

#if( BIFSYSTEM == BIFWIN16 )
		::CreateCaret( pPs->GetWnd()->GetHwnd(), NULL, m_nCsrWidth, m_nCsrHeight );
#endif
#if( BIFSYSTEM == BIFOS2 )
		::WinCreateCursor( pPs->GetWnd()->GetHwnd(), 0, 0, m_nCsrWidth, m_nCsrHeight, CURSOR_SOLID | CURSOR_FLASH, NULL );
#endif
	m_bEnabled = True;
	return True;
}

Bool FCaret::Destroy( FClientPaintSpace* pPs )
{
#if( BIFSYSTEM == BIFWIN16 )
	::DestroyCaret();
#endif
#if( BIFSYSTEM == BIFOS2 )
	::WinDestroyCursor( pPs->GetWnd()->GetHwnd() );
#endif
	m_bEnabled = False;
	return True;
}

Bool FCaret::Show( FClientPaintSpace* pPs )
{
	if( !m_bEnabled )
		return False;

#if( BIFSYSTEM == BIFWIN16 )
	::ShowCaret( pPs->GetWnd()->GetHwnd() );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::WinShowCursor( pPs->GetWnd()->GetHwnd(), True );
#endif
	return True;
}

Bool FCaret::Hide( FClientPaintSpace* pPs )
{
	if( !m_bEnabled )
		return False;

#if( BIFSYSTEM == BIFWIN16 )
	::HideCaret( pPs->GetWnd()->GetHwnd() );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::WinShowCursor( pPs->GetWnd()->GetHwnd(), False );
#endif
	return True;
}

Bool FCaret::Move( FClientPaintSpace* pPs, FPoint NewPos )
{
	if( !m_bEnabled )
		return False;

	FPoint l_Org( 0, 0 );

	pPs->PspLog2Pix( &l_Org, 1 );
	pPs->PspLog2Pix( &NewPos, 1 );

	int x = NewPos.GetX() - l_Org.GetX();
	int y = NewPos.GetY() - l_Org.GetX();

	if( x < 0 ) x = -x;
	if( y < 0 ) y = -y;

#if( BIFSYSTEM == BIFWIN16 )
	::SetCaretPos( x , y );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::WinCreateCursor( pPs->GetWnd()->GetHwnd(), x, y, 0, 0, CURSOR_SETPOS, NULL );
#endif
	return True;
}
/*--------------------------------------------------------------------------*/

FPaintBox::FPaintBox( void )
{
	m_pPaintSpace = NULL;
};

FPaintBox::~FPaintBox()
{
}

FPaintSpace* FPaintBox::GetPS( void )
{
	return m_pPaintSpace;
}

void FPaintBox::SetPS( FPaintSpace* pPS )
{
	m_pPaintSpace = pPS;
}

FObjectSelector::FObjectSelector( void )
{
	m_Selection.hOldPen = NULL;
	m_Selection.hOldBrush = NULL;
	m_Selection.hOldFont = NULL;
	m_Selection.hOldBitmap = NULL;
}

void FObjectSelector::SelectPen( FPen* pObject )
{
#if( BIFSYSTEM == BIFWIN16 )
	if( pObject ) {
		//We are selecting a new pen but there were already a pen selected
		if( m_Selection.hOldPen )
			//Remove the old selection
			SelectPen( NULL );
		//Set new selection
		m_Selection.hOldPen = (FPenHandle)SelectObject( GetPS()->PspGetHandle(), pObject->GetHandle() );
	}
	else {
		if( m_Selection.hOldPen )
			SelectObject( GetPS()->PspGetHandle(), m_Selection.hOldPen );
		m_Selection.hOldPen = NULL;
	}
#endif
#if( BIFSYSTEM == BIFOS2 )
	if( !pObject )
		return;
	FPenHandle lb = (FPenHandle)pObject->GetHandle();
	GpiSetAttrs( GetPS()->PspGetHandle(), PRIM_LINE, LBB_ALL, 0, lb );
#endif
}

void FObjectSelector::SelectBrush( FBrush* pObject )
{
#if( BIFSYSTEM == BIFWIN16 )
	if( pObject ) {
		if( m_Selection.hOldBrush )
			SelectBrush( NULL );
		m_Selection.hOldBrush = (FBrushHandle)SelectObject( GetPS()->PspGetHandle(), pObject->GetHandle() );
	}
	else {
		if( m_Selection.hOldBrush )
			SelectObject( GetPS()->PspGetHandle(), m_Selection.hOldBrush );
		m_Selection.hOldBrush = NULL;
	}
#endif
#if( BIFSYSTEM == BIFOS2 )
	if( !pObject )
		return;
	FBrushHandle lb = (FBrushHandle)pObject->GetHandle();
	GpiSetAttrs( GetPS()->PspGetHandle(), PRIM_AREA, ABB_ALL, 0, lb );
#endif
}

void FObjectSelector::SelectFont( FFont* pObject )
{
#if( BIFSYSTEM == BIFWIN16 )
	if( pObject ) {
		if( m_Selection.hOldFont )
			SelectFont( NULL );
		m_Selection.hOldFont = (FFontHandle)SelectObject( GetPS()->PspGetHandle(), pObject->GetHandle() );
	}
	else {
		if( m_Selection.hOldFont )
			SelectObject( GetPS()->PspGetHandle(), m_Selection.hOldFont );
		m_Selection.hOldBrush = NULL;
	}
#endif
#if( BIFSYSTEM == BIFOS2 )
/*
	m_LogFont.cb.lColor = SYSCLR_WINDOWTEXT;
	m_LogFont.cb.lBackColor = SYSCLR_WINDOW;
	m_LogFont.cb.usMixMode = FM_OVERPAINT;
	m_LogFont.cb.usBackMixMode = BM_LEAVEALONE;
	m_LogFont.cb.usPrecision = CH_MODE1;
	m_LogFont.cb.ptlAngle.x = 1;
	m_LogFont.cb.ptlAngle.y = 0;
	m_LogFont.cb.ptlShear.x = 0;
	m_LogFont.cb.ptlShear.x = 1;
	m_LogFont.cb.usDirection = CBB_DIRECTION;
	m_LogFont.cb.fxExtra = 0;
	m_LogFont.cb.fxBreakExtra = 0;
	m_LogFont.cb.usSet = ;
	m_LogFont.cb.sizfxCell = ;
	m_LogFont.cb.usTextAlign = ;
*/
	if( pObject ) {
		FFontHandle lb = (FFontHandle)pObject->GetHandle();
		GpiCreateLogFont( GetPS()->PspGetHandle(), NULL, 1, lb );
		GpiSetCharSet( GetPS()->PspGetHandle(), 1 );
	}
	else {
		GpiDeleteSetId( GetPS()->PspGetHandle(), 1);
	}
#endif
}

void FObjectSelector::SelectBitmap( FBitmap* pObject )
{
#if( BIFSYSTEM == BIFWIN16 )
	if( pObject ) {
		if( m_Selection.hOldBitmap )
			SelectBitmap( NULL );
		m_Selection.hOldBitmap = (FBitmapHandle)SelectObject( GetPS()->PspGetHandle(), pObject->GetHandle() );
	}
	else {
		if( m_Selection.hOldBitmap )
			SelectObject( GetPS()->PspGetHandle(), m_Selection.hOldBitmap );
		m_Selection.hOldBitmap = NULL;
	}
#endif
#if( BIFSYSTEM == BIFOS2 )
#endif
}

FBlitterPaintBox::FBlitterPaintBox( FPaintSpace* pTargetPS, FPaintSpace* pSourcePS )
{
	m_pTargetPS = pTargetPS;
	m_pSourcePS = pSourcePS;
}

FBlitterPaintBox::~FBlitterPaintBox()
{
}

Bool FBlitterPaintBox::BitBlit( FPoint offset, FPoint size )
{
#if( BIFSYSTEM == BIFWIN16 )
	return Bool( ::BitBlt( m_pTargetPS->PspGetHandle(), offset.GetX(), offset.GetY(), size.GetX(), size.GetY(), m_pSourcePS->PspGetHandle(), 0, 0, SRCCOPY ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
	//GpiBitBlt( m_pTargetPS->GetHandle(), m_pSourcePS->GetHandle()
	return False;
#endif
}

Bool FBlitterPaintBox::StrechBlt( FRect targetrect, FRect sourcerect, FBlitterMethod nMethod )
{
#if( BIFSYSTEM == BIFWIN16 )
	unsigned long l_method = 0;
	if( nMethod & rop_copy )
		l_method |= SRCCOPY;

	return Bool( ::StretchBlt( m_pTargetPS->PspGetHandle(),
								targetrect.GetTop(),
								targetrect.GetLeft(),
								targetrect.GetRight(),
								targetrect.GetBottom(),
								m_pSourcePS->PspGetHandle(),
								sourcerect.GetTop(),
								sourcerect.GetLeft(),
								sourcerect.GetRight(),
								sourcerect.GetBottom(),
								l_method ) );

#endif
#if( BIFSYSTEM == BIFOS2 )
	return False;
#endif
}

FBitmapPaintBox::FBitmapPaintBox( FPaintSpace* pTargetPS )
{
	m_pTargetPS = pTargetPS;
	m_pSourcePS = new FCompatiblePaintSpace( pTargetPS );
}

FBitmapPaintBox::~FBitmapPaintBox()
{
	if( m_pSourcePS )
		delete m_pSourcePS;
}

Bool FBitmapPaintBox::CopyBitmap( FBitmap* pBitMap, FPoint offset )
{
	FPoint l_BitmapSize;

	pBitMap->GetSize( &l_BitmapSize );

	FObjectSelector* l_pObjectSelector = new FObjectSelector;
	l_pObjectSelector->SetPS( m_pSourcePS );

	m_pSourcePS->PspUse();

	l_pObjectSelector->SelectBitmap( pBitMap );

	FBlitterPaintBox* l_pBlitterPaintBox = new FBlitterPaintBox( m_pTargetPS, m_pSourcePS );
	l_pBlitterPaintBox->BitBlit( offset, l_BitmapSize );
	delete l_pBlitterPaintBox;

	l_pObjectSelector->SelectBitmap( NULL );

	m_pSourcePS->PspRelease();

	delete l_pObjectSelector;

	return True;
}

Bool FPositionPaintBox::MoveTo( FPoint p )
{
#if( BIFSYSTEM == BIFWIN16 )
	return Bool( ::MoveTo( GetPS()->PspGetHandle(), p.x, p.y ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return Bool( GpiMove( GetPS()->PspGetHandle(), &p ) );
#endif
}

FPoint FPositionPaintBox::Where()
{
#if( BIFSYSTEM == BIFWIN16 )
	unsigned long p = GetCurrentPosition( GetPS()->PspGetHandle() );

	return FPoint( LOWORD( p ), HIWORD( p ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
	FPoint l_Point;

	GpiQueryCurrentPosition( GetPS()->PspGetHandle(), &l_Point );

	return l_Point;
#endif
}

/*--------------------------------------------------------------------------*/
Bool FGraphPaintBox::LineTo( FPoint p )
{
#if( BIFSYSTEM == BIFWIN16 )
	return Bool( ::LineTo( GetPS()->PspGetHandle(), p.x, p.y ) );
#endif

#if( BIFSYSTEM == BIFOS2 )
	return Bool( ::GpiLine( GetPS()->PspGetHandle(), &p ) );
#endif
}

Bool FGraphPaintBox::Rect( FRect r )
{
#if( BIFSYSTEM == BIFWIN16 )
	return Bool( ::Rectangle( GetPS()->PspGetHandle(), r.left, r.top, r.right, r.bottom ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
	FPoint l_TopLeft( r.GetLeft(), r.GetTop() );
	FPoint l_BottomRight( r.GetRight(), r.GetBottom() );

	GpiMove( GetPS()->PspGetHandle(), &l_TopLeft );
	return Bool( GpiBox( GetPS()->PspGetHandle(), DRO_OUTLINEFILL, &l_BottomRight,  0, 0 ) );
#endif
}

Bool FGraphPaintBox::Ellipse( FRect r )
{
#if( BIFSYSTEM == BIFWIN16 )
	return Bool( ::Ellipse( GetPS()->PspGetHandle(), r.left, r.top, r.right, r.bottom ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return False;
#endif
}

Bool FGraphPaintBox::Arc( FRect bounding, FPoint start, FPoint end )
{
#if( BIFSYSTEM == BIFWIN16 )
	return Bool( ::Arc( GetPS()->PspGetHandle(),
		bounding.GetLeft(),
		bounding.GetTop(),
		bounding.GetRight(),
		bounding.GetBottom(),
		start.GetX(),
		start.GetY(),
		end.GetX(),
		end.GetY()));
#endif
#if( BIFSYSTEM == BIFOS2 )
	return False;
#endif
}

Bool FGraphPaintBox::Pixel( FPoint p, FColorRef nColor )
{
#if( BIFSYSTEM == BIFWIN16 )
	return Bool( SetPixel( GetPS()->PspGetHandle(), p.GetX(), p.GetY(), nColor ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
	return Bool( GpiSetPel( GetPS()->PspGetHandle(), &p ) );
#endif
}

/*--------------------------------------------------------------------------*/
void FBackgrPaintBox::SetBackgroundColor( FColorRef nBaseColor )
{
#if( BIFSYSTEM == BIFWIN16 )
	SetBkColor( GetPS()->PspGetHandle(), nBaseColor );
#endif
#if( BIFSYSTEM == BIFOS2 )
	GpiSetBackColor( GetPS()->PspGetHandle(), nBaseColor );
#endif
}

void FBackgrPaintBox::SetBackgroundMode( FBackgroundMode mode )
{
#if( BIFSYSTEM == BIFWIN16 )
	SetBkMode( GetPS()->PspGetHandle(), mode );
#endif
#if( BIFSYSTEM == BIFOS2 )
	GpiSetBackMix( GetPS()->PspGetHandle(), mode );
#endif
}
void FBackgrPaintBox::DrawBackgroundRect( FWnd* pWnd )
{
#if( BIFSYSTEM == BIFWIN16 )

#endif
#if( BIFSYSTEM == BIFOS2 )
	//GetPS()->PspGetSize( FPoint* pPoint )
	FRect r;
	WinQueryWindowRect( pWnd->GetHwnd(), &r );
	WinFillRect( GetPS()->PspGetHandle(), &r, FColors::GetWindowBackgroundColor()/*FColors::RGB_LtGrey*/ );
#endif
}
/*--------------------------------------------------------------------------*/
#if( BIFSYSTEM == BIFWIN16 )
char* FTextPaintBox::Helvetica = "Arial";
char* FTextPaintBox::TimesRoman = "Times New Roman";
char* FTextPaintBox::Courier = "Curier";
char* FTextPaintBox::SmallFixed = "Terminal";
char* FTextPaintBox::LargeFixed = "System";
#endif

#if( BIFSYSTEM == BIFOS2 )
char* FTextPaintBox::Helvetica = "Helv";
char* FTextPaintBox::TimesRoman = "Tms Rmn";
char* FTextPaintBox::Courier = "Curier";
char* FTextPaintBox::SmallFixed = "System VIO";
char* FTextPaintBox::LargeFixed = "System VIO";
#endif

Bool FTextPaintBox::TextOut( FPoint start, const char* pszText, int nLength, FRect* pClipBox )
{
#if( BIFSYSTEM == BIFWIN16 )
	if( pClipBox )
		return Bool( ::ExtTextOut( GetPS()->PspGetHandle(), start.x, start.y, ETO_CLIPPED, pClipBox, pszText, nLength, (LPINT) NULL) );
	else
		return Bool( ::TextOut( GetPS()->PspGetHandle(), start.x, start.y, pszText, nLength ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
	if( pClipBox )
		return Bool( ::GpiCharStringPosAt( GetPS()->PspGetHandle(), &start, pClipBox, CHS_CLIP, nLength, (char*)pszText, NULL ) );
	else
	  return Bool( ::GpiCharStringAt( GetPS()->PspGetHandle(), &start, /*nLength, (char*)pszText*/ 5, "Howdy" ) );
#endif
}

Bool FTextPaintBox::SetHorizTextAlign( UINT fuAlign )
{
#if( BIFSYSTEM == BIFWIN16 )
	::SetTextAlign( GetPS()->PspGetHandle(), fuAlign );
	return True;
#endif
#if( BIFSYSTEM == BIFOS2 )
	LONG lHoriz, lVert;
	::GpiQueryTextAlignment( GetPS()->PspGetHandle(), &lHoriz, &lVert );
	return Bool( GpiSetTextAlignment( GetPS()->PspGetHandle(), fuAlign, lVert ) );
#endif
}

Bool FTextPaintBox::SetVertTextAlign( UINT fuAlign )
{
#if( BIFSYSTEM == BIFWIN16 )
	::SetTextAlign( GetPS()->PspGetHandle(), fuAlign );
	return False;
#endif
#if( BIFSYSTEM == BIFOS2 )
	LONG lHoriz, lVert;
	GpiQueryTextAlignment( GetPS()->PspGetHandle(), &lHoriz, &lVert );
	return Bool( GpiSetTextAlignment( GetPS()->PspGetHandle(), lHoriz, fuAlign ) );
#endif
}

void FTextPaintBox::GetTextExtent( const char* pszString, FPoint* extent, int nStrPos )
{
	if( nStrPos == -1 )
		nStrPos = strlen( pszString );
#if( BIFSYSTEM == BIFWIN16 )
	DWORD r = ::GetTextExtent( GetPS()->PspGetHandle(), pszString, nStrPos );
	*extent = FPoint( LOWORD( r ), HIWORD( r ) );
#endif
#if( BIFSYSTEM == BIFOS2 )
	POINTL appl[TXTBOX_COUNT];

	GpiQueryTextBox( GetPS()->PspGetHandle(), nStrPos, (char*)pszString, TXTBOX_COUNT, appl );
	*extent = FPoint( appl[3].x - appl[1].x, appl[2].y - appl[1].y );
#endif
}

void FTextPaintBox::SetTextColor( FColorRef nBaseColor )
{
#if( BIFSYSTEM == BIFWIN16 )
	::SetTextColor( GetPS()->PspGetHandle(), nBaseColor );
#endif
#if( BIFSYSTEM == BIFOS2 )
	::GpiSetColor( GetPS()->PspGetHandle(), nBaseColor );
#endif
}

FGrey3DPaintBox::FGrey3DPaintBox( void )
{
	m_pFillerBrush = new FSolidBrush( FColors::Get3DFaceColor() );
	m_pFillerPen = new FSolidPen( FColors::Get3DFaceColor() );
	m_pDimPen = new FSolidPen( FColors::Get3DShadeColor() );
	m_pBrightPen = new FSolidPen( FColors::Get3DHighLightColor() );
}

FGrey3DPaintBox::~FGrey3DPaintBox()
{
	delete m_pFillerBrush;
	delete m_pFillerPen;
	delete m_pDimPen;
	delete m_pBrightPen;
}

void FGrey3DPaintBox::RaisedRect( FRect r )
{
	FObjectSelector os;
	os.SetPS( GetPS() );
	FGraphPaintBox pb;
	pb.SetPS( GetPS() );
	FPositionPaintBox ppb;
	ppb.SetPS( GetPS() );

	os.SelectPen( m_pFillerPen );
	os.SelectBrush( m_pFillerBrush );
	pb.Rect( r );
	os.SelectBrush( NULL );
	os.SelectPen( NULL );
	os.SelectPen( m_pBrightPen );
	ppb.MoveTo( FPoint( r.GetLeft(), r.GetBottom() ) );
	pb.LineTo( FPoint( r.GetLeft(), r.GetTop() ) );
	pb.LineTo( FPoint( r.GetRight(), r.GetTop() ) );
	os.SelectPen( NULL );
	os.SelectPen( m_pDimPen );
	pb.LineTo( FPoint( r.GetRight(), r.GetBottom() ) );
	pb.LineTo( FPoint( r.GetLeft(), r.GetBottom() ) );
	os.SelectPen( NULL );
}

void FGrey3DPaintBox::LoweredRect( FRect r )
{
	FObjectSelector os;
	os.SetPS( GetPS() );
	FGraphPaintBox pb;
	pb.SetPS( GetPS() );
	FPositionPaintBox ppb;
	ppb.SetPS( GetPS() );

	os.SelectPen( m_pFillerPen );
	os.SelectBrush( m_pFillerBrush );
	pb.Rect( r );
	os.SelectBrush( NULL );
	os.SelectPen( NULL );
	os.SelectPen( m_pDimPen );
	ppb.MoveTo( FPoint( r.GetLeft(), r.GetBottom() ) );
	pb.LineTo( FPoint( r.GetLeft(), r.GetTop() ) );
	pb.LineTo( FPoint( r.GetRight(), r.GetTop() ) );
	os.SelectPen( NULL );
	os.SelectPen( m_pBrightPen );
	pb.LineTo( FPoint( r.GetRight(), r.GetBottom() ) );
	pb.LineTo( FPoint( r.GetLeft(), r.GetBottom() ) );
	os.SelectPen( NULL );
}

// Made local for this module /BL
static int iabs( int a )
{
	if( a < 0 )
		return -a;
  return a;
}

void BottomRect( FRect* r, FPaintSpace* p, double factor )
{
	FPoint l_Point;
	FPoint l_Org( 0, 0 );
	FPoint l_TextSize;

	p->PspGetSize( &l_Point );

	FTextPaintBox tpb;
	tpb.SetPS( p );
	tpb.GetTextExtent( "Mjgh", &l_TextSize, 4 );

	p->PspLog2Pix( &l_TextSize, 1 );
	p->PspLog2Pix( &l_Org, 1 );

	int th = iabs( l_Org.GetY() - l_TextSize.GetY() );

	FPoint lt( l_Org.GetX(), l_Point.GetY() - th * factor );
	FPoint rb( l_Point.GetX(), l_Point.GetY() );

	p->PspPix2Log( &lt, 1 );
	p->PspPix2Log( &rb, 1 );

	*r = FRect( lt.GetX(), lt.GetY(), rb.GetX(), rb.GetY() );
}

void FGrey3DPaintBox::StatusBar( void )
{
	FRect r;

	BottomRect( &r, GetPS(), 1.2 );

	LoweredRect( r );
}

void FGrey3DPaintBox::StatusText( int nOffset, int nSize, char* pszText )
{
	FRect r;

	BottomRect( &r, GetPS(), 1.1 );
	int x11 = r.GetTop();
	BottomRect( &r, GetPS(), 0.1 );
	int x01 = r.GetTop();
	BottomRect( &r, GetPS(), 1.0 );
	r.SetLeft( nOffset );
	r.SetRight( nOffset + nSize );

	FTextPaintBox tpb;
	tpb.SetPS( GetPS() );
	LoweredRect( FRect(  nOffset, x11, nOffset+nSize, x01 ) );
	tpb.TextOut( FPoint(  nOffset, x11 ), pszText, strlen( pszText ), &r );
}


FField::FField( void )
{
	m_pszRecord = NULL;
	m_pszPicture = NULL;
	m_pszMerge = NULL;
}

FField::~FField()
{
	if( m_pszRecord )
		delete m_pszRecord;
	if( m_pszPicture )
		free( m_pszPicture );
	if( m_pszMerge )
		delete( m_pszMerge );
}

void FField::SetPicture( char* pszPicture )
{
	if( m_pszPicture )
		free( m_pszPicture );
	m_pszPicture = strdup( pszPicture );
}

void FField::SetRecord( char* pszText )
{
	strcpy( m_pszRecord, pszText );
}

const char* FField::GetMerge( void )
{
	return m_pszMerge;
}

int FField::GetMergePos( int nRecPos )
{
	int nonedit = 0;
	if( nRecPos == 0 )
		return 0;
	int q = nRecPos;

	for( int i = 1; i <= q; i++ ) {
		if( m_pszPicture[i] == 0 )
			return 0;

		if( m_pszPicture[i] != '#' ) {
			nonedit++;
			q++;
		}
	}
	return nRecPos + nonedit;
}

void FField::UpdateMerge( void )
{
	if( !m_pszPicture )
		return;

	for( int i = 0, q = 0; i < strlen( m_pszPicture ); i++ ) {
		if( m_pszPicture[i] == '#' ) {
			if( m_pszRecord[q] != 0 ) {
				m_pszMerge[i] = m_pszRecord[q];
				q++;
			}
			else
				m_pszMerge[i] = ' ';
		}
		else {
			m_pszMerge[i] = m_pszPicture[i];
		}
	}
	m_pszMerge[i] = '\x0';
}

Bool FField::InsertChar( int nOffset, char Ch )
{
	if( nOffset > strlen( m_pszRecord ) )
		return False;

	memmove( m_pszRecord + nOffset + 1, m_pszRecord + nOffset, strlen( m_pszRecord) - nOffset + 1 );
	*(m_pszRecord + nOffset ) = Ch;
	return True;
}

Bool FField::DeleteChar( int nOffset )
{
	if( strlen( m_pszRecord ) == 0 )
		return False;
	if( nOffset > strlen( m_pszRecord ) )
		return False;
	if( nOffset == strlen( m_pszRecord ) ) {
		m_pszRecord[nOffset] = '\x0';
		return True;
	}
	memmove( m_pszRecord + nOffset, m_pszRecord + nOffset + 1, strlen( m_pszRecord) - nOffset );
	return True;
}

void FField::Paint( FPaintSpace* pPs, FRect& ClipBox )
{
	SetPS( pPs );
	UpdateMerge();
	TextOut( FPoint( ClipBox.GetLeft(), ClipBox.GetTop() ), GetMerge(), strlen( GetMerge() ), &ClipBox );
}

void FStrField::Create( int nSize, char* pszPicture )
{
	m_pszRecord = new char[nSize];
	m_pszMerge = new char[nSize];
	if( pszPicture )
		SetPicture( pszPicture );
}

/////////////////////////
// End of FPaint.Cpp


