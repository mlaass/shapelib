#ifndef _SHAPEFILE_H_INCLUDED
#define _SHAPEFILE_H_INCLUDED

/*
 * Copyright (c) 1995 Frank Warmerdam
 *
 * This code is in the public domain.
 *
 * $Log$
 * Revision 1.11  1999-05-18 19:11:11  warmerda
 * Added example searching capability
 *
 * Revision 1.10  1999/05/18 17:49:38  warmerda
 * added initial quadtree support
 *
 * Revision 1.9  1999/05/11 03:19:28  warmerda
 * added new Tuple api, and improved extension handling - add from candrsn
 *
 * Revision 1.8  1999/03/23 17:22:27  warmerda
 * Added extern "C" protection for C++ users of shapefil.h.
 *
 * Revision 1.7  1998/12/31 15:31:07  warmerda
 * Added the TRIM_DBF_WHITESPACE and DISABLE_MULTIPATCH_MEASURE options.
 *
 * Revision 1.6  1998/12/03 15:48:15  warmerda
 * Added SHPCalculateExtents().
 *
 * Revision 1.5  1998/11/09 20:57:16  warmerda
 * Altered SHPGetInfo() call.
 *
 * Revision 1.4  1998/11/09 20:19:33  warmerda
 * Added 3D support, and use of SHPObject.
 *
 * Revision 1.3  1995/08/23 02:24:05  warmerda
 * Added support for reading bounds.
 *
 * Revision 1.2  1995/08/04  03:17:39  warmerda
 * Added header.
 *
 */

#include <stdio.h>

#ifdef USE_DBMALLOC
#include <dbmalloc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
/************************************************************************/
/*                        Configuration options.                        */
/************************************************************************/

/* -------------------------------------------------------------------- */
/*      Should the DBFReadStringAttribute() strip leading and           */
/*      trailing white space?                                           */
/* -------------------------------------------------------------------- */
#define TRIM_DBF_WHITESPACE

/* -------------------------------------------------------------------- */
/*      Should we write measure values to the Multipatch object?        */
/*      Reportedly ArcView crashes if we do write it, so for now it     */
/*      is disabled.                                                    */
/* -------------------------------------------------------------------- */
#define DISABLE_MULTIPATCH_MEASURE

/************************************************************************/
/*                             SHP Support.                             */
/************************************************************************/
typedef	struct
{
    FILE        *fpSHP;
    FILE	*fpSHX;

    int		nShapeType;				/* SHPT_* */
    
    int		nFileSize;				/* SHP file */

    int         nRecords;
    int		nMaxRecords;
    int		*panRecOffset;
    int		*panRecSize;

    double	adBoundsMin[4];
    double	adBoundsMax[4];

    int		bUpdated;
} SHPInfo;

typedef SHPInfo * SHPHandle;

/* -------------------------------------------------------------------- */
/*      Shape types (nSHPType)                                          */
/* -------------------------------------------------------------------- */
#define SHPT_POINT	1
#define SHPT_ARC	3
#define SHPT_POLYGON	5
#define SHPT_MULTIPOINT	8
#define SHPT_POINTZ	11
#define SHPT_ARCZ	13
#define SHPT_POLYGONZ	15
#define SHPT_MULTIPOINTZ 18
#define SHPT_POINTM	21
#define SHPT_ARCM	23
#define SHPT_POLYGONM	25
#define SHPT_MULTIPOINTM 28
#define SHPT_MULTIPATCH 31


/* -------------------------------------------------------------------- */
/*      Part types - everything but SHPT_MULTIPATCH just uses           */
/*      SHPP_RING.                                                      */
/* -------------------------------------------------------------------- */

#define SHPP_TRISTRIP	0
#define SHPP_TRIFAN	1
#define SHPP_OUTERRING	2
#define SHPP_INNERRING	3
#define SHPP_FIRSTRING	4
#define SHPP_RING	5

/* -------------------------------------------------------------------- */
/*      SHPObject - represents on shape (without attributes) read       */
/*      from the .shp file.                                             */
/* -------------------------------------------------------------------- */
typedef struct
{
    int		nSHPType;

    int		nShapeId; /* -1 is unknown/unassigned */

    int		nParts;
    int		*panPartStart;
    int		*panPartType;
    
    int		nVertices;
    double	*padfX;
    double	*padfY;
    double	*padfZ;
    double	*padfM;

    double	dfXMin;
    double	dfYMin;
    double	dfZMin;
    double	dfMMin;

    double	dfXMax;
    double	dfYMax;
    double	dfZMax;
    double	dfMMax;
} SHPObject;

/* -------------------------------------------------------------------- */
/*      SHP API Prototypes                                              */
/* -------------------------------------------------------------------- */
SHPHandle SHPOpen( const char * pszShapeFile, const char * pszAccess );
SHPHandle SHPCreate( const char * pszShapeFile, int nShapeType );
void	SHPGetInfo( SHPHandle hSHP, int * pnEntities, int * pnShapeType,
                    double * padfMinBound, double * padfMaxBound );

SHPObject *SHPReadObject( SHPHandle hSHP, int iShape );
int	SHPWriteObject( SHPHandle hSHP, int iShape, SHPObject * psObject );

void	SHPDestroyObject( SHPObject * psObject );
void	SHPComputeExtents( SHPObject * psObject );
SHPObject *SHPCreateObject( int nSHPType, int nShapeId,
                            int nParts, int * panPartStart, int * panPartType,
                            int nVertices, double * padfX, double * padfY,
                            double * padfZ, double * padfM );
SHPObject *SHPCreateSimpleObject( int nSHPType, int nVertices,
                              double * padfX, double * padfY, double * padfZ );

void	SHPClose( SHPHandle hSHP );

const char *SHPTypeName( int nSHPType );
const char *SHPPartTypeName( int nPartType );

/* -------------------------------------------------------------------- */
/*      Shape quadtree indexing API.                                    */
/* -------------------------------------------------------------------- */

typedef struct shape_tree_node
{
    /* region covered by this node */
    double	adfBoundsMin[4];
    double	adfBoundsMax[4];

    /* list of shapes stored at this node.  The papsShapeObj pointers
       or the whole list can be NULL */
    int		nShapeCount;
    int		*panShapeIds;
    SHPObject   **papsShapeObj;
    
    struct shape_tree_node *psSubNode1;
    struct shape_tree_node *psSubNode2;
    
} SHPTreeNode;

typedef struct
{
    SHPHandle   hSHP;
    
    int		nMaxDepth;
    int		nDimension;
    
    SHPTreeNode	*psRoot;
} SHPTree;

SHPTree *SHPCreateTree( SHPHandle hSHP, int nDimension, int nMaxDepth,
                        double *padfBoundsMin, double *padfBoundsMax );
void     SHPDestroyTree( SHPTree * hTree );

int	SHPWriteTree( SHPTree *hTree, const char * pszFilename );
SHPTree SHPReadTree( const char * pszFilename );

int	SHPTreeAddObject( SHPTree * hTree, SHPObject * psObject );
int	SHPTreeAddShapeId( SHPTree * hTree, SHPObject * psObject );
int	SHPTreeRemoveShapeId( SHPTree * hTree, int nShapeId );

int    *SHPTreeFindLikelyShapes( SHPTree * hTree,
                                 double * padfBoundsMin,
                                 double * padfBoundsMax,
                                 int * );
int     SHPCheckBoundsOverlap( double *, double *, double *, double *, int );

/************************************************************************/
/*                             DBF Support.                             */
/************************************************************************/
typedef	struct
{
    FILE	*fp;

    int         nRecords;

    int		nRecordLength;
    int		nHeaderLength;
    int		nFields;
    int		*panFieldOffset;
    int		*panFieldSize;
    int		*panFieldDecimals;
    char	*pachFieldType;

    char	*pszHeader;

    int		nCurrentRecord;
    int		bCurrentRecordModified;
    char	*pszCurrentRecord;
    
    int		bNoHeader;
    int		bUpdated;
} DBFInfo;

typedef DBFInfo * DBFHandle;

typedef enum {
  FTString,
  FTInteger,
  FTDouble,
  FTInvalid
} DBFFieldType;

#define XBASE_FLDHDR_SZ       32

DBFHandle DBFOpen( const char * pszDBFFile, const char * pszAccess );
DBFHandle DBFCreate( const char * pszDBFFile );

int	DBFGetFieldCount( DBFHandle psDBF );
int	DBFGetRecordCount( DBFHandle psDBF );
int	DBFAddField( DBFHandle hDBF, const char * pszFieldName,
		     DBFFieldType eType, int nWidth, int nDecimals );

DBFFieldType DBFGetFieldInfo( DBFHandle psDBF, int iField, 
			      char * pszFieldName, 
			      int * pnWidth, int * pnDecimals );

int 	DBFReadIntegerAttribute( DBFHandle hDBF, int iShape, int iField );
double 	DBFReadDoubleAttribute( DBFHandle hDBF, int iShape, int iField );
const char *DBFReadStringAttribute( DBFHandle hDBF, int iShape, int iField );

int DBFWriteIntegerAttribute( DBFHandle hDBF, int iShape, int iField, 
			      int nFieldValue );
int DBFWriteDoubleAttribute( DBFHandle hDBF, int iShape, int iField,
			     double dFieldValue );
int DBFWriteStringAttribute( DBFHandle hDBF, int iShape, int iField,
			     const char * pszFieldValue );

const char *DBFReadTuple(DBFHandle psDBF, int hEntity );
int DBFWriteTuple(DBFHandle psDBF, int hEntity, void * pRawTuple );

DBFHandle DBFCloneEmpty(DBFHandle psDBF, const char * pszFilename );
 
void	DBFClose( DBFHandle hDBF );

#ifdef __cplusplus
}
#endif

#endif /* ndef _SHAPEFILE_H_INCLUDED */
