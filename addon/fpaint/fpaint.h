/****************************************************************************
FILE
	FPaint.H
AUTHOR
	Carsten Mouritzen 2:238/10.17@FidoNet
DESCRIPTION
	See the implinentation files for more info.
**************************************************************************/
#ifndef DEVCON_H
#define DEVCON_H 1

class FDevice;

#if( BIFSYSTEM == BIFWIN16 )
typedef HDC FPSHandle;
typedef COLORREF FColorRef;

typedef LOGPEN  FLogPen;
typedef LOGBRUSH FLogBrush;
typedef LOGFONT FLogFont;
typedef BITMAP FLogBitmap;

typedef HGDIOBJ FGdiObjHandle;
typedef HPEN FPenHandle;
typedef HBRUSH FBrushHandle;
typedef HFONT FFontHandle;
typedef HBITMAP FBitmapHandle;

//Used by device enumerator
typedef struct tagFDeviceInfo {
	char* pszDevice;
	char* pszDriver;
	char* pszPort;
} FDeviceInfo;

//Used by FDevice
typedef DOCINFO FJobInfo;
#endif
#if( BIFSYSTEM == BIFOS2 )
typedef HPS FPSHandle;
typedef unsigned long FColorRef;

typedef FATTRS FLogFont;
typedef LINEBUNDLE FLogPen;
typedef AREABUNDLE FLogBrush;
typedef int FLogBitmap;

typedef void* FGdiObjHandle;
typedef FLogPen* FPenHandle;
typedef FLogBrush* FBrushHandle;
typedef FLogFont* FFontHandle;
typedef HBITMAP FBitmapHandle;

//Used by device enumerator
typedef DEVOPENSTRUC FDeviceInfo;

//Used by FDevice
typedef struct {
	DEVOPENSTRUC OpenStruc;
	HDC OS2PrintDC;
} FJobInfo;

#define LBB_ALL LBB_COLOR | LBB_BACK_COLOR | LBB_MIX_MODE | LBB_BACK_MIX_MODE | LBB_WIDTH | LBB_GEOM_WIDTH | LBB_TYPE | LBB_END | LBB_JOIN
#define CBB_ALL CBB_COLOR | CBB_BACK_COLOR | CBB_MIX_MODE | CBB_BACK_MIX_MODE | CBB_SET | CBB_MODE | CBB_BOX | CBB_ANGLE | CBB_SHEAR | CBB_DIRECTION | CBB_TEXT_ALIGN | CBB_EXTRA | CBB_BREAK_EXTRA
#define MBB_ALL MBB_COLOR | MBB_BACK_COLOR | MBB_MIX_MODE | MBB_BACK_MIX_MODE | MBB_SET MBB_SYMBOL | MBB_BOX
#define ABB_ALL	ABB_COLOR | ABB_BACK_COLOR | ABB_MIX_MODE | ABB_BACK_MIX_MODE | ABB_SET | ABB_SYMBOL | ABB_REF_POINT
#define IBB_ALL IBB_COLOR | IBB_BACK_COLOR | IBB_MIX_MODE | IBB_BACK_MIX_MODE

#endif
/*-------------------------------------------------------------------------*/
//Device handler with static functions, should keep track of all the
//device resources.
class FDeviceEnumerator {
public:
	//Querys a certain device, device numbers start at 0, returns FALSE
	//when there are no more devices
	static Bool EnumDevices( int nDeviceIndex, FDeviceInfo* pDeviceInfo );

	//Opens default printer device
	static FDevice* OpenDefaultDevice( void );
	//Opens a device number by creating an instance of it, call CloseDevice
	//when you are finished using it
	static FDevice* OpenDevice( int nDeviceIndex );
	//Opens a device by letting the user select from a standard selection
	//box, returns NULL if the user canceled.
	static FDevice* OpenViaStdDialog( void );

	//Close an open device
	static Bool CloseDevice( FDevice* pDevice );
};

class FMapper;
/*-------------------------------------------------------------------------*/
//PaintSpace base class, is a simple wrapper around a HDC structure, note that
//all mapping stuff is handled here.
//If a device has a special way of creating a HDC it should have it's own
//class, and implement the 2 key functions Use = make or get a DC,
//Release = the opposite.
enum FAxisOrigin {
	ao_center,
	ao_bottom_left,
	ao_top_left,
	ao_bottom_right,
	ao_top_right
};

enum FScaleUnits {
	su_centimeters,
	su_inches,
	su_pixels
};

enum FProjectionMethods {
#if( BIFSYSTEM == BIFWIN16 )
	pm_isotropic = MM_ISOTROPIC,
	pm_anisotropic = MM_ANISOTROPIC
#endif
#if( BIFSYSTEM == BIFOS2 )
	pm_isotropic = PU_ARBITRARY,
	pm_anisotropic = PU_ARBITRARY
#endif
};

class FPaintSpace {
public:
	FPaintSpace( void );
	virtual ~FPaintSpace();

	virtual void PspUse( void ) = 0;
	virtual void PspRelease( void ) = 0;

	virtual void PspGetSize( FPoint* pPoint ) = 0;
	virtual FPSHandle PspGetHandle( void );

	//Mapping
	void PspSetMapper( FMapper* pMapper );
	//The internal hack is used to compensate when build-in mapmodes
	//are used
	void PspPix2Log( FPoint* pPixPoint, int nPointCount, Bool fInternal = False );
	void PspLog2Pix( FPoint* pLogPoint, int nPointCount, Bool fInternal = False );
	void PspSetAxisOrigin( FAxisOrigin nOrigin );
	void PspSetTranslaion( FPoint* pOffset );
	void PspSetScalation( FPoint* pScale );
protected:
	FMapper* m_pMapper;
	FPSHandle m_hPS;
};

class FMapper {
public:
	FMapper( void );
	virtual ~FMapper();

	virtual void Use( FPaintSpace* pPS ) = 0;

	virtual void Pix2Log( FPaintSpace* pPS, FPoint* pPixPoint, int nPointCount, Bool fInternal = False ) = 0;
	virtual void Log2Pix( FPaintSpace* pPS, FPoint* pLogPoint, int nPointCount, Bool fInternal = False ) = 0;

	virtual void SetAxisOrigin( FPaintSpace* pPS, FAxisOrigin nOrigin ) = 0;
	virtual void SetTranslaion( FPaintSpace* pPS, FPoint* pOffset ) = 0;
	virtual void SetScalation( FPaintSpace* pPS, FPoint* pScale ) = 0;
};

//All default mappings are absolute, that means they are related to the
//size of the screen not the size of the window.
class FDefaultMapper : public FMapper {
public:
	FDefaultMapper( void );
	virtual ~FDefaultMapper();

	virtual void Pix2Log( FPaintSpace* pPS, FPoint* pPixPoint, int nPointCount, Bool fInternal = False );
	virtual void Log2Pix( FPaintSpace* pPS, FPoint* pLogPoint, int nPointCount, Bool fInternal = False );

	virtual void SetTranslaion( FPaintSpace* pPS, FPoint* pOffset );
	virtual void SetScalation( FPaintSpace* pPS, FPoint* pScale );
	void FlipXY( FPaintSpace* pPS );
protected:
	void SetPhysicalDimensions( FPaintSpace* pPS, FAxisOrigin org, FPoint v, int ymult );
};

class FAbsoluteMapper : public FDefaultMapper {
public:
	FAbsoluteMapper( int nMapMode );
	virtual ~FAbsoluteMapper();

	virtual void Use( FPaintSpace* pPS );
	virtual void SetAxisOrigin( FPaintSpace* pPS, FAxisOrigin nOrigin );
protected:
	int m_MapMode;
};

class FRelativeMapper : public FDefaultMapper {
public:
	FRelativeMapper( FPoint extent, FProjectionMethods nProjectionMethod );
	virtual ~FRelativeMapper();

	virtual void Use( FPaintSpace* pPS );
	virtual void SetAxisOrigin( FPaintSpace* pPS, FAxisOrigin nOrigin );
private:
	FPoint m_RelativeSize;
	FProjectionMethods m_ProjectionMethod;
};


#define BIFINCL_DEFAULTMAPMODES

#ifdef BIFINCL_DEFAULTMAPMODES
	extern FAbsoluteMapper HiEnglishMapper;
	extern FAbsoluteMapper LoEnglishMapper;
	extern FAbsoluteMapper HiMetricMapper;
	extern FAbsoluteMapper LoMetricMapper;
	extern FAbsoluteMapper PixelMapper;
	extern FRelativeMapper IsotropicMapper;
	extern FRelativeMapper AnIsotropicMapper;
#endif

/*-------------------------------------------------------------------------*/
//Device instance, note the tricky private constructor, this is because
//all FDevice instances should be controlled by the FDeviceEnumerator class
class FDevice {
private:
	FDevice( FDeviceInfo* pDeviceInfo );
	virtual ~FDevice();
public:
	friend class FDeviceEnumerator;
	friend class FDevicePaintSpace;
	//This is not implemented at the moment
	Bool SetupDialog( void );

	int GetBinCount( void );
	int GetPaperCount( void );
	int GetMaxNumberOfCopies( void );
	Bool GetBinName( int nBinNumber, char* pszBuffer );
	Bool GetPaperName( int nBinNumber, char* pszBuffer );

	//Query device capabilities, buffer = NULL means return required size
	//Quite tricky to use, so perhaps it was better to use the above
	//functions
	unsigned long Capabilities( int fwCapability, char* pszBuffer );
private:
	FDeviceInfo m_DevInfo;
};

/*-------------------------------------------------------------------------*/
//PaintSpace class for a device, it is special in the way that it uses Create
//instead of GetDC, also note that i have included the functions special
//for printer PaintSpace's ( AbortJob etc. ) and a method of naming output.
class FDevicePaintSpace : public FPaintSpace {
public:
	FDevicePaintSpace( FDevice* pDevice );
	virtual ~FDevicePaintSpace();

	virtual void PspUse( void );
	virtual void PspRelease( void );

	virtual void PspGetSize( FPoint* pPoint );

	int PspStartJob( const char* pszDocumentName );
	int PspAbortJob( void );
	int PspEndJob( void );

	int PspStartPage( void );
	int PspEndPage( void );

#if( BIFSYSTEM == BIFWIN16 )
		int PspSetAbortProc( ABORTPROC pfnAbortProc );
#endif

	void PspRedirectOutput( const char* pszFileName );
private:
	FDevice* m_pDevice;
	FJobInfo m_JobInfo;
};

/*-------------------------------------------------------------------------*/
//PaintSpace class for a normal client window
class FClientPaintSpace : public FPaintSpace {
	friend class FEventPaintSpace;
	friend class FWindowPaintSpace;
public:
	FClientPaintSpace( FWnd* pWnd );
	virtual ~FClientPaintSpace( void );

	virtual void PspUse( void );
	virtual void PspRelease( void );

	virtual void PspGetSize( FPoint* pPoint );

	FWnd* GetWnd( void ) { return m_pWnd; }
private:
	FWnd* m_pWnd;
};

/*-------------------------------------------------------------------------*/
//PaintSpace class for an entire window ( including non client )
class FWindowPaintSpace : public FClientPaintSpace {
public:
	FWindowPaintSpace( FWnd* pWnd );
	virtual ~FWindowPaintSpace();

	virtual void PspUse( void );
	virtual void PspRelease( void );

	virtual void PspGetSize( FPoint* pPoint );
};

class FEventPaintSpace : public FClientPaintSpace {
public:
	FEventPaintSpace( FWnd* pWnd );
	virtual ~FEventPaintSpace();

	virtual void PspUse( void );
	virtual void PspRelease( void );
private:
#if( BIFSYSTEM == BIFWIN16 )
	PAINTSTRUCT m_PaintStruct;
#endif
};

class FDesktopPaintSpace : public FWindowPaintSpace {
public:
	FDesktopPaintSpace( void ) :
		FWindowPaintSpace( GetDesktop() ) {
	}
	virtual ~FDesktopPaintSpace( void ) {}
};
/*-------------------------------------------------------------------------*/
//PaintSpace class for all compatible DC's, note that it insists on being
//created from an existing PaintSpace ( as it should be! )
class FCompatiblePaintSpace : public FPaintSpace {
public:
	FCompatiblePaintSpace( FPaintSpace* pTemplatePS );
	virtual ~FCompatiblePaintSpace();

	virtual void PspUse( void );
	virtual void PspRelease( void );

	virtual void PspGetSize( FPoint* pPoint );
private:
	FPaintSpace* m_TemplatePaintSpace;
#if( BIFSYSTEM == BIFOS2 )
	HDC m_hOS2DC;
#endif
};

/*-------------------------------------------------------------------------*/
class FColors {
public:
	static FColorRef RGB2Ref( int nRed, int nGreen, int nBlue ) {
#if( BIFSYSTEM == BIFWIN16 )
		return RGB( nRed, nGreen, nBlue );
#endif
#if( BIFSYSTEM == BIFOS2 )
		FColorRef q = nRed;
		q = q << 8;
		q = q + nGreen;
		q = q << 8;
		q = q + nBlue;
		return q;
#endif
	}
	static FColorRef RGBPct2Ref( int nRed, int nGreen, int nBlue ) {
		return RGB2Ref( ( nRed * 255 ) / 100, ( nGreen * 255 ) / 100, ( nBlue * 255 ) / 100 );
	}
	static FColorRef CMYPct2Ref( int nCyan, int nMagenta, int nYellow ) {
		return RGB2Ref( 255 - ( ( nCyan * 255 ) / 100 ),
			255 - ( ( nMagenta * 255 ) / 100 ),
			255 - ( ( nYellow * 255 ) / 100 ) );
	}
	FColorRef CMYKPct2Ref( int nCyan, int nMagenta, int nYellow, int nBlack ) {
		return CMYPct2Ref( ( nCyan * ( 100 - nBlack ) ) / 100,
			 ( nMagenta * ( 100 - nBlack ) ) / 100,
			 ( nYellow * ( 100 - nBlack ) ) / 100 );
	}
/*	FColorRef YIQPct2Ref( int y, int i, int q ) {
		R  1   0.956  0.621   Y
		G  1  -0.272 -0.647   I
		B  1  -1.105  1.702   Q
	}
	FColorRef YUVPct2Ref( int y, int u, int v ) {
		R  1   0      1.140   Y
		G  1  -0.394 -0.581   U
		B  1   2.028  0       V
	}
	FColorRef HSV2Ref( int h, int s, int v ) {
		int r, g, b;

		if( s == 0 ) {
			//No color saturation at all, hue has no meaning and only intensity
			//contributes to the result
			return RGB( v, v, v );
		}
		if( h == 360 )
			h = 0;

	} */
	static FColorRef Yellow;
	static FColorRef Warm_Red;
	static FColorRef Rubine_Red;
	static FColorRef Rhodamine_Red;
	static FColorRef Purple;
	static FColorRef Reflex_Blue;
	static FColorRef Process_Blue;
	static FColorRef Green;
	static FColorRef Black;
	static FColorRef Violet;

	static FColorRef RGB_Red;
	static FColorRef RGB_Green;
	static FColorRef RGB_Blue;
	static FColorRef RGB_Cyan;
	static FColorRef RGB_Pink;
	static FColorRef RGB_Yellow;
	static FColorRef RGB_White;
	static FColorRef RGB_LtGrey;
#if( BIFSYSTEM == BIFWIN16 )
	static FColorRef Get3DFaceColor( void ) {return ::GetSysColor( COLOR_BTNFACE );}
	static FColorRef Get3DShadeColor( void ) {return ::GetSysColor( COLOR_BTNSHADOW );}
	static FColorRef Get3DHighLightColor( void ) {return ::GetSysColor( COLOR_BTNHIGHLIGHT );}
	static FColorRef GetWindowBackgroundColor( void ) {return ::GetSysColor( COLOR_WINDOW );}
	static FColorRef GetDesktopColor( void ) {return ::GetSysColor( COLOR_BACKGROUND );}
#endif
#if( BIFSYSTEM == BIFOS2 )
	static FColorRef Get3DFaceColor( void ) {
		return WinQuerySysColor( HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0L );
	}
	static FColorRef Get3DShadeColor( void ) {return Black;}
	static FColorRef Get3DHighLightColor( void ) {return RGB_White;}
	static FColorRef GetWindowBackgroundColor( void ) {
		return WinQuerySysColor( HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0L );
	}
	static FColorRef GetDesktopColor( void ) {return RGB_Yellow;}
#endif
};

//Base class for all GDI objects, more to come such as bitmaps and pens
//I dont know whether it is the best to have garbage collection ( i have )
//or whether we should let the user do it himself
class FPaintTool {
public :
	FPaintTool( void ) {}
	virtual ~FPaintTool() { /*ASSERT( m_ReferenceCount != 0 );*/ }
	virtual Bool Delete( void ) = 0;
	virtual FGdiObjHandle GetHandle( void ) = 0;

	void Reference( void ) { m_ReferenceCount++; }
	void DeReference( void ) { m_ReferenceCount--; }
private:
	int m_ReferenceCount;
};

class FBitmap : FPaintTool {
public:
	enum FType {
		bt_logical = 0
	};
	FBitmap( FBitmapHandle hTemplate );
	virtual ~FBitmap();

	FGdiObjHandle GetHandle( void );

	virtual void GetSize( FPoint* pPoint );

	Bool Create( FPoint size, int cbPlanes, int cbBits, void* lpvBits );
	Bool Create( const char* pszResourceName );

	Bool Delete( void );
private:
	FBitmapHandle m_hBitmap;
	FLogBitmap m_Bitmap;
};

class FPen : public FPaintTool {
public:
	enum FPenStyle {
#if( BIFSYSTEM == BIFWIN16 )
		ps_solid = PS_SOLID,
		ps_dash = PS_DASH,
		ps_dot = PS_DOT,
		ps_dashdot = PS_DASHDOT,
		ps_dashdotdot = PS_DASHDOTDOT,
		ps_null = PS_NULL,
		//Non portables
		ps_insideframe = PS_INSIDEFRAME
#endif
#if( BIFSYSTEM == BIFOS2 )
		ps_solid = LINETYPE_SOLID,
		ps_dash = LINETYPE_SHORTDASH,
		ps_dot = LINETYPE_DOT,
		ps_dashdot = LINETYPE_DASHDOT,
		ps_dashdotdot = LINETYPE_DASHDOUBLEDOT,
		ps_null = LINETYPE_INVISIBLE,
		//Non portables
		ps_doubledot = LINETYPE_DOUBLEDOT,
		ps_longdash = LINETYPE_LONGDASH
#endif
	};
	FPen( FPenHandle hTemplate );
	FPen( FPenStyle nStyle, FColorRef nBaseColor, FPoint size );
	virtual ~FPen();

	virtual FGdiObjHandle GetHandle( void );

	Bool Create( FPenStyle nStyle, FColorRef nBaseColor, FPoint size );
	virtual Bool Delete( void );
protected:
	FPenHandle m_hPen;
	FLogPen m_LogPen;
};

class FSolidPen : public FPen {
public:
	FSolidPen( FPenHandle hTemplate );
	FSolidPen( FColorRef nBaseColor );
	virtual ~FSolidPen();

	Create( FColorRef nBaseColor );
};

class FBrush : public FPaintTool {
public:
	enum FBrushStyle {
#if( BIFSYSTEM == BIFWIN16 )
		bs_dibpattern = BS_DIBPATTERN,
		bs_solid = BS_SOLID,
		bs_hollow = BS_HOLLOW,
		bs_pattern = BS_PATTERN
#endif
#if( BIFSYSTEM == BIFOS2 )
		bs_solid = PATSYM_SOLID
#endif
   	};
   	enum FHatchMethod {
#if( BIFSYSTEM == BIFWIN16 )
		hs_none = 0,
		hs_bdiagonal = HS_BDIAGONAL,
		hs_cross = HS_CROSS,
		hs_diagcross = HS_DIAGCROSS,
		hs_fdiagonal = HS_FDIAGONAL,
		hs_horizontal = HS_HORIZONTAL,
		hs_vertical = HS_VERTICAL
#endif
#if( BIFSYSTEM == BIFOS2 )
		hs_none
#endif
	};
	FBrush( FBrushHandle hTemplate );
	virtual ~FBrush();

	virtual FGdiObjHandle GetHandle( void );

	Bool Create( FBrushStyle nStyle, FColorRef nBaseColor, FHatchMethod hatchmethod );
	virtual Bool Delete( void );
protected:
	FBrushHandle m_hBrush;
	FLogBrush m_LogBrush;
};

class FSolidBrush : public FBrush {
public:
	FSolidBrush( FBrushHandle hTemplate );
	FSolidBrush( FColorRef nBaseColor );
	virtual ~FSolidBrush();

	Bool Create( FColorRef nBaseColor );
};

/*-------------------------------------------------------------------------*/
//Font object, implementation as an example of a GDI object, not complete,
//not at all complete - but it works
class FFont : public FPaintTool {
public :
	FFont( FFontHandle hTemplate );
	FFont( int nHeight, const char* pszFontName );
	virtual ~FFont();

	virtual FGdiObjHandle GetHandle( void );

	//This is the longest parameterline i have ever seen
	Bool Create( int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight,
					BYTE fbItalic, BYTE fbUnderline, BYTE fbStrikeOut, BYTE fbCharSet, BYTE fbOutputPrecision,
					BYTE fbClipPrecision, BYTE fbQuality, BYTE fbPitchAndFamily, const char* pszFace );
	//This one is a bit easyer.
	Bool Create( int nHeight, const char* pszFontName );
	Bool Delete( void );

private:
	FFontHandle m_hFont;
	FLogFont m_LogFont;
};

/*-------------------------------------------------------------------------*/
//base class for PaintBox
class FPaintBox {
public:
	FPaintBox( void );
	virtual ~FPaintBox();

	FPaintSpace* GetPS( void );
	void SetPS( FPaintSpace* pPS );
private:
	FPaintSpace* m_pPaintSpace;
};


typedef struct tagFGdiObjectSelection {
	FPenHandle hOldPen;
	FBrushHandle hOldBrush;
	FFontHandle hOldFont;
	FBitmapHandle hOldBitmap;
} FGdiObjectSelection;

class FObjectSelector : virtual public FPaintBox {
public:
	FObjectSelector( void );
	virtual ~FObjectSelector() {}

	//Select an object into the paintbox, selecting a value of NULL
	//restores previous selection
	void SelectPen( FPen* pObject );
	void SelectBrush( FBrush* pObject );
	void SelectFont( FFont* pObject );
	void SelectBitmap( FBitmap* pObject );
	void GetCurrentSelection( FGdiObjectSelection* pSelection ) {}
	void RestoreCurrentSelection( FGdiObjectSelection* pSelection ) {}
private:
	FGdiObjectSelection m_Selection;
};

class FCaret {
public:
	static Bool Create( FClientPaintSpace* pPs, FPoint Size );
	static Bool Destroy( FClientPaintSpace* pPs );

	static Bool Show( FClientPaintSpace* pPs );
	static Bool Hide( FClientPaintSpace* pPs );

	static Bool Move( FClientPaintSpace* pPs, FPoint NewPos );
private:
	static Bool m_bEnabled;
	static int m_nCsrWidth;
	static int m_nCsrHeight;
};

class FBlitterPaintBox {
public:
	enum FBlitterMethod {
		rop_copy = 1
	};
	FBlitterPaintBox( FPaintSpace* pTargetPS, FPaintSpace* pSourcePS );
	virtual ~FBlitterPaintBox();

	Bool BitBlit( FPoint offset, FPoint size );
	Bool StrechBlt( FRect targetrect, FRect sourcerect, FBlitterMethod nMethod );
private:
	FPaintSpace* m_pTargetPS;
	FPaintSpace* m_pSourcePS;
};

class FBitmapPaintBox {
public:
	FBitmapPaintBox( FPaintSpace* pTargetPS );
	virtual ~FBitmapPaintBox();

	Bool CopyBitmap( FBitmap* pBitMap, FPoint offset );
private:
	FPaintSpace* m_pTargetPS;
	FPaintSpace* m_pSourcePS;
};

//base class for PaintBox
class FPositionPaintBox : virtual public FPaintBox {
public:
	FPositionPaintBox( void ) {}
	virtual ~FPositionPaintBox() {}

	Bool MoveTo( FPoint p );
	FPoint Where( void );
};

/*-------------------------------------------------------------------------*/
//An advanced paintbox, hmmm demonstrates the increese of functionality
//through single inheritance
class FGraphPaintBox : virtual public FPaintBox {
public:
	FGraphPaintBox( void ) {}
	virtual ~FGraphPaintBox() {}

	virtual Bool LineTo( FPoint p );
	virtual Bool Rect( FRect r );
	virtual Bool Ellipse( FRect r );
	virtual Bool Arc( FRect bounding, FPoint start, FPoint end );
	virtual Bool Pixel( FPoint p, FColorRef nColor );
};

class FBackgrPaintBox : virtual public FPaintBox {
public:
	FBackgrPaintBox( void ) {}
	virtual ~FBackgrPaintBox() {}

	enum FBackgroundMode {
#if( BIFSYSTEM == BIFWIN16 )
		bm_opaque = OPAQUE,
		bm_transparent = TRANSPARENT
#endif
#if( BIFSYSTEM == BIFOS2 )
		bm_opaque = BM_OVERPAINT,
		bm_transparent = BM_LEAVEALONE
#endif
	};
	void SetBackgroundColor( FColorRef nBaseColor );
	void SetBackgroundMode( FBackgroundMode mode );
	void DrawBackgroundRect( FWnd* pWnd );
};
/*-------------------------------------------------------------------------*/
//Paintbox that handles everything that is text related, host hark - will
//come to handle everything
class FTextPaintBox : virtual public FPaintBox {
public:
	FTextPaintBox( void ) {}
	virtual ~FTextPaintBox() {}

	Bool TextOut( FPoint start, const char* pszText, int nLength, FRect* pClipBox = NULL );
	Bool SetHorizTextAlign( UINT fuAlign );
	Bool SetVertTextAlign( UINT fuAlign );
	void GetTextExtent( const char* pszString, FPoint* extent, int nStrPos = -1 );
	void SetTextColor( FColorRef nBaseColor );

	static char* Helvetica;
	static char* TimesRoman;
	static char* Courier;
	static char* SmallFixed;
	static char* LargeFixed;
};
/*
#define DEPTH                   8       // Bezier recursion depth
#define MAX_BEZIER_POINTS       ((1 << DEPTH) + 1)

class FParametricCurvePaintbox {
	public:
		Bezier( FPoint p[] ) {
			FPoint res = new FPoint[10];
			BezierSubdivide( p, &res );
			delete res;
		}
	private:
		void BezierSubdivide( FPoint p[], FPoint* res ) {
			static int depth = DEPTH;
			FPoint q[8];

			if( !depth ) {
				*res = p[2];
				res++;
				return;
			}
			q[0] = p[0];
			q[4] = p[2];

			q[1].x = ( p[0].x + p[1].x ) >> 1;
			q[3].x = ( p[1].x + p[2].x ) >> 1;
			q[2].x = ( q[1].x + q[3].x ) >> 1;

			q[1].y = ( p[0].y + p[1].y ) >> 1;
			q[3].y = ( p[1].y + p[2].y ) >> 1;
			q[2].y = ( q[1].y + q[3].y ) >> 1;

			depth--;
			sub_divide( q );
			sub_divide( q + 2 );
			depth++;
		}

};*/

class FGrey3DPaintBox: virtual public FPaintBox {
public:
	FGrey3DPaintBox( void );
	virtual ~FGrey3DPaintBox();

	void RaisedRect( FRect r );
	void LoweredRect( FRect r );
	void StatusBar( void );
	void StatusText( int nOffset, int nSize, char* pszText );
private:
	FSolidBrush* m_pFillerBrush;
	FSolidPen* m_pFillerPen;
	FSolidPen* m_pDimPen;
	FSolidPen* m_pBrightPen;
};

class FFieldPaintBox:
	public FObjectSelector,
	public FPositionPaintBox,
	public FGraphPaintBox,
	public FTextPaintBox,
	public FBackgrPaintBox,
	public FGrey3DPaintBox {
};

class FField : public FFieldPaintBox {
public:
	FField( void );
	virtual ~FField();

	void SetPicture( char* pszPicture );
	void SetRecord( char* pszText );

	Bool InsertChar( int nOffset, char Ch );
	Bool DeleteChar( int nOffset );

	int GetMergePos( int nRecPos );
	const char* GetMerge( void );

	virtual void Paint( FPaintSpace* pPs, FRect& ClipBox );
	operator char*( void ) { return m_pszRecord; }
protected:
	virtual void UpdateMerge( void );
	char* m_pszRecord;
	char* m_pszPicture;
	char* m_pszMerge;
};

class FStrField : public FField {
public:
	FStrField( void ) {}
	virtual ~FStrField() {}

	void Create( int nSize, char* pszPicture = NULL );
};

/*-------------------------------------------------------------------------*/
//An example of how to combine paintboxes to get the functionallity you want
//using multible inheritance
class FMyPaintBox : public FObjectSelector,
										public FPositionPaintBox,
										public FGraphPaintBox,
										public FTextPaintBox,
										public FBackgrPaintBox,
										public FGrey3DPaintBox {
	public:
		FMyPaintBox( void ) {}
		virtual ~FMyPaintBox() {}

//		void SetPS( FPaintSpace* pPS );
};
#endif	// End of DEVCON_H
