/**********************************************************************
 *<
	FILE: gfx.h

	DESCRIPTION: main graphics system include file.

	CREATED BY: Don Brittain

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#if !defined(_GFX_H_)

#define _GFX_H_

#include "geomlib.h"
#include "export.h"
#include "tab.h"
#include "mtl.h"
#include "BaseInterface.h"

typedef Tab<DWORD> DWTab;
typedef unsigned short MtlID;

// strip-related stuff

class Strip {
public:
	MtlID	mID;
	DWORD	smGrp;
	DWTab	v;
	DWTab	n;
	DWTab	tv;
	void AddVert(DWORD vtx)	{ v.Append(1, &vtx); }
	void AddVert(DWORD vtx, DWORD tvtx) { v.Append(1, &vtx); tv.Append(1, &tvtx); }
	void AddVertN(DWORD vtx, DWORD nor) { v.Append(1, &vtx); n.Append(1, &nor); }
	void AddVertN(DWORD vtx, DWORD tvtx, DWORD nor) { v.Append(1, &vtx); tv.Append(1, &tvtx); n.Append(1, &nor); }
};

typedef Strip *StripPtr;
typedef Tab<StripPtr> StripTab;


// Face Flags: (moved from mesh.h)
// 		3 LSBs hold the edge visibility flags
// 		Bit 3 indicates the presence of texture verticies

// if bit is 1, edge is visible
#define EDGE_VIS			1
#define EDGE_INVIS			0

// first edge-visibility bit field
#define VIS_BIT				0x0001
#define VIS_MASK			0x0007

#define EDGE_A		(1<<0)
#define EDGE_B		(1<<1)
#define EDGE_C		(1<<2)
#define EDGE_ALL	(EDGE_A|EDGE_B|EDGE_C)

#define FACE_HIDDEN	(1<<3)
#define HAS_TVERTS	(1<<4)	// DO NOT USE: This flag is obselete.
#define FACE_WORK	(1<<5) // used in various algorithms
#define FACE_STRIP	(1<<6)

// flags to indicate that face normal is used because no smoothing group
// normal is found
#define FACE_NORM_A	(1<<8)
#define FACE_NORM_B	(1<<9)
#define FACE_NORM_C	(1<<10)
#define FACE_NORM_MASK	0x0700

// The mat ID is stored in the HIWORD of the face flags
#define FACE_MATID_SHIFT	16
#define FACE_MATID_MASK		0xFFFF


class GWFace {
public:
	DWORD	v[3];	// indexed references to the vertex array
	DWORD	flags;	// see face flags description above
};

// Display flags -- for processWireFaces (and general mesh class use: moved from mesh.h)
#define DISP_VERTTICKS		(1<<0)
#define DISP_SELVERTS		(1<<10)
#define DISP_SELFACES		(1<<11)
#define DISP_SELEDGES		(1<<12)
#define DISP_SELPOLYS		(1<<13)

#define DISP_OBJSELECTED	(1<<8)		// mimics COMP_OBJSELECTED in mesh.h


// General definitions

#define WM_SHUTDOWN			(WM_USER+2001)
#define WM_INIT_COMPLETE	(WM_USER+2002)

#define GW_MAX_FILE_LEN		128
#define GW_MAX_CAPTION_LEN	128

#define GW_MAX_VERTS		32
#define GFX_MAX_STRIP		100
#define GFX_MAX_TEXTURES	8

typedef BOOL	(*HitFunc)(int, int, void *);

// Rendering modes
#define GW_NO_ATTS			0x000000
#define GW_WIREFRAME		0x000001
#define GW_ILLUM			0x000002
#define GW_FLAT				0x000004
#define GW_SPECULAR			0x000008
#define GW_TEXTURE			0x000010
#define GW_Z_BUFFER			0x000020
#define GW_PERSP_CORRECT	0x000040
#define GW_POLY_EDGES		0x000080
#define GW_BACKCULL			0x000100
#define GW_TWO_SIDED		0x000200
#define GW_COLOR_VERTS		0x000400
#define GW_SHADE_CVERTS		0x000800
#define GW_PICK				0x001000
#define GW_BOX_MODE			0x002000
#define GW_ALL_EDGES		0x004000
#define GW_VERT_TICKS		0x008000
#define GW_SHADE_SEL_FACES	0x010000
#define GW_TRANSPARENCY		0x020000
#define GW_TRANSPARENT_PASS	0x040000
#define GW_EMISSIVE_VERTS	0x080000
#define GW_ALL_OPAQUE		0x100000
#define GW_EDGES_ONLY		0x200000
#define GW_CONSTANT			0x400000

#define GW_LIGHTING			(GW_ILLUM | GW_SPECULAR)


// spotlight shapes
#define GW_SHAPE_RECT		0
#define GW_SHAPE_CIRCULAR	1

// texture tiling
#define GW_TEX_NO_TILING	0
#define GW_TEX_REPEAT		1
#define GW_TEX_MIRROR		2

// texture operations
#define GW_TEX_LEAVE				0	// Use the source pixel value
#define GW_TEX_REPLACE				1	// Use the texture pixel value
#define GW_TEX_MODULATE				2	// Multiply the source with the texture
#define GW_TEX_ADD					3	// Add the source and texture
#define GW_TEX_ADD_SIGNED			4	// Add the source and texture with an 0.5 subtraction
#define GW_TEX_SUBTRACT				5	// Subtract the source from the texture
#define GW_TEX_ADD_SMOOTH			6	// Add the source and the texture then subtract their product
#define GW_TEX_ALPHA_BLEND			7	// Alpha blend the texture with the source
#define GW_TEX_PREMULT_ALPHA_BLEND	8	// Alpha blend the the source with a premultiplied alpha texture

// texture scale factors
#define GW_TEX_SCALE_1X		0	// Multiply the tex op result by 1
#define GW_TEX_SCALE_2X		1	// Multiply the tex op result by 2
#define GW_TEX_SCALE_4X		2	// Multiply the tex op result by 4

// texture alpha sources
#define GW_TEX_ZERO			0	// Use no alpha value
#define GW_TEX_SOURCE		1	// Use the source alpha
#define GW_TEX_TEXTURE		2	// Use the texture alpha
#define GW_TEX_CONSTANT		3	// Use a constant BGRA color as an alpha
#define GW_TEX_PREVIOUS		4	// Use the previous texture stage alpha

// View volume clip flags
#define GW_LEFT_PLANE		0x0100
#define GW_RIGHT_PLANE		0x0200
#define GW_BOTTOM_PLANE		0x0400
#define GW_TOP_PLANE		0x0800
#define GW_FRONT_PLANE		0x1000
#define GW_BACK_PLANE		0x2000
#define GW_PLANE_MASK		0x3f00

// edge styles
#define GW_EDGE_SKIP		0
#define GW_EDGE_VIS			1
#define GW_EDGE_INVIS		2

// buffer types (for dual-plane stuff)
#define BUF_F_BUFFER		0
#define BUF_Z_BUFFER		1

// support method return values
#define GW_DOES_SUPPORT			TRUE
#define GW_DOES_NOT_SUPPORT		FALSE

// support queries
#define GW_SPT_TXT_CORRECT		0	// allow persp correction to be toggled?
#define GW_SPT_GEOM_ACCEL		1	// do 3D xforms, clipping, lighting thru driver?
#define GW_SPT_TRI_STRIPS		2	// send down strips instead of individual triangles?
#define GW_SPT_DUAL_PLANES		3	// allow dual planes to be used?
#define GW_SPT_SWAP_MODEL		4	// update viewports with complete redraw on WM_PAINT?
#define GW_SPT_INCR_UPDATE		5	// redraw only damaged areas on object move?
#define GW_SPT_1_PASS_DECAL		6	// do decaling with only one pass?
#define GW_SPT_DRIVER_CONFIG	7	// allow driver config dialog box?
#define GW_SPT_TEXTURED_BKG		8	// is viewport background a texture?
#define GW_SPT_VIRTUAL_VPTS		9	// are viewports bigger than the window allowed?
#define GW_SPT_PAINT_DOES_BLIT	10	// does WM_PAINT cause a backbuffer blit?
#define GW_SPT_WIREFRAME_STRIPS	11	// if true, wireframe objects are sent as tristrips
#define GW_SPT_ORG_UPPER_LEFT	12	// true if device origin is at upper left, false o/w
#define GW_SPT_ARRAY_PROCESSING	13	// true if the driver can handle vertex array data
#define GW_SPT_NUM_LIGHTS		14	// number of lights supported
#define GW_SPT_NUM_TEXTURES		15	// number of multitexture stages supported
#define GW_SPT_WIRE_FACES		16	// support for wireframe faces with visibility flags
#define GW_SPT_TOTAL			17	// always the max number of spt queries

// display state of the graphics window
#define GW_DISPLAY_MAXIMIZED	1
#define GW_DISPLAY_WINDOWED		2
#define GW_DISPLAY_INVISIBLE	3

// multi-pass rendering
#define GW_PASS_ONE				0
#define GW_PASS_TWO				1

// light types
enum LightType { OMNI_LGT, SPOT_LGT, DIRECT_LGT, AMBIENT_LGT };

// Light attenuation types -- not fully implemented
#define GW_ATTEN_NONE		0x0000
#define GW_ATTEN_START		0x0001
#define GW_ATTEN_END		0x0002
#define GW_ATTEN_LINEAR		0x0010
#define GW_ATTEN_QUAD		0x0020

// General 3D light structure
class Light : public BaseInterfaceServer {
public:
    DllExport Light();
    LightType		type;
    Point3			color;
    int				attenType;
    float			attenStart;
	float			attenEnd;
    float			intensity;
    float			hotSpotAngle;
	float			fallOffAngle;
	int				shape;
	float			aspect;
	int				overshoot;
	BOOL 			affectDiffuse;
	BOOL 			affectSpecular;
};

enum CameraType { PERSP_CAM, ORTHO_CAM };

// General camera structure
class Camera : public BaseInterfaceServer {
public:
	DllExport Camera();
	void			setPersp(float f, float asp)
						{ type = PERSP_CAM; persp.fov = f; 
						  persp.aspect = asp; makeMatrix(); }
	void			setOrtho(float l, float t, float r, float b)
						{ type = ORTHO_CAM; ortho.left = l; ortho.top = t; 
						  ortho.right = r; ortho.bottom = b; makeMatrix(); }
	void			setClip(float h, float y) 
						{ hither = h; yon = y; makeMatrix(); }
	CameraType		getType(void)	{ return type; }
	float			getHither(void) { return hither; }
	float			getYon(void)	{ return yon; }
	DllExport void	reset();
	void			getProj(float mat[4][4])	
						{ memcpy(mat, proj, 16 * sizeof(float)); }
private:
	DllExport void	makeMatrix();
	float			proj[4][4];
	CameraType		type;
	union {
	    struct {
            float	fov;
            float	aspect;
		} persp;
		struct {
		    float	left;
		    float	right;
		    float	bottom;
		    float	top;
		} ortho;
	};
	float			hither;
	float			yon;
};

const double pi        = 3.141592653589793;
const double piOver180 = 3.141592653589793 / 180.0;

// Color types (used by setColor)
enum ColorType { LINE_COLOR, FILL_COLOR, TEXT_COLOR, CLEAR_COLOR };

// Marker types
enum MarkerType  { POINT_MRKR, HOLLOW_BOX_MRKR, PLUS_SIGN_MRKR, 
						   ASTERISK_MRKR, X_MRKR, BIG_BOX_MRKR, 
						   CIRCLE_MRKR, TRIANGLE_MRKR, DIAMOND_MRKR,
						   SM_HOLLOW_BOX_MRKR, SM_CIRCLE_MRKR, 
						   SM_TRIANGLE_MRKR, SM_DIAMOND_MRKR,
						   DOT_MRKR, SM_DOT_MRKR,
						   BOX2_MRKR, BOX3_MRKR, BOX4_MRKR, BOX5_MRKR,
						   BOX6_MRKR, BOX7_MRKR,
						   DOT2_MRKR, DOT3_MRKR, DOT4_MRKR, DOT5_MRKR,
						   DOT6_MRKR, DOT7_MRKR
};


#define AC_DIR_RL_CROSS		0	// right->left => crossing (AutoCAD compatible)
#define AC_DIR_LR_CROSS		1	// left->right => crossing (ACAD incompatible)

DllExport void	setAutoCross(int onOff);
DllExport int	getAutoCross();
DllExport void	setAutoCrossDir(int dir);
DllExport int	getAutoCrossDir();

// Region types (for built-in hit-testing)
#define POINT_RGN	0x0001
#define	RECT_RGN	0x0002
#define CIRCLE_RGN	0x0004
#define FENCE_RGN	0x0008

// region directions (left or right)
#define RGN_DIR_UNDEF	-1
#define RGN_DIR_RIGHT	0	
#define RGN_DIR_LEFT	1

typedef struct tagCIRCLE
{
    LONG  x;
    LONG  y;
	LONG  r;
} CIRCLE;

class HitRegion {
	DWORD size;
public:
	int				type;
	int				dir;		// region direction
	int				crossing;	// not used for point
	int				epsilon;	// not used for rect or circle
	union {
		POINT		pt;
		RECT		rect;
		CIRCLE		circle;
		POINT *		pts;
	};
	HitRegion()		{ dir = RGN_DIR_UNDEF; size = sizeof(HitRegion);}
};

inline int ABS(const int x) { return (x > 0) ? x : -x; }

typedef void (*GFX_ESCAPE_FN)(void *);


// driver types for getDriver() method
#define GW_DRV_RENDERER		0
#define GW_DRV_DEVICE		1

// for possible future implementation
#define GW_HEIDI			0
#define GW_OPENGL			1
#define GW_DIRECT3D			2
#define GW_HEIDI3D			3
#define GW_NULL				4
#define GW_CUSTOM			5

// graphics window setup structure
class GWinSetup {
public:
    DllExport GWinSetup();
    TCHAR		caption[GW_MAX_CAPTION_LEN];
	TCHAR		renderer[GW_MAX_FILE_LEN];
	TCHAR		device[GW_MAX_FILE_LEN];
	DWORD		winStyle;
	POINT		size;
	POINT		place;
	INT_PTR			id;
				// WIN64 Cleanup: Shuler
	int			type;
};

// abstract graphics window class
class GraphicsWindow : public InterfaceServer {
public:
	virtual	~GraphicsWindow() {}
	virtual void	postCreate(int ct, GraphicsWindow **gw) = 0;
	virtual void	shutdown() = 0;
	virtual int		getVersion() = 0;
	virtual TCHAR * getDriverString(void) = 0;
	virtual void	config(HWND hWnd) = 0;
	virtual int		querySupport(int what) = 0;

	virtual HWND	getHWnd(void) = 0;
	virtual void	setPos(int x, int y, int w, int h) = 0;
	virtual void	setDisplayState(int s) = 0;
	virtual int		getDisplayState() = 0;
	virtual int		getWinSizeX() = 0;
	virtual int		getWinSizeY() = 0;
	virtual DWORD	getWinDepth(void) = 0;
	virtual DWORD	getHitherCoord(void) = 0;
	virtual DWORD	getYonCoord(void) = 0;
	virtual void	getTextExtents(TCHAR *text, SIZE *sp) = 0;
	virtual int		getMaxStripLength() { return GFX_MAX_STRIP; }
	virtual void	setFlags(DWORD f) = 0;
	virtual DWORD	getFlags() = 0;

	virtual void	resetUpdateRect() = 0;
	virtual void	enlargeUpdateRect(RECT *rp) = 0;
	virtual int		getUpdateRect(RECT *rp) = 0;
    virtual void	updateScreen() = 0;

	virtual BOOL	setBufAccess(int which, int b) = 0;
	virtual BOOL	getBufAccess(int which) = 0;
	virtual BOOL	getBufSize(int which, int *size) = 0;
	virtual BOOL	getBuf(int which, int size, void *buf) = 0;
	virtual BOOL	setBuf(int which, int size, void *buf, RECT *rp) = 0;
	virtual BOOL	getDIB(BITMAPINFO *bmi, int *size) = 0;
	virtual BOOL	setBackgroundDIB(int width, int height, BITMAPINFO *bmi) = 0;
	virtual void	setBackgroundOffset(int x, int y) = 0;
	virtual int		useClosestTextureSize(int bkg=FALSE) = 0;
	virtual int		getTextureSize(int bkg=FALSE) = 0;
	virtual DWORD_PTR	getTextureHandle(BITMAPINFO *bmi) = 0;
				// WIN64 Cleanup: Shuler
	virtual void	freeTextureHandle(DWORD_PTR handle) = 0;
				// WIN64 Cleanup: Shuler
	virtual BOOL	setTextureByHandle(DWORD_PTR handle, int texStage=0) = 0;
				// WIN64 Cleanup: Shuler
    virtual void	setTextureColorOp(int texStage=0, int texOp=GW_TEX_MODULATE, int texAlphaSource=GW_TEX_TEXTURE, int texScale=GW_TEX_SCALE_1X) = 0;
    virtual void	setTextureAlphaOp(int texStage=0, int texOp=GW_TEX_MODULATE, int texAlphaSource=GW_TEX_TEXTURE, int texScale=GW_TEX_SCALE_1X) = 0;
	virtual BOOL	setTextureTiling(int u, int v, int w=GW_TEX_NO_TILING, int texStage=0) = 0;
	virtual int		getTextureTiling(int which, int texStage=0) = 0;

	virtual void	beginFrame() = 0;
	virtual void	endFrame() = 0;
	virtual void	setViewport(int x, int y, int w, int h) = 0;
	virtual void	setVirtualViewportParams(float zoom, float xOffset, float yOffset) = 0;
	virtual void	setUseVirtualViewport(int onOff) = 0;
    virtual void	clearScreen(RECT *rp, int useBkg = FALSE) = 0;
    virtual void	setTransform(const Matrix3 &m) = 0;
	virtual Matrix3	getTransform(void) = 0;
	virtual void	multiplePass(int pass, BOOL onOff, float scaleFact = 1.005f) = 0;
    virtual void	setTransparency(DWORD settings) = 0;
    virtual void	setTexTransform(const Matrix3 &m, int texStage=0) = 0;
	virtual BOOL	getFlipped(void)=0;
	virtual void	setSkipCount(int c) = 0;
	virtual int		getSkipCount(void) = 0;
	virtual void	setViewportLimits(DWORD l) = 0;
	virtual DWORD	getViewportLimits(void) = 0;
    virtual void	setRndLimits(DWORD l) = 0;
	virtual DWORD 	getRndLimits(void) = 0;
	virtual DWORD 	getRndMode(void) = 0;
	virtual int		getMaxLights(void) = 0;
    virtual void	setLight(int num, const Light *l) = 0;
	virtual void	setLightExclusion(DWORD exclVec) = 0;
    virtual void	setCamera(const Camera &c) = 0;
	virtual void	setCameraMatrix(float mat[4][4], Matrix3 *invTM, int persp, float hither, float yon) = 0;
	virtual void	getCameraMatrix(float mat[4][4], Matrix3 *invTM, int *persp, float *hither, float *yon) = 0;
    virtual void	setColor(ColorType t, float r, float g, float b) = 0;
			void	setColor(ColorType t, Point3 clr) { setColor(t,clr.x,clr.y,clr.z); }
    virtual void	setMaterial(const Material &m, int index=0) = 0;
	virtual Material *getMaterial(void) = 0;
	virtual int		getMaxTextures(void) = 0;

	virtual DWORD	hTransPoint(const Point3 *in, IPoint3 *out) = 0;
	virtual DWORD	wTransPoint(const Point3 *in, IPoint3 *out) = 0;
	virtual DWORD	transPoint(const Point3 *in, Point3 *out) = 0;
	virtual void	lightVertex(const Point3 &pos, const Point3 &nor, Point3 &rgb) = 0;

	virtual void	hText(IPoint3 *xyz, TCHAR *s) = 0;
	virtual void	hMarker(IPoint3 *xyz, MarkerType type) = 0;
	virtual void	hPolyline(int ct, IPoint3 *xyz, Point3 *rgb, int closed, int *es) = 0;
			void	hPolyline(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int closed, int *es)
					{ hPolyline(ct, xyz, rgb, closed, es); }
	virtual void	hPolygon(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	virtual void	hTriStrip(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;

	virtual void	wText(IPoint3 *xyz, TCHAR *s) = 0;
	virtual void	wMarker(IPoint3 *xyz, MarkerType type) = 0;
	virtual void	wPolyline(int ct, IPoint3 *xyz, Point3 *rgb, int closed, int *es) = 0;
			void	wPolyline(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int closed, int *es)
					{ wPolyline(ct, xyz, rgb, closed, es); }
	virtual void	wPolygon(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	virtual void	wTriStrip(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;

    virtual void 	text(Point3 *xyz, TCHAR *s) = 0;
	virtual void	startMarkers() = 0;
    virtual void	marker(Point3 *xyz, MarkerType type) = 0;
	virtual void	endMarkers() = 0;
	virtual void	polyline(int ct, Point3 *xyz, Point3 *rgb, int closed, int *es) = 0;
			void	polyline(int ct, Point3 *xyz, Point3 *rgb, Point3 *uvw, int closed, int *es)
					{ polyline(ct, xyz, rgb, closed, es); }
	virtual void	polylineN(int ct, Point3 *xyz, Point3 *nor, int closed, int *es) = 0;
	virtual void	startSegments() = 0;
	virtual void	segment(Point3 *xyz, int vis) = 0;
	virtual void	endSegments() = 0;
	virtual void 	polygon(int ct, Point3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	virtual void 	polygonN(int ct, Point3 *xyz, Point3 *nor, Point3 *uvw, int texNum=1) = 0;
	virtual void	triStrip(int ct, Point3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	virtual void	triStripN(int ct, Point3 *xyz, Point3 *nor, Point3 *uvw, int texNum=1) = 0;
	virtual void	startTriangles() = 0;
	virtual void	triangle(Point3 *xyz, Point3 *rgb) = 0;
	virtual void	triangleN(Point3 *xyz, Point3 *nor, Point3 *uvw, int texNum=1) = 0;
	virtual void	triangleNC(Point3 *xyz, Point3 *nor, Point3 *rgb) = 0;
	virtual void	triangleNCT(Point3 *xyz, Point3 *nor, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	virtual void	triangleW(Point3 *xyz, int *es) = 0;
	virtual void	triangleNW(Point3 *xyz, Point3 *nor, int *es) = 0;
	virtual void	endTriangles() = 0;
	virtual void	loadMeshData(DWORD_PTR id, int xyzCt, Point3 *xyz, int norCt, Point3 *nor, int texNum, int uvwCt, Point3 *uvw, int mtlCt, Material *mtl) = 0;
	virtual void	processStrips(DWORD_PTR id, int stripCt, StripTab *s, GFX_ESCAPE_FN fn) = 0;
	virtual void	processWireFaces(int xyzCt, Point3 *xyz, int faceCt, GWFace *face, int dispFlags, BitArray *faceSel, BitArray *edgeSel, int mtlCt, Material *mtl, GFX_ESCAPE_FN fn) = 0;

	virtual void	setHitRegion(HitRegion *rgn) = 0;
	virtual void	clearHitCode(void) = 0;
	virtual BOOL	checkHitCode(void) = 0;
	virtual void	setHitCode(BOOL h) = 0;
	virtual DWORD	getHitDistance(void) = 0;
	virtual void	setHitDistance(DWORD d)	= 0;

	virtual int		isPerspectiveView(void) = 0;
	virtual float	interpWorld(Point3 *world1, Point3 *world2, float sParam, Point3 *interpPt) = 0;

	virtual void	escape(GFX_ESCAPE_FN fn, void *data) = 0;
};

// for Windows int coords with origin at upper-left
inline int wIsFacingBack(const IPoint3 &v0, const IPoint3 &v1, const IPoint3 &v2, int flip=0 )
{
	int s = ( (v0[0]-v1[0])*(v2[1]-v1[1]) - (v2[0]-v1[0])*(v0[1]-v1[1]) ) < 0;
	return flip ? !s : s;
}

// for HEIDI int coords with origin at lower-left
inline int hIsFacingBack(const IPoint3 &v0, const IPoint3 &v1, const IPoint3 &v2, int flip=0 )
{
	int s = ( (v0[0]-v1[0])*(v2[1]-v1[1]) - (v2[0]-v1[0])*(v0[1]-v1[1]) );
	return flip ? s < 0 : s > 0;
}

// CAL-03/05/03: include side facing in the facing type
enum FacingType {kFrontFacing, kSideFacing, kBackFacing};

inline FacingType wFacingType(const IPoint3 &v0, const IPoint3 &v1, const IPoint3 &v2, int flip=0 )
{
	int s = ( (v0[0]-v1[0])*(v2[1]-v1[1]) - (v2[0]-v1[0])*(v0[1]-v1[1]) );
	return (s == 0) ? kSideFacing : ((flip ? s > 0 : s < 0) ? kBackFacing : kFrontFacing);
}

inline FacingType hFacingType(const IPoint3 &v0, const IPoint3 &v1, const IPoint3 &v2, int flip=0 )
{
	int s = ( (v0[0]-v1[0])*(v2[1]-v1[1]) - (v2[0]-v1[0])*(v0[1]-v1[1]) );
	return (s == 0) ? kSideFacing : ((flip ? s < 0 : s > 0) ? kBackFacing : kFrontFacing);
}

DllExport HINSTANCE GetGraphicsLibHandle(TCHAR *driverLibName);
DllExport BOOL GraphicsSystemIsAvailable(HINSTANCE drv);
DllExport BOOL GraphicsSystemCanConfigure(HINSTANCE drv);
DllExport BOOL GraphicsSystemConfigure(HWND hWnd, HINSTANCE drv);
DllExport void FreeGraphicsLibHandle(HINSTANCE drv);

DllExport GraphicsWindow *createGW(HWND hWnd, GWinSetup &gws);

DllExport void getRegionRect(HitRegion *hr, RECT *rect);
DllExport BOOL pointInRegion(int x, int y, HitRegion *hr);

DllExport int distToLine(int x, int y, int *p1, int *p2);
DllExport int zDepthToLine(int x, int y, int *p1, int *p2);

DllExport int lineCrossesRect(RECT *rc, int *p1, int *p2);
DllExport int segCrossesRect(RECT *rc, int *p1, int *p2);
DllExport int segCrossesCircle(int cx, int cy, int r, int *p1, int *p2);
DllExport BOOL insideTriangle(IPoint3 &p0, IPoint3 &p1, IPoint3 &p2, IPoint3 &q);
DllExport int getZfromTriangle(IPoint3 &p0, IPoint3 &p1, IPoint3 &p2, IPoint3 &q);

DllExport int getClosestPowerOf2(int num);

// colors for drawing in viewports
#define COLOR_SELECTION				0
#define COLOR_SUBSELECTION			1
#define COLOR_FREEZE				2
#define COLOR_GRID					3
#define COLOR_GRID_INTENS			4
#define COLOR_SF_LIVE				5
#define COLOR_SF_ACTION				6
#define COLOR_SF_TITLE				7
#define COLOR_VP_LABELS				8
#define COLOR_VP_INACTIVE			9
#define COLOR_ARCBALL				10
#define COLOR_ARCBALL_HILITE		11
#define COLOR_ANIM_BUTTON			12
#define COLOR_SEL_BOXES				13
#define COLOR_LINK_LINES			14
#define COLOR_TRAJECTORY			15
#define COLOR_ACTIVE_AXIS			16
#define COLOR_INACTIVE_AXIS			17
#define COLOR_SPACE_WARPS			18
#define COLOR_DUMMY_OBJ				19
#define COLOR_POINT_OBJ				20
#define COLOR_POINT_AXES			21
#define COLOR_TAPE_OBJ				22
#define COLOR_BONES					23
#define COLOR_GIZMOS				24
#define COLOR_SEL_GIZMOS			25
#define COLOR_SPLINE_VECS			26
#define COLOR_SPLINE_HANDLES		27
#define COLOR_PATCH_LATTICE			28	// No longer used -- TH 2/20/99
#define COLOR_PARTICLE_EM			29
#define COLOR_CAMERA_OBJ			30
#define COLOR_CAMERA_CONE			31
#define COLOR_CAMERA_HORIZ			32
#define COLOR_NEAR_RANGE			33
#define COLOR_FAR_RANGE				34
#define COLOR_LIGHT_OBJ				35
#define COLOR_TARGET_LINE			36
#define COLOR_HOTSPOT				37
#define COLOR_FALLOFF				38
#define COLOR_START_RANGE			39
#define COLOR_END_RANGE				40
#define COLOR_VIEWPORT_BKG			41
#define COLOR_TRAJ_TICS_1			42
#define COLOR_TRAJ_TICS_2			43
#define COLOR_TRAJ_TICS_3			44
#define COLOR_GHOST_BEFORE			45
#define COLOR_GHOST_AFTER			46
#define COLOR_12FIELD_GRID			47
#define COLOR_START_RANGE1			48
#define COLOR_END_RANGE1			49
#define COLOR_CAMERA_CLIP  			50
#define COLOR_NURBS_CV				51
#define COLOR_NURBS_LATTICE			52
#define COLOR_NURBS_CP				53
#define COLOR_NURBS_FP				54
#define COLOR_NURBS_DEP				55
#define COLOR_NURBS_ERROR			56
#define COLOR_NURBS_HILITE			57
#define COLOR_NURBS_FUSE			58
#define COLOR_END_EFFECTOR			59
#define COLOR_END_EFFECTOR_STRING	60
#define COLOR_JOINT_LIMIT_SEL		61
#define COLOR_JOINT_LIMIT_UNSEL		62
#define COLOR_IK_TERMINATOR			63
#define COLOR_SF_USER				64
#define COLOR_VERT_TICKS			65
#define COLOR_XRAY					66
#define COLOR_GROUP_OBJ				67
#define COLOR_MANIPULATOR_X			68
#define COLOR_MANIPULATOR_Y			69
#define COLOR_MANIPULATOR_Z			70
#define COLOR_MANIPULATOR_ACTIVE	71
#define COLOR_VPT_CLIPPING			72
#define COLOR_DECAY_RADIUS			73
#define COLOR_VERT_NUMBERS			74
#define COLOR_CROSSHAIR_CURSOR		75

#define COLOR_SV_WINBK              76 // SV Window Background
#define COLOR_SV_NODEBK             77 // SV Default Node Background
#define COLOR_SV_SELNODEBK          78 // SV Selected Node Background
#define COLOR_SV_NODE_HIGHLIGHT     79 // SV Viewport Selected Node Highlight
#define COLOR_SV_MATERIAL_HIGHLIGHT 80 // SV MEDIT Selected Node Highlight
#define COLOR_SV_MODIFIER_HIGHLIGHT 81 // SV Selected Modifier Highlight
#define COLOR_SV_PLUGIN_HIGHLIGHT   82 // SV Plug-in Highlight
#define COLOR_SV_SUBANIM_LINE       83 // SV Subanim line color
#define COLOR_SV_CHILD_LINE         84 // SV Child node line color
#define COLOR_SV_FRAME              85 // SV Frame color
#define COLOR_SV_SELTEXT            86 // SV Selected Label Color
#define COLOR_SV_TEXT               87 // SV Label Color

#define COLOR_UNSEL_TAB				88
#define COLOR_ATMOS_APPARATUS		89	// mjm - 1.21.99
#define COLOR_SUBSELECTION_HARD		90  //2-3-99 watje
#define COLOR_SUBSELECTION_MEDIUM	91  //2-3-99 watje
#define COLOR_SUBSELECTION_SOFT		92  //2-3-99 watje

#define COLOR_SV_UNFOLD_BUTTON      93 // SV Unfold Button
#define COLOR_SV_GEOMOBJECT_BK      94 // Geometry Object Node Background
#define COLOR_SV_LIGHT_BK           95 // Light Node Background
#define COLOR_SV_CAMERA_BK          96 // Camera Node Background
#define COLOR_SV_SHAPE_BK           97 // Shape Node Background
#define COLOR_SV_HELPER_BK          98 // Helper Node Background
#define COLOR_SV_SYSTEM_BK          99 // System Node Background
#define COLOR_SV_CONTROLLER_BK     100 // Controller Node Background
#define COLOR_SV_MODIFIER_BK       101 // Modifier Node Background
#define COLOR_SV_MATERIAL_BK       102 // Material Node Background
#define COLOR_SV_MAP_BK            103 // Map Node Background
#define COLOR_SETKEY_BUTTON        104

#define COLOR_BACK_LINES           105 // Backface lines on selected objects
#define COLOR_BACK_VERTS           106 // Backface vertices on selected objects

#define COLOR_MANIPULATOR_CONTOUR  107 // Background color for the rotation gizmo
#define COLOR_MANIPULATOR_SCREEN   108 // screen space manipulator handle color for the rotation gizmo
#define COLOR_MANIPULATOR_TRAIL    109 // move gizmo displacement trail color

const int kColorNormalsUnspecified = 110;
const int kColorNormalsSpecified = 111;
const int kColorNormalsExplicit = 112;

#define COLOR_SV_GRID              113 // SV Grid
#define COLOR_SV_REL_INSTANCE      114 // SV Relationship Instances
#define COLOR_SV_REL_CONSTRAINT    115 // SV Relationship Contraints
#define COLOR_SV_REL_PARAMWIRE     116 // SV Relationship Param Wires
#define COLOR_SV_REL_LIGHT         117 // SV Relationship Lights
#define COLOR_SV_REL_MODIFIER      118 // SV Relationship Modifiers
#define COLOR_SV_REL_CONTROLLER    119 // SV Relationship Controllers
#define COLOR_SV_REL_OTHER         120 // SV Relationship Others
#define COLOR_SV_SPACEWARP_BK      121 // SV SpaceWarp
#define COLOR_SV_BASEOBJECT_BK     122 // SV BaseObject

#define COLOR_TOTAL                123	// always the max number of colors

// Returns/sets color values for drawing in the viewport (selection, subsel, etc)
DllExport Point3 GetUIColor(int which);
DllExport void SetUIColor(int which, Point3 *clr);
DllExport Point3 GetDefaultUIColor(int which);

#define GetSelColor()		GetUIColor(COLOR_SELECTION)
#define GetSubSelColor()	GetUIColor(COLOR_SUBSELECTION)
#define GetFreezeColor()	GetUIColor(COLOR_FREEZE)


	
#endif // _GFX_H_

