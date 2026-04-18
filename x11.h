typedef unsigned char KeyCode; typedef int Bool; typedef unsigned long Atom; typedef unsigned long KeySym;
#define	XkbActionMessageLength 6
#define XA_ATOM ((Atom) 4)
#define XA_CARDINAL ((Atom) 6)
#define XA_WINDOW ((Atom) 33)
typedef int XrmQuark, *XrmQuarkList;
typedef char *XPointer;
typedef struct { unsigned int size; XPointer addr; } XrmValue, *XrmValuePtr;
typedef struct _XrmHashBucketRec *XrmDatabase;
typedef unsigned int XcursorUInt; typedef XcursorUInt XcursorDim; typedef XcursorUInt XcursorPixel;
typedef struct _XcursorImage { XcursorUInt version; XcursorDim size,width,height,xhot,yhot; XcursorUInt delay; XcursorPixel *pixels; } XcursorImage;
typedef unsigned short	Rotation;
typedef unsigned short	SubpixelOrder;
typedef unsigned short	Connection;
#define RROutputChangeNotifyMask (1L << 2)
#define RRNotify 1
#define RR_Rotate_90 2
#define RR_Rotate_270 8
#define RR_Interlace 0x00000010
#define RR_Connected 0
typedef struct { long flags; int x,y, width,height,min_width,min_height,max_width,max_height,width_inc,height_inc; struct {int x; int y;} min_aspect,max_aspect; int base_width, base_height; int win_gravity; } XSizeHints;
typedef unsigned long XID,Mask,Atom,VisualID,Time;
typedef XID Window;
typedef XID Drawable;
typedef XID Font;
typedef XID Pixmap;
typedef XID Cursor;
typedef XID Colormap;
#define None                 0L	/* universal null resource or null atom */
#define CurrentTime          0L	/* special Time */
#define KeyPressMask			(1L<<0)
#define KeyReleaseMask			(1L<<1)
#define ButtonPressMask			(1L<<2)
#define ButtonReleaseMask		(1L<<3)
#define EnterWindowMask			(1L<<4)
#define LeaveWindowMask			(1L<<5)
#define PointerMotionMask		(1L<<6)
#define ExposureMask			(1L<<15)
#define VisibilityChangeMask		(1L<<16)
#define StructureNotifyMask		(1L<<17)
#define SubstructureNotifyMask		(1L<<19)
#define SubstructureRedirectMask	(1L<<20)
#define FocusChangeMask			(1L<<21)
#define PropertyChangeMask		(1L<<22)
#define KeyPress		2
#define KeyRelease		3
#define ButtonPress		4
#define ButtonRelease		5
#define MotionNotify		6
#define EnterNotify		7
#define LeaveNotify		8
#define FocusIn			9
#define FocusOut		10
#define VisibilityNotify	15
#define DestroyNotify		17
#define ReparentNotify		21
#define ConfigureNotify		22
#define PropertyNotify		28
#define ClientMessage		33
#define GenericEvent		35
#define NotifyGrab		1
#define NotifyUngrab		2
#define PropertyNewValue	0
#define GrabModeAsync		1
#define RevertToParent		2
#define Success		   0	/* everything's okay */
#define BadWindow	   3	/* parameter not a Window */
#define InputOutput		1
#define InputOnly		2
#define CWBorderPixel           (1L<<3)
#define CWOverrideRedirect	(1L<<9)
#define CWEventMask		(1L<<11)
#define CWColormap		(1L<<13)
#define StaticGravity		10
#define IsViewable		2
#define PropModeReplace         0
#define PropModeAppend          2
#define DontPreferBlanking	0
#define DefaultExposures	2

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif

typedef char *XPointer;
#define Bool int
#define Status int
#define True 1
#define False 0
#define ConnectionNumber(dpy) 	(((_XPrivDisplay)(dpy))->fd)
#define RootWindow(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->root)
#define DefaultScreen(dpy) 	(((_XPrivDisplay)(dpy))->default_screen)
#define DefaultVisual(dpy, scr) (ScreenOfDisplay(dpy,scr)->root_visual)
#define QLength(dpy) 		(((_XPrivDisplay)(dpy))->qlen)
#define DisplayWidth(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->width)
#define DisplayHeight(dpy, scr) (ScreenOfDisplay(dpy,scr)->height)
#define DisplayWidthMM(dpy, scr)(ScreenOfDisplay(dpy,scr)->mwidth)
#define DisplayHeightMM(dpy, scr)(ScreenOfDisplay(dpy,scr)->mheight)
#define DefaultDepth(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->root_depth)
#define ScreenOfDisplay(dpy, scr)(&((_XPrivDisplay)(dpy))->screens[scr])
typedef struct _XExtData {
	int number;		/* number returned by XRegisterExtension */
	struct _XExtData *next;	/* next item on list of data for structure */
	int (*free_private)(	/* called to free private storage */
	struct _XExtData *extension
	);
	XPointer private_data;	/* data private to this extension. */
} XExtData;


typedef struct {		/* public to extension, cannot be changed */
	int extension;		/* extension number */
	int major_opcode;	/* major op-code assigned by server */
	int first_event;	/* first event number for the extension */
	int first_error;	/* first error number for the extension */
} XExtCodes;

/*
 * Data structure for retrieving info about pixmap formats.
 */

typedef struct {
    int depth;
    int bits_per_pixel;
    int scanline_pad;
} XPixmapFormatValues;


/*
 * Data structure for setting graphics context.
 */
typedef struct {
	int function;		/* logical operation */
	unsigned long plane_mask;/* plane mask */
	unsigned long foreground;/* foreground pixel */
	unsigned long background;/* background pixel */
	int line_width;		/* line width */
	int line_style;	 	/* LineSolid, LineOnOffDash, LineDoubleDash */
	int cap_style;	  	/* CapNotLast, CapButt,
				   CapRound, CapProjecting */
	int join_style;	 	/* JoinMiter, JoinRound, JoinBevel */
	int fill_style;	 	/* FillSolid, FillTiled,
				   FillStippled, FillOpaqueStippled */
	int fill_rule;	  	/* EvenOddRule, WindingRule */
	int arc_mode;		/* ArcChord, ArcPieSlice */
	Pixmap tile;		/* tile pixmap for tiling operations */
	Pixmap stipple;		/* stipple 1 plane pixmap for stippling */
	int ts_x_origin;	/* offset for tile or stipple operations */
	int ts_y_origin;
        Font font;	        /* default text font for text operations */
	int subwindow_mode;     /* ClipByChildren, IncludeInferiors */
	Bool graphics_exposures;/* boolean, should exposures be generated */
	int clip_x_origin;	/* origin for clipping */
	int clip_y_origin;
	Pixmap clip_mask;	/* bitmap clipping; other calls for rects */
	int dash_offset;	/* patterned/dashed line information */
	char dashes;
} XGCValues;

/*
 * Graphics context.  The contents of this structure are implementation
 * dependent.  A GC should be treated as opaque by application code.
 */

typedef struct _XGC
#ifdef XLIB_ILLEGAL_ACCESS
{
    XExtData *ext_data;	/* hook for extension to hang data */
    GContext gid;	/* protocol ID for graphics context */
    /* there is more to this structure, but it is private to Xlib */
}
#endif
*GC;

/*
 * Visual structure; contains information about colormapping possible.
 */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	VisualID visualid;	/* visual id of this visual */
#if defined(__cplusplus) || defined(c_plusplus)
	int c_class;		/* C++ class of screen (monochrome, etc.) */
#else
	int class;		/* class of screen (monochrome, etc.) */
#endif
	unsigned long red_mask, green_mask, blue_mask;	/* mask values */
	int bits_per_rgb;	/* log base 2 of distinct color values */
	int map_entries;	/* color map entries */
} Visual;

typedef struct { int depth,nvisuals; Visual *visuals; } Depth;
struct _XDisplay;		/* Forward declare before use for C++ */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XDisplay *display;/* back pointer to display structure */
	Window root;		/* Root window id. */
	int width, height;	/* width and height of screen */
	int mwidth, mheight;	/* width and height of  in millimeters */
	int ndepths;		/* number of depths possible */
	Depth *depths;		/* list of allowable depths on the screen */
	int root_depth;		/* bits per pixel */
	Visual *root_visual;	/* root visual */
	GC default_gc;		/* GC for the root root visual */
	Colormap cmap;		/* default color map */
	unsigned long white_pixel;
	unsigned long black_pixel;	/* White and Black pixel values */
	int max_maps, min_maps;	/* max and min color maps */
	int backing_store;	/* Never, WhenMapped, Always */
	Bool save_unders;
	long root_input_mask;	/* initial root input mask */
} Screen;

/*
 * Format structure; describes ZFormat data the screen will understand.
 */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	int depth;		/* depth of this image format */
	int bits_per_pixel;	/* bits/pixel at this depth */
	int scanline_pad;	/* scanline must padded to this multiple */
} ScreenFormat;

/*
 * Data structure for setting window attributes.
 */
typedef struct {
    Pixmap background_pixmap;	/* background or None or ParentRelative */
    unsigned long background_pixel;	/* background pixel */
    Pixmap border_pixmap;	/* border of the window */
    unsigned long border_pixel;	/* border pixel value */
    int bit_gravity;		/* one of bit gravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preserved if possible */
    unsigned long backing_pixel;/* value to use in restoring planes */
    Bool save_under;		/* should bits under be saved? (popups) */
    long event_mask;		/* set of events that should be saved */
    long do_not_propagate_mask;	/* set of events that should not propagate */
    Bool override_redirect;	/* boolean value for override-redirect */
    Colormap colormap;		/* color map to be associated with window */
    Cursor cursor;		/* cursor to be displayed (or None) */
} XSetWindowAttributes;

typedef struct {
    int x, y;			/* location of window */
    int width, height;		/* width and height of window */
    int border_width;		/* border width of window */
    int depth;          	/* depth of window */
    Visual *visual;		/* the associated visual structure */
    Window root;        	/* root of screen containing window */
#if defined(__cplusplus) || defined(c_plusplus)
    int c_class;		/* C++ InputOutput, InputOnly*/
#else
    int class;			/* InputOutput, InputOnly*/
#endif
    int bit_gravity;		/* one of bit gravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preserved if possible */
    unsigned long backing_pixel;/* value to be used when restoring planes */
    Bool save_under;		/* boolean, should bits under be saved? */
    Colormap colormap;		/* color map to be associated with window */
    Bool map_installed;		/* boolean, is color map currently installed*/
    int map_state;		/* IsUnmapped, IsUnviewable, IsViewable */
    long all_event_masks;	/* set of events all people have interest in*/
    long your_event_mask;	/* my event mask */
    long do_not_propagate_mask; /* set of events that should not propagate */
    Bool override_redirect;	/* boolean value for override-redirect */
    Screen *screen;		/* back pointer to correct screen */
} XWindowAttributes;

#ifndef XLIB_ILLEGAL_ACCESS
typedef struct _XDisplay Display;
#endif

struct _XPrivate;		/* Forward declare before use for C++ */
struct _XrmHashBucketRec;

typedef struct
#ifdef XLIB_ILLEGAL_ACCESS
_XDisplay
#endif
{
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XPrivate *private1;
	int fd;			/* Network socket. */
	int private2;
	int proto_major_version;/* major version of server's X protocol */
	int proto_minor_version;/* minor version of servers X protocol */
	char *vendor;		/* vendor of the server hardware */
        XID private3;
	XID private4;
	XID private5;
	int private6;
	XID (*resource_alloc)(	/* allocator function */
		struct _XDisplay*
	);
	int byte_order;		/* screen byte order, LSBFirst, MSBFirst */
	int bitmap_unit;	/* padding and data requirements */
	int bitmap_pad;		/* padding requirements on bitmaps */
	int bitmap_bit_order;	/* LeastSignificant or MostSignificant */
	int nformats;		/* number of pixmap formats in list */
	ScreenFormat *pixmap_format;	/* pixmap format list */
	int private8;
	int release;		/* release of the server */
	struct _XPrivate *private9, *private10;
	int qlen;		/* Length of input event queue */
	unsigned long last_request_read; /* seq number of last event read */
	unsigned long request;	/* sequence number of last request. */
	XPointer private11;
	XPointer private12;
	XPointer private13;
	XPointer private14;
	unsigned max_request_size; /* maximum number 32 bit words in request*/
	struct _XrmHashBucketRec *db;
	int (*private15)(
		struct _XDisplay*
		);
	char *display_name;	/* "host:display" string used on this connect*/
	int default_screen;	/* default screen for operations */
	int nscreens;		/* number of screens on this server*/
	Screen *screens;	/* pointer to list of screens */
	unsigned long motion_buffer;	/* size of motion buffer */
	unsigned long private16;
	int min_keycode;	/* minimum defined keycode */
	int max_keycode;	/* maximum defined keycode */
	XPointer private17;
	XPointer private18;
	int private19;
	char *xdefaults;	/* contents of defaults from server */
	/* there is more to this structure, but it is private to Xlib */
}
#ifdef XLIB_ILLEGAL_ACCESS
Display,
#endif
*_XPrivDisplay;

#undef _XEVENT_
#ifndef _XEVENT_
/*
 * Definitions of specific events.
 */
typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window it is reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int keycode;	/* detail */
	Bool same_screen;	/* same screen flag */
} XKeyEvent;
typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;

typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window it is reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int button;	/* detail */
	Bool same_screen;	/* same screen flag */
} XButtonEvent;
typedef XButtonEvent XButtonPressedEvent;
typedef XButtonEvent XButtonReleasedEvent;

typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	char is_hint;		/* detail */
	Bool same_screen;	/* same screen flag */
} XMotionEvent;
typedef XMotionEvent XPointerMovedEvent;

typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
	int detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior,
	 * NotifyNonlinear,NotifyNonlinearVirtual
	 */
	Bool same_screen;	/* same screen flag */
	Bool focus;		/* boolean focus */
	unsigned int state;	/* key or button mask */
} XCrossingEvent;
typedef XCrossingEvent XEnterWindowEvent;
typedef XCrossingEvent XLeaveWindowEvent;

typedef struct {
	int type;		/* FocusIn or FocusOut */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;		/* window of event */
	int mode;		/* NotifyNormal, NotifyWhileGrabbed,
				   NotifyGrab, NotifyUngrab */
	int detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior,
	 * NotifyNonlinear,NotifyNonlinearVirtual, NotifyPointer,
	 * NotifyPointerRoot, NotifyDetailNone
	 */
} XFocusChangeEvent;
typedef XFocusChangeEvent XFocusInEvent;
typedef XFocusChangeEvent XFocusOutEvent;

/* generated on EnterWindow and FocusIn  when KeyMapState selected */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	char key_vector[32];
} XKeymapEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int x, y;
	int width, height;
	int count;		/* if non-zero, at least this many more */
} XExposeEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Drawable drawable;
	int x, y;
	int width, height;
	int count;		/* if non-zero, at least this many more */
	int major_code;		/* core is CopyArea or CopyPlane */
	int minor_code;		/* not defined in the core */
} XGraphicsExposeEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Drawable drawable;
	int major_code;		/* core is CopyArea or CopyPlane */
	int minor_code;		/* not defined in the core */
} XNoExposeEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int state;		/* Visibility state */
} XVisibilityEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;		/* parent of the window */
	Window window;		/* window id of window created */
	int x, y;		/* window location */
	int width, height;	/* size of window */
	int border_width;	/* border width */
	Bool override_redirect;	/* creation should be overridden */
} XCreateWindowEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
} XDestroyWindowEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	Bool from_configure;
} XUnmapEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	Bool override_redirect;	/* boolean, is override set... */
} XMapEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
} XMapRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	Window parent;
	int x, y;
	Bool override_redirect;
} XReparentEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int x, y;
	int width, height;
	int border_width;
	Window above;
	Bool override_redirect;
} XConfigureEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int x, y;
} XGravityEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int width, height;
} XResizeRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
	int x, y;
	int width, height;
	int border_width;
	Window above;
	int detail;		/* Above, Below, TopIf, BottomIf, Opposite */
	unsigned long value_mask;
} XConfigureRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
	int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom atom;
	Time time;
	int state;		/* NewValue, Deleted */
} XPropertyEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom selection;
	Time time;
} XSelectionClearEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window owner;
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;
	Time time;
} XSelectionRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;		/* ATOM or None */
	Time time;
} XSelectionEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Colormap colormap;	/* COLORMAP or None */
#if defined(__cplusplus) || defined(c_plusplus)
	Bool c_new;		/* C++ */
#else
	Bool new;
#endif
	int state;		/* ColormapInstalled, ColormapUninstalled */
} XColormapEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom message_type;
	int format;
	union {
		char b[20];
		short s[10];
		long l[5];
		} data;
} XClientMessageEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;		/* unused */
	int request;		/* one of MappingModifier, MappingKeyboard,
				   MappingPointer */
	int first_keycode;	/* first keycode */
	int count;		/* defines range of change w. first_keycode*/
} XMappingEvent;

typedef struct {
	int type;
	Display *display;	/* Display the event was read from */
	XID resourceid;		/* resource id */
	unsigned long serial;	/* serial number of failed request */
	unsigned char error_code;	/* error code of failed request */
	unsigned char request_code;	/* Major op-code of failed request */
	unsigned char minor_code;	/* Minor op-code of failed request */
} XErrorEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;/* Display the event was read from */
	Window window;	/* window on which event was requested in event mask */
} XAnyEvent;


/***************************************************************
 *
 * GenericEvent.  This event is the standard event for all newer extensions.
 */

typedef struct
    {
    int            type;         /* of event. Always GenericEvent */
    unsigned long  serial;       /* # of last request processed */
    Bool           send_event;   /* true if from SendEvent request */
    Display        *display;     /* Display the event was read from */
    int            extension;    /* major opcode of extension that caused the event */
    int            evtype;       /* actual event type. */
    } XGenericEvent;

typedef struct {
    int            type;         /* of event. Always GenericEvent */
    unsigned long  serial;       /* # of last request processed */
    Bool           send_event;   /* true if from SendEvent request */
    Display        *display;     /* Display the event was read from */
    int            extension;    /* major opcode of extension that caused the event */
    int            evtype;       /* actual event type. */
    unsigned int   cookie;
    void           *data;
} XGenericEventCookie;

/*
 * this union is defined so Xlib can always use the same sized
 * event structure internally, to avoid memory fragmentation.
 */
typedef union _XEvent {
        int type;		/* must not be changed; first element */
	XAnyEvent xany;
	XKeyEvent xkey;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XCrossingEvent xcrossing;
	XFocusChangeEvent xfocus;
	XExposeEvent xexpose;
	XGraphicsExposeEvent xgraphicsexpose;
	XNoExposeEvent xnoexpose;
	XVisibilityEvent xvisibility;
	XCreateWindowEvent xcreatewindow;
	XDestroyWindowEvent xdestroywindow;
	XUnmapEvent xunmap;
	XMapEvent xmap;
	XMapRequestEvent xmaprequest;
	XReparentEvent xreparent;
	XConfigureEvent xconfigure;
	XGravityEvent xgravity;
	XResizeRequestEvent xresizerequest;
	XConfigureRequestEvent xconfigurerequest;
	XCirculateEvent xcirculate;
	XCirculateRequestEvent xcirculaterequest;
	XPropertyEvent xproperty;
	XSelectionClearEvent xselectionclear;
	XSelectionRequestEvent xselectionrequest;
	XSelectionEvent xselection;
	XColormapEvent xcolormap;
	XClientMessageEvent xclient;
	XMappingEvent xmapping;
	XErrorEvent xerror;
	XKeymapEvent xkeymap;
	XGenericEvent xgeneric;
	XGenericEventCookie xcookie;
	long pad[24];
} XEvent;
#endif


typedef struct _XIM *XIM;
typedef struct _XIC *XIC;
typedef void (*XIMProc)( XIM, XPointer, XPointer);
typedef void (*XIDProc)( Display*, XPointer, XPointer);
typedef unsigned long XIMStyle;
typedef struct { unsigned short count_styles; XIMStyle *supported_styles; } XIMStyles;
#define XIMPreeditNothing	0x0008L
#define XIMStatusNothing	0x0400L
#define XNQueryInputStyle "queryInputStyle"
#define XNClientWindow "clientWindow"
#define XNInputStyle "inputStyle"
#define XNFocusWindow "focusWindow"
#define XNDestroyCallback "destroyCallback"
#define XNFilterEvents "filterEvents"
typedef struct { XPointer client_data; XIMProc callback; } XIMCallback;
typedef int (*XErrorHandler) (Display*, XErrorEvent*);
extern XErrorHandler XSetErrorHandler(XErrorHandler);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#define XK_BackSpace                     0xff08  /* Back space, back char */
#define XK_Tab                           0xff09
#define XK_Return                        0xff0d  /* Return, enter */
#define XK_Pause                         0xff13  /* Pause, hold */
#define XK_Scroll_Lock                   0xff14
#define XK_Escape                        0xff1b
#define XK_Delete                        0xffff  /* Delete, rubout */
#define XK_Home                          0xff50
#define XK_Left                          0xff51  /* Move left, left arrow */
#define XK_Up                            0xff52  /* Move up, up arrow */
#define XK_Right                         0xff53  /* Move right, right arrow */
#define XK_Down                          0xff54  /* Move down, down arrow */
#define XK_Page_Up                       0xff55
#define XK_Page_Down                     0xff56
#define XK_End                           0xff57  /* EOL */
#define XK_Print                         0xff61
#define XK_Insert                        0xff63  /* Insert, insert here */
#define XK_Menu                          0xff67
#define XK_Mode_switch                   0xff7e  /* Character set switch */
#define XK_Num_Lock                      0xff7f
#define XK_KP_Space                      0xff80  /* Space */
#define XK_KP_Tab                        0xff89
#define XK_KP_Enter                      0xff8d  /* Enter */
#define XK_KP_F1                         0xff91  /* PF1, KP_A, ... */
#define XK_KP_F2                         0xff92
#define XK_KP_F3                         0xff93
#define XK_KP_F4                         0xff94
#define XK_KP_Home                       0xff95
#define XK_KP_Left                       0xff96
#define XK_KP_Up                         0xff97
#define XK_KP_Right                      0xff98
#define XK_KP_Down                       0xff99
#define XK_KP_Prior                      0xff9a
#define XK_KP_Page_Up                    0xff9a
#define XK_KP_Next                       0xff9b
#define XK_KP_Page_Down                  0xff9b
#define XK_KP_End                        0xff9c
#define XK_KP_Begin                      0xff9d
#define XK_KP_Insert                     0xff9e
#define XK_KP_Delete                     0xff9f
#define XK_KP_Equal                      0xffbd  /* Equals */
#define XK_KP_Multiply                   0xffaa
#define XK_KP_Add                        0xffab
#define XK_KP_Separator                  0xffac  /* Separator, often comma */
#define XK_KP_Subtract                   0xffad
#define XK_KP_Decimal                    0xffae
#define XK_KP_Divide                     0xffaf
#define XK_KP_0                          0xffb0
#define XK_KP_1                          0xffb1
#define XK_KP_2                          0xffb2
#define XK_KP_3                          0xffb3
#define XK_KP_4                          0xffb4
#define XK_KP_5                          0xffb5
#define XK_KP_6                          0xffb6
#define XK_KP_7                          0xffb7
#define XK_KP_8                          0xffb8
#define XK_KP_9                          0xffb9
#define XK_F1                            0xffbe
#define XK_F2                            0xffbf
#define XK_F3                            0xffc0
#define XK_F4                            0xffc1
#define XK_F5                            0xffc2
#define XK_F6                            0xffc3
#define XK_F7                            0xffc4
#define XK_F8                            0xffc5
#define XK_F9                            0xffc6
#define XK_F10                           0xffc7
#define XK_F11                           0xffc8
#define XK_F12                           0xffc9
#define XK_F13                           0xffca
#define XK_F14                           0xffcb
#define XK_F15                           0xffcc
#define XK_F16                           0xffcd
#define XK_F17                           0xffce
#define XK_F18                           0xffcf
#define XK_F19                           0xffd0
#define XK_F20                           0xffd1
#define XK_F21                           0xffd2
#define XK_F22                           0xffd3
#define XK_F23                           0xffd4
#define XK_F24                           0xffd5
#define XK_F25                           0xffd6
#define XK_Shift_L                       0xffe1  /* Left shift */
#define XK_Shift_R                       0xffe2  /* Right shift */
#define XK_Control_L                     0xffe3  /* Left control */
#define XK_Control_R                     0xffe4  /* Right control */
#define XK_Caps_Lock                     0xffe5  /* Caps lock */
#define XK_Shift_Lock                    0xffe6  /* Shift lock */
#define XK_Meta_L                        0xffe7  /* Left meta */
#define XK_Meta_R                        0xffe8  /* Right meta */
#define XK_Alt_L                         0xffe9  /* Left alt */
#define XK_Alt_R                         0xffea  /* Right alt */
#define XK_Super_L                       0xffeb  /* Left super */
#define XK_Super_R                       0xffec  /* Right super */
#define XK_ISO_Level3_Shift              0xfe03
#define XK_space                         0x0020  /* U+0020 SPACE */
#define XK_apostrophe                    0x0027  /* U+0027 APOSTROPHE */
#define XK_comma                         0x002c  /* U+002C COMMA */
#define XK_minus                         0x002d  /* U+002D HYPHEN-MINUS */
#define XK_period                        0x002e  /* U+002E FULL STOP */
#define XK_slash                         0x002f  /* U+002F SOLIDUS */
#define XK_0                             0x0030  /* U+0030 DIGIT ZERO */
#define XK_1                             0x0031  /* U+0031 DIGIT ONE */
#define XK_2                             0x0032  /* U+0032 DIGIT TWO */
#define XK_3                             0x0033  /* U+0033 DIGIT THREE */
#define XK_4                             0x0034  /* U+0034 DIGIT FOUR */
#define XK_5                             0x0035  /* U+0035 DIGIT FIVE */
#define XK_6                             0x0036  /* U+0036 DIGIT SIX */
#define XK_7                             0x0037  /* U+0037 DIGIT SEVEN */
#define XK_8                             0x0038  /* U+0038 DIGIT EIGHT */
#define XK_9                             0x0039  /* U+0039 DIGIT NINE */
#define XK_semicolon                     0x003b  /* U+003B SEMICOLON */
#define XK_less                          0x003c  /* U+003C LESS-THAN SIGN */
#define XK_equal                         0x003d  /* U+003D EQUALS SIGN */
#define XK_A                             0x0041  /* U+0041 LATIN CAPITAL LETTER A */
#define XK_B                             0x0042  /* U+0042 LATIN CAPITAL LETTER B */
#define XK_C                             0x0043  /* U+0043 LATIN CAPITAL LETTER C */
#define XK_D                             0x0044  /* U+0044 LATIN CAPITAL LETTER D */
#define XK_E                             0x0045  /* U+0045 LATIN CAPITAL LETTER E */
#define XK_F                             0x0046  /* U+0046 LATIN CAPITAL LETTER F */
#define XK_G                             0x0047  /* U+0047 LATIN CAPITAL LETTER G */
#define XK_H                             0x0048  /* U+0048 LATIN CAPITAL LETTER H */
#define XK_I                             0x0049  /* U+0049 LATIN CAPITAL LETTER I */
#define XK_J                             0x004a  /* U+004A LATIN CAPITAL LETTER J */
#define XK_K                             0x004b  /* U+004B LATIN CAPITAL LETTER K */
#define XK_L                             0x004c  /* U+004C LATIN CAPITAL LETTER L */
#define XK_M                             0x004d  /* U+004D LATIN CAPITAL LETTER M */
#define XK_N                             0x004e  /* U+004E LATIN CAPITAL LETTER N */
#define XK_O                             0x004f  /* U+004F LATIN CAPITAL LETTER O */
#define XK_P                             0x0050  /* U+0050 LATIN CAPITAL LETTER P */
#define XK_Q                             0x0051  /* U+0051 LATIN CAPITAL LETTER Q */
#define XK_R                             0x0052  /* U+0052 LATIN CAPITAL LETTER R */
#define XK_S                             0x0053  /* U+0053 LATIN CAPITAL LETTER S */
#define XK_T                             0x0054  /* U+0054 LATIN CAPITAL LETTER T */
#define XK_U                             0x0055  /* U+0055 LATIN CAPITAL LETTER U */
#define XK_V                             0x0056  /* U+0056 LATIN CAPITAL LETTER V */
#define XK_W                             0x0057  /* U+0057 LATIN CAPITAL LETTER W */
#define XK_X                             0x0058  /* U+0058 LATIN CAPITAL LETTER X */
#define XK_Y                             0x0059  /* U+0059 LATIN CAPITAL LETTER Y */
#define XK_Z                             0x005a  /* U+005A LATIN CAPITAL LETTER Z */
#define XK_bracketleft                   0x005b  /* U+005B LEFT SQUARE BRACKET */
#define XK_backslash                     0x005c  /* U+005C REVERSE SOLIDUS */
#define XK_bracketright                  0x005d  /* U+005D RIGHT SQUARE BRACKET */
#define XK_grave                         0x0060  /* U+0060 GRAVE ACCENT */
#define XK_a                             0x0061  /* U+0061 LATIN SMALL LETTER A */
#define XK_b                             0x0062  /* U+0062 LATIN SMALL LETTER B */
#define XK_c                             0x0063  /* U+0063 LATIN SMALL LETTER C */
#define XK_d                             0x0064  /* U+0064 LATIN SMALL LETTER D */
#define XK_e                             0x0065  /* U+0065 LATIN SMALL LETTER E */
#define XK_f                             0x0066  /* U+0066 LATIN SMALL LETTER F */
#define XK_g                             0x0067  /* U+0067 LATIN SMALL LETTER G */
#define XK_h                             0x0068  /* U+0068 LATIN SMALL LETTER H */
#define XK_i                             0x0069  /* U+0069 LATIN SMALL LETTER I */
#define XK_j                             0x006a  /* U+006A LATIN SMALL LETTER J */
#define XK_k                             0x006b  /* U+006B LATIN SMALL LETTER K */
#define XK_l                             0x006c  /* U+006C LATIN SMALL LETTER L */
#define XK_m                             0x006d  /* U+006D LATIN SMALL LETTER M */
#define XK_n                             0x006e  /* U+006E LATIN SMALL LETTER N */
#define XK_o                             0x006f  /* U+006F LATIN SMALL LETTER O */
#define XK_p                             0x0070  /* U+0070 LATIN SMALL LETTER P */
#define XK_q                             0x0071  /* U+0071 LATIN SMALL LETTER Q */
#define XK_r                             0x0072  /* U+0072 LATIN SMALL LETTER R */
#define XK_s                             0x0073  /* U+0073 LATIN SMALL LETTER S */
#define XK_t                             0x0074  /* U+0074 LATIN SMALL LETTER T */
#define XK_u                             0x0075  /* U+0075 LATIN SMALL LETTER U */
#define XK_v                             0x0076  /* U+0076 LATIN SMALL LETTER V */
#define XK_w                             0x0077  /* U+0077 LATIN SMALL LETTER W */
#define XK_x                             0x0078  /* U+0078 LATIN SMALL LETTER X */
#define XK_y                             0x0079  /* U+0079 LATIN SMALL LETTER Y */
#define XK_z                             0x007a  /* U+007A LATIN SMALL LETTER Z */
#define PPosition	(1L << 2) /* program specified position */
#define PMinSize	(1L << 4) /* program specified minimum size */
#define PMaxSize	(1L << 5) /* program specified maximum size */
#define PAspect		(1L << 7) /* program specified min and max aspect ratios */
#define PWinGravity	(1L << 9) /* program specified window gravity */
typedef struct { long flags; Bool input; int initial_state; Pixmap icon_pixmap; Window icon_window;	int icon_x, icon_y;	Pixmap icon_mask; XID window_group; } XWMHints;
#define StateHint 		(1L << 1)
#define WithdrawnState 0	/* for windows that are not mapped */
#define NormalState 1	/* most applications want to start this way */
#define IconicState 3	/* application wants to start as an icon */
typedef struct { char *res_name; char *res_class; } XClassHint;
typedef struct _XComposeStatus { XPointer compose_ptr; int chars_matched; } XComposeStatus;
typedef struct _XRegion *Region;
typedef struct { Visual *visual; VisualID visualid; int screen; int depth; int class; unsigned long red_mask; unsigned long green_mask; unsigned long blue_mask; int colormap_size; int bits_per_rgb; } XVisualInfo;
typedef int XContext;
typedef XID PictFormat;
typedef struct { short red,redMask,green,greenMask,blue,blueMask,alpha,alphaMask; } XRenderDirectFormat;
typedef struct { PictFormat id; int type; int depth; XRenderDirectFormat direct; Colormap colormap; } XRenderPictFormat;
typedef XID RROutput;
typedef XID RRCrtc;
typedef XID RRMode;
typedef unsigned long XRRModeFlags;
typedef struct _XRRModeInfo { RRMode id; unsigned int width; unsigned int height; unsigned long dotClock; unsigned int hSyncStart; unsigned int hSyncEnd; unsigned int hTotal; unsigned int hSkew; unsigned int vSyncStart; unsigned int vSyncEnd; unsigned int vTotal; char *name; unsigned int nameLength; XRRModeFlags modeFlags; } XRRModeInfo;
typedef struct _XRRScreenResources { Time timestamp; Time configTimestamp; int ncrtc; RRCrtc *crtcs; int noutput; RROutput *outputs; int nmode; XRRModeInfo *modes; } XRRScreenResources;
typedef struct _XRROutputInfo { Time timestamp; RRCrtc crtc; char *name; int nameLen; unsigned long mm_width; unsigned long mm_height; Connection connection; SubpixelOrder subpixel_order; int ncrtc; RRCrtc *crtcs; int nclone; RROutput *clones; int nmode; int npreferred; RRMode *modes; } XRROutputInfo;
typedef struct _XRRCrtcInfo { Time timestamp; int x, y; unsigned int width, height; RRMode mode; Rotation rotation; int noutput; RROutput *outputs; Rotation rotations; int npossible; RROutput *possible; } XRRCrtcInfo;

#if __has_attribute(nonstring)
    # define _X_NONSTRING __attribute__((nonstring))
#else
    # define _X_NONSTRING
#endif
#define	XkbEventCode			0
#define	XkbStateNotify			2
#define	XkbGroupStateMask		(1L << 4)
#define	XkbUseCoreKbd		0x0100
#define	XkbNumKbdGroups		4
#define	XkbMaxLegalKeyCode	255
#define	XkbPerKeyBitArraySize	((XkbMaxLegalKeyCode+1)/8)
#define	XkbNumVirtualMods	16
#define	XkbNumIndicators	32
#define	XkbKeyNameLength	4
#define	XkbKeyNamesMask		(1<<9)
#define	XkbKeyAliasesMask	(1<<10)
#define	XkbCharToInt(v)		((v)&0x80?(int)((v)|(~0xff)):(int)((v)&0x7f))
#define	XkbIntTo2Chars(i,h,l)	(((h)=((i>>8)&0xff)),((l)=((i)&0xff)))
#define	Xkb2CharsToInt(h,l)	((short)(((h)<<8)|(l)))
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif
typedef struct _XkbStateRec {
	unsigned char	group;
	unsigned char   locked_group;
	unsigned short	base_group;
	unsigned short	latched_group;
	unsigned char	mods;
	unsigned char	base_mods;
	unsigned char	latched_mods;
	unsigned char	locked_mods;
	unsigned char	compat_state;
	unsigned char	grab_mods;
	unsigned char	compat_grab_mods;
	unsigned char	lookup_mods;
	unsigned char	compat_lookup_mods;
	unsigned short	ptr_buttons;
} XkbStateRec,*XkbStatePtr;
#define	XkbModLocks(s)	 ((s)->locked_mods)
#define	XkbStateMods(s)	 ((s)->base_mods|(s)->latched_mods|XkbModLocks(s))
#define	XkbGroupLock(s)	 ((s)->locked_group)
#define	XkbStateGroup(s) ((s)->base_group+(s)->latched_group+XkbGroupLock(s))
#define	XkbStateFieldFromRec(s)	XkbBuildCoreState((s)->lookup_mods,(s)->group)
#define	XkbGrabStateFromRec(s)	XkbBuildCoreState((s)->grab_mods,(s)->group)

typedef struct _XkbMods {
	unsigned char	mask;	/* effective mods */
	unsigned char	real_mods;
	unsigned short	vmods;
} XkbModsRec,*XkbModsPtr;

typedef struct _XkbKTMapEntry {
	Bool		active;
	unsigned char	level;
	XkbModsRec	mods;
} XkbKTMapEntryRec,*XkbKTMapEntryPtr;

typedef struct _XkbKeyType {
	XkbModsRec		mods;
	unsigned char	  	num_levels;
	unsigned char	  	map_count;
	/* map is an array of map_count XkbKTMapEntryRec structs */
	XkbKTMapEntryPtr  	map;
	/* preserve is an array of map_count XkbModsRec structs */
	XkbModsPtr  		preserve;
	Atom		  	name;
	/* level_names is an array of num_levels Atoms */
	Atom *			level_names;
} XkbKeyTypeRec, *XkbKeyTypePtr;

#define	XkbNumGroups(g)			((g)&0x0f)
#define	XkbOutOfRangeGroupInfo(g)	((g)&0xf0)
#define	XkbOutOfRangeGroupAction(g)	((g)&0xc0)
#define	XkbOutOfRangeGroupNumber(g)	(((g)&0x30)>>4)
#define	XkbSetGroupInfo(g,w,n)	(((w)&0xc0)|(((n)&3)<<4)|((g)&0x0f))
#define	XkbSetNumGroups(g,n)	(((g)&0xf0)|((n)&0x0f))

	/*
	 * Structures and access macros used primarily by the server
	 */

typedef struct _XkbBehavior {
	unsigned char	type;
	unsigned char	data;
} XkbBehavior;

#define	XkbAnyActionDataSize 7
typedef	struct _XkbAnyAction {
	unsigned char	type;
	unsigned char	data[XkbAnyActionDataSize];
} XkbAnyAction;

typedef struct _XkbModAction {
	unsigned char	type;
	unsigned char	flags;
	unsigned char	mask;
	unsigned char	real_mods;
	unsigned char	vmods1;
	unsigned char	vmods2;
} XkbModAction;
#define	XkbModActionVMods(a)      \
	((short)(((a)->vmods1<<8)|((a)->vmods2)))
#define	XkbSetModActionVMods(a,v) \
	(((a)->vmods1=(((v)>>8)&0xff)),(a)->vmods2=((v)&0xff))

typedef struct _XkbGroupAction {
	unsigned char	type;
	unsigned char	flags;
	char		group_XXX;
} XkbGroupAction;
#define	XkbSAGroup(a)		(XkbCharToInt((a)->group_XXX))
#define	XkbSASetGroup(a,g)	((a)->group_XXX=(g))

typedef struct _XkbISOAction {
	unsigned char	type;
	unsigned char	flags;
	unsigned char	mask;
	unsigned char	real_mods;
	char		group_XXX;
	unsigned char	affect;
	unsigned char	vmods1;
	unsigned char	vmods2;
} XkbISOAction;

typedef struct _XkbPtrAction {
	unsigned char	type;
	unsigned char	flags;
	unsigned char	high_XXX;
	unsigned char	low_XXX;
	unsigned char	high_YYY;
	unsigned char	low_YYY;
} XkbPtrAction;
#define	XkbPtrActionX(a)      (Xkb2CharsToInt((a)->high_XXX,(a)->low_XXX))
#define	XkbPtrActionY(a)      (Xkb2CharsToInt((a)->high_YYY,(a)->low_YYY))
#define	XkbSetPtrActionX(a,x) (XkbIntTo2Chars(x,(a)->high_XXX,(a)->low_XXX))
#define	XkbSetPtrActionY(a,y) (XkbIntTo2Chars(y,(a)->high_YYY,(a)->low_YYY))

typedef struct _XkbPtrBtnAction {
	unsigned char	type;
	unsigned char	flags;
	unsigned char	count;
	unsigned char	button;
} XkbPtrBtnAction;

typedef struct _XkbPtrDfltAction {
	unsigned char	type;
	unsigned char	flags;
	unsigned char	affect;
	char		valueXXX;
} XkbPtrDfltAction;
#define	XkbSAPtrDfltValue(a)		(XkbCharToInt((a)->valueXXX))
#define	XkbSASetPtrDfltValue(a,c)	((a)->valueXXX= ((c)&0xff))

typedef struct _XkbSwitchScreenAction {
	unsigned char	type;
	unsigned char	flags;
	char		screenXXX;
} XkbSwitchScreenAction;
#define	XkbSAScreen(a)			(XkbCharToInt((a)->screenXXX))
#define	XkbSASetScreen(a,s)		((a)->screenXXX= ((s)&0xff))

typedef struct _XkbCtrlsAction {
	unsigned char	type;
	unsigned char	flags;
	unsigned char	ctrls3;
	unsigned char	ctrls2;
	unsigned char	ctrls1;
	unsigned char	ctrls0;
} XkbCtrlsAction;
#define	XkbActionSetCtrls(a,c)	(((a)->ctrls3=(((c)>>24)&0xff)),\
					((a)->ctrls2=(((c)>>16)&0xff)),\
					((a)->ctrls1=(((c)>>8)&0xff)),\
					((a)->ctrls0=((c)&0xff)))
#define	XkbActionCtrls(a) ((((unsigned int)(a)->ctrls3)<<24)|\
			   (((unsigned int)(a)->ctrls2)<<16)|\
			   (((unsigned int)(a)->ctrls1)<<8)|\
			   ((unsigned int)((a)->ctrls0)))

typedef struct _XkbMessageAction {
	unsigned char	type;
	unsigned char	flags;
	unsigned char	message[6];
} XkbMessageAction;

typedef struct	_XkbRedirectKeyAction {
	unsigned char	type;
	unsigned char	new_key;
	unsigned char	mods_mask;
	unsigned char	mods;
	unsigned char	vmods_mask0;
	unsigned char	vmods_mask1;
	unsigned char	vmods0;
	unsigned char	vmods1;
} XkbRedirectKeyAction;

#define	XkbSARedirectVMods(a)		((((unsigned int)(a)->vmods1)<<8)|\
					((unsigned int)(a)->vmods0))
#define	XkbSARedirectSetVMods(a,m)	(((a)->vmods1=(((m)>>8)&0xff)),\
					 ((a)->vmods0=((m)&0xff)))
#define	XkbSARedirectVModsMask(a)	((((unsigned int)(a)->vmods_mask1)<<8)|\
					((unsigned int)(a)->vmods_mask0))
#define	XkbSARedirectSetVModsMask(a,m)	(((a)->vmods_mask1=(((m)>>8)&0xff)),\
					 ((a)->vmods_mask0=((m)&0xff)))

typedef struct _XkbDeviceBtnAction {
	unsigned char	type;
	unsigned char	flags;
	unsigned char	count;
	unsigned char	button;
	unsigned char	device;
} XkbDeviceBtnAction;

typedef struct _XkbDeviceValuatorAction {
	unsigned char	type;
	unsigned char	device;
	unsigned char	v1_what;
	unsigned char	v1_ndx;
	unsigned char	v1_value;
	unsigned char	v2_what;
	unsigned char	v2_ndx;
	unsigned char	v2_value;
} XkbDeviceValuatorAction;

typedef	union _XkbAction {
	XkbAnyAction		any;
	XkbModAction		mods;
	XkbGroupAction		group;
	XkbISOAction		iso;
	XkbPtrAction		ptr;
	XkbPtrBtnAction		btn;
	XkbPtrDfltAction	dflt;
	XkbSwitchScreenAction	screen;
	XkbCtrlsAction		ctrls;
	XkbMessageAction	msg;
	XkbRedirectKeyAction	redirect;
	XkbDeviceBtnAction	devbtn;
	XkbDeviceValuatorAction	devval;
	unsigned char 		type;
} XkbAction;

typedef	struct _XkbControls {
	unsigned char	mk_dflt_btn;
	unsigned char	num_groups;
	unsigned char	groups_wrap;
	XkbModsRec	internal;
	XkbModsRec	ignore_lock;
	unsigned int	enabled_ctrls;
	unsigned short	repeat_delay;
	unsigned short	repeat_interval;
	unsigned short	slow_keys_delay;
	unsigned short	debounce_delay;
	unsigned short	mk_delay;
	unsigned short	mk_interval;
	unsigned short	mk_time_to_max;
	unsigned short	mk_max_speed;
		 short	mk_curve;
	unsigned short	ax_options;
	unsigned short	ax_timeout;
	unsigned short	axt_opts_mask;
	unsigned short	axt_opts_values;
	unsigned int	axt_ctrls_mask;
	unsigned int	axt_ctrls_values;
	unsigned char	per_key_repeat[XkbPerKeyBitArraySize];
} XkbControlsRec, *XkbControlsPtr;

#define	XkbAX_AnyFeedback(c)	((c)->enabled_ctrls&XkbAccessXFeedbackMask)
#define	XkbAX_NeedOption(c,w)	((c)->ax_options&(w))
#define	XkbAX_NeedFeedback(c,w)	(XkbAX_AnyFeedback(c)&&XkbAX_NeedOption(c,w))

typedef struct _XkbServerMapRec {
	/* acts is an array of XkbActions structs, with size_acts entries
	   allocated, and num_acts entries used. */
	unsigned short		 num_acts;
	unsigned short		 size_acts;
	XkbAction		*acts;

	/* behaviors, key_acts, explicit, & vmodmap are all arrays with
	   (xkb->max_key_code + 1) entries allocated for each. */
	XkbBehavior		*behaviors;
	unsigned short		*key_acts;
#if defined(__cplusplus) || defined(c_plusplus)
	/* explicit is a C++ reserved word */
	unsigned char		*c_explicit;
#else
	unsigned char		*explicit;
#endif
	unsigned char		 vmods[XkbNumVirtualMods];
	unsigned short		*vmodmap;
} XkbServerMapRec, *XkbServerMapPtr;

#define	XkbSMKeyActionsPtr(m,k) (&(m)->acts[(m)->key_acts[k]])

	/*
	 * Structures and access macros used primarily by clients
	 */

typedef	struct _XkbSymMapRec {
	unsigned char	 kt_index[XkbNumKbdGroups];
	unsigned char	 group_info;
	unsigned char	 width;
	unsigned short	 offset;
} XkbSymMapRec, *XkbSymMapPtr;

typedef struct _XkbClientMapRec {
	/* types is an array of XkbKeyTypeRec structs, with size_types entries
	   allocated, and num_types entries used. */
	unsigned char		 size_types;
	unsigned char		 num_types;
	XkbKeyTypePtr		 types;

	/* syms is an array of size_syms KeySyms, in which num_syms are used */
	unsigned short		 size_syms;
	unsigned short		 num_syms;
	KeySym			*syms;
	/* key_sym_map is an array of (max_key_code + 1) XkbSymMapRec structs */
	XkbSymMapPtr		 key_sym_map;

	/* modmap is an array of (max_key_code + 1) unsigned chars */
	unsigned char		*modmap;
} XkbClientMapRec, *XkbClientMapPtr;

#define	XkbCMKeyGroupInfo(m,k)  ((m)->key_sym_map[k].group_info)
#define	XkbCMKeyNumGroups(m,k)	 (XkbNumGroups((m)->key_sym_map[k].group_info))
#define	XkbCMKeyGroupWidth(m,k,g) (XkbCMKeyType(m,k,g)->num_levels)
#define	XkbCMKeyGroupsWidth(m,k) ((m)->key_sym_map[k].width)
#define	XkbCMKeyTypeIndex(m,k,g) ((m)->key_sym_map[k].kt_index[g&0x3])
#define	XkbCMKeyType(m,k,g)	 (&(m)->types[XkbCMKeyTypeIndex(m,k,g)])
#define	XkbCMKeyNumSyms(m,k) (XkbCMKeyGroupsWidth(m,k)*XkbCMKeyNumGroups(m,k))
#define	XkbCMKeySymsOffset(m,k)	((m)->key_sym_map[k].offset)
#define	XkbCMKeySymsPtr(m,k)	(&(m)->syms[XkbCMKeySymsOffset(m,k)])

	/*
	 * Compatibility structures and access macros
	 */

typedef struct _XkbSymInterpretRec {
	KeySym		sym;
	unsigned char	flags;
	unsigned char	match;
	unsigned char	mods;
	unsigned char	virtual_mod;
	XkbAnyAction	act;
} XkbSymInterpretRec,*XkbSymInterpretPtr;

typedef struct _XkbCompatMapRec {
	/* sym_interpret is an array of XkbSymInterpretRec structs,
	   in which size_si are allocated & num_si are used. */
	XkbSymInterpretPtr	 sym_interpret;
	XkbModsRec		 groups[XkbNumKbdGroups];
	unsigned short		 num_si;
	unsigned short		 size_si;
} XkbCompatMapRec, *XkbCompatMapPtr;

typedef struct _XkbIndicatorMapRec {
	unsigned char	flags;
	unsigned char	which_groups;
	unsigned char	groups;
	unsigned char	which_mods;
	XkbModsRec	mods;
	unsigned int	ctrls;
} XkbIndicatorMapRec, *XkbIndicatorMapPtr;

#define	XkbIM_IsAuto(i)	((((i)->flags&XkbIM_NoAutomatic)==0)&&\
			    (((i)->which_groups&&(i)->groups)||\
			     ((i)->which_mods&&(i)->mods.mask)||\
			     ((i)->ctrls)))
#define	XkbIM_InUse(i)	(((i)->flags)||((i)->which_groups)||\
					((i)->which_mods)||((i)->ctrls))


typedef struct _XkbIndicatorRec {
	unsigned long	  	phys_indicators;
	XkbIndicatorMapRec	maps[XkbNumIndicators];
} XkbIndicatorRec,*XkbIndicatorPtr;

typedef	struct _XkbKeyNameRec {
	char	name[XkbKeyNameLength]	_X_NONSTRING;
} XkbKeyNameRec,*XkbKeyNamePtr;

typedef struct _XkbKeyAliasRec {
	char	real[XkbKeyNameLength]	_X_NONSTRING;
	char	alias[XkbKeyNameLength]	_X_NONSTRING;
} XkbKeyAliasRec,*XkbKeyAliasPtr;

	/*
	 * Names for everything
	 */
typedef struct _XkbNamesRec {
	Atom		  keycodes;
	Atom		  geometry;
	Atom		  symbols;
	Atom              types;
	Atom		  compat;
	Atom		  vmods[XkbNumVirtualMods];
	Atom		  indicators[XkbNumIndicators];
	Atom		  groups[XkbNumKbdGroups];
	/* keys is an array of (xkb->max_key_code + 1) XkbKeyNameRec entries */
	XkbKeyNamePtr	  keys;
	/* key_aliases is an array of num_key_aliases XkbKeyAliasRec entries */
	XkbKeyAliasPtr	  key_aliases;
	/* radio_groups is an array of num_rg Atoms */
	Atom		 *radio_groups;
	Atom		  phys_symbols;

	/* num_keys seems to be unused in libX11 */
	unsigned char	  num_keys;
	unsigned char	  num_key_aliases;
	unsigned short	  num_rg;
} XkbNamesRec,*XkbNamesPtr;

typedef	struct _XkbGeometry	*XkbGeometryPtr;
	/*
	 * Tie it all together into one big keyboard description
	 */
typedef	struct _XkbDesc {
	struct _XDisplay *	dpy;
	unsigned short	 	flags;
	unsigned short		device_spec;
	KeyCode			min_key_code;
	KeyCode			max_key_code;

	XkbControlsPtr		ctrls;
	XkbServerMapPtr		server;
	XkbClientMapPtr		map;
	XkbIndicatorPtr		indicators;
	XkbNamesPtr		names;
	XkbCompatMapPtr		compat;
	XkbGeometryPtr		geom;
} XkbDescRec, *XkbDescPtr;
#define	XkbKeyKeyTypeIndex(d,k,g)	(XkbCMKeyTypeIndex((d)->map,k,g))
#define	XkbKeyKeyType(d,k,g)		(XkbCMKeyType((d)->map,k,g))
#define	XkbKeyGroupWidth(d,k,g)		(XkbCMKeyGroupWidth((d)->map,k,g))
#define	XkbKeyGroupsWidth(d,k)		(XkbCMKeyGroupsWidth((d)->map,k))
#define	XkbKeyGroupInfo(d,k)		(XkbCMKeyGroupInfo((d)->map,(k)))
#define	XkbKeyNumGroups(d,k)		(XkbCMKeyNumGroups((d)->map,(k)))
#define	XkbKeyNumSyms(d,k)		(XkbCMKeyNumSyms((d)->map,(k)))
#define	XkbKeySymsPtr(d,k)		(XkbCMKeySymsPtr((d)->map,(k)))
#define	XkbKeySym(d,k,n)		(XkbKeySymsPtr(d,k)[n])
#define	XkbKeySymEntry(d,k,sl,g) \
	(XkbKeySym(d,k,((XkbKeyGroupsWidth(d,k)*(g))+(sl))))
#define	XkbKeyAction(d,k,n) \
	(XkbKeyHasActions(d,k)?&XkbKeyActionsPtr(d,k)[n]:NULL)
#define	XkbKeyActionEntry(d,k,sl,g) \
	(XkbKeyHasActions(d,k)?\
		XkbKeyAction(d,k,((XkbKeyGroupsWidth(d,k)*(g))+(sl))):NULL)

#define	XkbKeyHasActions(d,k)	((d)->server->key_acts[k]!=0)
#define	XkbKeyNumActions(d,k)	(XkbKeyHasActions(d,k)?XkbKeyNumSyms(d,k):1)
#define	XkbKeyActionsPtr(d,k)	(XkbSMKeyActionsPtr((d)->server,k))
#define	XkbKeycodeInRange(d,k)	(((k)>=(d)->min_key_code)&&\
				 ((k)<=(d)->max_key_code))
#define	XkbNumKeys(d)		((d)->max_key_code-(d)->min_key_code+1)


	/*
	 * The following structures can be used to track changes
	 * to a keyboard device
	 */
typedef struct _XkbMapChanges {
	unsigned short		 changed;
	KeyCode			 min_key_code;
	KeyCode			 max_key_code;
	unsigned char		 first_type;
	unsigned char		 num_types;
	KeyCode			 first_key_sym;
	unsigned char		 num_key_syms;
	KeyCode			 first_key_act;
	unsigned char		 num_key_acts;
	KeyCode			 first_key_behavior;
	unsigned char		 num_key_behaviors;
	KeyCode 		 first_key_explicit;
	unsigned char		 num_key_explicit;
	KeyCode			 first_modmap_key;
	unsigned char		 num_modmap_keys;
	KeyCode			 first_vmodmap_key;
	unsigned char		 num_vmodmap_keys;
	unsigned char		 pad;
	unsigned short		 vmods;
} XkbMapChangesRec,*XkbMapChangesPtr;

typedef struct _XkbControlsChanges {
	unsigned int 		 changed_ctrls;
	unsigned int		 enabled_ctrls_changes;
	Bool			 num_groups_changed;
} XkbControlsChangesRec,*XkbControlsChangesPtr;

typedef struct _XkbIndicatorChanges {
	unsigned int		 state_changes;
	unsigned int		 map_changes;
} XkbIndicatorChangesRec,*XkbIndicatorChangesPtr;

typedef struct _XkbNameChanges {
	unsigned int 		changed;
	unsigned char		first_type;
	unsigned char		num_types;
	unsigned char		first_lvl;
	unsigned char		num_lvls;
	unsigned char		num_aliases;
	unsigned char		num_rg;
	unsigned char		first_key;
	unsigned char		num_keys;
	unsigned short		changed_vmods;
	unsigned long		changed_indicators;
	unsigned char		changed_groups;
} XkbNameChangesRec,*XkbNameChangesPtr;

typedef struct _XkbCompatChanges {
	unsigned char		changed_groups;
	unsigned short		first_si;
	unsigned short		num_si;
} XkbCompatChangesRec,*XkbCompatChangesPtr;

typedef struct _XkbChanges {
	unsigned short		 device_spec;
	unsigned short		 state_changes;
	XkbMapChangesRec	 map;
	XkbControlsChangesRec	 ctrls;
	XkbIndicatorChangesRec	 indicators;
	XkbNameChangesRec	 names;
	XkbCompatChangesRec	 compat;
} XkbChangesRec, *XkbChangesPtr;

	/*
	 * These data structures are used to construct a keymap from
	 * a set of components or to list components in the server
	 * database.
	 */
typedef struct _XkbComponentNames {
	char *			 keymap;
	char *			 keycodes;
	char *			 types;
	char *			 compat;
	char *			 symbols;
	char *			 geometry;
} XkbComponentNamesRec, *XkbComponentNamesPtr;

typedef struct _XkbComponentName {
	unsigned short		flags;
	char *			name;
} XkbComponentNameRec,*XkbComponentNamePtr;

typedef struct _XkbComponentList {
	int			num_keymaps;
	int			num_keycodes;
	int			num_types;
	int			num_compat;
	int			num_symbols;
	int			num_geometry;
	XkbComponentNamePtr	keymaps;
	XkbComponentNamePtr 	keycodes;
	XkbComponentNamePtr	types;
	XkbComponentNamePtr	compat;
	XkbComponentNamePtr	symbols;
	XkbComponentNamePtr	geometry;
} XkbComponentListRec, *XkbComponentListPtr;

	/*
	 * The following data structures describe and track changes to a
	 * non-keyboard extension device
	 */
typedef struct _XkbDeviceLedInfo {
	unsigned short			led_class;
	unsigned short			led_id;
	unsigned int			phys_indicators;
	unsigned int			maps_present;
	unsigned int			names_present;
	unsigned int			state;
	Atom 				names[XkbNumIndicators];
	XkbIndicatorMapRec		maps[XkbNumIndicators];
} XkbDeviceLedInfoRec,*XkbDeviceLedInfoPtr;

typedef struct _XkbDeviceInfo {
	char *			name;
	Atom			type;
	unsigned short		device_spec;
	Bool			has_own_state;
	unsigned short		supported;
	unsigned short		unsupported;

	/* btn_acts is an array of num_btn XkbAction entries */
	unsigned short		num_btns;
	XkbAction *		btn_acts;

	unsigned short		sz_leds;
	unsigned short		num_leds;
	unsigned short		dflt_kbd_fb;
	unsigned short		dflt_led_fb;
	/* leds is an array of XkbDeviceLedInfoRec in which
	   sz_leds entries are allocated and num_leds entries are used */
	XkbDeviceLedInfoPtr	leds;
} XkbDeviceInfoRec,*XkbDeviceInfoPtr;

#define	XkbXI_DevHasBtnActs(d)	(((d)->num_btns>0)&&((d)->btn_acts!=NULL))
#define	XkbXI_LegalDevBtn(d,b)	(XkbXI_DevHasBtnActs(d)&&((b)<(d)->num_btns))
#define	XkbXI_DevHasLeds(d)	(((d)->num_leds>0)&&((d)->leds!=NULL))

typedef struct _XkbDeviceLedChanges {
	unsigned short		led_class;
	unsigned short		led_id;
	unsigned int		defined; /* names or maps changed */
	struct _XkbDeviceLedChanges *next;
} XkbDeviceLedChangesRec,*XkbDeviceLedChangesPtr;

typedef struct _XkbDeviceChanges {
	unsigned int		changed;
	unsigned short		first_btn;
	unsigned short		num_btns;
	XkbDeviceLedChangesRec 	leds;
} XkbDeviceChangesRec,*XkbDeviceChangesPtr;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

typedef struct _XkbAnyEvent { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; unsigned int device; } XkbAnyEvent;
typedef struct _XkbNewKeyboardNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; int old_device; int min_key_code; int max_key_code; int old_min_key_code; int old_max_key_code; unsigned int changed; char req_major; char req_minor; } XkbNewKeyboardNotifyEvent;
typedef struct _XkbMapNotifyEvent { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed; unsigned int flags; int first_type; int num_types; KeyCode min_key_code; KeyCode max_key_code; KeyCode first_key_sym; KeyCode first_key_act; KeyCode first_key_behavior; KeyCode first_key_explicit; KeyCode first_modmap_key; KeyCode first_vmodmap_key; int num_key_syms; int num_key_acts; int num_key_behaviors; int num_key_explicit; int num_modmap_keys; int num_vmodmap_keys; unsigned int vmods; } XkbMapNotifyEvent;
typedef struct _XkbStateNotifyEvent { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed; int group; int base_group; int latched_group; int locked_group; unsigned int mods; unsigned int base_mods; unsigned int latched_mods; unsigned int locked_mods; int compat_state; unsigned char grab_mods; unsigned char compat_grab_mods; unsigned char lookup_mods; unsigned char compat_lookup_mods; int ptr_buttons; KeyCode keycode; char event_type; char req_major; char req_minor; } XkbStateNotifyEvent;
typedef struct _XkbControlsNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed_ctrls; unsigned int enabled_ctrls; unsigned int enabled_ctrl_changes; int num_groups; KeyCode keycode; char event_type; char req_major; char req_minor; } XkbControlsNotifyEvent;
typedef struct _XkbIndicatorNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed; unsigned int state; } XkbIndicatorNotifyEvent;
typedef struct _XkbNamesNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed; int first_type; int num_types; int first_lvl; int num_lvls; int num_aliases; int num_radio_groups; unsigned int changed_vmods; unsigned int changed_groups; unsigned int changed_indicators; int first_key; int num_keys; } XkbNamesNotifyEvent;
typedef struct _XkbCompatMapNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int changed_groups; int first_si; int num_si; int num_total_si; } XkbCompatMapNotifyEvent;
typedef struct _XkbBellNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; int percent; int pitch; int duration; int bell_class; int bell_id; Atom name; Window window; Bool event_only; } XkbBellNotifyEvent;
typedef struct _XkbActionMessage { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; KeyCode keycode; Bool press; Bool key_event_follows; int group; unsigned int mods; char message[XkbActionMessageLength+1]; } XkbActionMessageEvent;
typedef struct _XkbAccessXNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; int detail; int keycode; int sk_delay; int debounce_delay; } XkbAccessXNotifyEvent;
typedef struct _XkbExtensionDeviceNotify { int type; unsigned long serial; Bool send_event; Display *display; Time time; int xkb_type; int device; unsigned int reason; unsigned int supported; unsigned int unsupported; int first_btn; int num_btns; unsigned int leds_defined; unsigned int led_state; int led_class; int led_id; } XkbExtensionDeviceNotifyEvent;
typedef union _XkbEvent { int type; XkbAnyEvent any; XkbNewKeyboardNotifyEvent new_kbd; XkbMapNotifyEvent map; XkbStateNotifyEvent state; XkbControlsNotifyEvent ctrls; XkbIndicatorNotifyEvent indicators; XkbNamesNotifyEvent names; XkbCompatMapNotifyEvent compat; XkbBellNotifyEvent bell; XkbActionMessageEvent message; XkbAccessXNotifyEvent accessx; XkbExtensionDeviceNotifyEvent device; XEvent core; } XkbEvent;
typedef struct { int deviceid,mask_len; unsigned char* mask; } XIEventMask;
#define XISetMask(ptr,event)   (((unsigned char*)(ptr))[(event)>>3] |=  (1 << ((event) & 7)))
#define XIMaskIsSet(ptr,event) (((unsigned char*)(ptr))[(event)>>3] &   (1 << ((event) & 7)))
#define XIMaskLen(event)        (((event) >> 3) + 1)
#define XIAllMasterDevices 1
#define XI_RawMotion 17
typedef struct { int mask_len; unsigned char *mask; double *values; } XIValuatorState;
typedef struct { int type; unsigned long serial; Bool send_event; Display *display; int extension,evtype; Time time; int deviceid,sourceid,detail,flags; XIValuatorState valuators; double *raw_values; } XIRawEvent;
typedef XID GLXWindow,GLXDrawable;
typedef struct __GLXFBConfig* GLXFBConfig;
typedef struct __GLXcontext* GLXContext;
typedef void (*__GLXextproc)(void);
typedef XClassHint* (* PFN_XAllocClassHint)(void);
typedef XSizeHints* (* PFN_XAllocSizeHints)(void);
typedef XWMHints* (* PFN_XAllocWMHints)(void);
typedef int (* PFN_XChangeProperty)(Display*,Window,Atom,Atom,int,int,const unsigned char*,int);
typedef int (* PFN_XChangeWindowAttributes)(Display*,Window,unsigned long,XSetWindowAttributes*);
typedef Bool (* PFN_XCheckIfEvent)(Display*,XEvent*,Bool(*)(Display*,XEvent*,XPointer),XPointer);
typedef Bool (* PFN_XCheckTypedWindowEvent)(Display*,Window,int,XEvent*);
typedef int (* PFN_XCloseDisplay)(Display*);
typedef Status (* PFN_XCloseIM)(XIM);
typedef int (* PFN_XConvertSelection)(Display*,Atom,Atom,Atom,Window,Time);
typedef Colormap (* PFN_XCreateColormap)(Display*,Window,Visual*,int);
typedef Cursor (* PFN_XCreateFontCursor)(Display*,unsigned int);
typedef XIC (* PFN_XCreateIC)(XIM,...);
typedef Region (* PFN_XCreateRegion)(void);
typedef Window (* PFN_XCreateWindow)(Display*,Window,int,int,unsigned int,unsigned int,unsigned int,int,unsigned int,Visual*,unsigned long,XSetWindowAttributes*);
typedef int (* PFN_XDefineCursor)(Display*,Window,Cursor);
typedef int (* PFN_XDeleteContext)(Display*,XID,XContext);
typedef int (* PFN_XDeleteProperty)(Display*,Window,Atom);
typedef void (* PFN_XDestroyIC)(XIC);
typedef int (* PFN_XDestroyRegion)(Region);
typedef int (* PFN_XDisplayKeycodes)(Display*,int*,int*);
typedef int (* PFN_XEventsQueued)(Display*,int);
typedef Bool (* PFN_XFilterEvent)(XEvent*,Window);
typedef int (* PFN_XFindContext)(Display*,XID,XContext,XPointer*);
typedef int (* PFN_XFlush)(Display*);
typedef int (* PFN_XFree)(void*);
typedef int (* PFN_XFreeColormap)(Display*,Colormap);
typedef int (* PFN_XFreeCursor)(Display*,Cursor);
typedef void (* PFN_XFreeEventData)(Display*,XGenericEventCookie*);
typedef int (* PFN_XGetErrorText)(Display*,int,char*,int);
typedef Bool (* PFN_XGetEventData)(Display*,XGenericEventCookie*);
typedef char* (* PFN_XGetICValues)(XIC,...);
typedef char* (* PFN_XGetIMValues)(XIM,...);
typedef int (* PFN_XGetInputFocus)(Display*,Window*,int*);
typedef KeySym* (* PFN_XGetKeyboardMapping)(Display*,KeyCode,int,int*);
typedef int (* PFN_XGetScreenSaver)(Display*,int*,int*,int*,int*);
typedef Window (* PFN_XGetSelectionOwner)(Display*,Atom);
typedef XVisualInfo* (* PFN_XGetVisualInfo)(Display*,long,XVisualInfo*,int*);
typedef Status (* PFN_XGetWMNormalHints)(Display*,Window,XSizeHints*,long*);
typedef Status (* PFN_XGetWindowAttributes)(Display*,Window,XWindowAttributes*);
typedef int (* PFN_XGetWindowProperty)(Display*,Window,Atom,long,long,Bool,Atom,Atom*,int*,unsigned long*,unsigned long*,unsigned char**);
typedef int (* PFN_XGrabPointer)(Display*,Window,Bool,unsigned int,int,int,Window,Cursor,Time);
typedef Status (* PFN_XIconifyWindow)(Display*,Window,int);
typedef Status (* PFN_XInitThreads)(void);
typedef Atom (* PFN_XInternAtom)(Display*,const char*,Bool);
typedef int (* PFN_XLookupString)(XKeyEvent*,char*,int,KeySym*,XComposeStatus*);
typedef int (* PFN_XMapRaised)(Display*,Window);
typedef int (* PFN_XMapWindow)(Display*,Window);
typedef int (* PFN_XMoveResizeWindow)(Display*,Window,int,int,unsigned int,unsigned int);
typedef int (* PFN_XMoveWindow)(Display*,Window,int,int);
typedef int (* PFN_XNextEvent)(Display*,XEvent*);
typedef Display* (* PFN_XOpenDisplay)(const char*);
typedef XIM (* PFN_XOpenIM)(Display*,XrmDatabase*,char*,char*);
typedef int (* PFN_XPeekEvent)(Display*,XEvent*);
typedef int (* PFN_XPending)(Display*);
typedef Bool (* PFN_XQueryExtension)(Display*,const char*,int*,int*,int*);
typedef Bool (* PFN_XQueryPointer)(Display*,Window,Window*,Window*,int*,int*,int*,int*,unsigned int*);
typedef int (* PFN_XRaiseWindow)(Display*,Window);
typedef Bool (* PFN_XRegisterIMInstantiateCallback)(Display*,void*,char*,char*,XIDProc,XPointer);
typedef int (* PFN_XResizeWindow)(Display*,Window,unsigned int,unsigned int);
typedef char* (* PFN_XResourceManagerString)(Display*);
typedef int (* PFN_XSaveContext)(Display*,XID,XContext,const char*);
typedef int (* PFN_XSelectInput)(Display*,Window,long);
typedef Status (* PFN_XSendEvent)(Display*,Window,Bool,long,XEvent*);
typedef int (* PFN_XSetClassHint)(Display*,Window,XClassHint*);
typedef XErrorHandler (* PFN_XSetErrorHandler)(XErrorHandler);
typedef void (* PFN_XSetICFocus)(XIC);
typedef char* (* PFN_XSetIMValues)(XIM,...);
typedef int (* PFN_XSetInputFocus)(Display*,Window,int,Time);
typedef char* (* PFN_XSetLocaleModifiers)(const char*);
typedef int (* PFN_XSetScreenSaver)(Display*,int,int,int,int);
typedef int (* PFN_XSetSelectionOwner)(Display*,Atom,Window,Time);
typedef int (* PFN_XSetWMHints)(Display*,Window,XWMHints*);
typedef void (* PFN_XSetWMNormalHints)(Display*,Window,XSizeHints*);
typedef Status (* PFN_XSetWMProtocols)(Display*,Window,Atom*,int);
typedef Bool (* PFN_XSupportsLocale)(void);
typedef int (* PFN_XSync)(Display*,Bool);
typedef Bool (* PFN_XTranslateCoordinates)(Display*,Window,Window,int,int,int*,int*,Window*);
typedef int (* PFN_XUndefineCursor)(Display*,Window);
typedef int (* PFN_XUngrabPointer)(Display*,Time);
typedef int (* PFN_XUnmapWindow)(Display*,Window);
typedef void (* PFN_XUnsetICFocus)(XIC);
typedef VisualID (* PFN_XVisualIDFromVisual)(Visual*);
typedef int (* PFN_XWarpPointer)(Display*,Window,Window,int,int,unsigned int,unsigned int,int,int);
typedef void (* PFN_XkbFreeKeyboard)(XkbDescPtr,unsigned int,Bool);
typedef void (* PFN_XkbFreeNames)(XkbDescPtr,unsigned int,Bool);
typedef XkbDescPtr (* PFN_XkbGetMap)(Display*,unsigned int,unsigned int);
typedef Status (* PFN_XkbGetNames)(Display*,unsigned int,XkbDescPtr);
typedef Status (* PFN_XkbGetState)(Display*,unsigned int,XkbStatePtr);
typedef KeySym (* PFN_XkbKeycodeToKeysym)(Display*,KeyCode,int,int);
typedef Bool (* PFN_XkbQueryExtension)(Display*,int*,int*,int*,int*,int*);
typedef Bool (* PFN_XkbSelectEventDetails)(Display*,unsigned int,unsigned int,unsigned long,unsigned long);
typedef Bool (* PFN_XkbSetDetectableAutoRepeat)(Display*,Bool,Bool*);
typedef void (* PFN_XrmDestroyDatabase)(XrmDatabase);
typedef Bool (* PFN_XrmGetResource)(XrmDatabase,const char*,const char*,char**,XrmValue*);
typedef XrmDatabase (* PFN_XrmGetStringDatabase)(const char*);
typedef void (* PFN_XrmInitialize)(void);
typedef XrmQuark (* PFN_XrmUniqueQuark)(void);
typedef Bool (* PFN_XUnregisterIMInstantiateCallback)(Display*,void*,char*,char*,XIDProc,XPointer);
typedef int (* PFN_Xutf8LookupString)(XIC,XKeyPressedEvent*,char*,int,KeySym*,Status*);
typedef void (* PFN_Xutf8SetWMProperties)(Display*,Window,const char*,const char*,char**,int,XSizeHints*,XWMHints*,XClassHint*);
#define XAllocClassHint _glfw.x11.xlib.AllocClassHint
#define XAllocSizeHints _glfw.x11.xlib.AllocSizeHints
#define XAllocWMHints _glfw.x11.xlib.AllocWMHints
#define XChangeProperty _glfw.x11.xlib.ChangeProperty
#define XChangeWindowAttributes _glfw.x11.xlib.ChangeWindowAttributes
#define XCheckIfEvent _glfw.x11.xlib.CheckIfEvent
#define XCheckTypedWindowEvent _glfw.x11.xlib.CheckTypedWindowEvent
#define XCloseDisplay _glfw.x11.xlib.CloseDisplay
#define XCloseIM _glfw.x11.xlib.CloseIM
#define XConvertSelection _glfw.x11.xlib.ConvertSelection
#define XCreateColormap _glfw.x11.xlib.CreateColormap
#define XCreateFontCursor _glfw.x11.xlib.CreateFontCursor
#define XCreateIC _glfw.x11.xlib.CreateIC
#define XCreateRegion _glfw.x11.xlib.CreateRegion
#define XCreateWindow _glfw.x11.xlib.CreateWindow
#define XDefineCursor _glfw.x11.xlib.DefineCursor
#define XDeleteContext _glfw.x11.xlib.DeleteContext
#define XDeleteProperty _glfw.x11.xlib.DeleteProperty
#define XDestroyIC _glfw.x11.xlib.DestroyIC
#define XDestroyRegion _glfw.x11.xlib.DestroyRegion
#define XDisplayKeycodes _glfw.x11.xlib.DisplayKeycodes
#define XEventsQueued _glfw.x11.xlib.EventsQueued
#define XFilterEvent _glfw.x11.xlib.FilterEvent
#define XFindContext _glfw.x11.xlib.FindContext
#define XFlush _glfw.x11.xlib.Flush
#define XFree _glfw.x11.xlib.Free
#define XFreeColormap _glfw.x11.xlib.FreeColormap
#define XFreeCursor _glfw.x11.xlib.FreeCursor
#define XFreeEventData _glfw.x11.xlib.FreeEventData
#define XGetErrorText _glfw.x11.xlib.GetErrorText
#define XGetEventData _glfw.x11.xlib.GetEventData
#define XGetICValues _glfw.x11.xlib.GetICValues
#define XGetIMValues _glfw.x11.xlib.GetIMValues
#define XGetInputFocus _glfw.x11.xlib.GetInputFocus
#define XGetKeyboardMapping _glfw.x11.xlib.GetKeyboardMapping
#define XGetScreenSaver _glfw.x11.xlib.GetScreenSaver
#define XGetSelectionOwner _glfw.x11.xlib.GetSelectionOwner
#define XGetVisualInfo _glfw.x11.xlib.GetVisualInfo
#define XGetWMNormalHints _glfw.x11.xlib.GetWMNormalHints
#define XGetWindowAttributes _glfw.x11.xlib.GetWindowAttributes
#define XGetWindowProperty _glfw.x11.xlib.GetWindowProperty
#define XGrabPointer _glfw.x11.xlib.GrabPointer
#define XIconifyWindow _glfw.x11.xlib.IconifyWindow
#define XInternAtom _glfw.x11.xlib.InternAtom
#define XLookupString _glfw.x11.xlib.LookupString
#define XMapRaised _glfw.x11.xlib.MapRaised
#define XMapWindow _glfw.x11.xlib.MapWindow
#define XMoveResizeWindow _glfw.x11.xlib.MoveResizeWindow
#define XMoveWindow _glfw.x11.xlib.MoveWindow
#define XNextEvent _glfw.x11.xlib.NextEvent
#define XOpenIM _glfw.x11.xlib.OpenIM
#define XPeekEvent _glfw.x11.xlib.PeekEvent
#define XPending _glfw.x11.xlib.Pending
#define XQueryExtension _glfw.x11.xlib.QueryExtension
#define XQueryPointer _glfw.x11.xlib.QueryPointer
#define XRaiseWindow _glfw.x11.xlib.RaiseWindow
#define XRegisterIMInstantiateCallback _glfw.x11.xlib.RegisterIMInstantiateCallback
#define XResizeWindow _glfw.x11.xlib.ResizeWindow
#define XResourceManagerString _glfw.x11.xlib.ResourceManagerString
#define XSaveContext _glfw.x11.xlib.SaveContext
#define XSelectInput _glfw.x11.xlib.SelectInput
#define XSendEvent _glfw.x11.xlib.SendEvent
#define XSetClassHint _glfw.x11.xlib.SetClassHint
#define XSetErrorHandler _glfw.x11.xlib.SetErrorHandler
#define XSetICFocus _glfw.x11.xlib.SetICFocus
#define XSetIMValues _glfw.x11.xlib.SetIMValues
#define XSetInputFocus _glfw.x11.xlib.SetInputFocus
#define XSetLocaleModifiers _glfw.x11.xlib.SetLocaleModifiers
#define XSetScreenSaver _glfw.x11.xlib.SetScreenSaver
#define XSetSelectionOwner _glfw.x11.xlib.SetSelectionOwner
#define XSetWMHints _glfw.x11.xlib.SetWMHints
#define XSetWMNormalHints _glfw.x11.xlib.SetWMNormalHints
#define XSetWMProtocols _glfw.x11.xlib.SetWMProtocols
#define XSupportsLocale _glfw.x11.xlib.SupportsLocale
#define XSync _glfw.x11.xlib.Sync
#define XTranslateCoordinates _glfw.x11.xlib.TranslateCoordinates
#define XUndefineCursor _glfw.x11.xlib.UndefineCursor
#define XUngrabPointer _glfw.x11.xlib.UngrabPointer
#define XUnmapWindow _glfw.x11.xlib.UnmapWindow
#define XUnsetICFocus _glfw.x11.xlib.UnsetICFocus
#define XVisualIDFromVisual _glfw.x11.xlib.VisualIDFromVisual
#define XWarpPointer _glfw.x11.xlib.WarpPointer
#define XkbFreeKeyboard _glfw.x11.xkb.FreeKeyboard
#define XkbFreeNames _glfw.x11.xkb.FreeNames
#define XkbGetMap _glfw.x11.xkb.GetMap
#define XkbGetNames _glfw.x11.xkb.GetNames
#define XkbGetState _glfw.x11.xkb.GetState
#define XkbKeycodeToKeysym _glfw.x11.xkb.KeycodeToKeysym
#define XkbQueryExtension _glfw.x11.xkb.QueryExtension
#define XkbSelectEventDetails _glfw.x11.xkb.SelectEventDetails
#define XkbSetDetectableAutoRepeat _glfw.x11.xkb.SetDetectableAutoRepeat
#define XrmDestroyDatabase _glfw.x11.xrm.DestroyDatabase
#define XrmGetResource _glfw.x11.xrm.GetResource
#define XrmGetStringDatabase _glfw.x11.xrm.GetStringDatabase
#define XrmUniqueQuark _glfw.x11.xrm.UniqueQuark
#define XUnregisterIMInstantiateCallback _glfw.x11.xlib.UnregisterIMInstantiateCallback
#define Xutf8LookupString _glfw.x11.xlib.utf8LookupString
#define Xutf8SetWMProperties _glfw.x11.xlib.utf8SetWMProperties
typedef void (* PFN_XRRFreeCrtcInfo)(XRRCrtcInfo*);
typedef void (* PFN_XRRFreeOutputInfo)(XRROutputInfo*);
typedef void (* PFN_XRRFreeScreenResources)(XRRScreenResources*);
typedef XRRCrtcInfo* (* PFN_XRRGetCrtcInfo) (Display*,XRRScreenResources*,RRCrtc);
typedef XRROutputInfo* (* PFN_XRRGetOutputInfo)(Display*,XRRScreenResources*,RROutput);
typedef RROutput (* PFN_XRRGetOutputPrimary)(Display*,Window);
typedef XRRScreenResources* (* PFN_XRRGetScreenResourcesCurrent)(Display*,Window);
typedef Bool (* PFN_XRRQueryExtension)(Display*,int*,int*);
typedef Status (* PFN_XRRQueryVersion)(Display*,int*,int*);
typedef void (* PFN_XRRSelectInput)(Display*,Window,int);
typedef Status (* PFN_XRRSetCrtcConfig)(Display*,XRRScreenResources*,RRCrtc,Time,int,int,RRMode,Rotation,RROutput*,int);
typedef int (* PFN_XRRUpdateConfiguration)(XEvent*);
#define XRRFreeCrtcInfo _glfw.x11.randr.FreeCrtcInfo
#define XRRFreeOutputInfo _glfw.x11.randr.FreeOutputInfo
#define XRRFreeScreenResources _glfw.x11.randr.FreeScreenResources
#define XRRGetCrtcInfo _glfw.x11.randr.GetCrtcInfo
#define XRRGetOutputInfo _glfw.x11.randr.GetOutputInfo
#define XRRGetOutputPrimary _glfw.x11.randr.GetOutputPrimary
#define XRRGetScreenResourcesCurrent _glfw.x11.randr.GetScreenResourcesCurrent
#define XRRQueryExtension _glfw.x11.randr.QueryExtension
#define XRRQueryVersion _glfw.x11.randr.QueryVersion
#define XRRSelectInput _glfw.x11.randr.SelectInput
#define XRRSetCrtcConfig _glfw.x11.randr.SetCrtcConfig
#define XRRUpdateConfiguration _glfw.x11.randr.UpdateConfiguration
typedef XcursorImage* (* PFN_XcursorImageCreate)(int,int);
typedef void (* PFN_XcursorImageDestroy)(XcursorImage*);
typedef Cursor (* PFN_XcursorImageLoadCursor)(Display*,const XcursorImage*);
typedef char* (* PFN_XcursorGetTheme)(Display*);
typedef int (* PFN_XcursorGetDefaultSize)(Display*);
typedef XcursorImage* (* PFN_XcursorLibraryLoadImage)(const char*,const char*,int);
#define XcursorImageCreate _glfw.x11.xcursor.ImageCreate
#define XcursorImageDestroy _glfw.x11.xcursor.ImageDestroy
#define XcursorImageLoadCursor _glfw.x11.xcursor.ImageLoadCursor
#define XcursorGetTheme _glfw.x11.xcursor.GetTheme
#define XcursorGetDefaultSize _glfw.x11.xcursor.GetDefaultSize
#define XcursorLibraryLoadImage _glfw.x11.xcursor.LibraryLoadImage
typedef XID xcb_window_t;
typedef XID xcb_visualid_t;
typedef struct xcb_connection_t xcb_connection_t;
typedef xcb_connection_t* (* PFN_XGetXCBConnection)(Display*);
#define XGetXCBConnection _glfw.x11.x11xcb.GetXCBConnection
typedef Bool (* PFN_XF86VidModeQueryExtension)(Display*,int*,int*);
#define XF86VidModeQueryExtension _glfw.x11.vidmode.QueryExtension
typedef Status (* PFN_XIQueryVersion)(Display*,int*,int*);
typedef int (* PFN_XISelectEvents)(Display*,Window,XIEventMask*,int);
#define XIQueryVersion _glfw.x11.xi.QueryVersion
#define XISelectEvents _glfw.x11.xi.SelectEvents
typedef Bool (* PFN_XRenderQueryExtension)(Display*,int*,int*);
typedef Status (* PFN_XRenderQueryVersion)(Display*dpy,int*,int*);
typedef XRenderPictFormat* (* PFN_XRenderFindVisualFormat)(Display*,Visual const*);
#define XRenderQueryExtension _glfw.x11.xrender.QueryExtension
#define XRenderQueryVersion _glfw.x11.xrender.QueryVersion
#define XRenderFindVisualFormat _glfw.x11.xrender.FindVisualFormat
typedef Bool (* PFN_XShapeQueryExtension)(Display*,int*,int*);
typedef Status (* PFN_XShapeQueryVersion)(Display*dpy,int*,int*);
typedef void (* PFN_XShapeCombineRegion)(Display*,Window,int,int,int,Region,int);
typedef void (* PFN_XShapeCombineMask)(Display*,Window,int,int,int,Pixmap,int);
#define XShapeQueryExtension _glfw.x11.xshape.QueryExtension
#define XShapeQueryVersion _glfw.x11.xshape.QueryVersion
#define XShapeCombineRegion _glfw.x11.xshape.ShapeCombineRegion
#define XShapeCombineMask _glfw.x11.xshape.ShapeCombineMask
typedef int (*PFNGLXGETFBCONFIGATTRIBPROC)(Display*,GLXFBConfig,int,int*);
typedef const char* (*PFNGLXGETCLIENTSTRINGPROC)(Display*,int);
typedef Bool (*PFNGLXQUERYEXTENSIONPROC)(Display*,int*,int*);
typedef Bool (*PFNGLXQUERYVERSIONPROC)(Display*,int*,int*);
typedef Bool (*PFNGLXMAKECURRENTPROC)(Display*,GLXDrawable,GLXContext);
typedef void (*PFNGLXSWAPBUFFERSPROC)(Display*,GLXDrawable);
typedef const char* (*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display*,int);
typedef GLXFBConfig* (*PFNGLXGETFBCONFIGSPROC)(Display*,int,int*);
typedef GLXContext (*PFNGLXCREATENEWCONTEXTPROC)(Display*,GLXFBConfig,int,GLXContext,Bool);
typedef __GLXextproc (* PFNGLXGETPROCADDRESSPROC)(const GLubyte *procName);
typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*,GLXDrawable,int);
typedef XVisualInfo* (*PFNGLXGETVISUALFROMFBCONFIGPROC)(Display*,GLXFBConfig);
typedef GLXWindow (*PFNGLXCREATEWINDOWPROC)(Display*,GLXFBConfig,Window,const int*);
typedef int (*PFNGLXSWAPINTERVALMESAPROC)(int);
typedef int (*PFNGLXSWAPINTERVALSGIPROC)(int);
typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*,GLXFBConfig,GLXContext,Bool,const int*);
#define glXGetFBConfigAttrib _glfw.glx.GetFBConfigAttrib
#define glXGetClientString _glfw.glx.GetClientString
#define glXQueryExtension _glfw.glx.QueryExtension
#define glXQueryVersion _glfw.glx.QueryVersion
#define glXMakeCurrent _glfw.glx.MakeCurrent
#define glXSwapBuffers _glfw.glx.SwapBuffers
#define glXQueryExtensionsString _glfw.glx.QueryExtensionsString
#define glXCreateNewContext _glfw.glx.CreateNewContext
#define glXGetVisualFromFBConfig _glfw.glx.GetVisualFromFBConfig
#define glXCreateWindow _glfw.glx.CreateWindow
typedef unsigned long int nfds_t;
struct pollfd { int fd; short events,revents;}; int poll(struct pollfd *fds, nfds_t nfds, int timeout);
#define POLLIN 0x001
GLFWbool _glfwPollPOSIX(struct pollfd* fds, nfds_t count, double* timeout);
#define GLFW_X11_WINDOW_STATE           _GLFWwindowX11 x11;
#define GLFW_X11_LIBRARY_WINDOW_STATE   _GLFWlibraryX11 x11;
#define GLFW_X11_MONITOR_STATE          _GLFWmonitorX11 x11;
#define GLFW_X11_CURSOR_STATE           _GLFWcursorX11 x11;
#define GLFW_GLX_CONTEXT_STATE          _GLFWcontextGLX glx;
#define GLFW_GLX_LIBRARY_CONTEXT_STATE  _GLFWlibraryGLX glx;
typedef struct _GLFWcontextGLX { GLXContext handle; GLXWindow window; GLXFBConfig fbconfig; } _GLFWcontextGLX;
typedef struct _GLFWlibraryGLX {
    int major, minor,eventBase,errorBase; void* handle;
    PFNGLXGETFBCONFIGSPROC              GetFBConfigs;
    PFNGLXGETFBCONFIGATTRIBPROC         GetFBConfigAttrib;
    PFNGLXGETCLIENTSTRINGPROC           GetClientString;
    PFNGLXQUERYEXTENSIONPROC            QueryExtension;
    PFNGLXQUERYVERSIONPROC              QueryVersion;
    PFNGLXMAKECURRENTPROC               MakeCurrent;
    PFNGLXSWAPBUFFERSPROC               SwapBuffers;
    PFNGLXQUERYEXTENSIONSSTRINGPROC     QueryExtensionsString;
    PFNGLXCREATENEWCONTEXTPROC          CreateNewContext;
    PFNGLXGETVISUALFROMFBCONFIGPROC     GetVisualFromFBConfig;
    PFNGLXCREATEWINDOWPROC              CreateWindow;
    PFNGLXGETPROCADDRESSPROC            GetProcAddress;
    PFNGLXGETPROCADDRESSPROC            GetProcAddressARB;
    PFNGLXSWAPINTERVALSGIPROC           SwapIntervalSGI;
    PFNGLXSWAPINTERVALEXTPROC           SwapIntervalEXT;
    PFNGLXSWAPINTERVALMESAPROC          SwapIntervalMESA;
    PFNGLXCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
    GLFWbool EXT_swap_control,ARB_framebuffer_sRGB,EXT_framebuffer_sRGB,ARB_create_context,ARB_create_context_profile;
} _GLFWlibraryGLX;

typedef struct _GLFWwindowX11 { Colormap colormap; Window handle,parent; XIC ic; GLFWbool overrideRedirect,iconified,maximized; int width,height,xpos,ypos,lastCursorPosX,lastCursorPosY,warpCursorPosX,warpCursorPosY; Time keyPressTimes[256]; } _GLFWwindowX11;
typedef struct _GLFWlibraryX11 {
    Display* display;
    int screen;
    Window root;
    float contentScaleX,contentScaleY;
    Window helperWindowHandle;
    Cursor hiddenCursorHandle;
    XContext context;
    XIM im;
    XErrorHandler errorHandler;
    int errorCode;
    char keynames[GLFW_KEY_LAST + 1][5];
    short int keycodes[256],scancodes[GLFW_KEY_LAST + 1];
    double restoreCursorPosX, restoreCursorPosY;
    _GLFWwindow* disabledCursorWindow;
    int emptyEventPipe[2];
    Atom NET_SUPPORTED,NET_SUPPORTING_WM_CHECK,WM_PROTOCOLS,WM_STATE,WM_DELETE_WINDOW;
    Atom NET_WM_NAME,NET_WM_ICON_NAME,NET_WM_ICON,NET_WM_PID,NET_WM_PING,NET_WM_WINDOW_TYPE,NET_WM_WINDOW_TYPE_NORMAL,NET_WM_STATE,NET_WM_STATE_ABOVE,NET_WM_STATE_FULLSCREEN,NET_WM_STATE_MAXIMIZED_VERT;
    Atom NET_WM_STATE_MAXIMIZED_HORZ,NET_WM_STATE_DEMANDS_ATTENTION,NET_WM_BYPASS_COMPOSITOR,NET_WM_FULLSCREEN_MONITORS,NET_WM_WINDOW_OPACITY,NET_WM_CM_Sx,NET_WORKAREA,NET_CURRENT_DESKTOP,NET_ACTIVE_WINDOW;
    Atom NET_FRAME_EXTENTS,NET_REQUEST_FRAME_EXTENTS,MOTIF_WM_HINTS,XdndAware,XdndEnter,XdndPosition,XdndStatus,XdndActionCopy,XdndDrop,XdndFinished,XdndSelection,XdndTypeList,text_uri_list,UTF8_STRING;
    struct {
        void*       handle;
        GLFWbool    utf8;
        PFN_XAllocClassHint AllocClassHint;
        PFN_XAllocSizeHints AllocSizeHints;
        PFN_XAllocWMHints AllocWMHints;
        PFN_XChangeProperty ChangeProperty;
        PFN_XChangeWindowAttributes ChangeWindowAttributes;
        PFN_XCheckIfEvent CheckIfEvent;
        PFN_XCheckTypedWindowEvent CheckTypedWindowEvent;
        PFN_XCloseDisplay CloseDisplay;
        PFN_XCloseIM CloseIM;
        PFN_XConvertSelection ConvertSelection;
        PFN_XCreateColormap CreateColormap;
        PFN_XCreateFontCursor CreateFontCursor;
        PFN_XCreateIC CreateIC;
        PFN_XCreateRegion CreateRegion;
        PFN_XCreateWindow CreateWindow;
        PFN_XDefineCursor DefineCursor;
        PFN_XDeleteContext DeleteContext;
        PFN_XDeleteProperty DeleteProperty;
        PFN_XDestroyIC DestroyIC;
        PFN_XDestroyRegion DestroyRegion;
        PFN_XDisplayKeycodes DisplayKeycodes;
        PFN_XEventsQueued EventsQueued;
        PFN_XFilterEvent FilterEvent;
        PFN_XFindContext FindContext;
        PFN_XFlush Flush;
        PFN_XFree Free;
        PFN_XFreeColormap FreeColormap;
        PFN_XFreeCursor FreeCursor;
        PFN_XFreeEventData FreeEventData;
        PFN_XGetErrorText GetErrorText;
        PFN_XGetEventData GetEventData;
        PFN_XGetICValues GetICValues;
        PFN_XGetIMValues GetIMValues;
        PFN_XGetInputFocus GetInputFocus;
        PFN_XGetKeyboardMapping GetKeyboardMapping;
        PFN_XGetScreenSaver GetScreenSaver;
        PFN_XGetSelectionOwner GetSelectionOwner;
        PFN_XGetVisualInfo GetVisualInfo;
        PFN_XGetWMNormalHints GetWMNormalHints;
        PFN_XGetWindowAttributes GetWindowAttributes;
        PFN_XGetWindowProperty GetWindowProperty;
        PFN_XGrabPointer GrabPointer;
        PFN_XIconifyWindow IconifyWindow;
        PFN_XInternAtom InternAtom;
        PFN_XLookupString LookupString;
        PFN_XMapRaised MapRaised;
        PFN_XMapWindow MapWindow;
        PFN_XMoveResizeWindow MoveResizeWindow;
        PFN_XMoveWindow MoveWindow;
        PFN_XNextEvent NextEvent;
        PFN_XOpenIM OpenIM;
        PFN_XPeekEvent PeekEvent;
        PFN_XPending Pending;
        PFN_XQueryExtension QueryExtension;
        PFN_XQueryPointer QueryPointer;
        PFN_XRaiseWindow RaiseWindow;
        PFN_XRegisterIMInstantiateCallback RegisterIMInstantiateCallback;
        PFN_XResizeWindow ResizeWindow;
        PFN_XResourceManagerString ResourceManagerString;
        PFN_XSaveContext SaveContext;
        PFN_XSelectInput SelectInput;
        PFN_XSendEvent SendEvent;
        PFN_XSetClassHint SetClassHint;
        PFN_XSetErrorHandler SetErrorHandler;
        PFN_XSetICFocus SetICFocus;
        PFN_XSetIMValues SetIMValues;
        PFN_XSetInputFocus SetInputFocus;
        PFN_XSetLocaleModifiers SetLocaleModifiers;
        PFN_XSetScreenSaver SetScreenSaver;
        PFN_XSetSelectionOwner SetSelectionOwner;
        PFN_XSetWMHints SetWMHints;
        PFN_XSetWMNormalHints SetWMNormalHints;
        PFN_XSetWMProtocols SetWMProtocols;
        PFN_XSupportsLocale SupportsLocale;
        PFN_XSync Sync;
        PFN_XTranslateCoordinates TranslateCoordinates;
        PFN_XUndefineCursor UndefineCursor;
        PFN_XUngrabPointer UngrabPointer;
        PFN_XUnmapWindow UnmapWindow;
        PFN_XUnsetICFocus UnsetICFocus;
        PFN_XVisualIDFromVisual VisualIDFromVisual;
        PFN_XWarpPointer WarpPointer;
        PFN_XUnregisterIMInstantiateCallback UnregisterIMInstantiateCallback;
        PFN_Xutf8LookupString utf8LookupString;
        PFN_Xutf8SetWMProperties utf8SetWMProperties;
    } xlib;

    struct { PFN_XrmDestroyDatabase DestroyDatabase; PFN_XrmGetResource GetResource; PFN_XrmGetStringDatabase GetStringDatabase; PFN_XrmUniqueQuark UniqueQuark; } xrm;
    struct { GLFWbool available; void* handle; int eventBase,errorBase,major,minor; GLFWbool monitorBroken; PFN_XRRFreeCrtcInfo FreeCrtcInfo; PFN_XRRFreeOutputInfo FreeOutputInfo; PFN_XRRFreeScreenResources FreeScreenResources; PFN_XRRGetCrtcInfo GetCrtcInfo; PFN_XRRGetOutputInfo GetOutputInfo; PFN_XRRGetOutputPrimary GetOutputPrimary; PFN_XRRGetScreenResourcesCurrent GetScreenResourcesCurrent; PFN_XRRQueryExtension QueryExtension; PFN_XRRQueryVersion QueryVersion; PFN_XRRSelectInput SelectInput; PFN_XRRSetCrtcConfig SetCrtcConfig; PFN_XRRUpdateConfiguration UpdateConfiguration; } randr;
    struct { GLFWbool available,detectable; int majorOpcode,eventBase,errorBase,major,minor; unsigned int group; PFN_XkbFreeKeyboard FreeKeyboard; PFN_XkbFreeNames FreeNames; PFN_XkbGetMap GetMap; PFN_XkbGetNames GetNames; PFN_XkbGetState GetState; PFN_XkbKeycodeToKeysym KeycodeToKeysym; PFN_XkbQueryExtension QueryExtension; PFN_XkbSelectEventDetails SelectEventDetails; PFN_XkbSetDetectableAutoRepeat SetDetectableAutoRepeat; } xkb;
    struct { int count,timeout,interval,blanking,exposure; } saver;
    struct { int version; Window source; Atom format; } xdnd;
    struct { void* handle; PFN_XcursorImageCreate ImageCreate; PFN_XcursorImageDestroy ImageDestroy; PFN_XcursorImageLoadCursor ImageLoadCursor; PFN_XcursorGetTheme GetTheme; PFN_XcursorGetDefaultSize GetDefaultSize; PFN_XcursorLibraryLoadImage LibraryLoadImage; } xcursor;
    struct { void* handle; PFN_XGetXCBConnection GetXCBConnection; } x11xcb;
    struct { GLFWbool available; void* handle; int eventBase,errorBase; PFN_XF86VidModeQueryExtension QueryExtension; } vidmode;
    struct { GLFWbool available; void* handle; int majorOpcode,eventBase,errorBase,major,minor; PFN_XIQueryVersion QueryVersion; PFN_XISelectEvents SelectEvents; } xi;
    struct { GLFWbool available; void* handle; int major,minor,eventBase,errorBase; PFN_XRenderQueryExtension QueryExtension; PFN_XRenderQueryVersion QueryVersion; PFN_XRenderFindVisualFormat FindVisualFormat; } xrender;
    struct { GLFWbool available; void* handle; int major,minor,eventBase, errorBase; PFN_XShapeQueryExtension QueryExtension; PFN_XShapeCombineRegion ShapeCombineRegion; PFN_XShapeQueryVersion QueryVersion; PFN_XShapeCombineMask ShapeCombineMask; } xshape;
} _GLFWlibraryX11;

typedef struct _GLFWmonitorX11 { RROutput output; RRCrtc crtc; RRMode oldMode; int index; } _GLFWmonitorX11;
typedef struct _GLFWcursorX11 { Cursor handle; } _GLFWcursorX11;
